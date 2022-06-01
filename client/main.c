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
#define TRUE (1)
#define FALSE (0)

static s_exit = 0;
static int s_client_socket;
static pthread_t s_thread_send;
static pthread_t s_thread_receive;

typedef enum ERROR {
    SUCCESS = 1,
    ERROR_SOCKET = -1,
    ERROR_CONNECT = -2,
    ERROR_READ = -3
} error_t;

static char* trim(char* msg)
{
    char* p = msg;
    while (*p != '\n' && *p != '\0') {
        ++p;
    }
    *p = '\0';

    return msg;
}

static void SIGINT_handler(int sig)
{
    pthread_kill(s_thread_send, sig);
    pthread_kill(s_thread_receive, sig);

    close(s_client_socket);

    printf("* sig int %d\n", sig);

    exit(1);
}

void* send_message(void* p)
{
    int client_socket = *(int*)p;
    char buffer[READ_SIZE];

    do {        
        if (fgets(buffer, READ_SIZE, stdin) == NULL) {
            printf("* failed to read from stdin\n");
            break;
        }

        if (strcmp(buffer, "exit\n") == 0) {
            s_exit = 1;
            shutdown(client_socket, SHUT_RDWR);
            printf("* socket shutdown...\n");
            break;
        } else if (s_exit == 1) {
            printf("* unexpected thread exit\n");
            break;
        }

        write(client_socket, trim(buffer), READ_SIZE);
    } while (TRUE);

    pthread_exit((void*)0);

    return NULL;
}

void* receive_message(void* p)
{
    int client_socket = *(int*)p;
    char buffer[READ_SIZE];

    do {
        int read_len;

        if (s_exit == 1) {
            printf("* unexpected thread exit\n");
            break;
        }

        read_len = read(client_socket, buffer, sizeof(buffer) - 1);
        if (read_len == 0 || read_len == -1) {
            s_exit = 1;
            shutdown(client_socket, SHUT_RDWR);
            printf("* ECONNRESET : connection closed\n");
            break;
        }

        buffer[READ_SIZE - 1] = '\0';
        printf("* received message : %s\n", buffer);
    } while (1);

    pthread_exit((void*)0);

    return NULL;
    
}

error_t create_client(const char* server_host, const char* server_port)
{
    struct sockaddr_in server_addr;
    
    s_client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (s_client_socket == -1) {
        printf("* socket error\n");
        return ERROR_SOCKET;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_host);
    server_addr.sin_port = htons(atoi(server_port));

    if (connect(s_client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("* connect error\n");
        return ERROR_CONNECT;
    }

    pthread_create(&s_thread_send, NULL, send_message, &s_client_socket);
    pthread_create(&s_thread_receive, NULL, receive_message, &s_client_socket);
    
    pthread_join(s_thread_send, NULL);
    pthread_join(s_thread_receive, NULL);

    if (s_exit != 1) {
        close(s_client_socket);
    }
    
    return SUCCESS;
}

int main(int argc, char** argv)
{
    signal(SIGPIPE, SIG_IGN); /* ignore EPIPE(broken pipe) signal */
    signal(SIGINT, SIGINT_handler);

    create_client(argv[1], argv[2]);
}
