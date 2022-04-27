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
size_t g_client_counts = 0;
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
    int client_socket = *((int*)p);
    char* pa_tmp_message = malloc(MESSAGE_SIZE);

    /* add user */
    pthread_mutex_lock(&g_mutex);
    {
        g_client_sockets[g_client_counts++] = client_socket;
    }
    pthread_mutex_unlock(&g_mutex);
    
    do {
        pthread_mutex_lock(&g_mutex);
        {
            size_t i;
            int str_len;
            printf("read message..\n");
            str_len = read(client_socket, pa_tmp_message, MESSAGE_SIZE); /* read */
            printf("errno : %d, str_len : %d\n", errno, strlen(pa_tmp_message));
            if (str_len == -1 || str_len == 0) {
                printf("ECONNRESET : connection closed\n");
                break;
                // goto end;
            }

            printf("write message from %lu...\n", client_socket);
            for (i = 0; i < g_client_counts; ++i) { /* write */
                if (g_client_sockets[i] != client_socket) {
                    write(g_client_sockets[i], pa_tmp_message, strlen(pa_tmp_message) + 1);
                }
            }
        }
        pthread_mutex_unlock(&g_mutex);
    } while (1);

end:
    printf("thread gracfully closing...\n");

    pthread_mutex_unlock(&g_mutex);
    close(client_socket);
    shutdown(client_socket, SHUT_RDWR);
    free(pa_tmp_message);

    /* remove user */
    pthread_mutex_lock(&g_mutex);
    {
        --g_client_counts;
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
        printf("start accepting client...\n");
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        printf("* client accepted : %d\n", client_socket);

        pthread_create(&thread, NULL, communicate_thread, &client_socket);
        printf("* new thread for (%d) created\n", client_socket);
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
