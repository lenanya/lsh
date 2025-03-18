#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define __USE_XOPEN_EXTENDED
#include <sys/wait.h>
#include <stdio.h>
#include <sys/stat.h>


// thanks chatgpt lol, this is just not the challenge this is supposed to be
// so i just asked chatgpt :3
char **split_string(const char *str, int *count, const char *needle) {
    char *temp = strdup(str);
    char *token;
    char **result = NULL;
    int size = 0;
    token = strtok(temp, needle);
    while (token) {
        result = realloc(result, (size + 1) * sizeof(char *));
        result[size] = strdup(token);
        size++;
        token = strtok(NULL, needle);
    }
    free(temp);
    *count = size;
    return result;
}

int last_occurence(const char *str, char needle) {
    for (int i = strlen(str) - 1; i >= 0; --i) {
        if (str[i] == needle) return i;
    }
    return -1;
}

#define MAX_CMD_SIZE 1024

int main(int argc, char **argv, char **envp) {
    struct stat statbuffer;
    char **argv_cmd;
    char *cwd;
    char full_command[MAX_CMD_SIZE];
    char *PATH = getenv("PATH");
    int path_count;
    char **BIN = split_string(PATH, &path_count, ":");
    int changed_path = 0;
    for (;;) {
        if (changed_path) {
            changed_path = 0;
            chdir(cwd);
        }
        cwd = getcwd(NULL, 0);
        write(STDOUT_FILENO, cwd, strlen(cwd));
        write(STDOUT_FILENO, " # ", 3);
        int count = read(STDIN_FILENO, full_command, 255);
        full_command[count - 1] = 0;
        int args_count;
        argv_cmd = split_string(full_command, &args_count, " ");
        int pid = fork();
        if (pid == 0) {
            if (strcmp(argv_cmd[0], "cd") == 0) {
                break;
            } else if (strcmp(argv_cmd[0], "exit") == 0) {
                break;
            }
            for (int i = 0; i < path_count; ++i) {
                char pathbuffer[1024];
                sprintf(pathbuffer, "%s/%s", BIN[i], argv_cmd[0]);
                if (access(pathbuffer, F_OK) == 0) {
                    execve(pathbuffer, argv_cmd, envp);
                    break;
                }
            }
            printf("Invalid Command :3\n");
            break;
        } else {
            if (strcmp(argv_cmd[0], "cd") == 0) {
                char fullpath[1024];
                sprintf(fullpath, "%s/%s", cwd, argv_cmd[1]);
                if (strcmp(argv_cmd[1], "..") == 0) {
                    char new_pwd[1024];
                    int len = last_occurence(cwd, *"/");
                    strncpy(new_pwd, cwd, len);
                    new_pwd[len] = 0;
                    changed_path = 1;
                    cwd = malloc(strlen(new_pwd));
                    strcpy(cwd, new_pwd);
                } else if (argv_cmd[1][0] == *"/") {
                    if (stat(argv_cmd[1], &statbuffer) == 0) {
                        if (S_ISDIR(statbuffer.st_mode)) {
                            changed_path = 1;
                            cwd = malloc(strlen(argv_cmd[1]));
                            strcpy(cwd, argv_cmd[1]);
                        }
                    }
                } else if (stat(fullpath, &statbuffer) == 0) {
                    if (S_ISDIR(statbuffer.st_mode)) {
                        char new_pwd[1024];
                        sprintf(new_pwd, "%s/%s", cwd, argv_cmd[1]);
                        changed_path = 1;
                        cwd = malloc(strlen(new_pwd));
                        strcpy(cwd, new_pwd);
                    } else {
                        printf("not a directory :3");
                    }
                }
            } else if (strcmp(argv_cmd[0], "exit") == 0) {
                waitid(P_PID, pid, 0, WEXITED);
                break;
            }
            waitid(P_PID, pid, 0, WEXITED);
        }
    }
    return 0;
}