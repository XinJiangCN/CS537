#include<stdio.h>
#include<stdlib.h>
#include<string.h>
/*
 * the main funtion of the program
 * argc: the number of arguments
 * argv: the array of values of arguments
 *
 */
int main(int argc, char *argv[]) {
    FILE *fp;
    char *line = NULL;  //the line used to store the content from file
    size_t len = 0;     //length of the line
    ssize_t read;       //the sizes of blocks to be read
    if (argc == 1) {
        //if there is no passed filename, exit
        printf("my-grep: searchterm [file ...]\n");
        exit(1);
    } else if(argc == 2){
        //if there is only the search term, request for the input and search
        fp = stdin;
        //read from the command line
        while ((read = getline(&line, &len, fp)) != -1) {
            if (strstr(line, argv[1])) 
                printf("%s", line);
            else 
                continue;
        }
    } else {
        //if there is indicated files, search the terms
        for (int i = 2; i < argc; i++) {
            fp = fopen(argv[i], "r");
            if (fp == NULL) {
                //if cannot open the file, exit
                printf("my-grep: cannot open file\n");
                exit(1);
            }
            //read from the file, seach and match the term required
            while ((read = getline(&line, &len, fp)) != -1) {
                if (strstr(line, argv[1])) 
                    printf("%s", line);
                else 
                    //when reach the end of file, break
                    continue;

            }
            fclose(fp);
        }
    }
    if(line)
        free(line);
    return 0;
}

