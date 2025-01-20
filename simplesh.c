#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<dirent.h>
#include<sys/stat.h>
#include<signal.h>

#define SH_RL_BUFSIZE 1024
#define size_of_buff 1024
#define MAX_ARGS_SIZE 1024
#define BUFFER_SIZE 1024

#define ANSI_COLOR_GREEN   "\x1b[32;1m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void simplesh_clear();
void simplesh_history_last();
void simplesh_history(int no_args);

char history_name[1024], no_args_name[1024];
extern char **environ;

void simplesh_env()
{
    char** env = environ;
    while (*env)
    {
        printf("%s\n", *env);
        env++;
    }
}

void simplesh_pwd()
{
    char *pwd;
    char buf[1024];
    pwd = getcwd(buf, 1024);
    if (pwd == NULL)
    {
        printf("Error in getcwd() command\n");
    }
    else
    {
        printf("Current working directory: %s\n", pwd);
    }
}

void simplesh_clear() {
    if (!printf("\033[2J\033[1;1H")) {
        perror("");
    }
}

char* simplesh_read() {
    char* line = NULL;
    ssize_t bufsize = 0;
    int no = getline(&line, &bufsize, stdin);
    if (no == -1) { // Handle EOF or error
        perror("Error reading input");
        exit(EXIT_FAILURE);
    }
    return line;
}

void simplesh_history_last() {
    FILE *file1 = fopen(history_name, "r");
    if (file1 == NULL) {
        perror("Error opening the history file");
        return;
    }

    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, file1) != -1) {
        continue;
    }
    printf("%s", line);
    fclose(file1);
    if (line) free(line);
}

void simplesh_history(int no_args) {
    char *line = NULL;
    size_t len = 0;

    if (no_args <= 0) {
        printf("Invalid option\n");
        return;
    }

    FILE *file1 = fopen(history_name, "r");
    if (file1 == NULL) {
        file1 = fopen(history_name, "w");
        fclose(file1);
        file1 = fopen(history_name, "r");
        if (file1 == NULL) {
            printf("Error while reading from history\n");
            return;
        }
    }

    int count = 0, i = 0;
    while (getline(&line, &len, file1) != -1) {
        count++;
    }
    fclose(file1);

    file1 = fopen(history_name, "r");
    while (getline(&line, &len, file1) != -1) {
        if (i >= (count - no_args))
            printf("%s", line);
        i++;
    }
    fclose(file1);
    if (line) free(line);
}

char** parse_command(char* line, int *cnt, int *bg) {
    int state = 0, i = 0, j = 0, k = 0;
    char buf[BUFFER_SIZE];
    char **words = (char**)malloc(MAX_ARGS_SIZE * (sizeof(char*)));

    while (1) {
        while (line[i] != '\0') {
            switch (state) {
                case 0:
                    if (line[i] == '"') {
                        state = 2;
                    } else if (!isspace(line[i])) {
                        state = 1;
                        buf[j++] = line[i];
                    }
                    break;

                case 1:
                    if (line[i] == '"') {
                        state = 2;
                    } else if (!isspace(line[i])) {
                        buf[j++] = line[i];
                    } else {
                        if (j > 0 && buf[j - 1] == '&') {
                            *bg = 1;
                            buf[j - 1] = '\0';
                        }
                        state = 0;
                        buf[j++] = '\0';
                        words[k++] = strdup(buf);
                        j = 0;
                    }
                    break;

                case 2:
                    if (line[i] == '"') {
                        state = 3;
                    } else {
                        buf[j++] = line[i];
                    }
                    break;

                case 3:
                    if (line[i] == '"') {
                        state = 2;
                    } else if (!isspace(line[i])) {
                        state = 1;
                        buf[j++] = line[i];
                    } else {
                        *bg = 0;
                        state = 0;
                        buf[j++] = '\0';
                        words[k++] = strdup(buf);
                        j = 0;
                    }
                    break;
            }
            i++;
        }

        if (j == 0) {
            // Handling for history and no_args file
            FILE *file1, *file2;
            file2 = fopen(no_args_name, "r");
            if (file2 == NULL) {
                file2 = fopen(no_args_name, "w");
                fprintf(file2, "0");
                fclose(file2);
                file2 = fopen(no_args_name, "r");
                if (file2 == NULL) {
                    perror("Error opening no_args file");
                }
            }

            int offset;
            fscanf(file2, "%d", &offset);

            char buf[size_of_buff];
            buf[0] = '\0';
            char str[15];
            offset++;
            sprintf(str, "%d", offset);
            strcat(buf, str);
            strcat(buf, " ");
            strcat(buf, line);

            file1 = fopen(history_name, "a");
            if (file1 == NULL) {
                perror("Error opening history file");
            } else {
                fprintf(file1, "%s", buf);
                fclose(file2);
                file2 = fopen(no_args_name, "w");
                fprintf(file2, "%d", offset);
                fclose(file2);
                fclose(file1);
            }
            break;
        }

        buf[j++] = '\0';
        words[k++] = strdup(buf);
        j = 0;
        i = 0;

        free(line);
        line = simplesh_read();
    }

    *cnt = k;
    return words;
}


void execute(char* command, char** args, int bg)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        int program = execvp(command, args);
        if (program == -1)
        {
            perror("Error running new program");
            exit(0);
        }
    }
    else
    {
        if (bg == 0)
        {
            waitpid(pid, NULL, 0);
        }
    }
}

void simplesh_cd(char* dir)
{
    if (dir == NULL)
    {
        char *home = getenv("HOME");
        if (home == NULL)
        {
            fprintf(stderr, "simplesh: cd: HOME not set\n");
            return;
        }
        if (chdir(home) != 0)
        {
            perror("simplesh: cd");
        }
    }
    else if (strcmp(dir, "..") == 0)
    {
        if (chdir("..") != 0)
        {
            perror("simplesh: cd");
        }
    }
    else
    {
        if (chdir(dir) != 0)
        {
            perror("simplesh: cd");
        }
    }
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("Current working directory: %s\n", cwd);
    }
    else
    {
        perror("simplesh: cd");
    }
}

void simplesh_mkdir(char* dir)
{
    if (mkdir(dir, 0777) == 0) {
        printf("Directory '%s' created successfully\n", dir);
    } else {
        perror("simplesh: mkdir");
    }
}

void simplesh_rmdir(char* dir)
{
    if (rmdir(dir) == 0) {
        printf("Directory '%s' removed successfully\n", dir);
    } else {
        perror("simplesh: rmdir");
    }
}

void simplesh() {
    char *line, **args;
    while (1) {
        printf(ANSI_COLOR_GREEN "%s> " ANSI_COLOR_RESET, getenv("PWD"));
        line = simplesh_read();
        int bg = 0, cnt = 0;
        args = parse_command(line, &cnt, &bg);

        if (cnt == 0) {
            free(line);
            continue;
        }

        char *command = args[0];

        if (strcmp(command, "history") == 0) {
            if (cnt == 1) {
                simplesh_history_last();
            } else {
                int n_arg = atoi(args[1]);
                simplesh_history(n_arg);
            }
        }
        else if (strcmp(command, "clear") == 0) {
            simplesh_clear();
        }
        else if (strcmp(command, "env") == 0) {
            simplesh_env();
        }
        else if (strcmp(command, "pwd") == 0) {
            simplesh_pwd();
        }
        else if (strcmp(command, "cd") == 0) {
            if (cnt <= 1) {
                simplesh_cd(NULL);
            } else {
                simplesh_cd(args[1]);
            }
        }
        else if (strcmp(command, "mkdir") == 0) {
            if (cnt <= 1) {
                perror("Please mention a directory to create");
            } else {
                for (int i = 1; i < cnt; i++) {
                    simplesh_mkdir(args[i]);
                }
            }
        }
        else if (strcmp(command, "rmdir") == 0) {
            if (cnt <= 1) {
                perror("Please mention directories to remove");
            } else {
                for (int i = 1; i < cnt; i++) {
                    simplesh_rmdir(args[i]);
                }
            }
        }
        else if (strcmp(command, "exit") == 0) {
            exit(0);
        }
        else {
            execute(command, args, bg);
        }

        for (int i = 0; i < cnt; i++) {
            free(args[i]);
        }
        free(args);
        free(line);
    }
}

int main() {
    strcpy(history_name, getenv("HOME"));
    strcpy(no_args_name, getenv("HOME"));
    strcat(history_name, "/.history");
    strcat(no_args_name, "/.no_args");
    simplesh_clear();
    simplesh();
    return 0;
}
