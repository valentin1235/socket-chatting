#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define DEFAULT_PORT ("3000")
#define READ_SIZE (100)

int g_exit = 0;

typedef enum ERROR {
    SUCCESS = 1,
    ERROR_SOCKET = -1,
    ERROR_CONNECT = -2,
    ERROR_READ = -3
} error_t;

void* send_message(void* p)
{
    int client_socket = *(int*)p;
    char buffer[READ_SIZE];

    do {
        // if (g_exit == 1) {
        //     goto end;
        // }
        if (fgets(buffer, READ_SIZE, stdin) == NULL) {
            printf("failed to read from stdin\n");
            goto end;
        }
        if (strcmp(buffer, "exit\n") == 0) {
            g_exit = 1;
            shutdown(client_socket, SHUT_RDWR);
            printf("socket shutdown... %d\n", errno);
            goto end;
        }
        write(client_socket, buffer, READ_SIZE);
    } while (1);

end:
    pthread_exit((void*)0);

    return NULL;
}

void* receive_message(void* p)
{
    int client_socket = *(int*)p;
    char buffer[READ_SIZE];

    do {
        int receiving_len;
        if (g_exit == 1) {
            goto end;
        }
        receiving_len = read(client_socket, buffer, sizeof(buffer) - 1);
        if (receiving_len == -1) {
            printf("failed to read from server\n");
            goto end;
        }
        printf("message from server : %s\n", buffer);
    } while (1);

end:
    pthread_exit((void*)0);

    return NULL;
    
}

error_t create_client(const char* server_host, const char* server_port)
{
    int client_socket;
    struct sockaddr_in server_addr;
    pthread_t thread_send;
    pthread_t thread_receive;
    
    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        printf("socket error\n");
        return ERROR_SOCKET;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_host);
    server_addr.sin_port = htons(atoi(server_port));

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("connect error\n");
        return ERROR_CONNECT;
    }

    pthread_create(&thread_send, NULL, send_message, &client_socket);
    pthread_create(&thread_receive, NULL, receive_message, &client_socket);
    
    pthread_join(thread_send, NULL);
    printf("hih\n");
    pthread_join(thread_receive, NULL);
    printf("hih2\n");

end:
    printf("terminate!\n");
    if (g_exit == 1) {
        close(client_socket);
    }
    
    return SUCCESS;
}

int main(int argc, char** argv)
{
    create_client(argv[1], argv[2]);
}
