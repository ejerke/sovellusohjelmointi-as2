#include "client.h"

static volatile int quit = 0;
void sig_handler(int);
static int master_socket = 0;

int main(){
    setvbuf(stdin,NULL,_IOLBF,0);
    setvbuf(stdout,NULL,_IOLBF,0);
    // Catch SIGINT
    signal(SIGINT, sig_handler);


    // Initialize socket on client side
    int master_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(MASTER_IN_SOCKET);
    client_addr.sin_addr.s_addr = INADDR_ANY;

    // connect to the masters socket to get directed to service
    if ( connect(master_socket, (const struct sockaddr*)&client_addr, sizeof(client_addr)) )
        exit(0);

    // Read the socket empty
    char message[MAX_BUFFER_SIZE];
    memset(message, 0, sizeof(message));
    // Write a message to socket
    // snprintf(message, sizeof(message), "connection work?");
    // write(master_socket, message, strlen(message)+1);

    // Wait for a reply
    char* ready_msg;
    read(master_socket, message, sizeof(message));
    printf("%s", message);
    if ( !(ready_msg = strstr(message, READY_MESSAGE)) ){
        printf("server wasn't ready\n");
        exit(0);
    }

    memset(message, 0, sizeof(message));
    while( !quit )
    {
        memset(message, 0, sizeof(message));
        read(STDIN_FILENO, message, sizeof(message));
        write_to_socket(master_socket, message);
        if ( !strcmp("q\n", message))
            break;
        memset(message, 0, sizeof(message));
        read(master_socket, message, sizeof(message));
        if ( strcmp(message, KILL_MESSAGE) == 0)
            break;
        fprintf(stdout,"%s", message);
    }

    printf("done\n");
    // write_to_socket(master_socket, "q");
    exit(0);
}

void sig_handler(int sig){
    write_to_socket(master_socket, "q\n");
    exit(0);
}
