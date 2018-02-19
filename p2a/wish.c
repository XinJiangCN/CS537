#include<stdio.h>
#include<unistd.h>
#include<string.h>

char** readCommand(){
    //for user input
    char* line;
    size_t len = 0;
    ssize_t nread;
    //for sperating the user input
    char* args[100];
    char* sepPtr = NULL;
    int argsIndex = 0;

    if ((nread = getline(&line, &len, stdin)) != -1) {
        printf("Error reading line from user input");
        continue;
    }

    //exit if requested
    if (strcmp(line, "exit"))
        exit(0);
    //exit if EOF is read
    if (line[0] == EOF)
        exit(0);

    args[argsIndex++] = strtok(line, " ");
    while(1){
        args[argsIndex] = strtok(NULL, " ");
        if (args[argsIndex] == NULL)
            break;
        argsIndex++;
    }
    return args;
}
int pathSearch
int execute(char* args[]){
    int pid;
    int status;
    char* PATH;
    //test where is the expected executable file
    if (access(strcat("/bin/", args[0]), X_OK) == 0)
        PATH = strcat("/bin/", args[0]);
    else if (access(strcat("/usr/bin/", args[0]), X_OK) == 0)
        PATH = strcat("/usr/bin/", args[0]);
    else {
        printf("Expected Executable File Did Not Found");
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
            while(wait(&status) != pid);
    }

}
int main(int argc, char* argv[]){
    char* userArgs[argc - 1] = &argv[1];
    if (argc != 1) {

    }

    while(1) {
        printf("wish> ");
        char** args = readCommand();
        execute(args);

    }

}

