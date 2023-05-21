#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>

typedef int buffer_item;
#define BUFFER_SIZE 5
#define SLEEP_RANGE 5
#define RANDNUM_MAX 100

/* the buffer is a circular queue*/
buffer_item buffer[BUFFER_SIZE + 1];
int head = 0, tail = 0;

/* mutex and semaphore */
pthread_mutex_t buffer_mutex;
sem_t full;
sem_t empty;

/* insert item into buffer
return 0 if successful, otherwise return 1  */
int insert_item(buffer_item item)
{
    if ((tail + 1) % (BUFFER_SIZE + 1) == head)
        return 1;
    buffer[tail] = item;
    tail = (tail + 1) % (BUFFER_SIZE + 1);
    return 0;
}

/* remove an object from buffer
return 0 if successful, otherwise return 1 */
int remove_item(buffer_item *item)
{
    if (head == tail)
        return 1;
    *item = buffer[head];
    head = (head + 1) % (BUFFER_SIZE + 1);
    return 0;
}

void *producer(void *param)
{
    buffer_item item;
    while (1)
    {
        int sleep_time= rand() % SLEEP_RANGE;
        sleep(sleep_time);

        sem_wait(&empty);
        pthread_mutex_lock(&buffer_mutex);
        item = rand() % RANDNUM_MAX;
        if (insert_item(item))
            printf("Producer Insert Failure.\n");
        else
            printf("Producer produced %d.\n", item);
        pthread_mutex_unlock(&buffer_mutex);
        sem_post(&full);
    }
}

void *consumer(void *param)
{
    buffer_item item;
    while (1)
    {
        int sleep_time = rand() % SLEEP_RANGE;
        sleep(sleep_time);
        sem_wait(&full);
        pthread_mutex_lock(&buffer_mutex);
        if (remove_item(&item))
            printf("Consumer Remove Failure.\n");
        else
            printf("Consumer consumed %d.\n", item);
        pthread_mutex_unlock(&buffer_mutex);
        sem_post(&empty);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("You should input 3 arguments: sleep_time, num_producer, num_consumer\n");
        return 1;
    }

    // Converte the arguments to int
    int sleep_main = atoi(argv[1]);
    int num_producer = atoi(argv[2]);
    int num_consumer = atoi(argv[3]);

    // Initialization
    pthread_mutex_init(&buffer_mutex, NULL);
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);
    head = tail = 0;

    // Create producer threads and consumer threads
    pthread_t producer_thread[num_producer];
    pthread_t consumer_thread[num_consumer];
    for (int i = 0; i < num_producer; i++)
        pthread_create(&producer_thread[i], NULL, producer, NULL);
    for (int i = 0; i < num_consumer; i++)
        pthread_create(&consumer_thread[i], NULL, consumer, NULL);

    // Sleep for a while
    printf("Sleep for %d seconds.\n", sleep_main);
    sleep(sleep_main);
    printf("Wake up and terminate\n");

    // clear
    for (int i = 0; i < num_producer; i++)
        pthread_cancel(producer_thread[i]);
    for (int i = 0; i < num_consumer; i++)
        pthread_cancel(consumer_thread[i]);
    sem_destroy(&full);
    sem_destroy(&empty);
    pthread_mutex_destroy(&buffer_mutex);
    printf("Done!\n");
    return 0;
}