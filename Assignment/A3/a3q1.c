//-----------------------------------------
// NAME: Xiaoran Xie
// STUDENT NUMBER: 7884702
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Robert Guderian
// ASSIGNMENT: assignment 3
//
// REMARKS: MLFQ
//
//-----------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>
#include <sys/queue.h>
#include <time.h>
// define global variable
#define NANOS_PER_USEC 1000  // microsleep
#define USEC_PER_SEC 1000000 // microsleep
#define maxNum_eachLine 1024 //
int numOfCPUs;               // number of CPUs
char *workload;              // task set name, which is .txt file
int boostTime;               // time for boost up all tasks in MLFQ
int reading_done = 0;        // status of reading_thread
pthread_mutex_t mutex_reading = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_CPU = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_reading = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_CPU = PTHREAD_COND_INITIALIZER;
// define the node object
struct taskObj
{
    char task_name[maxNum_eachLine];
    int task_type;
    int task_length;
    int odds_of_IO;
};
// define queue node
struct node
{
    struct taskObj task;
    TAILQ_ENTRY(node)
    next;
};
// define queue head
TAILQ_HEAD(head, node);
// initialize queue
struct head ready_queue;
struct head running_queue_highPriority;
struct head running_queue_mediumPriority;
struct head running_queue_lowPriority;
struct head done_queue;
// initialize all methods
void *reading_thread(void *arg);
void *CPU_thread(void *arg);
static void microsleep(unsigned int usecs);

int main(int argc, char *argv[])
{
    // execption
    if (argc < 4)
    {
        perror("Invalid arguments!\n");
        exit(1);
    }
    // read script file
    numOfCPUs = atoi(argv[1]);
    boostTime = atoi(argv[2]);
    workload = argv[3];
    // initialize ready_queue
    TAILQ_INIT(&ready_queue);
    // create reading thread
    pthread_t P_reading;
    if (pthread_create(&P_reading, NULL, reading_thread, &ready_queue) != 0)
    {
        perror("Failed to create reading thread!\n");
        exit(1);
    }
    pthread_join(P_reading, NULL);
    /*//@Test
    // check all nodes by dequeue
    while (!TAILQ_EMPTY(&ready_queue))
    {
        struct node *first = TAILQ_FIRST(&ready_queue);
        printf("node: (%s %d %d %d)\n", first->task.task_name, first->task.task_type, first->task.task_length, first->task.odds_of_IO);
        TAILQ_REMOVE(&ready_queue, first, next);
        free(first);
    }*/
    // create CPU threads
    pthread_t P_CPUs[numOfCPUs];
    int i;
    for (i = 0; i < numOfCPUs; i++)
    {
        if (pthread_create(&P_CPUs[i], NULL, CPU_thread, (void *)&i) != 0)
        {
            perror("Failed to create CPU thread!\n");
            exit(1);
        }
        pthread_join(P_CPUs[i], NULL);
    }
    /*
    for (i = 0; i < numOfCPUs; i++)
    {
        pthread_join(P_CPUs[i], NULL);
    }*/
    return EXIT_SUCCESS;
}
//------------------------------------------------------
// myRoutine: reading_thread
//
// PURPOSE: add all tasks to ready_queue
// INPUT PARAMETERS:
//     void *arg
//------------------------------------------------------
void *reading_thread(void *arg)
{
    char line[maxNum_eachLine]; // save whole line
    char *buffer;               // save whole line without "\t\r\n"
    char *item;                 // save items in buffer
    int counter = 0;            //
    FILE *fp = NULL;            // open file
    // open workload, which is a .txt file
    fp = fopen(workload, "r");
    // exception
    if (fp == NULL)
    {
        perror("Failed to open .txt file!\n");
        exit(1);
    }

    pthread_mutex_lock(&mutex_reading);
    // read workload
    while (fgets(line, maxNum_eachLine, fp))
    {
        // get line
        buffer = strtok(line, "\t\r\n");
        // create node to save line
        struct node *newNode = malloc(sizeof(struct node));
        // split up the line to muti items and put them into task attributes
        item = strtok(buffer, " ");
        strcpy(newNode->task.task_name, item);      // save task_name
        item = strtok(NULL, " ");                   //
        newNode->task.task_type = atoi(item);       // save task_type
        if ((item = strtok(NULL, " ")) != NULL)     // check if there is DELAY
        {                                           //
            newNode->task.task_length = atoi(item); // save task_length
            item = strtok(NULL, " ");               //
            newNode->task.odds_of_IO = atoi(item);  // save odds_of_IO
            //@Test
            printf("%dth task: (%s %d %d %d)\n", counter, newNode->task.task_name, newNode->task.task_type, newNode->task.task_length, newNode->task.odds_of_IO);
            // enqueue to ready_queue
            TAILQ_INSERT_TAIL((struct head *)arg, newNode, next);
            counter++;
        }
        else
        {
            //@Test
            // printf("DELAY: (%s %d)\n", newNode->task.task_name, newNode->task.task_type);
            microsleep(newNode->task.task_type);
        }
    }
    pthread_mutex_unlock(&mutex_reading);
    // prompt CPU_threads can wake up
    reading_done = 1;
    pthread_cond_signal(&cond_reading);
    fclose(fp);
    pthread_exit((void *)pthread_self());
}
//------------------------------------------------------
// myRoutine: CPU_thread
//
// PURPOSE: allocate tasks and
// INPUT PARAMETERS:
//     int i  
//------------------------------------------------------
void *CPU_thread(void *arg)
{
    int *temp = (int *)arg;
    pthread_mutex_lock(&mutex_CPU);
    while (reading_done == 0)
    {
        printf("thread %d is waiting for ready_queue available...\n", *temp);
        pthread_cond_wait(&cond_reading, &mutex_CPU);
    }
    pthread_mutex_unlock(&mutex_CPU);
    printf("thread %d is free\n", *temp);
    pthread_exit((void *)pthread_self());
}
//------------------------------------------------------
// myRoutine: microsleep
//
// PURPOSE: microsleep
// INPUT PARAMETERS:
//     unsigned int usecs
//------------------------------------------------------
static void microsleep(unsigned int usecs)
{
    long seconds = usecs / USEC_PER_SEC;
    long nanos = (usecs % USEC_PER_SEC) * NANOS_PER_USEC;
    struct timespec t = {.tv_sec = seconds, .tv_nsec = nanos};
    int ret;
    do
    {
        ret = nanosleep(&t, &t);
        // need to loop, `nanosleep` might return before sleeping
        // for the complete time (see `man nanosleep` for details)
    } while (ret == -1 && (t.tv_sec || t.tv_nsec));
}
