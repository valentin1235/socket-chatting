#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define DEFAULT_PORT ("3000")
#define MAX_CLIENT (10)

typedef enum ERROR {
    SUCCESS = 1,
    ERROR_SOCKET = -1,
    ERROR_CONNECT = -2,
    ERROR_READ = -3
} error_t;

error_t create_client(const char* server_host, const char* server_port)
{
    int client_socket;
    struct sockaddr_in server_addr;
    
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

    while (1) { /* read */
        char sending[100];
        char receiving[100];
        int receiving_len;

        if (fgets(sending, 100, stdin) == NULL) {
            break;
        }
        if (strcmp(sending, "exit\n") == 0) {
            goto end;
        }
        write(client_socket, sending, 5);

        receiving_len = read(client_socket, receiving, sizeof(receiving) - 1);
        if (receiving_len == -1) {
            printf("read error");
            return ERROR_READ;
        }
        printf("message from server : %s\n", receiving);
        
    }

end:
    close(client_socket);

    return SUCCESS;
}

int main(int argc, char** argv)
{
    create_client(argv[1], argv[2]);
}
