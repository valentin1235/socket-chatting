#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define DEFAULT_PORT ("3000")
#define MAX_CLIENT (10)
#define MAX_MESSAGES (50)
#define MESSAGE_SIZE (200)

typedef struct message_form {
    char content[MESSAGE_SIZE];
    char created_at[MESSAGE_SIZE];
} message_form_t;

typedef struct chatroom {
    message_form_t messages[MAX_MESSAGES];
    size_t messages_count;
    int socket_nums[MAX_CLIENT];
    size_t socket_nums_count;
} chatroom_t;

typedef struct param {
    chatroom_t* chatroom;
    int socket_num;
} param_t;

pthread_mutex_t g_mutex;
int g_client_counts = 0;
int g_client_sockets[MAX_CLIENT];



typedef enum ERROR {
    SUCCESS = 1,
    ERROR_SOCKET = -1,
    ERROR_BIND = -2,
    ERROR_LISTEN = -3,
    ERROR_MESSAGE_FULL = -4
} error_t;

void init_chatroom(chatroom_t* chatroom) {
    size_t i;
    for (i = 0; i < MAX_CLIENT; ++i) {
        chatroom->socket_nums[i] = -1;
    }
    chatroom->socket_nums_count = 0;

    chatroom->messages_count = 0;
}


#if (0)
/* TODO : use this func */
static char* build_message_malloc(chatroom_t* chatroom)
{
    size_t i;
    char* buffer = malloc(MESSAGE_SIZE);
    size_t buffer_size;
    size_t max_size;

    max_size = MESSAGE_SIZE;

    buffer[0] = '\0';
    strcat(buffer, chatroom->messages[0].content);
    buffer_size = strlen(chatroom->messages[0].content);

    for (i = 0; i < chatroom->messages_count; ++i) {
        buffer_size += MESSAGE_SIZE;

        if (buffer_size > max_size) {
            max_size += MESSAGE_SIZE;
            buffer = realloc(buffer, buffer_size);
        }

        strcat(buffer, chatroom->messages[i].content);
    }

    return buffer;
}
#endif


void* communicate_thread(void* p)
{
    param_t* param = p;
    chatroom_t* chatroom = param->chatroom;
    char* pa_tmp_message = malloc(MESSAGE_SIZE);

    if (chatroom->messages_count >= MAX_MESSAGES) {
        printf("FULL OF MESSAGES\n");

        return NULL;
    }

    /* add user */
    pthread_mutex_lock(&g_mutex);
    {
        chatroom->socket_nums[chatroom->socket_nums_count++] = param->socket_num;
    }
    pthread_mutex_unlock(&g_mutex);
    
    do {
        /* read */
        pthread_mutex_lock(&g_mutex);
        {
            int str_len;
            printf("read message..\n");
            str_len = read(param->socket_num, pa_tmp_message, MESSAGE_SIZE);
            if (errno == ECONNRESET || str_len == -1) {
                printf("ECONNRESET : connection closed\n");
                goto end;
            }
            ++chatroom->messages_count;
        }
        pthread_mutex_unlock(&g_mutex);

        /* write */
        pthread_mutex_lock(&g_mutex);
        {
            size_t i;
            printf("write message to %lu users...\n", chatroom->socket_nums_count);
            for (i = 0; i < chatroom->socket_nums_count; ++i) {
                if (chatroom->socket_nums[i] != param->socket_num) {
                    write(chatroom->socket_nums[i], pa_tmp_message, strlen(pa_tmp_message) + 1);
                }
            }
        }
        pthread_mutex_unlock(&g_mutex);
    } while (1);

end:
    printf("thread gracfully closing...\n");

    pthread_mutex_unlock(&g_mutex);
    shutdown(param->socket_num, SHUT_RDWR);
    free(pa_tmp_message);

    /* remove user */
    pthread_mutex_lock(&g_mutex);
    {
        --chatroom->socket_nums_count;
        if (chatroom->socket_nums_count == 0) {
            chatroom->messages_count = 0;
        }
    }
    pthread_mutex_unlock(&g_mutex);

    pthread_exit((void*)0);

    return NULL;
}


error_t server_on(void)
{
    int server_socket;

    struct sockaddr_in server_addr;
    chatroom_t chatroom;
    pthread_t thread;

    signal(SIGPIPE, SIG_IGN); /* ignore EPIPE(broken pipe) signal */

    server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        printf("socket error\n");
        return ERROR_SOCKET;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(DEFAULT_PORT));

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("bind error\n");
        return ERROR_BIND;
    }

    if (listen(server_socket, 5) == -1) {
        printf("listen error\n");
        return ERROR_LISTEN;
    }

    init_chatroom(&chatroom);
    do {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size;
        int client_socket;

        client_addr_size = sizeof(client_addr);
        printf("* start accepting client...\n");
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);

        {
            param_t param;
            param.chatroom = &chatroom;
            param.socket_num = client_socket;

            printf("* create new thread...\n");
            pthread_create(&thread, NULL, communicate_thread, &param);
        }
    } while (1);

    printf("* waiting for the threads completed...\n");
    pthread_join(thread, NULL);

    close(server_socket);

    return SUCCESS;
}

int main(void)
{
    server_on();
}
