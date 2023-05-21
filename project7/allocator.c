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

void allocated_empty(proc_space **memory_head, proc_space *modified_empty);         // 在一次allocate之后，需要调整empty list
void remove_empty(proc_space *modified_empty);                                      // 将修改过的empty space从empty list中移除
void insert_empty(proc_space *modified_empty);                                      // 将修改过的empty space重新插入到empty list中
void allocate_head(proc_space **memory_head, proc_space *new_process);              // 如果分配的空间是 memory_head，需要特殊处理
void allocate(proc_space **memory_head, proc_space *new_process, proc_space *prev); // 如果分配的空间不是 memory_head
void merge_empty_right(proc_space *prev, proc_space *next);                         // 将两个相邻的空间合并，其中右侧原来就是空闲的
void merge_empty_left(proc_space *prev, proc_space *next);                          // 将两个相邻的空间合并，其中左侧原来就是空闲的

int main(int argc, char *argv[])
{
    proc_space *memory_head = (proc_space *)malloc(sizeof(proc_space));
    *memory_head = (proc_space){.pid = -1, .size = MEMORY_SIZE, .start = 0, .end = MEMORY_SIZE, .next = NULL, .empty_next = NULL, .empty_prev = NULL};
    empty_tail = empty_head = memory_head; // initialize the empty list

    char instruction[INPUT_SIZE];
    FILE *in = fopen("input.txt", "r");

    while (true)
    {
        // printf("\033[32mAllocator>\033[0m");
        // fgets(instruction, INPUT_SIZE, stdin);
        fgets(instruction, INPUT_SIZE, in);
        printf("\033[32mAllocator>\033[0m %s", instruction);

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
    fclose(in);
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
        // printf("DEBUG:size=%d\n", cur->size);
        cur = cur->next;
    }
    // printf("-------------------------------------\n");
    // proc_space *cur_empty = empty_head;
    // while (cur_empty != NULL)
    // {
    //     printf("Addresses [%d:%d] Unused\n", cur_empty->start, cur_empty->end - 1);
    //     // printf("size=%d\n", cur_empty->size);
    //     if (cur_empty == cur_empty->empty_next)
    //     {
    //         printf("ERROR:cur_empty==cur_empty->empty_next\n");
    //         break;
    //     }

    //     cur_empty = cur_empty->empty_next;
    // }
}

bool first_fit(proc_space **memory_head, proc_space *new_process)
{
    if ((*memory_head)->pid == -1 && (*memory_head)->size >= new_process->size) // 第一个空间将直接被分配
    {
        allocate_head(memory_head, new_process);
        allocated_empty(memory_head, (*memory_head)->next);
    }
    else // 被分配的位置在后面
    {
        proc_space *prev = *memory_head;
        while (prev->next != NULL && (prev->next->pid != -1 || prev->next->size < new_process->size))
            prev = prev->next;
        if (prev->next == NULL) // 没有找到合适的位置
            return false;
        else // 找到了可以分配的空间
        {
            allocate(memory_head, new_process, prev);
            // printf("DEBUG:allocate() was excuted.\n");
            // printf("DEBUG:prev->next->pid=%d, prev->next->next->pid=%d\n", prev->next->pid, prev->next->next->pid);
            // printf("DEBUG:enter the allocated_empty().\n");
            allocated_empty(memory_head, prev->next->next);
        }
    }
    return true;
}

bool best_fit(proc_space **memory_head, proc_space *new_process)
{
    proc_space *cur_empty = empty_head;
    while (cur_empty != NULL && cur_empty->size < new_process->size)
        cur_empty = cur_empty->empty_next;
    if (cur_empty == NULL) // 没有找到合适的位置
        return false;
    else // 找到了可以分配的空间
    {
        if (cur_empty == *memory_head) // 第一个空间将直接被分配
        {
            allocate_head(memory_head, new_process);
            allocated_empty(memory_head, (*memory_head)->next);
        }
        else // 被分配的位置在后面
        {
            proc_space *prev = *memory_head;
            while (prev->next != NULL && prev->next != cur_empty)
                prev = prev->next;
            if (prev->next == NULL) // 程序出现问题
                return false;
            else // 找到了可以分配的空间
            {
                allocate(memory_head, new_process, prev);
                allocated_empty(memory_head, prev->next->next);
            }
        }
    }
    return true;
}

bool worst_fit(proc_space **memory_head, proc_space *new_process)
{
    if (empty_tail->size < new_process->size)
        return false;
    else
    {
        if (empty_tail == *memory_head)
        {
            allocate_head(memory_head, new_process);
            allocated_empty(memory_head, (*memory_head)->next);
        }
        else
        {
            proc_space *prev = *memory_head;
            while (prev->next != NULL && prev->next != empty_tail)
                prev = prev->next;
            if (prev->next == NULL)
                return false;
            else
            {
                allocate(memory_head, new_process, prev);
                allocated_empty(memory_head, prev->next->next);
            }
        }
    }
    return true;
}

bool free_space(proc_space **memory_head, int pid)
{
    if ((*memory_head)->pid == pid) // 第一个空间将直接被释放
    {
        (*memory_head)->pid = -1;
        if ((*memory_head)->next != NULL && (*memory_head)->next->pid == -1) // 后面的空间也是空闲的，需要合并
        {
            merge_empty_right((*memory_head), (*memory_head)->next);
        }
        else // 不需要要做空间合并
        {
            (*memory_head)->size = (*memory_head)->end - (*memory_head)->start;
            insert_empty((*memory_head));
        }
    }
    else // 被释放的位置在后面
    {
        proc_space *prev = *memory_head;
        while (prev->next != NULL && prev->next->pid != pid)
            prev = prev->next;
        if (prev->next == NULL) // 没有找到合适的位置
            return false;
        else if (prev->next->next == NULL) // 被释放的位置在最后
        {
            prev->next->pid = -1;
            if (prev->pid == -1) // 前面的空间也是空闲的，需要合并
            {
                merge_empty_right(prev, prev->next);
            }
            else // 不需要要做空间合并
            {
                prev->next->size = prev->next->end - prev->next->start;
                insert_empty(prev->next);
            }
        }
        else // 被释放的位置在中间
        {
            prev->next->pid = -1;
            if (prev->pid == -1 && prev->next->next->pid == -1) // 前面和后面的空间也是空闲的，需要合并
            {
                // printf("DEBUG:prev->next->pid=%d, prev->next->next->pid=%d\n", prev->next->pid, prev->next->next->pid);
                merge_empty_left(prev, prev->next);
                // printf("DEBUG:prev->pid=%d, prev->next->pid=%d\n", prev->pid, prev->next->pid);
                // printf("DEBUG:#############\n");
                // show_state(*memory_head);
                // printf("DEBUG:#############\n");
                merge_empty_right(prev, prev->next);
            }
            else if (prev->pid == -1) // 前面的空间是空闲的，需要合并
            {
                merge_empty_left(prev, prev->next);
            }
            else if (prev->next->next->pid == -1) // 后面的空间是空闲的，需要合并
            {
                merge_empty_right(prev->next, prev->next->next);
            }
            else // 不需要要做空间合并
            {
                prev->next->size = prev->next->end - prev->next->start;
                insert_empty(prev->next);
            }
        }
    }
    return true;
}

void allocate_head(proc_space **memory_head, proc_space *new_process)
{
    new_process->start = (*memory_head)->start;
    new_process->end = new_process->start + (new_process->size / PAGE_SIZE + 1) * PAGE_SIZE;
    new_process->next = (*memory_head);
    (*memory_head)->start = new_process->end;
    (*memory_head)->size = (*memory_head)->end - (*memory_head)->start;
    (*memory_head) = new_process;
}

void allocate(proc_space **memory_head, proc_space *new_process, proc_space *prev)
{
    new_process->start = prev->next->start;
    new_process->end = new_process->start + (new_process->size / PAGE_SIZE + 1) * PAGE_SIZE;
    new_process->next = prev->next;
    prev->next->start = new_process->end;
    prev->next->size = prev->next->end - prev->next->start;
    prev->next = new_process;
}

void allocated_empty(proc_space **memory_head, proc_space *modified_empty)
{
    if (modified_empty->size == 0)
    {
        remove_empty(modified_empty);
        free(modified_empty);
    }
    else
    {
        // printf("DEBUG:modified_empty->size=%d\n", modified_empty->size);
        remove_empty(modified_empty);
        insert_empty(modified_empty);
    }
}

void remove_empty(proc_space *modified_empty)
{
    if (modified_empty == empty_head && modified_empty == empty_tail) // empty list中只有一个元素
    {
        empty_head = empty_tail = NULL;
    }
    else if (modified_empty == empty_head) // modified_empty 是第一个元素
    {
        empty_head = empty_head->empty_next;
        empty_head->empty_prev = NULL;
    }
    else if (modified_empty == empty_tail) // modified_empty 是最后一个元素
    {
        empty_tail = empty_tail->empty_prev;
        empty_tail->empty_next = NULL;
    }
    else // modified_empty 是中间某个元素
    {
        modified_empty->empty_prev->empty_next = modified_empty->empty_next;
        modified_empty->empty_next->empty_prev = modified_empty->empty_prev;
    }
}

void insert_empty(proc_space *modified_empty)
{
    if (empty_head == NULL) // empty list为空
    {
        empty_head = empty_tail = modified_empty;
        modified_empty->empty_prev = modified_empty->empty_next = NULL;
    }
    else if (modified_empty->size < empty_head->size) // modified_empty 是第一个元素
    {
        modified_empty->empty_prev = NULL;
        modified_empty->empty_next = empty_head;
        modified_empty->empty_next->empty_prev = modified_empty;
        empty_head = modified_empty;
    }
    else if (modified_empty->size >= empty_tail->size) // modified_empty 是最后一个元素
    {
        modified_empty->empty_prev = empty_tail;
        modified_empty->empty_next = NULL;
        modified_empty->empty_prev->empty_next = modified_empty;
        empty_tail = modified_empty;
    }
    else // modified_empty 是中间某个元素
    {
        proc_space *cur = empty_head;
        while (cur != NULL && cur->size > modified_empty->size)
            cur = cur->empty_next;
        modified_empty->empty_prev = cur;
        modified_empty->empty_next = cur->empty_next;
        modified_empty->empty_prev->empty_next = modified_empty;
        modified_empty->empty_next->empty_prev = modified_empty;
    }
}

void merge_empty_right(proc_space *prev, proc_space *next)
{
    prev->end = next->end;
    prev->size = prev->end - prev->start;
    prev->next = next->next;
    for (proc_space *cur_empty = empty_head; cur_empty != NULL; cur_empty = cur_empty->empty_next)
    {
        if (cur_empty == next)
            remove_empty(next);
        if (cur_empty == prev)
            remove_empty(prev);
    }
    free(next);
    insert_empty(prev);
}

void merge_empty_left(proc_space *prev, proc_space *next)
{
    prev->end = next->end;
    prev->size = prev->end - prev->start;
    prev->next = next->next;
    for (proc_space *cur_empty = empty_head; cur_empty != NULL; cur_empty = cur_empty->empty_next)
    {
        if (cur_empty == next)
            remove_empty(next);
        if (cur_empty == prev)
            remove_empty(prev);
    }
    free(next);
    insert_empty(prev);
}
