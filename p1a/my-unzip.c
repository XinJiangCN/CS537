#include<stdlib.h>
#include<stdio.h>
/*
 * This is the main function of the program
 * argc: the number of the arguments
 * argv: the array of the passed aurgments
 */
int main(int argc, char* argv[]) {
    if (argc == 1) {
        //if there is no required filename, exit
       printf("my-unzip: file1 [file2 ...]\n");
       exit(1); 
    } else {
        FILE *fp;
        char ch[1];     //the character need to be printed
        int count[1];   //the number of repeated characters
        for(int i = 1; i < argc; i++) {
            fp = fopen(argv[i], "r");
            if (fp == NULL){
                //if cannot open the file, exit
                printf("my-unzip: cannot open file\n");
                exit(1);
            }
            //when there is an number read
            while(fread(count, 4, 1, fp) == 1) {
                if(fread(ch, 1, 1, fp) != 1)
                    break;
                else
                    //when get a character, print the expected number of characters
                    for (int j = 0; j < count[0]; j++){
                        printf("%c", ch[0]);
                    }
            }
            fclose(fp); 
        }
    }
    return 0;
}
