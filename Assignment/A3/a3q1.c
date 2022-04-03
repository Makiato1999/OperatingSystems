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
#include <stdatomic.h>
// define global variable
#define NANOS_PER_USEC 1000   // microsleep
#define USEC_PER_SEC 1000000  // microsleep
#define maxNum_eachLine 1024  //
int numOfAllTasks;            // number of all tasks
atomic_int numOfDoneTasks;    // nnumber of done tasks
int numOfCPUs;                // number of CPUs we need to create
char *workload;               // task set name, which is .txt file
int boostTime;                // time for boost up all tasks in MLFQ to high priority
int allTasksHaveCostTime = 0; // all tasks have cost time
int reading_done = 0;         // status of reading_thread
int quantumLength = 50;       // the time slice in MLFQ each layer
int allotmentTime = 200;      // task will reduce priority if its time is over allotmentTime
int boostIndex = 0;
pthread_mutex_t mutex_reading = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_CPU_wait = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_CPU_prepare = PTHREAD_MUTEX_INITIALIZER;
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
    int needRunTime;
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
    /*
    //@Test
    // check all nodes by dequeue
    while (!TAILQ_EMPTY(&done_queue))
    {
        struct node *first = TAILQ_FIRST(&done_queue);
        printf("done task: (%s %d %d %d)\n", first->task.task_name, first->task.task_type, first->task.task_length, first->task.odds_of_IO);
        TAILQ_REMOVE(&done_queue, first, next);
        free(first);
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
void *reading_thread()
{
    char line[maxNum_eachLine]; // save whole line
    char *buffer;               // save whole line without "\t\r\n"
    char *item;                 // save items in buffer
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
            // enqueue to ready_queue
            TAILQ_INSERT_TAIL(&ready_queue, newNode, next);
            numOfAllTasks += 1;
        }
        else
        {
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
    printf("CPU is working...\n");
    pthread_mutex_unlock(&mutex_CPU_wait);

    while (1)
    {
        pthread_mutex_lock(&mutex_CPU_prepare);
        if (allTasksHaveCostTime >= boostTime)
        {
            assert(allTasksHaveCostTime >= boostTime);
            while (!TAILQ_EMPTY(&running_queue_mediumPriority))
            {
                struct node *temp = TAILQ_FIRST(&running_queue_mediumPriority);
                assert(temp->task.priority == 2);
                // dequeue from running_queue_mediumPriority
                TAILQ_REMOVE(&running_queue_mediumPriority, temp, next);
                // restore the high priority
                temp->task.priority = 3;
                // update the hasCostTime
                temp->task.hasCostTime = 0;
                // enqueue back to running_queue_highPriority
                TAILQ_INSERT_TAIL(&running_queue_highPriority, temp, next);
                printf("++++>%dth, (%s) has been boosted up\n", boostIndex, temp->task.task_name);
            }
            while (!TAILQ_EMPTY(&running_queue_lowPriority))
            {
                struct node *temp = TAILQ_FIRST(&running_queue_lowPriority);
                assert(temp->task.priority == 1);
                // dequeue from running_queue_lowPriority
                TAILQ_REMOVE(&running_queue_lowPriority, temp, next);
                // restore the high priority
                temp->task.priority = 3;
                // update the hasCostTime
                temp->task.hasCostTime = 0;
                // enqueue back to running_queue_highPriority
                TAILQ_INSERT_TAIL(&running_queue_highPriority, temp, next);
                printf("++++>%dth, (%s) has been boosted up\n", boostIndex, temp->task.task_name);
            }
            allTasksHaveCostTime = 0;
            boostIndex += 1;
        }
        assert(allTasksHaveCostTime < boostTime);
        if (!TAILQ_EMPTY(&ready_queue))
        {
            // dequeue ready_queue top node, which is the task needs to run
            struct node *first = TAILQ_FIRST(&ready_queue);
            TAILQ_REMOVE(&ready_queue, first, next);
            // enqueue running_queue_highPriority first, because it fetched from ready_queue
            TAILQ_INSERT_TAIL(&running_queue_highPriority, first, next);
            // process this task
            scheduler(first);
        }
        else
        {
            assert(TAILQ_EMPTY(&ready_queue));
            if (numOfDoneTasks == numOfAllTasks)
            {
                assert(numOfDoneTasks == numOfAllTasks);
                pthread_mutex_unlock(&mutex_CPU_prepare);
                break;
            }
            assert(numOfDoneTasks < numOfAllTasks);
            // when ready_queue is empty
            // checck running_queue_highPriority
            if (!TAILQ_EMPTY(&running_queue_highPriority))
            {
                // dequeue running_queue_highPriority top node
                // which is the task needs to run
                struct node *first = TAILQ_FIRST(&running_queue_highPriority);
                assert(first->task.priority == 3);
                // process this task
                scheduler(first);
            }
            // when running_queue_highPriority is empty
            // checck running_queue_mediumPriority
            else if (!TAILQ_EMPTY(&running_queue_mediumPriority))
            {
                // dequeue running_queue_mediumPriority top node
                // which is the task needs to run
                struct node *first = TAILQ_FIRST(&running_queue_mediumPriority);
                assert(first->task.priority == 2);
                // process this task
                scheduler(first);
            }
            // when running_queue_lowPriority is empty
            // check running_queue_lowPriority
            else if (!TAILQ_EMPTY(&running_queue_lowPriority))
            {
                // dequeue running_queue_lowPriority top node
                // which is the task needs to run
                struct node *first = TAILQ_FIRST(&running_queue_lowPriority);
                assert(first->task.priority == 1);
                // process this task
                scheduler(first);
            }
            else
            {
                assert(TAILQ_EMPTY(&ready_queue) && TAILQ_EMPTY(&running_queue_highPriority) && TAILQ_EMPTY(&running_queue_mediumPriority) && TAILQ_EMPTY(&running_queue_lowPriority));
                printf("numOfDoneTasks: %d, allTasks: %d\n", numOfDoneTasks, numOfAllTasks);
            }
        }
    }
    printf("CPU is quiting...\n");
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
    // check if this task will request I/O
    // pthread_mutex_lock(&mutex_CPU_prepare);
    int randomNum_ifIO = 0;      // random number to check if this task request I/O
    int preemptive_IOLength = 0; // random number to decide the I/O runtime

    randomNum_ifIO = rand() % 101;
    // task willnot do I/O
    if (randomNum_ifIO > thisTask->task.odds_of_IO)
    {
        assert(randomNum_ifIO > thisTask->task.odds_of_IO);
        // printf("-->(%s) has no preemptive I/O, keep running\n", thisTask->task.task_name);
        // pthread_mutex_unlock(&mutex_CPU_prepare);
        // run its time slice
        runTask(thisTask);
    }
    // task will do I/O
    else
    {
        assert(randomNum_ifIO <= thisTask->task.odds_of_IO);
        // printf("-->(%s) has preemptive I/O (%d <= %d), reschedule\n",
        //        thisTask->task.task_name, randomNum_ifIO, thisTask->task.odds_of_IO);
        //  generate preemptive I/O runtime
        preemptive_IOLength = rand() % (quantumLength + 1);
        if (thisTask->task.priority == 3)
        {
            assert(thisTask->task.priority == 3);
            // dequeue original task from running_queue_highPriority
            // (here)
            // (dequeue operation)
            TAILQ_REMOVE(&running_queue_highPriority, thisTask, next);
            // enqueue original task back to scheduler
            // ennqueue original task to running_queue_highPriority
            TAILQ_INSERT_TAIL(&running_queue_highPriority, thisTask, next);
        }
        else if (thisTask->task.priority == 2)
        {
            assert(thisTask->task.priority == 2);
            // dequeue original task from running_queue_mediumPriority
            // (here)
            // (dequeue operation)
            TAILQ_REMOVE(&running_queue_mediumPriority, thisTask, next);
            // enqueue original task back to scheduler
            // ennqueue original task to running_queue_mediumPriority
            TAILQ_INSERT_TAIL(&running_queue_mediumPriority, thisTask, next);
        }
        else if (thisTask->task.priority == 1)
        {
            assert(thisTask->task.priority == 1);
            // dequeue original task from running_queue_lowPriority
            // (here)
            // (dequeue operation)
            TAILQ_REMOVE(&running_queue_lowPriority, thisTask, next);
            // enqueue original task back to scheduler
            // ennqueue original task to running_queue_lowPriority
            TAILQ_INSERT_TAIL(&running_queue_lowPriority, thisTask, next);
        }
        pthread_mutex_unlock(&mutex_CPU_prepare);
        // excute this task(preemptive I/O) instantly
        microsleep(preemptive_IOLength);
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
    // pthread_mutex_lock(&mutex_CPU_prepare);
    if (thisTask->task.hasCostTime >= allotmentTime && thisTask->task.priority > 1)
    {
        assert(thisTask->task.hasCostTime >= allotmentTime);
        assert(thisTask->task.priority > 1);
        // printf("  |-->(%s) is over allotmentTime, reduce priority (hasCostTime: %d, restRunTime: %d)\n",
        //        thisTask->task.task_name, thisTask->task.hasCostTime, thisTask->task.restRunTime);
        // reduce priority
        thisTask->task.priority -= 1;
        // update the hasCostTime as 0, because it will run in a new queue
        thisTask->task.hasCostTime = 0;
        // restRunTime will not update
        if (thisTask->task.priority == 2)
        {
            assert(thisTask->task.priority == 2);
            // dequeue this task from running_queue_highPriority
            // (here)
            // (dequeue operation)
            TAILQ_REMOVE(&running_queue_highPriority, thisTask, next);
            // ennqueue this task(still has rest part needs to do) to running_queue_mediumPriority
            TAILQ_INSERT_TAIL(&running_queue_mediumPriority, thisTask, next);
        }
        else if (thisTask->task.priority == 1)
        {
            assert(thisTask->task.priority == 1);
            // dequeue this task from running_queue_mediumPriority
            // (here)
            // (dequeue operation)
            TAILQ_REMOVE(&running_queue_mediumPriority, thisTask, next);
            // ennqueue this task(still has rest part needs to do) to running_queue_lowPriority
            TAILQ_INSERT_TAIL(&running_queue_lowPriority, thisTask, next);
            // printf("insertQ********(%s)\n", thisTask->task.task_name);
        }
        pthread_mutex_unlock(&mutex_CPU_prepare);
    }
    // not need to reduce priority
    // it could be hasCostTime < allotmentTime && thisTask->task.priority == 3
    // it could be hasCostTime < allotmentTime && thisTask->task.priority == 2
    // it could be hasCostTime >= allotmentTime && thisTask->task.priority == 1
    // it could be hasCostTime < allotmentTime && thisTask->task.priority == 1
    else
    {
        // printf("   |-->(%s) is not over allotmentTime (hasCostTime: %d, restRunTime: %d)\n",
        //        thisTask->task.task_name, thisTask->task.hasCostTime, thisTask->task.restRunTime);
        if (thisTask->task.priority == 3)
        {
            assert(thisTask->task.priority == 3);
            // printf("      |-->(%s) is priority 3\n", thisTask->task.task_name);

            if (thisTask->task.restRunTime <= quantumLength)
            {
                assert(thisTask->task.restRunTime <= quantumLength);
                // dequeue this task from running_queue_highPriority
                // (here)
                // (dequeue operation)
                TAILQ_REMOVE(&running_queue_highPriority, thisTask, next);
                // update the allTasksHaveCostTime
                allTasksHaveCostTime += thisTask->task.restRunTime;
                // update the hasCostTime (new hasCostTime should be taskLength in priority 3)
                thisTask->task.hasCostTime += thisTask->task.restRunTime;
                // update the restRunTime (new restRunTime should be 0)
                thisTask->task.restRunTime -= thisTask->task.restRunTime;
                assert(thisTask->task.restRunTime == 0);
                // update microsleep
                thisTask->task.needRunTime = thisTask->task.restRunTime;
                // ennqueue task to done_queue
                TAILQ_INSERT_TAIL(&done_queue, thisTask, next);
                // this task has done
                numOfDoneTasks += 1;
                printf("         |-->(%s) is done (hasCostTime: %d, restRunTime: %d, doneTasks: %d)\n",
                       thisTask->task.task_name, thisTask->task.hasCostTime, thisTask->task.restRunTime, numOfDoneTasks);
            }
            else
            {
                assert(thisTask->task.restRunTime > quantumLength);
                // dequeue this task from running_queue_highPriority
                // (here)
                // (dequeue operation)
                TAILQ_REMOVE(&running_queue_highPriority, thisTask, next);
                // update the allTasksHaveCostTime
                allTasksHaveCostTime += quantumLength;
                // update the hasCostTime as quantumLength
                thisTask->task.hasCostTime += quantumLength;
                // update the restRunTime
                thisTask->task.restRunTime -= quantumLength;
                // update microsleep
                thisTask->task.needRunTime = quantumLength;
                // ennqueue this task(still has rest part needs to do) to running_queue_highPriority
                TAILQ_INSERT_TAIL(&running_queue_highPriority, thisTask, next);
            }
        }
        else if (thisTask->task.priority == 2)
        {
            assert(thisTask->task.priority == 2);
            // printf("      |-->(%s) is priority 2\n", thisTask->task.task_name);

            if (thisTask->task.restRunTime <= quantumLength)
            {
                assert(thisTask->task.restRunTime <= quantumLength);
                // dequeue this task from running_queue_mediumPriority
                // (here)
                // (dequeue operation)
                TAILQ_REMOVE(&running_queue_mediumPriority, thisTask, next);
                // update the allTasksHaveCostTime
                allTasksHaveCostTime += thisTask->task.restRunTime;
                // update the hasCostTime
                thisTask->task.hasCostTime += thisTask->task.restRunTime;
                // update the restRunTime (new restRunTime should be 0)
                thisTask->task.restRunTime -= thisTask->task.restRunTime;
                assert(thisTask->task.restRunTime == 0);
                // update microsleep
                thisTask->task.needRunTime = thisTask->task.restRunTime;
                // ennqueue task to done_queue
                TAILQ_INSERT_TAIL(&done_queue, thisTask, next);
                // this task has done
                numOfDoneTasks += 1;
                printf("         |-->(%s) is done (hasCostTime: %d, restRunTime: %d, doneTasks: %d)\n",
                       thisTask->task.task_name, thisTask->task.hasCostTime, thisTask->task.restRunTime, numOfDoneTasks);
            }
            else
            {
                assert(thisTask->task.restRunTime > quantumLength);
                // dequeue this task from running_queue_mediumPriority
                // (here)
                // (dequeue operation)
                TAILQ_REMOVE(&running_queue_mediumPriority, thisTask, next);
                // update the allTasksHaveCostTime
                allTasksHaveCostTime += quantumLength;
                // update the hasCostTime as quantumLength
                thisTask->task.hasCostTime += quantumLength;
                // update the restRunTime
                thisTask->task.restRunTime -= quantumLength;
                // update microsleep
                thisTask->task.needRunTime = quantumLength;
                // ennqueue this task(still has rest part needs to do) to running_queue_mediumPriority
                TAILQ_INSERT_TAIL(&running_queue_mediumPriority, thisTask, next);
            }
        }
        else if (thisTask->task.priority == 1)
        {
            assert(thisTask->task.priority == 1);
            // printf("      |-->(%s) is priority 1\n", thisTask->task.task_name);

            if (thisTask->task.restRunTime <= quantumLength)
            {
                assert(thisTask->task.restRunTime <= quantumLength);
                // dequeue this task from running_queue_lowPriority
                // (here)
                // (dequeue operation)
                TAILQ_REMOVE(&running_queue_lowPriority, thisTask, next);
                // update the allTasksHaveCostTime
                allTasksHaveCostTime += thisTask->task.restRunTime;
                // update the hasCostTime
                thisTask->task.hasCostTime += thisTask->task.restRunTime;
                // update the restRunTime (new restRunTime should be 0)
                thisTask->task.restRunTime -= thisTask->task.restRunTime;
                assert(thisTask->task.restRunTime == 0);
                // update microsleep
                thisTask->task.needRunTime = thisTask->task.restRunTime;
                // ennqueue task to done_queue
                TAILQ_INSERT_TAIL(&done_queue, thisTask, next);
                // this task has done
                numOfDoneTasks += 1;
                printf("         |-->(%s) is done (hasCostTime: %d, restRunTime: %d, doneTasks: %d)\n",
                       thisTask->task.task_name, thisTask->task.hasCostTime, thisTask->task.restRunTime, numOfDoneTasks);
            }
            else
            {
                assert(thisTask->task.restRunTime > quantumLength);
                // dequeue this task from running_queue_lowPriority
                // (here)
                // (dequeue operation)
                TAILQ_REMOVE(&running_queue_lowPriority, thisTask, next);
                // update the allTasksHaveCostTime
                allTasksHaveCostTime += quantumLength;
                // update the hasCostTime as quantumLength
                thisTask->task.hasCostTime += quantumLength;
                // update the restRunTime
                thisTask->task.restRunTime -= quantumLength;
                // update microsleep
                thisTask->task.needRunTime = quantumLength;
                // ennqueue this task(still has rest part needs to do) to running_queue_lowPriority
                TAILQ_INSERT_TAIL(&running_queue_lowPriority, thisTask, next);
            }
        }
        pthread_mutex_unlock(&mutex_CPU_prepare);
        microsleep(thisTask->task.needRunTime);
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
