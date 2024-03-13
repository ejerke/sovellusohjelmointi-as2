#include "socket.h"

// Send always a package of size MAX_BUFFER_SIZE

int write_to_socket(int sockfd, const char* msg){
    char to_send[MAX_BUFFER_SIZE];
    memset(to_send, '\0', MAX_BUFFER_SIZE);

    // snprintf(to_send, sizeof(to_send), msg);

    strncpy(to_send, msg,
          (strlen(msg) < MAX_BUFFER_SIZE ? strlen(msg) : MAX_BUFFER_SIZE));
    int ret =  write(sockfd, to_send, sizeof(to_send));
    FILE* logfd = fopen(LOG_FILE, "a");
    if ( !logfd )
        return(-1);
    fputs(msg, logfd);
    fclose(logfd);

    return(ret);
}
