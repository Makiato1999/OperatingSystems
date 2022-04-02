int numOfRunningCPU = 0;
int numOfCPUs = 3;

while (1)
{
    pthread_mutex_lock(&mutex_CPU_prepare);
    if (!readyQ.isEmpty)
    {
        node = dequeue(readyQ);
        enqueue(highQ);
        pthread_mutex_unlock(&mutex_CPU_prepare);
        scheduler(node);
    }
    else
    {
        if (numOfDoneTasks == numOfTasks)
        {
            pthread_mutex_unlock(&mutex_CPU_prepare);
            break;
        }
        if (!highQ.isEmpty)
        {
            node = dequeue(highQ);
            pthread_mutex_unlock(&mutex_CPU_prepare);
            scheduler(node);
        }
        else if (!mediumQ.isEmpty)
        {
            node = dequeue(mediumQ);
            pthread_mutex_unlock(&mutex_CPU_prepare);
            scheduler(node);
        }
        else if (!lowQ.isEmpty)
        {
            node = dequeue(lowQ);
            pthread_mutex_unlock(&mutex_CPU_prepare);
            scheduler(node);
        }
    }
}
void scheduler(node)
{
    // check io
    // check mlfq
}

printf("running_queue_highPriority node: (%s %d %d %d)(hasCostTime: %d, restRunTime: %d, priority: %d)\n",
       first->task.task_name, first->task.task_type, first->task.task_length, first->task.odds_of_IO,
       first->task.hasCostTime, first->task.restRunTime, first->task.priority);