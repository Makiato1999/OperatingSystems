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
#include <assert.h>
// define global variable
#define NANOS_PER_USEC 1000  // microsleep
#define USEC_PER_SEC 1000000 // microsleep
#define maxNum_eachLine 1024 //
int numOfCPUs;               // number of CPUs we need to create
char *workload;              // task set name, which is .txt file
int boostTime;               // time for boost up all tasks in MLFQ
int reading_done = 0;        // status of reading_thread
int numOfRunningCPU = 0;     // number of running CPUs
int quantumLength = 50;      // the time slice in MLFQ each layer
int allotmentTime = 200;     // task will reduce priority if its time is over allotmentTime
pthread_mutex_t mutex_reading = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_CPU_wait = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_CPU_dequeue = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_CPU_enqueue = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_CPU_wakeup = PTHREAD_COND_INITIALIZER;
// define the node object
struct taskObj
{
    char task_name[maxNum_eachLine];
    int task_type;
    int task_length;
    int odds_of_IO;
    int hasCostTime;
    int restRunTime;
    int priority;
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
void *reading_thread();
void *CPU_thread();
void scheduler(struct node *thisTask);
void runTask(struct node *thisTask);
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
    // initialize all queues
    TAILQ_INIT(&ready_queue);
    TAILQ_INIT(&running_queue_highPriority);
    TAILQ_INIT(&running_queue_mediumPriority);
    TAILQ_INIT(&running_queue_lowPriority);
    TAILQ_INIT(&done_queue);
    // create reading thread
    pthread_t P_reading;
    if (pthread_create(&P_reading, NULL, reading_thread, NULL) != 0)
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
        if (pthread_create(&P_CPUs[i], NULL, CPU_thread, NULL) != 0)
        {
            perror("Failed to create CPU thread!\n");
            exit(1);
        }
    }
    for (int i = 0; i < numOfCPUs; i++)
    {
        pthread_join(P_CPUs[i], NULL);
    }
    return EXIT_SUCCESS;
}
//------------------------------------------------------
// myRoutine: reading_thread
//
// PURPOSE: add all tasks to ready_queue
// INPUT PARAMETERS:
//     void *arg
//------------------------------------------------------
void *reading_thread()
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
        strcpy(newNode->task.task_name, item);                     // save task_name
        item = strtok(NULL, " ");                                  //
        newNode->task.task_type = atoi(item);                      // save task_type
        if ((item = strtok(NULL, " ")) != NULL)                    // check if there is DELAY
        {                                                          //
            newNode->task.task_length = atoi(item);                // save task_length
            item = strtok(NULL, " ");                              //
            newNode->task.odds_of_IO = atoi(item);                 // save odds_of_IO
            newNode->task.hasCostTime = 0;                         // initialize hasCostTime as 0
            newNode->task.restRunTime = newNode->task.task_length; // initialize restrunTime as task_length
            newNode->task.priority = 3;                            // initialize highest priority as 3
            //@Test
            printf("%dth task: (%s %d %d %d)(hasCostTime: %d, restRunTime: %d, priority: %d)\n",
                   counter, newNode->task.task_name, newNode->task.task_type, newNode->task.task_length, newNode->task.odds_of_IO,
                   newNode->task.hasCostTime, newNode->task.restRunTime, newNode->task.priority);
            // enqueue to ready_queue
            TAILQ_INSERT_TAIL(&ready_queue, newNode, next);
            counter++;
        }
        else
        {
            //@Test
            // printf("DELAY: (%s %d)\n", newNode->task.task_name, newNode->task.task_type);
            microsleep(newNode->task.task_type);
        }
    }
    // prompt CPU_threads can wake up
    reading_done = 1;
    pthread_cond_signal(&cond_CPU_wakeup);
    pthread_mutex_unlock(&mutex_reading);
    fclose(fp);
    pthread_exit((void *)pthread_self());
}
//------------------------------------------------------
// myRoutine: CPU_thread
//
// PURPOSE: allocate tasks and
// INPUT PARAMETERS:
//------------------------------------------------------
void *CPU_thread()
{
    // wait until reading_thread has done
    pthread_mutex_lock(&mutex_CPU_wait);
    while (reading_done == 0)
    {
        //@Test
        printf("thread is waiting for ready_queue available...\n");
        pthread_cond_wait(&cond_CPU_wakeup, &mutex_CPU_wait);
    }
    //@Test
    printf("thread is free\n");
    pthread_mutex_unlock(&mutex_CPU_wait);

    // ckeck if ready_queue is empty
    while (!TAILQ_EMPTY(&ready_queue))
    {
        // if numOfRunningCPU(0,1,2...) is less than numOfCPUs(2/4/8...)
        // which means we need to fetch new task from ready_queue
        // because each CPU only can process one task each time
        if (numOfRunningCPU < numOfCPUs)
        {
            // dequeue ready_queue top node, which is the task needs to run
            struct node *first = TAILQ_FIRST(&ready_queue);
            pthread_mutex_lock(&mutex_CPU_dequeue);
            TAILQ_REMOVE(&ready_queue, first, next);
            pthread_mutex_unlock(&mutex_CPU_dequeue);
            // enqueue running_queue_highPriority first, because it fetched from ready_queue
            pthread_mutex_lock(&mutex_CPU_enqueue);
            TAILQ_INSERT_TAIL(&running_queue_highPriority, first, next);
            pthread_mutex_unlock(&mutex_CPU_enqueue);
            // update the numOfRunningCPU
            numOfRunningCPU += 1;
            // process this task
            scheduler(first);
            //@Test
            printf("enqueue running_queue_highPriority node: (%s %d %d %d)\n",
                   first->task.task_name, first->task.task_type,
                   first->task.task_length, first->task.odds_of_IO);
        }
        // now all CPUs are working
        // we dont need to fetch new task from ready_queue
        else
        {
            if (!TAILQ_EMPTY(&running_queue_highPriority))
            {
                // dequeue running_queue_highPriority top node
                // which is the task needs to run
                struct node *first = TAILQ_FIRST(&running_queue_highPriority);
                assert(first->task.priority == 3);
                // (dequeue operation)
                pthread_mutex_lock(&mutex_CPU_dequeue);
                TAILQ_REMOVE(&running_queue_highPriority, first, next);
                pthread_mutex_unlock(&mutex_CPU_dequeue);
                // process this task
                scheduler(first);
            }
            // when running_queue_highPriority is empty, and task is still not finished
            // process running_queue_mediumPriority
            else
            {
                if (!TAILQ_EMPTY(&running_queue_mediumPriority))
                {
                    // dequeue running_queue_mediumPriority top node
                    // which is the task needs to run
                    struct node *first = TAILQ_FIRST(&running_queue_mediumPriority);
                    assert(first->task.priority == 2);
                    // (dequeue operation)
                    pthread_mutex_lock(&mutex_CPU_dequeue);
                    TAILQ_REMOVE(&running_queue_mediumPriority, first, next);
                    pthread_mutex_unlock(&mutex_CPU_dequeue);
                    // process this task
                    scheduler(first);
                }
                // when running_queue_lowPriority is empty, and task is still not finished
                // process running_queue_lowPriority
                else
                {
                    if (!TAILQ_EMPTY(&running_queue_lowPriority))
                    {
                        // dequeue running_queue_lowPriority top node
                        // which is the task needs to run
                        struct node *first = TAILQ_FIRST(&running_queue_lowPriority);
                        assert(first->task.priority == 1);
                        // (dequeue operation)
                        pthread_mutex_lock(&mutex_CPU_dequeue);
                        TAILQ_REMOVE(&running_queue_lowPriority, first, next);
                        pthread_mutex_unlock(&mutex_CPU_dequeue);
                        // process this task
                        scheduler(first);
                    }
                    else
                    {
                        printf("uwu! looks this CPU has finished one task, it can process another one!\n");
                    }
                }
            }
        }
    }
    pthread_exit((void *)pthread_self());
}
//------------------------------------------------------
// myRoutine: scheduler
//
// PURPOSE: scheduler
// INPUT PARAMETERS:
//      struct node *thisTask
//------------------------------------------------------
void scheduler(struct node *thisTask)
{
    int randomNum_ifIO = 0;      // random number to check if this task request I/O
    int preemptive_IOLength = 0; // random number to decide the I/O runtime

    // check if this task will request I/O
    randomNum_ifIO = rand() % 101;
    // task willnot do I/O
    if (randomNum_ifIO > thisTask->task.odds_of_IO)
    {
        assert(randomNum_ifIO > thisTask->task.odds_of_IO);
        // run its time slice
        runTask(thisTask);
    }
    // task will do I/O
    else
    {
        assert(randomNum_ifIO <= thisTask->task.odds_of_IO);
        if (thisTask->task.priority == 3)
        {
            // dequeue original task from running_queue_highPriority
            // (we have do this dequeue operation in CPU_thread method)
            // generate preemptive I/O runtime
            preemptive_IOLength = rand() % (quantumLength + 1);
            // excute this task(preemptive I/O) instantly
            microsleep(preemptive_IOLength);
            // enqueue original task back to scheduler
            // ennqueue original task to running_queue_highPriority
            pthread_mutex_lock(&mutex_CPU_enqueue);
            TAILQ_INSERT_TAIL(&running_queue_highPriority, thisTask, next);
            pthread_mutex_unlock(&mutex_CPU_enqueue);
        }
        else if (thisTask->task.priority == 2)
        {
            // dequeue original task from running_queue_mediumPriority
            // (we have do this dequeue operation in CPU_thread method)
            // generate preemptive I/O runtime
            preemptive_IOLength = rand() % (quantumLength + 1);
            // excute this task(preemptive I/O) instantly
            microsleep(preemptive_IOLength);
            // enqueue original task back to scheduler
            // ennqueue original task to running_queue_mediumPriority
            pthread_mutex_lock(&mutex_CPU_enqueue);
            TAILQ_INSERT_TAIL(&running_queue_mediumPriority, thisTask, next);
            pthread_mutex_unlock(&mutex_CPU_enqueue);
        }
        else if (thisTask->task.priority == 1)
        {
            // dequeue original task from running_queue_lowPriority
            // (we have do this dequeue operation in CPU_thread method)
            // generate preemptive I/O runtime
            preemptive_IOLength = rand() % (quantumLength + 1);
            // excute this task(preemptive I/O) instantly
            microsleep(preemptive_IOLength);
            // enqueue original task back to scheduler
            // ennqueue original task to running_queue_lowPriority
            pthread_mutex_lock(&mutex_CPU_enqueue);
            TAILQ_INSERT_TAIL(&running_queue_lowPriority, thisTask, next);
            pthread_mutex_unlock(&mutex_CPU_enqueue);
        }
    }
}
//------------------------------------------------------
// myRoutine: runTask
//
// PURPOSE: runTask
// INPUT PARAMETERS:
//     struct node *ready_queue_first
//------------------------------------------------------
void runTask(struct node *thisTask)
{
    // need to reduce priority
    // it could be hasCostTime >= allotmentTime && thisTask->task.priority == 3
    // it could be hasCostTime >= allotmentTime && thisTask->task.priority == 2
    if (thisTask->task.hasCostTime >= allotmentTime && thisTask->task.priority > 1)
    {
        assert(thisTask->task.hasCostTime >= allotmentTime);
        assert(thisTask->task.priority > 1);
        // reduce priority
        thisTask->task.priority -= 1;
        // update the hasCostTime as 0, because it will run in a new queue
        thisTask->task.hasCostTime = 0;
        // restRunTime will not update
        if (thisTask->task.priority == 2)
        {
            // dequeue this task from running_queue_highPriority
            // (we have do this dequeue operation in CPU_thread method)
            // ennqueue this task(still has rest part needs to do) to running_queue_mediumPriority
            pthread_mutex_lock(&mutex_CPU_enqueue);
            TAILQ_INSERT_TAIL(&running_queue_mediumPriority, thisTask, next);
            pthread_mutex_unlock(&mutex_CPU_enqueue);
        }
        else if (thisTask->task.priority == 1)
        {
            // dequeue this task from running_queue_mediumPriority
            // (we have do this dequeue operation in CPU_thread method)
            // ennqueue this task(still has rest part needs to do) to running_queue_lowPriority
            pthread_mutex_lock(&mutex_CPU_enqueue);
            TAILQ_INSERT_TAIL(&running_queue_lowPriority, thisTask, next);
            pthread_mutex_unlock(&mutex_CPU_enqueue);
        }
    }
    // not need to reduce priority
    // it could be hasCostTime < allotmentTime && thisTask->task.priority == 3
    // it could be hasCostTime < allotmentTime && thisTask->task.priority == 2
    // it could be hasCostTime >= allotmentTime && thisTask->task.priority == 1
    // it could be hasCostTime < allotmentTime && thisTask->task.priority == 1
    else
    {
        if (thisTask->task.priority == 3)
        {
            if (thisTask->task.restRunTime <= quantumLength)
            {
                // dequeue this task from running_queue_highPriority
                // (we have do this dequeue operation in CPU_thread method)
                // excute this task by its runtime
                microsleep(thisTask->task.restRunTime);
                // update the hasCostTime (new hasCostTime should be taskLength in priority 3)
                thisTask->task.hasCostTime += thisTask->task.restRunTime;
                assert(thisTask->task.hasCostTime == thisTask->task.task_length);
                // update the restRunTime (new restRunTime should be 0)
                thisTask->task.restRunTime -= thisTask->task.restRunTime;
                assert(thisTask->task.restRunTime == 0);
                // ennqueue task to done_queue
                pthread_mutex_lock(&mutex_CPU_enqueue);
                TAILQ_INSERT_TAIL(&done_queue, thisTask, next);
                pthread_mutex_unlock(&mutex_CPU_enqueue);
                // this task has done
                numOfRunningCPU -= 1;
            }
            else
            {
                // dequeue this task from running_queue_highPriority
                // (we have do this dequeue operation in CPU_thread method)
                // excute this task by its runtime which is quantumLength
                microsleep(quantumLength);
                // update the hasCostTime as quantumLength
                thisTask->task.hasCostTime += quantumLength;
                // update the restRunTime
                thisTask->task.restRunTime -= quantumLength;
                // ennqueue this task(still has rest part needs to do) to running_queue_highPriority
                pthread_mutex_lock(&mutex_CPU_enqueue);
                TAILQ_INSERT_TAIL(&running_queue_highPriority, thisTask, next);
                pthread_mutex_unlock(&mutex_CPU_enqueue);
            }
        }
        else if (thisTask->task.priority == 2)
        {
            if (thisTask->task.restRunTime <= quantumLength)
            {
                // dequeue this task from running_queue_mediumPriority
                // (we have do this dequeue operation in CPU_thread method)
                // excute this task by its runtime
                microsleep(thisTask->task.restRunTime);
                // update the hasCostTime
                thisTask->task.hasCostTime += thisTask->task.restRunTime;
                // update the restRunTime (new restRunTime should be 0)
                thisTask->task.restRunTime -= thisTask->task.restRunTime;
                assert(thisTask->task.restRunTime == 0);
                // ennqueue task to done_queue
                pthread_mutex_lock(&mutex_CPU_enqueue);
                TAILQ_INSERT_TAIL(&done_queue, thisTask, next);
                pthread_mutex_unlock(&mutex_CPU_enqueue);
                // this task has done
                numOfRunningCPU -= 1;
            }
            else
            {
                // dequeue this task from running_queue_mediumPriority
                // (we have do this dequeue operation in CPU_thread method)
                // excute this task by its runtime which is quantumLength
                microsleep(quantumLength);
                // update the hasCostTime as quantumLength
                thisTask->task.hasCostTime += quantumLength;
                // update the restRunTime
                thisTask->task.restRunTime -= quantumLength;
                // ennqueue this task(still has rest part needs to do) to running_queue_mediumPriority
                pthread_mutex_lock(&mutex_CPU_enqueue);
                TAILQ_INSERT_TAIL(&running_queue_mediumPriority, thisTask, next);
                pthread_mutex_unlock(&mutex_CPU_enqueue);
            }
        }
        else if (thisTask->task.priority == 1)
        {
            if (thisTask->task.restRunTime <= quantumLength)
            {
                // dequeue this task from running_queue_lowPriority
                // (we have do this dequeue operation in CPU_thread method)
                // excute this task by its runtime
                microsleep(thisTask->task.restRunTime);
                // update the hasCostTime
                thisTask->task.hasCostTime += thisTask->task.restRunTime;
                // update the restRunTime (new restRunTime should be 0)
                thisTask->task.restRunTime -= thisTask->task.restRunTime;
                assert(thisTask->task.restRunTime == 0);
                // ennqueue task to done_queue
                pthread_mutex_lock(&mutex_CPU_enqueue);
                TAILQ_INSERT_TAIL(&done_queue, thisTask, next);
                pthread_mutex_unlock(&mutex_CPU_enqueue);
                // this task has done
                numOfRunningCPU -= 1;
            }
            else
            {
                // dequeue this task from running_queue_lowPriority
                // (we have do this dequeue operation in CPU_thread method)
                // excute this task by its runtime which is quantumLength
                microsleep(quantumLength);
                // update the hasCostTime as quantumLength
                thisTask->task.hasCostTime += quantumLength;
                // update the restRunTime
                thisTask->task.restRunTime -= quantumLength;
                // ennqueue this task(still has rest part needs to do) to running_queue_lowPriority
                pthread_mutex_lock(&mutex_CPU_enqueue);
                TAILQ_INSERT_TAIL(&running_queue_lowPriority, thisTask, next);
                pthread_mutex_unlock(&mutex_CPU_enqueue);
            }
        }
    }
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