#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LINE 80 // The maximum length command
#define READ_END 0  // The index of read end of pipe
#define WRITE_END 1 // The index of write end of pipe

int input_func(char *input_sentence, char *history_sentence, int history_exist);
int parse_func(char *input_sentence, char *args[], int *arg_num, char *stop_word);
void pipe_func(char *args[], int *arg_num, int *pipe_exist, char *pipe_args[]);
int process_func(char *args[], int *arg_num, int *pipe_exist, int *fd, char *pipe_args[]);
void history_func(char *history_sentence, int *history_exist, char *input_sentence);

int main()
{
    char *args[MAX_LINE / 2 + 1]; // Command line arguments
    for (int k = 0; k < MAX_LINE / 2 + 1; k++)
        args[k] = NULL; // Initialize the array
    // each element is a pointer to a string
    char *pipe_args[MAX_LINE / 2 + 1]; // The arguments after the pipe
    for (int k = 0; k < MAX_LINE / 2 + 1; k++)
        pipe_args[k] = NULL; // Initialize the array

    char input_sentence[81];   // The input sentence
    char history_sentence[81]; // The history sentence
    char stop_word[] = "exit"; // The stop word, to exit the shell
    int history_exist = 0;     // Whether the history sentence exists
    int fd[2];                 // The pipe
    int arg_num = 0;           // The number of arguments
    int pipe_exist = 0;        // Whether the pipe exists
    
    while (1) // the code will always excute untill "exit" is called
    {
        printf("shell>");                                                               // Print the prompt
        int input_return = input_func(input_sentence, history_sentence, history_exist); // Get the input sentence
        if (input_return == 0)
            continue; // If the input sentence is empty, then continue the loop
        // printf("DEBUG: input success\n");

        int parse_return = parse_func(input_sentence, args, &arg_num, stop_word); // Parse the input sentence
        if (parse_return == 0)
            continue; // continue the loop
        else if (parse_return == 1)
            break; // If the input sentence is "exit", then break the loop
        // printf("DEBUG: parse success\n");

        pipe_func(args, &arg_num, &pipe_exist, pipe_args); // Detect the pipe
        // printf("DEBUG: pipe success\n");
        // for (int i = 0; args[i]!=NULL; ++i)
            // printf("DEBUG: args[%d] = %s\n", i, args[i]);
        // for (int i = 0; pipe_args[i]!=NULL; ++i)
            // printf("DEBUG: pipe_args[%d] = %s\n", i, pipe_args[i]);
        // printf("DEBUG: arg_num = %d\n", arg_num);

        int process_return = 0;
        process_return = process_func(args, &arg_num, &pipe_exist, fd, pipe_args); // Process the input sentence
        if (process_return == 1)                                                   // some error occurs
            return process_return;
        // printf("DEBUG: process success\n");

        // clear
        for (int i = 0; i < MAX_LINE / 2 + 1; ++i)
            if (args[i] != NULL){
                free(args[i]);
                args[i] = NULL;
            }
        for (int i = 0; i < MAX_LINE / 2 + 1; ++i)
            if (pipe_args[i] != NULL){
                free(pipe_args[i]);
                pipe_args[i] = NULL;
            }
        // printf("DEBUG: clear success\n");

        history_func(history_sentence, &history_exist, input_sentence); // Update the history sentence
        // printf("DEBUG: history success\n");
    }
}

int input_func(char *input_sentence, char *history_sentence, int history_exist)
{
    fgets(input_sentence, 81, stdin); // Get the input sentence from stdin, and the maximum length is 81, including the '\0'
    if (input_sentence[0] == '\n')
        return 0; // If the first character is '\n', then return 0 to remind the main function to continue the loop

    if (input_sentence[0] == '!' && input_sentence[1] == '!' && input_sentence[2] == '\n')
    {
        if (history_exist == 0)
        {
            printf("No commands in history!\n");
            return 0;
        }
        else
        {
            int length_history = 0;
            for (; history_sentence[length_history] != '\n'; ++length_history)
                ;
            for (int k = 0; k < length_history; ++k)
                printf("%c", history_sentence[k]);
            printf("\n");
            for (int k = 0; k < length_history; ++k)
                input_sentence[k] = history_sentence[k];
            input_sentence[length_history] = '\n';
            input_sentence[length_history + 1] = '\0';
        }
    }
    return 2; // nothing wrong
}

int parse_func(char *input_sentence, char *args[], int *arg_num, char *stop_word)
{
    *arg_num = 0; // Initialize the number of arguments
    int i = 0, j = 0;
    while (input_sentence[i] != '\n')
    {
        args[*arg_num] = (char *)malloc(sizeof(char) * 25);
        while (input_sentence[i] != ' ' && input_sentence[i] != '\n')
        {
            args[*arg_num][j] = input_sentence[i];
            ++i;
            ++j;
        }
        args[*arg_num][j] = '\0'; // Add the end of string
        *arg_num += 1;            // Increase the number of arguments
        j = 0;                    // Reset the index of the argument
        if (input_sentence[i] == ' ')
            ++i;
        if (input_sentence[i] == '\n')
            break; // If the end of the input sentence is reached, then break the loop
    }
    // the arg_num is the number of arguments
    if (strcmp(args[0], stop_word) == 0)
        return 1; // break

    if (strcmp(args[0], "!!") == 0)
    {
        printf("No commands in history!!\n");
        return 0; // continue
    }
}

void pipe_func(char *args[], int *arg_num, int *pipe_exist, char *pipe_args[])
{
    int pipe_position = 0;
    *pipe_exist = 0;
    for (int i = 0; i < *arg_num; ++i)
        if (strcmp(args[i], "|") == 0) // detect the pipe
        {
            pipe_position = i;
            *pipe_exist = 1;
            // break;
        };

    if (*pipe_exist == 1)
    {
        // create pipe args
        int total_num = *arg_num;

        for (int j = pipe_position + 1, i = 0; j < total_num; ++j, ++i)
        {
            int q = 0;
            pipe_args[i] = (char *)malloc(sizeof(char) * 25);
            for (; args[j][q] != '\0'; ++q)
                pipe_args[i][q] = args[j][q]; // copy the pipe_args from args
            pipe_args[i][q] = '\0';

            free(args[j - 1]);  // free the memory
            args[j - 1] = NULL; // set the pointer to NULL
            (*arg_num)--;
        }

        free(args[total_num - 1]);
        args[total_num - 1] = NULL;
        (*arg_num)--;
    }
}

int process_func(char *args[], int *arg_num, int *pipe_exist, int *fd, char *pipe_args[])
{
    // printf("DEBUG: proces_func begin\n");
    pid_t pid;
    pid = fork();
    // printf("DEBUG: pid = %d\n", pid);
    if (pid < 0)
    {
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if (pid == 0)
    {
        // printf("DEBUG: arg_num = %d\n", *arg_num);
        if (strcmp(args[*arg_num - 1], "&") == 0) 
        {
            args[*arg_num - 1] = NULL;
            (*arg_num)--;
        }
        if (*arg_num >= 2)
        {
            if (strcmp(args[*arg_num - 2], ">") == 0) // redirect std output to file
            {
                int origin_point = dup(STDOUT_FILENO);                                                    // save the origin point
                int file_write = open(args[*arg_num - 1], O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR); // create the file
                if (file_write == -1)
                    fprintf(stderr, "Create file Failed");
                dup2(file_write, STDOUT_FILENO); // redirect the output to the file
                args[*arg_num - 2] = NULL;       //
                args[*arg_num - 1] = NULL;
                execvp(args[0], args);             // execute the command
                close(file_write);                 // close the file
                dup2(origin_point, STDOUT_FILENO); // redirect the output to the origin point
                exit(0);
            }
            else if (strcmp(args[*arg_num - 2], "<") == 0) // redirect std input to file
            {
                int origin_point = dup(STDIN_FILENO);
                int file_read = open(args[*arg_num - 1], O_RDONLY);
                dup2(file_read, STDIN_FILENO);
                args[*arg_num - 2] = NULL;
                args[*arg_num - 1] = NULL;
                execvp(args[0], args);
                close(file_read);
                dup2(origin_point, STDIN_FILENO);
                exit(0);
            }
            else if (*pipe_exist == 1) // situation using pipe
            {
                pid_t pid2;
                if (pipe(fd) == -1) // create pipe
                {
                    fprintf(stderr, "Pipe failed");
                    return 1;
                }
                pid2 = fork(); // new  child process
                // printf("DEBUG: create child process\n");
                if (pid2 < 0)
                {
                    fprintf(stderr, "Fork Failed");
                    return 1;
                }
                else if (pid2 == 0) // child do former
                {
                    close(fd[READ_END]);                // close the read end
                    dup2(fd[WRITE_END], STDOUT_FILENO); // redirect the output to the write end
                    execvp(args[0], args);              // execute the command
                    close(fd[WRITE_END]);               // close the write end
                    exit(0);
                }
                else // parent do latter
                {
                    close(fd[WRITE_END]);
                    dup2(fd[READ_END], STDIN_FILENO);
                    execvp(pipe_args[0], pipe_args);
                    close(fd[READ_END]);
                    wait(NULL);
                }
            }
            else
                execvp(args[0], args);
        }
        else
            execvp(args[0], args);
    }
    else // parent process
        if (strcmp(args[*arg_num - 1], "&") != 0)
            wait(NULL);
}

void history_func(char *history_sentence, int *history_exist, char *input_sentence)
{
    // save history
    int m = 0;
    for (; input_sentence[m] != '\n'; ++m)
        history_sentence[m] = input_sentence[m];
    history_sentence[m] = '\n';
    history_sentence[m + 1] = '\0';
    *history_exist = 1;
}