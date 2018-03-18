#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/sysinfo.h>

pthread_mutex_t mutex;
int NUM_THREADS;
int done = 0;
FILE *fp;
int file_num;
int file_curr;
char* file_names[10];

char last = '\0';
char curr = '\0';
int count = 1;

// Function declearation
int open_file();
void* zip(void* args){

    pthread_mutex_lock(&mutex);
    if (fp == NULL){

        open_file();

    }


    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    pthread_mutex_unlock(&mutex);
    while (1){
        if ((nread = getline(&line, &len, fp)) == -1)
            break;
        pthread_mutex_lock(&mutex);
        if(fp == NULL)
            break;


        if (last == '\0' )
            last = getc(fp);
        while (1){
            if (*line == EOF || *line == '\0')
                break;
            if (curr == last) {
                // if the characters repeated
                count += 1;
            } else {
                //print the number as well as the character
                fwrite(&count, 4, 1, stdout);
                fwrite(&last, 1, 1, stdout);
                count = 1;
            }
            last = curr;
            line = line + 1;
        }

        pthread_mutex_unlock(&mutex);

    }

    if (curr == EOF){
        fclose(fp);
        fp = NULL;
    }
    return NULL;
}
int open_file(){

    if (file_curr <= file_num){
        fp = fopen(file_names[file_curr], "r");    
        if (fp == NULL){
            printf("ERROR OPENING FILE");
            exit(1);
        }
        file_curr++;
    }


    return 0;
}
int main(int argc, char* argv[]){
    if (argc == 1){
        printf("pzip: file1 [file2 ...] \n");
        exit(1); 
    } else {
        file_num = argc -1;
        file_curr = 1;
        for (int i = 1; i < argc + 1; i++){
            file_names[i] = argv[i];
        }
        NUM_THREADS = get_nprocs();
        pthread_t threads[NUM_THREADS];
        pthread_mutex_init(&mutex, NULL);

        int rc;
        for (int i = 0; i < NUM_THREADS; i++){
            rc = pthread_create(&threads[i], NULL, zip, (void*)&i);
            if (rc){
                printf("THREAD CREATE FAILED WITH CODE %i", rc);
                exit(1);
            }
        }
        for (int j = 0; j < NUM_THREADS; j++) {
            if(threads[j] != 0)
                if(pthread_join(threads[j], NULL) != 0)
                    printf("ERROR WAITING \n");
        }
        pthread_exit(NULL);
    }
}
