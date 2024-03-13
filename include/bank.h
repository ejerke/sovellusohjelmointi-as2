#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <ctype.h>
#include <signal.h>

#include "socket.h"
#include "constants.h"

#define MAX_THREAD_COUNT 10

void *service_desk(void* sock);

int manipulate_acc(int i, int accno, int amount);

void sig_handler(int);

void* queue_thread(void* a);
