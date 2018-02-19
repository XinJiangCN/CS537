#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<fcntl.h>
//method declearations
int execute(char* args[], char* retArgs[]);

//path for binary files
char* PATH[100];
int command_cd(char* args[], int numArgs){
    if (numArgs != 2) {
        printf("Incorrect number of arguments for cd");
        exit(1);
    }
    if (chdir(args[1]) == -1){
        printf("Error when trying to cd");
        exit(1);
    }
    return 0;
}
int command_path(char* args[], int numArgs){
    int i = 0;
    while(1){
        if((PATH[i] = args[i+1]) == NULL)
            break;
        i++;
    } 
    return 0;
}
int lineSeperate(char* line, char* args[]) {
    char *save;
    int argsIndex = 0;
    args[argsIndex++] = strtok_r(line, " ", &save);
    while(1){
        args[argsIndex] = strtok_r(NULL, " ",&save);
        if (args[argsIndex] == NULL)
            break;
        argsIndex++;
    }
    return argsIndex;
}
int redirect(char* ret, char* line) {
    char* progArgs[100];
    char* retArgs[100];
    ret[0] = '\0';
    ret = ret + 2;
    lineSeperate(line, progArgs);
    int retArgc = lineSeperate(ret, retArgs);
    if (retArgc != 1)
        return 1;
    execute(progArgs, retArgs);


    return 0; 
}

int readCommand(char* args[]){

    //for user input
    char* line;
    size_t len = 0;
    ssize_t nread;
    char* ret = NULL;
    if ((nread = getline(&line, &len, stdin)) == -1) {
        printf("Error reading line from user input");
        exit(1);
    }
    //omit the last \n of the string
    line[strlen(line) - 1] = '\0';
    //exit if EOF is read
    if (line[0] == EOF)
        exit(0);

    //for redirection, 
    if ((ret = strchr(line, '>'))){
        redirect(ret, line);
        return -1;
    }

    //seperate the line
    int argsIndex = lineSeperate(line, args);
    //exit if requested
    if (strcmp(args[0], "exit") == 0){
        exit(0);
    }
    if (strcmp(args[0], "cd") == 0){
        command_cd(args, argsIndex);
        return -1;  //-1 for built-in command call flag
    }
    if (strcmp(args[0], "path") == 0){
        command_path(args, argsIndex);
        return -1;
    }
    return 0;
}
int execute(char* args[], char* retArgs[]){
    int pid;
    int status;
    char commandPath[100];
    //test where is the expected executable file
    int i = 0; //index of PATH
    while(1) {
        if(PATH[i] == NULL)
            break;
        //copy the original string to a larger space
        char tempStr[100];
        if (!strcpy(tempStr, PATH[i])){
            printf("strcpy error\n");
            return 1;
        }
        int strLen = strlen(tempStr);
        tempStr[strLen] = '/';
        tempStr[strLen + 1] = '\0';
        strcat(tempStr, args[0]);
        if (access(tempStr, X_OK) == 0){
            strcpy(commandPath, tempStr);
            break;
        }
    } 
    if (commandPath == NULL) {
        printf("Expected Executable File Cannot Be Found\n");
        return 1;
    } 
    //fork and execute
    pid = fork();
    switch(pid){
        case -1:
            perror("fork failed");
            exit(1);
        case 0:
            if (retArgs){
                int fd_out = open(retArgs[0],O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
                if (fd_out > 0){
                    //redirect STDOUT for this process
                    dup2(fd_out, STDOUT_FILENO);
                    fflush(stdout);
                }
            }
            //the forked process will have a pid of 0 so it will execute  the file
            execv(commandPath, args);

        default: 
            //for father process it will goes to default and wait
            waitpid(pid, &status, 0);
    }
    return 0;
}
int main(int argc, char* argv[]){

    //char* userArgs[100] = &argv[1];
    if (argc != 1) {

    }

    while(1) {
        printf("wish> ");
        char* args[100];
        if (readCommand(args) == -1)
            continue;   //if a built-in command is called, continue;
        if (execute(args, NULL) == 1)
            continue;   //if there is an error, continue

    }
    return 0;
}

