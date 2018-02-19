#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h>

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
    int i = 1;
    while(1){
       if((PATH[i] = args[i]) == NULL)
           break;
        i++;
    } 
    return 0;
}
int readCommand(char* args[]){

    //for user input
    char* line;
    size_t len = 0;
    ssize_t nread;
    //for sperating the user input
    int argsIndex = 0;

    if ((nread = getline(&line, &len, stdin)) == -1) {
        printf("Error reading line from user input");
        exit(1);
    }
    //omit the last \n of the string
    line[strlen(line) - 1] = '\0';
    //exit if EOF is read
    if (line[0] == EOF)
        exit(0);

    char *save;
    args[argsIndex++] = strtok_r(line, " ", &save);
    while(1){
        args[argsIndex] = strtok_r(NULL, " ",&save);
        if (args[argsIndex] == NULL)
            break;
        argsIndex++;
    }
    //exit if requested
    if (strcmp(args[0], "exit") == 0){
        exit(0);
    }
    if (strcmp(args[0], "cd") == 0){
        command_cd(args, argsIndex);
        return -1;  //-1 for built-in command call flag
    }
    if (strcmp(args[0]), "path" == 0){
        command_cd(args, argsIndex);
        return -1;
    }
    return 0;
}
int execute(char* args[]){
    int pid;
    int status;
    char* commandPath = NULL;
    //test where is the expected executable file
    char tempPath1[128] = "/bin/";
    char tempPath2[128] = "/usr/bin/";
    while(1) {
        if(PATH[i] == NULL)
            break;
        //copy the original string to a larger space
        char* tempStr[100];
        strcpy(tempStr, PATH[i]);
        if (access(strcat(tempStr, args[0]), X_OK) == 0)
            commandPath = tempStr;
    } 
    if (commandPath == NULL) {
        printf("Expected Executable File Cannot Be Found");
        exit(1);
    } 
    //fork and execute
    pid = fork();
    switch(pid){
        case -1:
            perror("fork failed");
            exit(1);
        case 0:
            //the forked process will have a pid of 0 so it will execute  the file
            execv(PATH, args);
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
        execute(args);

    }
    return 0;
}

