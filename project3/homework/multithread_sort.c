#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int *array;
int *temp_array;

typedef struct
{
    int left;
    int right;
    int mid;
} range;

void swap(int *a, int *b)
{
    int tmp;
    tmp = *a;
    *a = *b;
    *b = tmp;
}

int compare(const void *a, const void *b)
{
    int num1 = *(const int *)a;
    int num2 = *(const int *)b;
    return num1 - num2; // 升序排序
}

void *sort(void *r)
{
    int left = ((range *)r)->left;
    int right = ((range *)r)->right;
    qsort(&array[left], right - left + 1, sizeof(int), compare);
}

void *merge(void *r)
{
    int left = ((range *)r)->left;
    int right = ((range *)r)->right;
    int mid = ((range *)r)->mid;

    int pointer1 = left;
    int pointer2 = mid + 1;
    int temp_pointer = left;

    while (pointer1 <= mid && pointer2 <= right)
    {
        if (array[pointer1] <= array[pointer2])
            temp_array[temp_pointer++] = array[pointer1++];
        else
            temp_array[temp_pointer++] = array[pointer2++];
    }

    while (pointer1 <= mid)
        temp_array[temp_pointer++] = array[pointer1++];
    while (pointer2 <= right)
        temp_array[temp_pointer++] = array[pointer2++];

    for (int i = left; i <= right; i++)
        array[i] = temp_array[i];
}

int main()
{
    int n;
    printf("Please input the number of values to be sorted: ");
    scanf("%d", &n);
    array = (int *)malloc(sizeof(int) * n);
    temp_array = (int *)malloc(sizeof(int) * n);

    printf("Your array:");
    for (int i = 0; i < n; ++i)
        scanf("%d", &array[i]);

    if (n <= 0)
    {
        printf("The number of sorted values must be positeve!\n");
        return 0;
    }
    else if (n == 1)
    {
        printf("Sort Result: %d\n", array[0]);
        return 0;
    }

    pthread_t tid[3];
    pthread_attr_t attr[3];
    for (int i = 0; i < 3; ++i)
        pthread_attr_init(&attr[i]);
    
    range *r[3];
    for (int i = 0; i < 3; ++i)
        r[i] = (range *)malloc(sizeof(range));

    int mid = (n - 1) / 2;
    *r[0] = (range){.left = 0, .right = mid};
    *r[1] = (range){.left = mid + 1, .right = n - 1};
    *r[2] = (range){.left = 0, .right = n - 1, .mid = mid};

    pthread_create(&tid[0], &attr[0], sort, r[0]); // sort
    pthread_create(&tid[1], &attr[1], sort, r[1]); // sort
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_create(&tid[2], &attr[2], merge, r[2]); // merge
    pthread_join(tid[2], NULL);

    printf("Sort Result: ");
    for (int i = 0; i < n; ++i)
        printf("%d ", array[i]);
    printf("\n");

    // free space
    for (int i = 0; i < 3; ++i)
        free(r[i]);
    free(temp_array);
    free(array);
}