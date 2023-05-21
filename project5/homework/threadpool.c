/**
 * Implementation of thread pool.
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include "threadpool.h"

#define QUEUE_SIZE 10
#define NUMBER_OF_THREADS 3

#define TRUE 1

// this represents work that has to be
// completed by a thread in the pool
typedef struct
{
    void (*function)(void *p);
    void *data;
} task;

task task_queue[QUEUE_SIZE + 1]; // the work queue
int queue_head, queue_tail;      // the head and tail of the queue
pthread_mutex_t queue_mutex;     // the mutex lock for queue

pthread_t thread_pool[NUMBER_OF_THREADS]; // the thread pool
int thread_working[NUMBER_OF_THREADS];    // work signal for threads;
sem_t thread_sem;                         // sem for thread

// insert a task into the queue
// returns 0 if successful or 1 otherwise,
int enqueue(task t)
{
    if ((queue_tail + 1) % (QUEUE_SIZE + 1) == queue_head)
    {
        printf("Enqueue Failure.\n");
        return 1;
    }
    else
    {
        pthread_mutex_lock(&queue_mutex);
        task_queue[queue_tail] = t;
        queue_tail = (queue_tail + 1) % (QUEUE_SIZE + 1);
        pthread_mutex_unlock(&queue_mutex);
        return 0;
    }
}

// remove a task from the queue
task dequeue()
{
    task worktodo;
    if (queue_head == queue_tail)
    {
        worktodo.data = NULL;
        worktodo.function = NULL;
        printf("Dequeue Failure.\n");
    }
    else
    {
        pthread_mutex_lock(&queue_mutex);
        worktodo = task_queue[queue_head];
        queue_head = (queue_head + 1) % (QUEUE_SIZE + 1);
        pthread_mutex_unlock(&queue_mutex);
    }
    return worktodo;
}

// the worker thread in the thread pool
void *worker(void *param)
{
    task worktodo;
    // execute the task
    int thread_id = *((int *)param);
    worktodo = dequeue();
    printf("Thread %d is working \n", thread_id);
    execute(worktodo.function, worktodo.data);

    // signal that the thread has completed execution
    thread_working[thread_id] = 0;
    sem_post(&thread_sem);
    pthread_exit(0);
}

/**
 * Executes the task provided to the thread pool
 */
void execute(void (*somefunction)(void *p), void *p)
{
    (*somefunction)(p);
}

/**
 * Submits work to the pool.
 */
int pool_submit(void (*somefunction)(void *p), void *p)
{
    task worktodo;

    // add task to the queue
    worktodo.function = somefunction;
    worktodo.data = p;

    if (enqueue(worktodo))
    {
        printf("Pool Submit Failure.\n");
        return 1;
    }
    sem_wait(&thread_sem);
    for (int id = 0; id < NUMBER_OF_THREADS; id++)
        if (!thread_working[id])
        {
            thread_working[id] = 1;
            pthread_create(&thread_pool[id], NULL, &worker, (void *)&id);
            break;
        }

    return 0;
}

// initialize the thread pool
void pool_init(void)
{
    queue_head = queue_tail = 0;
    pthread_mutex_init(&queue_mutex, NULL);        // initialize mutex
    sem_init(&thread_sem, 0, NUMBER_OF_THREADS);   // initialize sem
    for (int id = 0; id < NUMBER_OF_THREADS; id++) // initialize thread
        thread_working[id] = 0;
}

// shutdown the thread pool
void pool_shutdown(void)
{
    for (int id = 0; id < NUMBER_OF_THREADS; id++)
        pthread_join(thread_pool[id], NULL);
    pthread_mutex_destroy(&queue_mutex);
    sem_destroy(&thread_sem);
}
