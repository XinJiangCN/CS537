#include<stdio.h>
#include<stdlib.h>
/*
 * main funtion of the program
 * argc: the number of arguments
 * argv: the array of values of arguments
 */
int main(int argc, char *argv[]) {
    FILE *fp;
    char buffer[80];
    //loop until all the files indicated are scannec
    for (int i = 1; i < argc; i++) {
        fp = fopen(argv[i], "r");
        if (fp == NULL) {
            //if failed to open an file, exit
            printf("my-cat: cannot open file\n");
            exit(1);
        }
        while (1){
            //if there is a line, print it.
            if (fgets(buffer, 80, fp)!= NULL) 
                printf("%s", buffer);
        else
           //when reach the end, break the loop 
            break;
        }
        fclose(fp);
    }
}

