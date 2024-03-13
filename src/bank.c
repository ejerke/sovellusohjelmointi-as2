#include "bank.h"
 
static pthread_t threadit[MAX_THREAD_COUNT];
static int socketit[MAX_THREAD_COUNT];
static int thread_done[MAX_THREAD_COUNT];

static int* i_ptr[MAX_THREAD_COUNT];
static volatile int quit = 0;

static pthread_rwlock_t lukko = PTHREAD_RWLOCK_INITIALIZER;


// Bank server
int main(){
    pthread_t queue;
    signal(SIGINT, sig_handler);
    // All threads are free by default
    for (int i = 0; i < MAX_THREAD_COUNT; ++i ){
        thread_done[i] = 1;
    }

    if ( pthread_create(&queue, NULL, queue_thread, NULL) != 0 )
        printf("no queue\n");

    pthread_join(queue, NULL);

    for (int i = 0; i < MAX_THREAD_COUNT - 1; ++i){
        pthread_join(threadit[i], NULL);
    }

    pthread_rwlock_destroy(&lukko);
    printf("server is done\n");
    exit(0);
}

// Handle queue
void* queue_thread(void* a){
    // signal(SIGINT, sig_handler);

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(MASTER_IN_SOCKET);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    struct sockaddr_storage serverStorage;

    bind(serverSocket, (const struct sockaddr*)&server_addr, sizeof(server_addr));

    if ( listen(serverSocket, MAX_BACKLOG) )
    {
        printf("soketti rikki\n");
        return(NULL);
    }

    int i = 0;
    socklen_t addr_size;

    printf("server ready!\n");
    while ( !quit ) {
        addr_size = sizeof(serverStorage);

        // implement queue with a busy block here as long as no threads are ready
        // easy to do, but cant tell client the situation since connection
        // hasn't been established
        while ( !thread_done[i] ){
            i = (i+1) % MAX_THREAD_COUNT;
            if ( quit ){
                printf("exitt??\n");
                i = MAX_THREAD_COUNT-1;
                while ( i > 0 )
                    free(i_ptr[i--]);
                return(NULL);
            }
        }
        thread_done[i] = 0;

        // Extract the first connection in the queue
        socketit[i] = accept(serverSocket,
                           (struct sockaddr*)&serverStorage,
                           &addr_size);

        // Creater service desk thread
        int* give_i = malloc(sizeof(int));

        if ( !give_i ){
            perror("malloc error\n");
            return(NULL);
        }

        *give_i = i;

        if ( i_ptr[i] )
            free(i_ptr[i]);

        i_ptr[i] = give_i;
        if ( pthread_create(&threadit[i], NULL, service_desk, give_i) != 0 ){
            // Error in creating thread
            printf("Failed to create thread\n");
            quit = 1;
        }
    }

    i = MAX_THREAD_COUNT-1;
    while ( i > 0 )
        free(i_ptr[i--]);
    return(NULL);
}

void *service_desk(void* sock){
    int* i = (int*)sock;
    char message[MAX_BUFFER_SIZE];
    memset(message, 0, sizeof(message));

    int quit = 0;
    char *buf = malloc(MAX_BUFFER_SIZE);
    if ( buf == NULL) {
        thread_done[*i] = 1;
        return(NULL);
    }

    // Here we give client the new socket
    write_to_socket(socketit[*i], READY_MESSAGE);

    while ( !quit ) {
        // Read a command
        read(socketit[*i], buf, MAX_BUFFER_SIZE);

        int accno = -1, amount = 0;
        int correct = 0;
        // Process the command
        // First letter determines the command
        switch ( buf[0] ) { 
        // quit
        case 'q':
            quit = 1;
            break;

        // List account value
        case 'l': {
            int accno = -1;
            if ( sscanf(buf,"l %d",&accno) == 1 ) {
                pthread_rwlock_rdlock(&lukko);
                
                FILE* data = fopen(BANK_FILENAME, "r");
                if ( !data ){
                    perror("file not opened\n");
                    pthread_rwlock_unlock(&lukko);
                    break;
                }
                int find_accno;
                char temp[MAX_BUFFER_SIZE];
                int balance = 0;
                char* ret = NULL;
                memset(temp, 0, sizeof(temp));
                while( ( ret = fgets(temp, sizeof(temp), data)) != NULL ){
                    if ( sscanf(temp, "%d %d", &find_accno, &balance) == 2 )
                        if ( find_accno == accno )
                            break;
                    memset(temp, 0, sizeof(temp));
                }
                snprintf(message, sizeof(message), "balance of %d is %d€\n", accno, balance);
                write_to_socket(socketit[*i], message);
                fclose(data);
                pthread_rwlock_unlock(&lukko);
            } else { write_to_socket(socketit[*i], "fail: Error in command\n"); }
            break;
        }

        // Transfer from one account to another
        case 't': {
            int accno_2 = -1;
            int res = sscanf(buf, "t %d %d %d", &accno, &accno_2, &amount);
            if ( res != 3 ){
                write_to_socket(socketit[*i], "bad input, should be accno accno amount\n");
                break;
            }
            if ( accno <= 0 || accno_2 <= 0 || amount <= 0 ){
                write_to_socket(socketit[*i], "Bad account number or amount, all"
                                                  " should be positive\n");
            }
            // Take an additional write lock for both of the function calls
            // First try to take the money, if it succeeds, add it to the other account
            pthread_rwlock_wrlock(&lukko);
            res = manipulate_acc(*i, accno, amount);
            if ( res ){
                pthread_rwlock_unlock(&lukko);
                break;
            }

            res = manipulate_acc(*i, accno_2, -amount);
            pthread_rwlock_unlock(&lukko);
            if ( !res ){
                snprintf(message, sizeof(message), "transferred %d€ succesfully from acc:"
                                                   " %d to %d\n", amount, accno, accno_2);
                write_to_socket(socketit[*i], message);
            }
            break;
        }

        // Deposit to an account is done by withdrawing a negative amount
        case 'd':
            if ( sscanf(buf,"d %d %d", &accno, &amount) == 2 )
                correct = 1;

        // Withdraw from an account
        case 'w': {
            if ( buf[0] == 'w' && sscanf(buf,"w %d %d",&accno,&amount) == 2 )
                correct = 1;

            if ( !correct )
                write_to_socket(socketit[*i], "Bad command\n");

            if ( accno <= 0 || amount <= 0 )
            {
                write_to_socket(socketit[*i], "Bad account number or amount, both"
                                                " should be positive\n");
                break;
            }
            if ( buf[0] == 'd' )
            {
                pthread_rwlock_wrlock(&lukko);
                if ( !manipulate_acc(*i, accno, -amount) )
                    snprintf(message, sizeof(message), "deposited %d€ succesfully\n", amount);
                pthread_rwlock_unlock(&lukko);
            }
            else
            {
                pthread_rwlock_wrlock(&lukko);
                if ( !manipulate_acc(*i, accno, amount) )
                    snprintf(message, sizeof(message), "withdrew %d€ succesfully\n", amount);
                pthread_rwlock_unlock(&lukko);
            }
            write_to_socket(socketit[*i], message);
            break;
        }

        // Unknown command
        default:
            write_to_socket(socketit[*i], "fail: Unknown command\n");
            break;
        }
    }

    // Clean up
    free(buf);
    write_to_socket(socketit[*i], KILL_MESSAGE);
    thread_done[*i] = 1;
    return(NULL);
}

void sig_handler(int sig){
    quit = 1;
}

// All account manipulations with one function so the user of the function 
// must check that amount is negative or positive depending on the use case.
// amount should be negative to deposit.
int manipulate_acc(int socket_i, int accno, int amount){
    char message[MAX_BUFFER_SIZE];
    memset(message, 0, sizeof(message));
    // Wait for the lock and then open the file

    FILE* data_read = fopen(BANK_FILENAME, "r");
    if ( !data_read ){
        write_to_socket(socketit[socket_i], "Error opening the file\n");
        return(-1);
    }

    char* ret = NULL;
    char temp[MAX_BUFFER_SIZE];
    char new_file[MAX_BUFFER_SIZE];
    memset(temp, 0, sizeof(temp));
    memset(new_file, 0, sizeof(new_file));
    int new_i = 0, found_line = 0, balance = 0, find_accno = 0;
    int change = 0;

    // Go through the whole file so we can rewrite it later
    while( ( ret = fgets(temp, sizeof(temp), data_read)) != NULL ){
        strncpy(new_file+(new_i), temp, strlen(temp));
        if ( !found_line && (sscanf(temp, "%d %d", &find_accno, &balance) == 2) )
            if ( find_accno == accno )
            {
                char new_balance[11]; /* max int value is 10 chars long */
                char old_balance[11]; /* max int value is 10 chars long */
                char acc_text[11];    /* max int value is 10 chars long */
                snprintf(new_balance, sizeof(new_balance), "%d\n", balance-amount);
                snprintf(old_balance, sizeof(old_balance), "%d\n", balance);
                snprintf(acc_text, sizeof(acc_text), "%d", accno);

                strncpy(new_file+new_i+(strlen(acc_text)+1), new_balance, strlen(new_balance));

                change = strlen(new_balance)-strlen(old_balance);
                found_line = 1;
            }
        new_i += strlen(temp)+change;
        change = 0;

        memset(temp, 0, sizeof(temp));
    }
    // Done reading, close the file and open again with 'w' to overwrite
    fclose(data_read);

    // No account found
    if ( !found_line ){
        write_to_socket(socketit[socket_i], "No such account\n");
        return(2);
    }

    // Not enough money in the account
    if ( (balance - amount) < 0){
        write_to_socket(socketit[socket_i], "Insufficient funds\n");
        return(INSUFFICIENT_FUNDS);
    }

    FILE* data_write = fopen(BANK_FILENAME, "w");
    if ( !data_write ){
        write_to_socket(socketit[socket_i], "Error opening the file\n");
        return(-1);
    }
    int idx = 0;
    while(new_file[idx]){
        fputc(new_file[idx++], data_write);
    }
    fclose(data_read);

    return(0);
}
