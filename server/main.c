#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define DEFAULT_PORT ("3000")
#define MAX_CLIENT (10)

typedef struct message {
    char content[100];
    char created_at[100];
} message_t;

typedef struct chatroom {
    message_t messages[50];
    int socket_nums[MAX_CLIENT];
    size_t client_count;
} chatroom_t;


typedef enum ERROR {
    SUCCESS = 1,
    ERROR_SOCKET = -1,
    ERROR_BIND = -2,
    ERROR_LISTEN = -3
} error_t;

void init_chatroom(chatroom_t* chatroom, size_t client_count) {
    size_t i;
    for (i = 0; i < client_count; ++i) {
        chatroom->socket_nums[i] = -1;
    }
    chatroom->client_count = 0;
}

error_t create_server(void)
{
    int server_socket;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size;
    chatroom_t chatroom;

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

    init_chatroom(&chatroom, MAX_CLIENT);
    /* todo : 바깥 while문 없애고 thread생성해서 처리할것. read하고나서 write할때 */
    while (1) {
        int client_socket;
        char buffer[200];
        size_t content_len = 0;
        size_t i;

        client_addr_size = sizeof(client_addr);
        printf("1\n");
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_size);
    
        printf("success to accept client. %d : %d\n", client_addr.sin_addr, client_addr.sin_port);
        chatroom.socket_nums[chatroom.client_count++] = client_socket;

        printf("3\n");
        while (1) {
            content_len = read(client_socket, buffer, 200);
            printf("4. client_sock : %d\n", client_socket);

            printf("%s", buffer);
            printf("5\n");
        }
        
    }

    return SUCCESS;
}

int main(void)
{
    create_server();
}
