/**
 * Driver.c
 *
 * Schedule is in the format
 *
 *  [name] [priority] [CPU burst]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
// #include "list.h"
#include "schedulers.h"

#define SIZE 100

int main(int argc, char *argv[])
{
    FILE *in;
    char *temp;
    char task[SIZE];

    char *name;
    int priority;
    int burst;
    struct node **head = (struct node **)malloc(sizeof(struct node **)); // head of the linked list

    in = fopen(argv[1], "r"); // open the file for reading

    while (fgets(task, SIZE, in) != NULL) // read until end of file
    {
        temp = strdup(task);
        name = strsep(&temp, ",");
        priority = atoi(strsep(&temp, ","));
        burst = atoi(strsep(&temp, ","));

        // add the task to the scheduler's list of tasks
        add(name, priority, burst, head); 

        free(temp);
    }

    fclose(in);

    // invoke the scheduler
    schedule(head);

    return 0;
}
