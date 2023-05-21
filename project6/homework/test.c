#include <stdio.h>

#define ROWS 5
#define COLS 4

int main()
{
    FILE *in = fopen("maximum.txt", "r");
    if (in == NULL)
    {
        printf("File not found.\n");
        return 1;
    }

    int arr[ROWS][COLS];

    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            fscanf(in, "%d", &arr[i][j]);
            if (j != COLS - 1)
            {
                fgetc(in); // 读取并忽略逗号
            }
        }
    }

    fclose(in);

    // 打印读取到的数据
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            printf("%d ", arr[i][j]);
        }
        printf("\n");
    }

    return 0;
}
