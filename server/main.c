#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "utils.h"

#define DEFAULT_PORT ("3000")
#define MAX_CLIENT (10)
#define MAX_MESSAGES (50)
#define MESSAGE_SIZE (200)

static pthread_t s_thread_communicate;

pthread_mutex_t g_mutex;
size_t s_client_counts = 0;
int s_client_sockets[MAX_CLIENT];
int s_server_socket;

typedef enum ERROR {
    SUCCESS = 1,
    ERROR_SOCKET = -1,
    ERROR_BIND = -2,
    ERROR_LISTEN = -3,
    ERROR_MESSAGE_FULL = -4
} error_t;


static void SIGINT_handler(int sig)
{
    size_t i;

    pthread_kill(s_thread_communicate, sig);

    for (i = 0; i < s_client_counts; ++i) {
        close(s_client_sockets[i]);
    }

    close(s_server_socket);

    printf("* sig int %d\n", sig);

    exit(1);
}


void* communicate_thread(void* p)
{
    int client_socket = *((int*)p);
    char* pa_tmp_message = malloc(MESSAGE_SIZE);

    /* add user */
    pthread_mutex_lock(&g_mutex);
    {
        s_client_sockets[s_client_counts++] = client_socket;
    }
    pthread_mutex_unlock(&g_mutex);
    
    do {
        pthread_mutex_lock(&g_mutex);
        {
            size_t i;
            int read_len;
            printf("read message..\n");
            read_len = read(client_socket, pa_tmp_message, MESSAGE_SIZE); /* read */
            if (read_len == -1 || read_len == 0) {
                printf("ECONNRESET : connection closed\n");
                break;
            }

            printf("write message from %d...\n", client_socket);
            for (i = 0; i < s_client_counts; ++i) { /* write */
                if (s_client_sockets[i] != client_socket) {
                    write(s_client_sockets[i], pa_tmp_message, strlen(pa_tmp_message) + 1);
                }
            }
        }
        pthread_mutex_unlock(&g_mutex);
    } while (TRUE);

    printf("thread gracfully closing...\n");

    pthread_mutex_unlock(&g_mutex);
    close(client_socket);
    shutdown(client_socket, SHUT_RDWR);
    free(pa_tmp_message);

    /* remove user */
    pthread_mutex_lock(&g_mutex);
    {
        --s_client_counts;
    }
    pthread_mutex_unlock(&g_mutex);

    pthread_exit((void*)0);

    return NULL;
}


error_t server_on(void)
{
    
    struct sockaddr_in server_addr;

    s_server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (s_server_socket == -1) {
        printf("* socket error\n");
        return ERROR_SOCKET;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(DEFAULT_PORT));

    if (bind(s_server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("* bind error\n");
        return ERROR_BIND;
    }
    printf("* server run on 3000\n");

    if (listen(s_server_socket, 5) == -1) {
        printf("* listen error\n");
        return ERROR_LISTEN;
    }

    do {
        struct sockaddr_in client_addr;
        socklen_t client_addr_size;
        int client_socket;

        client_addr_size = sizeof(client_addr);
        printf("* start accepting client...\n");
        client_socket = accept(s_server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
        printf("* client accepted : %d\n", client_socket);

        pthread_create(&s_thread_communicate, NULL, communicate_thread, &client_socket);
        printf("* new thread for (%d) created\n", client_socket);
    } while (TRUE);

    printf("* waiting for the threads completed...\n");
    pthread_join(s_thread_communicate, NULL);

    close(s_server_socket);

    return SUCCESS;
}

int main(void)
{
    signal(SIGPIPE, SIG_IGN); /* ignore EPIPE(broken pipe) signal */
    signal(SIGINT, SIGINT_handler);

    server_on();
}
