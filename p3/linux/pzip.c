#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/sysinfo.h>
#include<string.h>
pthread_mutex_t mutex;
int NUM_THREADS;
int done = 0;
FILE *fps[10];
int file_num;
int file_curr;

char last = '\0';
char curr = '\0';
char *output;
int output_index = 0;
int *count_output;

int count = 1;
void* zip(void* args){
    char *str = NULL;
    size_t len = 0;
    ssize_t nread;
    int curr_file = 0;

    while (1){
        if(fps[curr_file] == NULL)
            break;

        if((nread = getline(&str, &len, fps[curr_file])) == -1){
            curr_file++;
            continue;
        }
        pthread_mutex_lock(&mutex);

        if (str[nread - 1] == '\n')
            str[nread - 1] = '\0';
        // if (last == '\0' || curr != EOF)
        //   last = *line;
        while (1){
            if(*str == '\0'){
                count_output[output_index] = count;
                output[output_index++] = last;
                output[output_index] = '\0';
                break;
            }
            curr = *str;

            if (curr == last) {
                // if the characters repeated
                count += 1;
            } else if(last != '\0'){
                //print the number as well as the character
                count_output[output_index] = count;
                output[output_index++] = last;
                output[output_index] = '\0';
                count = 1;
            }
            last = curr;
            str = str + 1;
        }
        pthread_mutex_unlock(&mutex);
        
    }
    return NULL;
}

int main(int argc, char* argv[]){
    if (argc == 1){
        printf("pzip: file1 [file2 ...] \n");
        exit(1); 
    } else {
        //Initialization
        file_num = argc -1;
        file_curr = 0;
        for (int i = 0; i < argc -1; i++){
            fps[i] = fopen(argv[i+1], "r");
            if (fps[i] == NULL){
                printf("ERROR OPENING FILE");
                exit(1);
            }
        }
        NUM_THREADS = get_nprocs();
        pthread_t threads[NUM_THREADS];
        pthread_mutex_init(&mutex, NULL);
        output = malloc(1000*sizeof(char));
        count_output = malloc(1000*sizeof(int));





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
        for (int k = 0; k < output_index; k++){
            fwrite(count_output, sizeof(int), 1, stdout);
            fwrite(output, sizeof(char), 1, stdout);
            count_output = count_output +1;
            output = output +1;
        }
        for(int i = 0; i < argc -1; i++)
            fclose(fps[i]);
        pthread_exit(NULL);
    }
}
