#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <stdarg.h>

#include "constants.h"

int write_to_socket(int, const char*);
