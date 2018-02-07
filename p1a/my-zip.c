#include<stdlib.h>
#include<stdio.h>
/*
 * This is the main function of the program
 * argc: the number of arguments 
 * argv: the array of values of arugments
 */
int main(int argc, char* argv[]) {
    if (argc == 1) {
        //if there is no passed filename, exit
        printf("my-zip: file1 [file2 ...]\n");
        exit(1);
    } else {
        FILE *fp;
        char last = '\0';  //last character
        char curr = '\0';  //the current character
        int count = 1;     //the number of repeated character
        for(int i = 1; i < argc; i++) {
            fp = fopen(argv[i], "r");
            if (fp == NULL) {
                //if cannot open the file, exit
                printf("my-zip: cannot open file\n");
                exit(1);
            }
            if (last == '\0')
                last = getc(fp);
            while ((curr = getc(fp)) != EOF){
                if (curr == last) {
                    //if the characters repeated
                    count += 1;
                } else {
                    //print the number as well as the character
                    fwrite(&count, 4, 1, stdout);
                    fwrite(&last, 1, 1, stdout);
                    count = 1;
                }
                //save the current as the "last" for the next one. 
                last = curr;
            }
            //if reach the end of all files, print the last combination
            if(i == argc - 1) {
                fwrite(&count, 4, 1, stdout);
                fwrite(&last, 1, 1, stdout);
                count = 1;
            }
            fclose(fp);
        }

    }
    return 0;
}

//OLD VERSION
/*while(1) {
  curr[0] = getc(fp);
//get the current character
if (curr[0] == last[0]) {
//if the current match the last character, keep counting without doing anything
count[0] += 1;
continue;
}
//if the current file is not the last character and it's not the last line,
//or if it's the last character of the last file, print the required stuff
if ((i != argc -1 && curr[0]!= EOF) || (i == argc -1 && curr[0] == EOF)){
if (count[0] > 0) {
//if there is repeated characters, print the number as well as the character.
count[0]++;
fwrite(&count, 4, 1, stdout);
fwrite(&last, 1, 1, stdout);
count[0] = 0;
} else if (curr[0] == '\n') {
//if meet an line change character, clean all the counts
last[0] = '\0';
// curr[0] = '\0';
//count[0] = 0;
continue;
} else if (last[0] != '\0') {
//if no repeat, just print the character.
fwrite(&last, 1, 1, stdout);
}


}

if (curr[0] == EOF)
//if reach the end of file, jump to the next file
break;
//mark the current character as the last one
last[0] = curr[0];
}*/




