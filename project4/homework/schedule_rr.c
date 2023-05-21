#include "schedulers.h"
#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>

void add(char *name, int priority, int burst, struct node **head)
{
    Task *new_task = (Task *)malloc(sizeof(Task *));
    *new_task = (Task){.name = name, .priority = priority, .burst = burst};
    insert(head, new_task);
}

void schedule(struct node **head)
{
    reverse(head);

    int total_turanround_time = 0, total_waiting_time = 0, total_response_time = 0;
    int clock = 0;
    int num_of_tasks = 0;

    int i = 0;
    while (1)
    {
        ++i;
        struct node *temp_node = *head;
        int flag = 1;
        while (temp_node != NULL)
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

    printf("Average turnaround time: %f\n", (float)total_turanround_time / num_of_tasks);
    printf("Average waiting time: %f\n", (float)total_waiting_time / num_of_tasks);
    printf("Average response time: %f\n", (float)total_response_time / num_of_tasks);
}