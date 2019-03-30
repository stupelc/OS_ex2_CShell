#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <errno.h>

#define SIGHN "> "
#define MAX_MESS 512
#define ERROR_MESS fprintf(stderr, "Error in system call\n");

typedef struct jobs {
    pid_t pid; //the pid of the job
    char jobName[MAX_MESS]; //the array of the jobs
} Job_Array;

int AfterSpling(char *line, char **execute);

void ExitCall();

void CdCall(char **execute);

void JobsCall(Job_Array *jobs, int *numOfJobs);

// previous_pwd - path to previous working directory for cd -
char previousPwd[MAX_MESS] = "not set";

int main() {
    char op[MAX_MESS];
    pid_t forkId;
    Job_Array jobs[MAX_MESS];
    char operation[MAX_MESS];
    char *token;
    char *args[MAX_MESS];
    int flag = 0;
    int numOfJobs = 0;

    while (1) {
        printf(SIGHN);
        fgets(operation, MAX_MESS + 1, stdin);
        operation[strlen(operation) - 1] = 0;
        //ignore from enter only
        if (strcmp(operation, "\0") == 0) {
            continue;
        }
        //saves the operation in op
        strcpy(op, operation);

        //splits our message to args. length is the number of args
        int length = AfterSpling(operation, args);

        token = strtok(op, "&");

        //if we had an & in the string
        if (!strcmp(args[length - 1], "&")) {
            // free(args[length - 1]);
            args[length - 1] = NULL;
            flag = 1;
        }
        //fist check if the commands are special commands
        if ((strcmp(args[0], "exit")) == 0) {
            ExitCall();
        } else if ((strcmp(args[0], "cd")) == 0) {
            CdCall(args);
        } else if ((strcmp(args[0], "jobs")) == 0) {
            JobsCall(jobs, &numOfJobs);
        } else {
            //if the command is a regular command
            forkId = fork();
            if (forkId == -1) {
                perror("fork failed");
            } else if (forkId == 0) {
                if (execvp(args[0], args) < 0) {
                    ERROR_MESS
                }
            } else {
                if (flag == 1) {
                    strcpy(jobs[numOfJobs].jobName, token);
                    jobs[numOfJobs].pid = forkId;
                    numOfJobs++;
                    printf("%d\n", forkId);
                    flag = 0;
                } else {
                    int waitRet = waitpid(forkId, &forkId, 0);
                    if (waitRet == -1) {
                        ERROR_MESS
                    }
                }
            }
        }
    }
}

//take a string and split and split it to an array of string
int AfterSpling(char *line, char **execute) {
    char *buffer;
    size_t pos = 0;
    int numOfArgs = 0;

    if (strchr(line, '\"') != NULL) {
        buffer = strtok(line, "\"");
        // split command and append tokens to array arguments
        while (buffer) {
            execute[pos++] = buffer;
            buffer = strtok(NULL, "\"");
        }
        execute[0] = "cd";
    } else if (strchr(line, '\'') != NULL) {
        buffer = strtok(line, "\'");
        // split command and append tokens to array arguments
        while (buffer) {
            execute[pos++] = buffer;
            buffer = strtok(NULL, "\'");
        }
        execute[0] = "cd";
    } else {

        buffer = strtok(line, " , \n");
        execute[pos++] = buffer;
        while (buffer != NULL) {
            buffer = strtok(NULL, " ,\n");
            execute[pos++] = buffer;
            numOfArgs++;
        }
    }
    return numOfArgs;

}

void ExitCall() {
    pid_t pid = getpid();
    printf("%d\n", pid);
    exit(0);
}

void CdCall(char **execute) {
    pid_t currentPID = getpid();
    printf("%d\n", currentPID);

    // save current working dir
    char currentPwd[MAX_MESS];
    if (getcwd(currentPwd, MAX_MESS) == NULL) {
        ERROR_MESS
    }
    // for global path:
    // "cd" or "cd ~ ..."
    if ((execute[1] == NULL) || (!strcmp(execute[1], "~"))) {
        // set working directory to HOME for global
        if (chdir(getenv("HOME")) != -1) {
            // if successfully changed wd to home, update prev p working dir
            strcpy(previousPwd, currentPwd);
        } else {
            ERROR_MESS;
        }
    } else if (strcmp(execute[1], "-") == 0) {
        // go to previous folder
        // chdir to father dir
        if (strcmp(previousPwd, "not set") != 0) {
            // go to previous folder
            if (chdir(previousPwd) == -1) {
                ERROR_MESS;
            } else {
                printf("%s\n", previousPwd);
                strcpy(previousPwd, currentPwd);
            }
        } else {
            //OLDWD not set
            fprintf(stderr, "cd: OLDWD not set\n");
        }
    } else {
        // change pwd path using chdir
        // set working directory to param path
        if (chdir(execute[1]) == -1) {
            ERROR_MESS;
        } else {
            strcpy(previousPwd, currentPwd);
        }
    }
}

void JobsCall(Job_Array *jobs, int *numOfJobs) {
    int size = 0;
    int i;
    for (i = 0; i <= *numOfJobs; i++) {
        if (waitpid(jobs[i].pid, &size, WNOHANG)) {
            //deleate from array
            int j;
            for (j = i; j <= *numOfJobs; j++) {
                jobs[j].pid = jobs[j + 1].pid;
                strcpy(jobs[j].jobName, jobs[j + 1].jobName);
            }
            *numOfJobs -= 1;
        }
    }

    if (*numOfJobs > 0) {
        int z;
        for (z = 0; z < *numOfJobs; z++) {
            printf("%d %s\n", jobs[z].pid, jobs[z].jobName);
        }
    }
}