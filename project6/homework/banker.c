#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 4
#define INPUT_SIZE 100
/* the available amount of each resource */
int available[NUMBER_OF_RESOURCES];
/*the maximum demand of each customer */
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
/* the amount currently allocated to each customer */
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
/* the remaining need of each customer */
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
/* user request resources */
int request_resources(int customer_id, int request[]);
/* user release resources */
int release_resources(int customer_id, int release[]);
/* show the value of all matrix */
void show();
/* safety_algo to check whether the state is safe */
int safety_algorithm();

int main(int argc, char *argv[])
{
    FILE *in = fopen("maximum.txt", "r");
    if (in == NULL)
    {
        printf("File Not Found.\n");
        return 0;
    }

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; ++i)
        for (int j = 0; j < NUMBER_OF_RESOURCES; ++j)
        {
            fscanf(in, "%d", &maximum[i][j]); // initialize array MAXIMUM
            // printf("DEBUG: %d\n", maximum[i][j]);
            if (j != NUMBER_OF_RESOURCES - 1)
                fgetc(in);              // read and ignore comma
            need[i][j] = maximum[i][j]; // initialize array NEED
            allocation[i][j] = 0;       // initialize array ALLOCATION
        }
    fclose(in);

    for (int i = 0; i < NUMBER_OF_RESOURCES; ++i) // initialize array AVAILABLE
        available[i] = atoi(argv[i + 1]);

    char input[INPUT_SIZE];
    while (1)
    {
        printf("banker>");
        fgets(input, INPUT_SIZE, stdin);

        if (strcmp(input, "exit\n") == 0)
            break;

        if (input[0] == '*')
        {
            show();
            continue;
        }

        if (input[0] == 'R')
        {
            if (input[1] == 'Q')
            {
                int customer_id;
                int request[NUMBER_OF_RESOURCES];
                sscanf(input, "RQ %d %d %d %d %d", &customer_id, &request[0], &request[1], &request[2], &request[3]);
                request_resources(customer_id, request);
                continue;
            }
            else if (input[1] == 'L')
            {
                int customer_id;
                int release[NUMBER_OF_RESOURCES];
                sscanf(input, "RL %d %d %d %d %d", &customer_id, &release[0], &release[1], &release[2], &release[3]);
                release_resources(customer_id, release);
                continue;
            }
        }
    }
}

int request_resources(int customer_id, int request[])
{
    // printf("DEBUG: customer_id=%d\n", customer_id);
    // for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
    //     printf("DEBUG: request[%d]=%d\n", i, request[i]);
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) // check
    {
        if (request[i] > need[customer_id][i])
        {
            printf("Request is larger than need!\n");
            return -1;
        }
        if (request[i] > available[i])
        {
            printf("No enough resources to allocate!\n");
            return -1;
        }
    }

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) // try to allocate
    {
        available[i] -= request[i];
        allocation[customer_id][i] += request[i];
        need[customer_id][i] -= request[i];
    }

    if (safety_algorithm())
        printf("Successfully allocate the resources!\n");
    else
    {
        printf("The state is not safe, rollback!\n");
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) // rollback
        {
            available[i] += request[i];
            allocation[customer_id][i] -= request[i];
            need[customer_id][i] += request[i];
        }
    }
}

int release_resources(int customer_id, int release[])
{
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) // check
    {
        if (release[i] > allocation[customer_id][i])
        {
            printf("Customer %d doesn't have so many resources!\n", customer_id);
            return -1;
        }
    }

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) // try to release
    {
        available[i] += release[i];
        allocation[customer_id][i] -= release[i];
        need[customer_id][i] += release[i];
    }

    printf("Successfully release the resources!\n");
}

void show()
{
    printf("Available:\n");
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
        printf("%d ", available[i]);
    printf("\n");

    printf("Maximum:\n");
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
    {
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
            printf("%d ", maximum[i][j]);
        printf("\n");
    }
    printf("\n");

    printf("Allocation:\n");
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
    {
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
            printf("%d ", allocation[i][j]);
        printf("\n");
    }
    printf("\n");

    printf("Need:\n");
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
    {
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
            printf("%d ", need[i][j]);
        printf("\n");
    }
    printf("\n");
}

int safety_algorithm()
{
    int current_avail[NUMBER_OF_RESOURCES];
    int finish[NUMBER_OF_CUSTOMERS];

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
        current_avail[i] = available[i];
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
        finish[i] = 0;

    while (1)
    {
        int flag = 1;
        for (int i = 0; i < NUMBER_OF_CUSTOMERS; ++i)
        {
            int satisfy = 1;
            for (int j = 0; j < NUMBER_OF_RESOURCES; ++j)
                if (finish[i] == 1 || need[i][j] > current_avail[j])
                {
                    satisfy = 0; // the current_avail is not enough for the customer or the customer has been satisfied
                    break;
                }
            // printf("DEBUG:customer[%d] satisfy=%d\n", i, satisfy);
            if (satisfy)
            {
                finish[i] = 1;
                for (int j = 0; j < NUMBER_OF_RESOURCES; ++j)
                    current_avail[j] += allocation[i][j];
                flag = 0;
            }
        }
        // printf("DEBUG:flag=%d\n", flag);
        // for (int i = 0; i < NUMBER_OF_CUSTOMERS; ++i)
        //     printf("DEBUG:finish[%d]=%d\n", i, finish[i]);
        if (flag) // no customer can be satisfied, or no customer remains
            break;
    }

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
        if (finish[i] == 0)
            return 0;

    return 1;
}