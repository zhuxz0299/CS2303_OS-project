#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define PAGE_SIZE 4096
#define MEMORY_SIZE PAGE_SIZE * 1000
#define INPUT_SIZE 100

typedef struct proc_space
{
    int pid; // the pid of empty space is -1
    int size;
    int start;
    int end;
    struct proc_space *next;
    struct proc_space *empty_prev; // empty_prev and empty_next are used to link the empty spaces
    struct proc_space *empty_next;
} proc_space;

proc_space *empty_head = NULL;
proc_space *empty_tail = NULL;

void show_state(proc_space *memory_head);
bool first_fit(proc_space **memory_head, proc_space *new_process);
bool best_fit(proc_space **memory_head, proc_space *new_process);
bool worst_fit(proc_space **memory_head, proc_space *new_process);
bool free_space(proc_space **memory_head, int pid);

void move_forward_from_tail(proc_space *empty_cur);
void move_forward(proc_space *empty_cur);
void move_backward_from_head(proc_space *empty_cur);
void move_backward(proc_space *empty_cur);
void insert_new_empty(proc_space *empty_cur);
void remove_empty(proc_space *empty_cur);

int main(int argc, char *argv[])
{
    proc_space *memory_head = (proc_space *)malloc(sizeof(proc_space));
    *memory_head = (proc_space){.pid = -1, .size = MEMORY_SIZE, .start = 0, .end = MEMORY_SIZE, .next = NULL, .empty_next = NULL, .empty_prev = NULL};
    empty_tail = empty_head = memory_head; // initialize the empty list

    char instruction[INPUT_SIZE];
    while (1)
    {
        printf("allocator>");
        fgets(instruction, INPUT_SIZE, stdin);

        if (strcmp(instruction, "exit\n") == 0)
            break;

        if (strcmp(instruction, "STAT\n") == 0)
        {
            show_state(memory_head);
            continue;
        }

        if (instruction[0] == 'R' && instruction[1] == 'Q')
        {
            int pid, size;
            char mode;
            sscanf(instruction, "RQ P%d %d %c", &pid, &size, &mode);

            proc_space *new_process = (proc_space *)malloc(sizeof(proc_space));
            *new_process = (proc_space){.pid = pid, .size = size, .start = 0, .end = MEMORY_SIZE, .next = NULL, .empty_next = NULL, .empty_prev = NULL}; // the .start, .end and .next will be modified later

            bool allo_success = false;
            if (mode == 'F')
                allo_success = first_fit(&memory_head, new_process); // the value of pointer memory_head will be modified in the function
            else if (mode == 'B')
                allo_success = best_fit(&memory_head, new_process);
            else if (mode == 'W')
                allo_success = worst_fit(&memory_head, new_process);
            else
                printf("Invalid mode.\n");
            if (allo_success)
                printf("Process P%d allocated %d bytes.\n", pid, size);
            else
                printf("Process P%d cannot be allocated %d bytes.\n", pid, size);

            // printf("DEBUG:memory_head->start=%d, memory_head->end=%d\n", memory_head->start, memory_head->end);

            continue;
        }

        if (instruction[0] == 'R' && instruction[1] == 'L')
        {
            int pid;
            sscanf(instruction, "RL P%d", &pid);

            bool free_success = free_space(&memory_head, pid);
            if (free_success)
                printf("Process P%d is freed.\n", pid);
            else
                printf("Process P%d is not found.\n", pid);

            continue;
        }
    }
}

void show_state(proc_space *memory_head)
{
    // printf("DEBUG:memory_head->start=%d, memory_head->end=%d\n", memory_head->start, memory_head->end);
    proc_space *cur = memory_head;
    while (cur != NULL)
    {
        if (cur->pid == -1)
            printf("Addresses [%d:%d] Unused\n", cur->start, cur->end - 1);
        else
            printf("Addresses [%d:%d] Process P%d\n", cur->start, cur->end - 1, cur->pid);
        cur = cur->next;
    }
}

bool first_fit(proc_space **memory_head, proc_space *new_process)
{
    if ((*memory_head)->pid == -1 && (*memory_head)->size >= new_process->size) // if the first space is empty and large enough
    {
        // manage the memory list
        new_process->start = (*memory_head)->start;
        new_process->end = (*memory_head)->start + (new_process->size / PAGE_SIZE + 1) * PAGE_SIZE;
        new_process->next = (*memory_head);
        (*memory_head)->start = new_process->end;
        (*memory_head)->size = (*memory_head)->end - (*memory_head)->start;
        (*memory_head) = new_process;

        // manage the empty list
        proc_space *empty_cur = empty_head;
        while (empty_cur->end != (*memory_head)->next->end && empty_cur != NULL) // find the modified empty space
            empty_cur = empty_cur->next;
        if (empty_cur == empty_head)
            ; // do nothing
        else if (empty_cur == empty_tail)
            move_forward_from_tail(empty_cur);
        else
            move_forward(empty_cur);

        return true;
    }
    else // if the first space is not empty or not large enough, find the following spaces
    {
        // manage the memory list
        proc_space *prev = (*memory_head);
        while (prev->next != NULL)
        {
            if (prev->next->pid == -1 && prev->next->size >= new_process->size) // if one of the following space is empty and large enough
            {
                new_process->start = prev->next->start;
                new_process->end = prev->next->start + (new_process->size / PAGE_SIZE + 1) * PAGE_SIZE;
                new_process->next = prev->next;
                prev->next->start = new_process->end;
                prev->next->size = prev->next->end - prev->next->start;
                prev->next = new_process;
                return true;
            }
            prev = prev->next;
        }

        // manage the empty list
        proc_space *empty_cur = empty_head;
        while (empty_cur->end != prev->end && empty_cur != NULL) // find the modified empty space
            empty_cur = empty_cur->next;
        if (empty_cur == empty_head)
            ; // do nothing
        else if (empty_cur == empty_tail)
            move_forward_from_tail(empty_cur);
        else
            move_forward(empty_cur);
    }
    return false;
}

bool best_fit(proc_space **memory_head, proc_space *new_process)
{
    proc_space *empty_cur = empty_head;
    while (empty_cur != NULL)
    {
        if (empty_cur->size >= new_process->size)
        {
            // manage the memory list
            new_process->start = empty_cur->start;
            new_process->end = empty_cur->start + (new_process->size / PAGE_SIZE + 1) * PAGE_SIZE;
            new_process->next = empty_cur;
            empty_cur->start = new_process->end;
            empty_cur->size = empty_cur->end - empty_cur->start;

            if (empty_cur == (*memory_head))
            {
                (*memory_head) = new_process;
            }
            else
            {
                proc_space *prev = (*memory_head);
                while (prev->next != empty_cur)
                    prev = prev->next;
                prev->next = new_process;
            }

            // manage the empty list
            if (empty_cur == empty_head)
                ; // do nothing
            else if (empty_cur == empty_tail)
                move_forward_from_tail(empty_cur);
            else
                move_forward(empty_cur);

            return true;
        }
        empty_cur = empty_cur->next;
    }
    return false;
}

bool worst_fit(proc_space **memory_head, proc_space *new_process)
{
    if (new_process->size <= empty_tail->size)
    {
        // manage the memory list
        new_process->start = empty_tail->start;
        new_process->end = empty_tail->start + (new_process->size / PAGE_SIZE + 1) * PAGE_SIZE;
        new_process->next = empty_tail;
        empty_tail->start = new_process->end;
        empty_tail->size = empty_tail->end - empty_tail->start;

        if (empty_tail == (*memory_head))
            (*memory_head) = new_process;
        else
        {
            proc_space *prev = (*memory_head);
            while (prev->next != empty_tail)
                prev = prev->next;
            prev->next = new_process;
        }

        // manage the empty list
        if (empty_tail == empty_head)
            ; // do nothing
        else
            move_forward_from_tail(empty_tail);

        return true;
    }
    return false;
}

bool free_space(proc_space **memory_head, int pid)
{
    if ((*memory_head)->pid == pid) // if the first space is the space to be freed
    {
        if ((*memory_head)->next->pid == -1) // if the next space is empty
        {
            // manage the memory list, two empty spaces are merged into one
            proc_space *temp = (*memory_head);
            (*memory_head) = (*memory_head)->next;
            (*memory_head)->start = temp->start;
            (*memory_head)->size = (*memory_head)->end - (*memory_head)->start;
            free(temp);

            // manage the empty list
            proc_space *empty_cur = empty_head;
            while (empty_cur->end != (*memory_head)->end && empty_cur != NULL) // find the modified empty space
                empty_cur = empty_cur->next;
            if (empty_cur == empty_tail)
                ; // do nothing
            else if (empty_cur == empty_head)
                move_backward_from_head(empty_cur);
            else
                move_backward(empty_cur);
        }
        else // if the next space is not empty
        {
            // manage the memory list
            (*memory_head)->pid = -1;

            // manage the empty list
            insert_new_empty(*memory_head);
        }
    }
    else // if the first space is not the space to be freed
    {
        proc_space *prev = (*memory_head);
        while (prev->next != NULL && prev->next->pid != pid)
            prev = prev->next;
        if (prev->next == NULL) // if the space to be freed is not found
            return false;
        else if (prev->pid >= 0 && (prev->next->next->pid >= 0 || prev->next->next == NULL)) // no empty space nearby
        {
            proc_space *temp = prev->next;
            temp->pid = -1;
            insert_new_empty(temp);
        }
        else if (prev->pid == -1 && (prev->next->next->pid >= 0 || prev->next->next == NULL)) // prev space is empty
        {
            // manage the memory list
            proc_space *temp = prev->next;
            prev->end = temp->end;
            prev->size = prev->end - prev->start;
            prev->next = temp->next;
            free(temp);

            // manage the empty list
            proc_space *empty_cur = empty_head;
            while (empty_cur->start != prev->start && empty_cur != NULL) // find the modified empty space
                empty_cur = empty_cur->next;
            if (empty_cur == empty_tail)
                ; // do nothing
            else if (empty_cur == empty_head)
                move_backward_from_head(empty_cur);
            else
                move_backward(empty_cur);
        }
        else if (prev->pid >= 0 && prev->next->next->pid == -1) // next space is empty
        {
            // manage the memory list
            proc_space *temp = prev->next;
            proc_space *next = prev->next->next;
            next->start = temp->start;
            next->size = next->end - next->start;
            prev->next = next;
            free(temp);

            // manage the empty list
            proc_space *empty_cur = empty_head;
            while (empty_cur->end != next->end && empty_cur != NULL) // find the modified empty space
                empty_cur = empty_cur->next;
            if (empty_cur == empty_tail)
                ; // do nothing
            else if (empty_cur == empty_head)
                move_backward_from_head(empty_cur);
            else
                move_backward(empty_cur);
        }
        else if (prev->pid == -1 && prev->next->next->pid == -1) // all the spaces nearby are empty
        {
            // manage the memory list
            proc_space *temp = prev->next;
            proc_space *next = prev->next->next;
            prev->end = next->end;
            prev->size = prev->end - prev->start;
            prev->next = next->next;
            free(temp);
            // next will be freed in remove_empty()

            // manage the empty list
            proc_space *empty_cur = empty_head;
            while (empty_cur->start != prev->start && empty_cur != NULL) // find the modified empty space
                empty_cur = empty_cur->next;
            if (empty_cur == empty_tail)
                ; // do nothing
            else if (empty_cur == empty_head)
                move_backward_from_head(empty_cur);
            else
                move_backward(empty_cur);

            while (empty_cur->end != next->end && empty_cur != NULL) // find the empty space to be freed
                empty_cur = empty_cur->next;
            remove_empty(empty_cur);
        }
    }
}

void move_forward_from_tail(proc_space *empty_cur)
{
    if (empty_cur->size < empty_cur->empty_prev->size)
    {
        // take the empty space out of the empty list
        empty_tail = empty_cur->empty_prev;
        empty_tail->empty_next = NULL;

        // insert the empty space into the empty list
        proc_space *temp = empty_tail;
        while (temp != NULL && temp->size > empty_cur->size)
            temp = temp->empty_prev;
        if (temp == NULL) // if the empty space is the smallest, insert it at the head
        {
            empty_cur->empty_next = empty_head;
            empty_cur->empty_prev = NULL;
            empty_head->empty_prev = empty_cur;
            empty_head = empty_cur;
        }
        else
        {
            empty_cur->empty_next = temp->empty_next;
            empty_cur->empty_prev = temp;
            temp->empty_next->empty_prev = empty_cur;
            temp->empty_next = empty_cur;
        }
    }
}

void move_backward_from_head(proc_space *empty_cur)
{
    if (empty_cur->size > empty_cur->empty_next->size)
    {
        // take the empty space out of the empty list
        empty_head = empty_cur->empty_next;
        empty_head->empty_prev = NULL;

        // insert the empty space into the empty list
        proc_space *temp = empty_head;
        while (temp != NULL && temp->size < empty_cur->size)
            temp = temp->empty_next;
        if (temp == NULL) // if the empty space is the largest, insert it at the tail
        {
            empty_cur->empty_prev = empty_tail;
            empty_cur->empty_next = NULL;
            empty_tail->empty_next = empty_cur;
            empty_tail = empty_cur;
        }
        else
        {
            empty_cur->empty_prev = temp->empty_prev;
            empty_cur->empty_next = temp;
            temp->empty_prev->empty_next = empty_cur;
            temp->empty_prev = empty_cur;
        }
    }
}

void move_forward(proc_space *empty_cur)
{
    if (empty_cur->size < empty_cur->empty_prev->size)
    {
        empty_cur->empty_prev->empty_next = empty_cur->empty_next;
        empty_cur->empty_next->empty_prev = empty_cur->empty_prev;

        proc_space *temp = empty_cur->empty_prev;
        while (temp != NULL && temp->size > empty_cur->size)
            temp = temp->empty_prev;
        if (temp == NULL)
        {
            empty_cur->empty_next = empty_head;
            empty_cur->empty_prev = NULL;
            empty_head->empty_prev = empty_cur;
            empty_head = empty_cur;
        }
        else
        {
            empty_cur->empty_next = temp->empty_next;
            empty_cur->empty_prev = temp;
            temp->empty_next->empty_prev = empty_cur;
            temp->empty_next = empty_cur;
        }
    }
}

void move_backward(proc_space *empty_cur)
{
    if (empty_cur->size > empty_cur->empty_next->size)
    {
        empty_cur->empty_prev->empty_next = empty_cur->empty_next;
        empty_cur->empty_next->empty_prev = empty_cur->empty_prev;

        proc_space *temp = empty_cur->empty_next;
        while (temp != NULL && temp->size < empty_cur->size)
            temp = temp->empty_next;
        if (temp == NULL)
        {
            empty_cur->empty_prev = empty_tail;
            empty_cur->empty_next = NULL;
            empty_tail->empty_next = empty_cur;
            empty_tail = empty_cur;
        }
        else
        {
            empty_cur->empty_prev = temp->empty_prev;
            empty_cur->empty_next = temp;
            temp->empty_prev->empty_next = empty_cur;
            temp->empty_prev = empty_cur;
        }
    }
}

void insert_new_empty(proc_space *empty_cur)
{
    proc_space *empty_tmp = empty_head;
    while (empty_tmp->size < empty_cur->size)
        empty_tmp = empty_tmp->next;
    if (empty_tmp == empty_head)
    {
        empty_head->empty_prev = empty_cur;
        empty_cur->empty_next = empty_head;
        empty_cur->empty_prev = NULL;
        empty_head = empty_cur;
    }
    else
    {
        empty_cur->empty_next = empty_tmp;
        empty_cur->empty_prev = empty_tmp->empty_prev;
        empty_tmp->empty_prev->empty_next = empty_cur;
        empty_tmp->empty_prev = empty_cur;
    }
}

void remove_empty(proc_space *empty_cur)
{
    if (empty_cur == empty_head)
    {
        empty_head = empty_cur->empty_next;
        empty_head->empty_prev = NULL;
    }
    else if (empty_cur == empty_tail)
    {
        empty_tail = empty_cur->empty_prev;
        empty_tail->empty_next = NULL;
    }
    else
    {
        empty_cur->empty_prev->empty_next = empty_cur->empty_next;
        empty_cur->empty_next->empty_prev = empty_cur->empty_prev;
    }

    free(empty_cur);
}