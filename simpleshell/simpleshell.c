//Arathi Zanvettor Guedes 194330 MC504-A
//Amanda Beatriz Cunha Teixeira 200763 MC504-A

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#define MAX_ARGS 256

char **copy_args(char *args[]) {
    int count = 0;

    while (args[count] != NULL) {
        count++;
    }

    char **new_args = malloc((count + 1) * sizeof(char *));
    if (new_args == NULL) {
        perror("Erro ao alocar memória para new_args");
        exit(1);
    }

    for (int i = 0; i < count; i++) {
        new_args[i] = malloc(strlen(args[i]) + 1);
        if (new_args[i] == NULL) {
            perror("Erro ao alocar memória para new_args[i]");
            exit(1);
        }
        strcpy(new_args[i], args[i]);
    }

    new_args[count] = NULL;

    return new_args;
}

void free_args(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        free(args[i]);
        i++;
    }
    free(args);
}

int read_command(char *args[], char *path, char **last_args) {
    
    char path_copy[256];
    strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';

    // Read the command from the user
    char cmd[256] = "\0"; 
    printf("simple-shell$: ");
    fgets(cmd,256, stdin);
    cmd[strcspn(cmd, "\n")] = '\0';  // Remove newline character

    // Split the command into the command and its arguments
    args[0] = strtok(cmd, " ");

   
    if (args[0]== NULL) { 
        return 2; 
    }
 
    
    else if (strcmp(args[0], "exit") == 0) { 
        return 1; 
    } 

    int i = 1;
    while ((args[i] = strtok(NULL, " ")) != NULL) {
        *last_args = args[i];
        i++;
    }
    if (*last_args != NULL && strcmp(*last_args, "&") == 0) {
        args[i - 1] = NULL;
    }

    char *dir = strtok(path, ":");
    static char full_cmd[256];
    
    while (dir != NULL) {
        snprintf(full_cmd, 512, "%s/%s", dir, cmd);
        if (access(full_cmd, X_OK) == 0) {  // Check if the command is executable
            args[0] = full_cmd;
            return 0;
        }
        dir = strtok(NULL, ":");
    }
    args[0] = NULL;  // Command not found
    return 0;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path>\n", argv[0]);
        exit(1);
    }

    char *args[256] = {NULL}; 
    pid_t pid;

    while(1) { 
        char *last_args = NULL;
        int stop_signal = read_command(args, argv[1], &last_args);
        
        if (stop_signal == 1) { break; }

      
        if (stop_signal == 2) { continue; }

      
        char **new_args = copy_args(args);

        if (args[0] == NULL) {
            printf("---------------------\n"); 
            fprintf(stderr, "Command not found\n");
            printf("---------------------\n"); 
            continue;
        }

        printf("---------------------\n"); 
        if (last_args != NULL && strcmp(last_args, "&") == 0 ) {
            printf("Execução em background \n");
            pid = fork();
            if (pid == 0) {

                int dev_null = open("/dev/null", O_WRONLY);
                if (dev_null == -1) {
                    perror("open");
                    exit(1);
                }

                // sent stdout and stderr to /dev/null
                dup2(dev_null, STDOUT_FILENO);
                dup2(dev_null, STDERR_FILENO);
                close(dev_null);
                execv(args[0], new_args);
                perror("Execv failed");  // if it fails
                free_args(new_args);
                exit(1);
            }
        }
        else {
            printf("Execução em primeiro plano \n");
            printf("---------------------\n");
            pid = fork();
            if (pid == 0) {
                execv(args[0], new_args);
                perror("Execv failed");  // if it fails
                free_args(new_args);
                exit(1);
            } else if (pid > 0){
                waitpid(pid, NULL, 0);
            }

        }

        printf("---------------------\n");      
        free_args(new_args);
    }
    
    printf("%s", "The End\n");
    return 0;
}
