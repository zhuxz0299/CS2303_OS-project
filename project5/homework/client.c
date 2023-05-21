/**
 * Example client program that uses thread pool.
 */

#include <stdio.h>
#include <unistd.h>
#include "threadpool.h"

struct data
{
    int a;
    int b;
};

void add(void *param)
{
    struct data *temp;
    temp = (struct data *)param;

    printf("I add two values %d and %d result = %d\n", temp->a, temp->b, temp->a + temp->b);
}

int main(void)
{
    // create some work to do
    struct data work = {5, 10};
    struct data work2 = {10, 20};
    struct data work3 = {20, 30};
    struct data work4 = (struct data){.a = 30, .b = 40};

    // initialize the thread pool
    pool_init();

    // submit the work to the queue
    pool_submit(&add, &work);
    pool_submit(&add, &work2);
    pool_submit(&add, &work3);
    pool_submit(&add, &work4);

    // may be helpful
    // sleep(3);

    pool_shutdown();

    return 0;
}
