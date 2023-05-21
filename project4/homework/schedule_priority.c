#include "schedulers.h"
#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>

void add(char *name, int priority, int burst, struct node **head)
{
    Task *new_task = (Task *)malloc(sizeof(Task *));
    *new_task = (Task){.name = name, .priority = priority, .burst = burst};

    if (*head == NULL)
    {
        insert(head, new_task);
    }
    else
    {
        if ((*head)->task->priority < new_task->priority)
        {
            insert(head, new_task);
            return;
        }
        struct node *temp_node = *head;
        while (temp_node->next != NULL && temp_node->next->task->priority >= new_task->priority)
        {
            temp_node = temp_node->next;
        }
        struct node *new_node = malloc(sizeof(struct node));
        new_node->task = new_task;
        new_node->next = temp_node->next;
        temp_node->next = new_node;
    }
}

void schedule(struct node **head)
{
    int total_turanround_time = 0, total_waiting_time = 0, total_response_time = 0;
    int clock = 0;
    int num_of_tasks = 0;

    struct node *temp_node = *head;
    while (temp_node != NULL)
    {
        ++num_of_tasks;
        total_response_time += clock;
        run(temp_node->task, temp_node->task->burst);
        clock += temp_node->task->burst;
        total_turanround_time += clock;
        total_waiting_time += clock - temp_node->task->burst;

        temp_node = temp_node->next;
    }

    printf("Average turnaround time: %f\n", (float)total_turanround_time / num_of_tasks);
    printf("Average waiting time: %f\n", (float)total_waiting_time / num_of_tasks);
    printf("Average response time: %f\n", (float)total_response_time / num_of_tasks);
}