#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int sudoku[9][9] = {{5, 3, 4, 6, 7, 8, 9, 1, 2}, {6, 7, 2, 1, 9, 5, 3, 4, 8}, {1, 9, 8, 3, 4, 2, 5, 6, 7}, {8, 5, 9, 7, 6, 1, 4, 2, 3}, {4, 2, 6, 8, 5, 3, 7, 9, 1}, {7, 1, 3, 9, 2, 4, 8, 5, 6}, {9, 6, 1, 5, 3, 7, 2, 8, 4}, {2, 8, 8, 4, 1, 9, 6, 3, 5}, {3, 4, 5, 2, 8, 6, 1, 7, 9}};
int checkbit[27] = {0};

enum
{
    COLUMN,
    ROW,
    BLOCK
};
typedef struct
{
    int type;
    int col_index;
    int row_index;
} parameter;

void *check(void *para)
{
    int type = ((parameter *)para)->type;
    int col_index = ((parameter *)para)->col_index;
    int row_index = ((parameter *)para)->row_index;
    int check[10] = {0};
    switch (type)
    {
    case COLUMN:
        for (int i = 0; i < 9; i++)
        {
            if (check[sudoku[i][col_index]] == 1)
                return NULL;
            else
                check[sudoku[i][col_index]] = 1;
        }
        checkbit[col_index] = 1;
        break;
    case ROW:
        for (int i = 0; i < 9; i++)
        {
            if (check[sudoku[row_index][i]] == 1)
                return NULL;
            else
                check[sudoku[row_index][i]] = 1;
        }
        checkbit[row_index + 9] = 1;
        break;
    case BLOCK:
        for (int i = row_index; i < row_index + 3; i++)
        {
            for (int j = col_index; j < col_index + 3; j++)
            {
                if (check[sudoku[i][j]] == 1)
                    return NULL;
                else
                    check[sudoku[i][j]] = 1;
            }
        }
        checkbit[row_index / 3 * 3 + col_index / 3 + 18] = 1;
        break;
    }
    return NULL; // optional
}

int main()
{
    pthread_t tid[27];
    parameter *para[27];
    for (int i = 0; i < 9; i++)
    {
        para[i] = (parameter *)malloc(sizeof(parameter));
        *para[i] = (parameter){.col_index = i, .row_index = 0, .type = COLUMN};
    }
    for (int i = 0; i < 9; i++)
    {
        para[i + 9] = (parameter *)malloc(sizeof(parameter));
        *para[i + 9] = (parameter){.col_index = 0, .row_index = i, .type = ROW};
    }
    for (int i = 0; i < 9; i++)
    {
        para[i + 18] = (parameter *)malloc(sizeof(parameter));
        *para[i + 18] = (parameter){.col_index = i % 3 * 3, .row_index = i / 3 * 3, .type = BLOCK};
    }
    for (int i = 0; i < 27; i++)
        pthread_create(&tid[i], NULL, check, para[i]);
    for (int i = 0; i < 27; i++)
        pthread_join(tid[i], NULL);
    for (int i = 0; i < 27; i++)
        if (checkbit[i] == 0)
        {
            printf("no\n");
            return 0;
        }
    printf("yes\n");
    return 0;
}