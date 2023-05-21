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

    struct node *temp_head = *head;
    while (temp_head)
    {
        int current_priority = (temp_head)->task->priority;
        int i = 0;
        while (1)
        {
            ++i;
            struct node *temp_node = temp_head;
            int flag = 1;
            while (temp_node && temp_node->task->priority == current_priority)
            {
                if (i == 1)
                {
                    total_response_time += clock;
                    ++num_of_tasks;
                }

                if (temp_node->task->burst > i * QUANTUM)
                {
                    run(temp_node->task, QUANTUM);
                    clock += QUANTUM;
                    flag = 0;
                }
                else if (temp_node->task->burst > (i - 1) * QUANTUM)
                {
                    run(temp_node->task, temp_node->task->burst - (i - 1) * QUANTUM);
                    clock += temp_node->task->burst - (i - 1) * QUANTUM;
                    total_turanround_time += clock;
                    total_waiting_time += clock - temp_node->task->burst;
                }
                temp_node = temp_node->next;
            }
            if (flag)
                break;
        }
        while (temp_head && temp_head->task->priority == current_priority)
            temp_head = temp_head->next;
    }

    printf("Average turnaround time: %f\n", (float)total_turanround_time / num_of_tasks);
    printf("Average waiting time: %f\n", (float)total_waiting_time / num_of_tasks);
    printf("Average response time: %f\n", (float)total_response_time / num_of_tasks);
}