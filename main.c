/***LIBRARIES***/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include <zconf.h>

/***CONSTANTS***/
#define MIN_ARG         2       //min required args on cmd line
#define MIN_INT         1       //min number to input
#define ASCII_OF_ZERO   48      //used to check if an input is a zero
#define PTHREAD_SYNC    1
/***FUNCTION PROTOTYPES***/
int validCmdLineInput(int argc, char *argv[]);
void printError();
void *simpleThread(void* currentThread);

int sharedVar = 0;
pthread_mutex_t mutex;
pthread_barrier_t barrier;

int main(int argc, char *argv[])
{
    if(!validCmdLineInput(argc, argv)){
        printError();
    }
    else{
        const char *nPtr = argv[1];
        char *endPtr = NULL;
        int base = 10;
        unsigned int numberOfThreads = 0;
        int threadIds[numberOfThreads];
        numberOfThreads = (unsigned int) strtol(nPtr, &endPtr, base);

        pthread_t threads[numberOfThreads];

        //barrier init
        if(pthread_barrier_init(&barrier, NULL, numberOfThreads)){
            fprintf(stderr, "Could not create a barrier\n");
        }

        if(pthread_mutex_init(&mutex, NULL)){
            fprintf(stderr, "Could not initialize mutex\n");
        }

        for(int i = 1; i <= numberOfThreads; ++i){
            int retThreadVal = 0;
            threadIds[i] = i;
            retThreadVal = pthread_create(&threads[i], NULL,
                                           simpleThread, &threadIds[i]);
            if(retThreadVal != 0)
                fprintf(stderr, "Could not create thread\n");

        }
        for(int i = 1; i <= numberOfThreads; ++i){
            pthread_join(threads[i], NULL);
        }
        pthread_mutex_destroy(&mutex);
    }

    return EXIT_SUCCESS;
}

//TODO: safely convert long to int. check its below Integer.maxvalue
int validCmdLineInput(int argc, char *argv[]){
    if(argc != MIN_ARG){
        return false;
    }

    const char *nPtr = argv[1];
    char *endPtr = NULL;
    int base = 10;
    long number = 0;

    errno = 0;

    number = strtol(nPtr, &endPtr, base);

    if(nPtr == endPtr) // no conversion performed, no digits found
        return false;
    else if(errno == ERANGE){ //input overflowed or underflowed
        return false;
    }
    else if(errno == EINVAL){ //input has unsupported base value
        return false;
    }
    else if(errno != 0 && number == 0){ //unspecified error occurred
        return false;
    }
    else if(errno == 0 && number < 1){ //input is zero
        return false;
    }

    return true;

}

void printError(){
    fprintf(stderr, "Invalid input\n");
}

void *simpleThread(void* currentThread){
    int* currentThreadId = (int*) currentThread;
    int val = 0;

    for(int i = 0; i < 20; i++){
        if(random() > RAND_MAX / 2)
            usleep(500);

#ifdef PTHREAD_SYNC
        //lock mutex around operation
        pthread_mutex_lock(&mutex);
#endif
        val = sharedVar;
        printf("***thread %d sees value %d\n", *currentThreadId, val);
        sharedVar = val + 1;

#ifdef PTHREAD_SYNC
        //unlock mutex
        pthread_mutex_unlock(&mutex);

        int rc = pthread_barrier_wait(&barrier);
        if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD){
            fprintf(stderr, "Could not wait on barrier\n");
            //force exit
        }
#endif
    }

    val = sharedVar;
    printf("Thread %d sees final value %d\n", *currentThreadId, val);
}