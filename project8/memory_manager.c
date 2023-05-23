#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define PAGE_NUM 256
#define PAGE_SIZE 256
#define FRAME_NUM 256
#define FRAME_SIZE 256
#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256

typedef struct page_table_entry
{
    int valid;
    int frame_number;
} page_table_entry;

typedef struct TLB_entry
{
    int access_time;
    int frame_number;
    int page_number;
} TLB_entry;

typedef struct frame
{
    int access_time;
    char data[FRAME_SIZE];
} frame;

void initialize(page_table_entry page_table[], TLB_entry TLB[], frame physical_memory[]);
void parse_logical_addr(int logical_address, int *page_number, int *offset);
bool TLB_access(TLB_entry TLB[], int page_number, int *frame_number, int *TLB_hit_count, int clock);
bool page_table_access(page_table_entry page_table[], TLB_entry TLB[], int page_number, int *frame_number, int *page_fault_count, int clock);
void page_fault_handler(page_table_entry page_table[], frame physical_memory[], TLB_entry TLB[], int page_number, int *frame_number, int clock, FILE *backing_store_file);
void TLB_replacement(TLB_entry TLB[], int page_number, int frame_number, int clock);

int main(int argc, char *argv[])
{
    FILE *addr_file = fopen(argv[1], "r");
    FILE *backing_store_file = fopen("BACKING_STORE.bin", "r");
    FILE *output_file = fopen("output.txt", "w");

    page_table_entry page_table[PAGE_TABLE_SIZE];
    TLB_entry TLB[TLB_SIZE];
    frame physical_memory[FRAME_NUM];
    initialize(page_table, TLB, physical_memory);

    int page_number, offset, logical_address, frame_number;
    int clock = 0, page_fault_count = 0, TLB_hit_count = 0;

    // printf("DEBUG: %s\n", argv[1]);
    while (fscanf(addr_file, "%d", &logical_address) == 1)
    {
        clock++;
        // parse logical address
        parse_logical_addr(logical_address, &page_number, &offset);
        // printf("DEBUG: %d %d\n", page_number, offset);
        // get frame number
        bool TLB_hit = TLB_access(TLB, page_number, &frame_number, &TLB_hit_count, clock);
        if (!TLB_hit)
        {
            bool page_table_hit = page_table_access(page_table, TLB, page_number, &frame_number, &page_fault_count, clock);
            // printf("DEBUG: %d\n", page_table_hit);
            if (!page_table_hit)
                page_fault_handler(page_table, physical_memory, TLB, page_number, &frame_number, clock, backing_store_file);
            // printf("DEBUG: %d\n", frame_number);
        }
        // get physical address and value
        int physical_address = frame_number * FRAME_SIZE + offset;
        int value = physical_memory[frame_number].data[offset];

        // output
        fprintf(output_file, "Virtual address: %d Physical address: %d Value: %d\n", logical_address, physical_address, value);
    }
    fclose(addr_file);
    fclose(backing_store_file);
    fclose(output_file);

    printf("Page fault rate: %.3f\n", (float)page_fault_count / (float)clock);
    printf("TLB hit rate: %.3f\n", (float)TLB_hit_count / (float)clock);
    return 0;
}

void initialize(page_table_entry page_table[], TLB_entry TLB[], frame physical_memory[])
{
    for (int i = 0; i < PAGE_TABLE_SIZE; i++)
    {
        page_table[i].valid = 0;
        page_table[i].frame_number = -1;
    }

    for (int i = 0; i < TLB_SIZE; i++)
    {
        TLB[i].access_time = -1;
        TLB[i].frame_number = -1;
        TLB[i].page_number = -1;
    }

    for (int i = 0; i < FRAME_NUM; i++)
    {
        physical_memory[i].access_time = -1;
    }
}

void parse_logical_addr(int logical_address, int *page_number, int *offset)
{
    *page_number = (logical_address & 0xFF00) >> 8;
    *offset = logical_address & 0x00FF;
}

bool TLB_access(TLB_entry TLB[], int page_number, int *frame_number, int *TLB_hit_count, int clock)
{
    for (int i = 0; i < TLB_SIZE; i++)
    {
        if (TLB[i].page_number == page_number)
        {
            *frame_number = TLB[i].frame_number;
            TLB[i].access_time = clock;
            (*TLB_hit_count)++;
            // printf("DEBUG: TLB_hit_count=%d\n", (*TLB_hit_count));
            return true;
        }
        // printf("DEBUG: TLB[%d].page_number=%d,\tpage_number=%d\n", i, TLB[i].page_number, page_number);
    }
    return false;
}

bool page_table_access(page_table_entry page_table[], TLB_entry TLB[], int page_number, int *frame_number, int *page_fault_count, int clock)
{
    if (page_table[page_number].valid == 1)
    {
        *frame_number = page_table[page_number].frame_number;
        TLB_replacement(TLB, page_number, *frame_number, clock);
        return true;
    }
    (*page_fault_count)++;
    return false;
}

void page_fault_handler(page_table_entry page_table[], frame physical_memory[], TLB_entry TLB[], int page_number, int *frame_number, int clock, FILE *backing_store_file)
{
    int min_time = 0x7FFFFFFF;
    int min_time_index = 0;
    // printf("DEBUG: page_number=%d\n", page_number);
    // LRU algorithm
    for (int i = 0; i < FRAME_NUM; i++)
        if (physical_memory[i].access_time < min_time)
        {
            min_time = physical_memory[i].access_time;
            min_time_index = i;
        }
    physical_memory[min_time_index].access_time = clock;
    *frame_number = min_time_index;
    // printf("DEBUG: frame_number=%d\n", *frame_number);
    // kick out the old page
    for (int i = 0; i < PAGE_TABLE_SIZE; i++)
        if (page_table[i].frame_number == min_time_index)
            page_table[i].valid = 0;
    // printf("DEBUG: kick out the old page\n");
    // read data from backing store
    fseek(backing_store_file, page_number * PAGE_SIZE, SEEK_SET);
    // printf("DEBUG: fseek\n");
    fread(physical_memory[*frame_number].data, sizeof(char), FRAME_SIZE, backing_store_file);
    // printf("DEBUG: physical_memory[%d].data=%s\n", *frame_number, physical_memory[*frame_number].data);
    // update page table
    page_table[page_number].frame_number = *frame_number;
    page_table[page_number].valid = 1;

    TLB_replacement(TLB, page_number, *frame_number, clock);
}

void TLB_replacement(TLB_entry TLB[], int page_number, int frame_number, int clock)
{
    int min_time = 0x7FFFFFFF;
    int min_time_index = 0;
    for (int i = 0; i < TLB_SIZE; i++)
    {
        if (TLB[i].access_time < min_time)
        {
            min_time = TLB[i].access_time;
            min_time_index = i;
        }
    }
    TLB[min_time_index].access_time = clock;
    TLB[min_time_index].frame_number = frame_number;
    TLB[min_time_index].page_number = page_number;
}
