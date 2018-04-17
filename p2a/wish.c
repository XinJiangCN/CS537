#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<fcntl.h>
#define DELIM " \t\r\n\a"
//method declearations
int execute(char* args[], char* retArgs[]);

//path for binary files
char* PATH[20];
void error_handler(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 
}
int command_cd(char* args[], int numArgs){
    if (numArgs != 2) {
        error_handler();
        return 1;
    }
    if (chdir(args[1]) == -1){
        error_handler();
        return 1;
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
int lineSeperate(char* line, char* args[], char* delim) {
    char *save;
    int argsIndex = 0;
    //args = malloc(100*sizeof(char*));
    if (!args)
        args = malloc(100*sizeof(char));
    args[argsIndex] = strtok_r(line, delim, &save);
    argsIndex++;
    while(1){
        if (!(args + argsIndex)){
            char* temp = (char*)(args + argsIndex);
            temp  = malloc(100*sizeof(char));
            temp++;
        }
        args[argsIndex] = strtok_r(NULL, delim,&save);
        if (args[argsIndex] == NULL)
            break;
        argsIndex++;
    }
    if (args[0] == NULL)
        return 0;
    return argsIndex;
}
int redirect(char* ret, char* line) {
    char* progArgs[100];
    char* retArgs[100];
    ret[0] = '\0';
    ret = ret + 1;
    int argsNum = lineSeperate(line, progArgs, DELIM);
    if (argsNum == 0){
        error_handler();
        return 1;
    }
    int retArgc = lineSeperate(ret, retArgs, DELIM);
    if (retArgc != 1){
        error_handler();
        return 1;
    }
    execute(progArgs, retArgs);
    return 0; 
}
int parallel(char* ret, char* line){
    char** commands = malloc(100*sizeof(char*));
    int numCommands = lineSeperate(line, commands,"&");
    char** args = malloc(50*sizeof(char*));
    char* retRedir = malloc(100*sizeof(char));
    for (int i = 0; i < numCommands; i++) {
        if ((retRedir = strchr(commands[i], '>'))){
            redirect(retRedir, commands[i]);
            continue;
        }
        lineSeperate(commands[i], args, DELIM);
        execute(args, NULL);
    }
    free(args);
    free(commands);
    return 0;
}
int readCommand(char* args[],FILE * fp){

    //for user input
    char* line = malloc(100*sizeof(char));
    size_t len = 0;
    ssize_t nread;
    char* retRedir = NULL;
    char* retParal = NULL;

    fflush(stdin);
    if ((nread = getline(&line, &len, fp)) == -1) {
        //error_handler();
        return 1;
    }
    if ((strcmp(line, "\n") == 0) || (strcmp(line, "") == 0))
        return -1;
    //omit the last \n of the string
    if (line[strlen(line) - 1] == '\n')
        line[strlen(line) - 1] = '\0';
    //exit if EOF is read
    if (line[0] == EOF){
        return 1;
    }
    //forparallel commands
    if ((retParal = strchr(line, '&'))){
        parallel(retParal, line);
        return -1;
    }
    //for redirection, 
    if ((retRedir = strchr(line, '>'))){
        redirect(retRedir, line);
        return -1;
    }
    //seperate the line
    int argsIndex = lineSeperate(line, args, DELIM);
    if (argsIndex == 0) {
        //error_handler();
        return 0;
    }
    //exit if requested
    if (strcmp(args[0], "exit") == 0){
        if (args[1])
            error_handler();
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
    char* commandPath = malloc(100*sizeof(char*));
    //test where is the expected executable file
    int i = 0; //index of PATH
    int isNotFound = 1;
    if (PATH[0] == NULL){
        error_handler();
        return 1;
    }
    if (args == NULL)
        return 1;
    if (args[0] == NULL)
        return 1;

    while(1) {
        if(PATH[i] == NULL)
            break;
        //copy the original string to a larger space
        char* tempStr = malloc(100*sizeof(char));
        if (!strcpy(tempStr, PATH[i])){
            error_handler();
            return 1;
        }
        int strLen = strlen(tempStr);
        tempStr[strLen] = '/';
        tempStr[strLen + 1] = '\0';
        strcat(tempStr, args[0]);
        if (access(tempStr, X_OK) == 0){
            strcpy(commandPath, tempStr);
            isNotFound = 0;
            free(tempStr);
            break;
        }
        free(tempStr);
        i++;
    } 
    if (isNotFound) {
        error_handler();
        return 1;
    } 
    //fork and execute
    pid = fork();
    switch(pid){
        case -1:
            printf("CNMLDG");
            error_handler();
            return 1;
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
    int isBatchMode = 0;
    PATH[0] = "/bin";

    FILE *fp;
    char** args;
    //char* userArgs[100] = &argv[1];
    if (argc == 2) {
        if (!(fp = fopen(argv[1], "r")) ){
            error_handler();
            exit(1);
        }
        isBatchMode = 1;
    } else if (argc < 1 || argc > 2) {
        error_handler();
        exit(1);
    }
    int isFinish = 0;
    while(1) {

        if (isBatchMode == 1){
            while(1){
                args = malloc(100*sizeof(char));
                int readStatus = readCommand(args, fp);
                fflush(fp);
                if (readStatus == -1)
                    continue;
                if (readStatus == 1) {
                    isFinish = 1;
                    break;
                }
                int errNum = execute(args, NULL);
                free(args);
                if (errNum == 1)
                    continue;
            }
            break;
        } else {
            fprintf(stdout, "wish> ");
            fflush(stdout);
            args = malloc(100*sizeof(char));
            int readStatus = readCommand(args, stdin);
            fflush(stdin);
            if (readStatus == -1)
                continue;   //if a built-in command is called, continue;
            if (readStatus == 1)
                break;
            if (execute(args, NULL) == 1)
                continue;   //if there is an error, continue
            free(args);
        }
        if (isFinish)
            break;
    }
    return 0;
}

