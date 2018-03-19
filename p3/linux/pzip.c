#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/sysinfo.h>
#include<string.h>
#include<assert.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/mman.h>

static int NUM_THREADS;
static int NUM_FILES;
char ***output;
int ***count_output;
char* final_result;
int* final_count;
typedef struct _zip_arg {
    size_t size;
    char* start;
    int thread_num;
    int file_num;
} zip_arg;
void* proc_zip(void* args){
    zip_arg *arg = (zip_arg*)args;
    char* pointer = arg -> start;
    size_t size = arg -> size;
    int th = arg -> thread_num;
    int fn = arg -> file_num;
    char curr;
    char last = '\0';
    int count = 1;
    int in = 0;
    for(int i = 0 ; i < size; i++){
        curr = *pointer;
        /*if (curr == '\n'){
            pointer++;
            continue;
        }*/
        if (curr == last){
            count++;
        } else if (last != '\0'){
            count_output[fn][th][in] = count;
            output[fn][th][in++] = last;
            count = 1;
        }
        last = curr;
        pointer++;
    }

    count_output[fn][th][in] = count;
    output[fn][th][in++] = last;
    output[fn][th][in] = '\0';

    return NULL;
}
int proc_result(){
    char last = '\0';
    char *curr = '\0';
    int length = 0;
    int final_index = 0;
    int * count =(int*) malloc(1000000*sizeof(int));
    char* ch = (char*) malloc(1000000*sizeof(char));
    int ch_counter = 0;
    for (int k = 0; k < NUM_FILES; k++){
        for (int j = 0; j < NUM_THREADS; j++){
            for(int i = 0; i < sizeof(output[k][j]); i++){
                if (output[k][j][i] == '\0')
                    break;
                count[ch_counter] = count_output[k][j][i];
                ch[ch_counter++] = output[k][j][i];

            }
        }
    }
    ch[ch_counter] = '\0';
    if(ch_counter == 1){
        final_result[0] = ch[0];
        final_count[0] = count[0];
        return 1;
    }
    for(int i = 0; i < (ch_counter - 1); i++){
        curr = &(ch[i]);
        int num = 0;
        char *ch1 = curr;
        int repeat_count = 0;
        while(1){
            if (last == '\0')
                break;
            if(*ch1 == '\0'){
                repeat_count += count[i+num - 1];

                break;
            }
            if (last == *ch1){
                repeat_count += count[i -1 + num];
                num++;
            } else {
                repeat_count += count[i+num - 1];
                break;
            }
            ch1++;
        }
        if(last != '\0') {
            final_result[final_index] = last;
            final_count[final_index++] = repeat_count;
            length++;
            if (num > 1)
                i = i + num - 1;
        }
        last = *curr;
    }
    /*
    int final;
    for(int j = 0; j < (ch_counter - 1); j++){
        if (ch[j] == '\0'){
            final = j -1;
        }

    }
    final_result[final_index] = last;
    final_count[final_index++] = count[final];
    final_result[final_index] = '\0';
    length++;
*/
    return length;

}
int main(int argc, char* argv[]){
    if (argc == 1){
        printf("pzip: file1 [file2 ...] \n");
        exit(1); 
    } 
    //Initialization
    int fp;
    NUM_THREADS = get_nprocs_conf() * 10;
    NUM_FILES = argc - 1;
    pthread_t threads[NUM_THREADS];
    output = (char***)malloc(sizeof(char**) * NUM_FILES);
    count_output = (int ***)malloc(sizeof(int **) * NUM_FILES);
    for(int j = 0; j<NUM_FILES;j++){
        output[j] = (char**)malloc(NUM_THREADS*sizeof(char*));
        count_output[j] = (int**)malloc(NUM_THREADS*sizeof(int*));
        for (int i = 0; i < NUM_THREADS; i++){
            output[j][i] = (char*)malloc(100000000*sizeof(char));
            count_output[j][i] = (int*)malloc(100000000*sizeof(int));
        }

    }
    final_result = (char*)malloc(100000000*sizeof(char));
    final_count = (int*)malloc(100000000*sizeof(int));



    size_t size;
    char* data;
    zip_arg arguments[NUM_THREADS];
    for (int in = 1; in < argc; in++){
        struct stat file_st;

        if((fp = open(argv[in], O_RDONLY, 0)) == -1){
            printf("FILE OPEN FAILED");
            exit(1);
        }

        if(fstat(fp,&file_st)==-1) {
            printf("get file stat fail");
            close(fp);
            exit(1);
        }
        size = file_st.st_size;
        data = (char*)mmap(NULL, size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fp, 0);
        if(data == MAP_FAILED){
            printf("map file fail");
            close(fp);
            exit(1);
        }
        close(fp);
        int rc;
        size_t part = size / NUM_THREADS;
        for (int i = 0; i < NUM_THREADS -1; i++){
            arguments[i] = (zip_arg){part, (data + i*part), i, in - 1};
            rc = pthread_create(&threads[i], NULL, proc_zip, &arguments[i]); assert(rc == 0);
        }

        arguments[NUM_THREADS - 1] = (zip_arg){(size - (NUM_THREADS - 1) * part), (data + (NUM_THREADS - 1)*part), (NUM_THREADS - 1), in - 1};
        rc = pthread_create(&threads[NUM_THREADS - 1], NULL, proc_zip, &arguments[NUM_THREADS - 1]); assert(rc == 0);

        for (int j = 0; j < NUM_THREADS; j++) {
            if(threads[j] != 0)
                rc = pthread_join(threads[j], NULL); assert(rc == 0);
        }
    }
    int length = proc_result();
    for( int j = 0; j < length; j++){
        fwrite(final_count, sizeof(int), 1, stdout);
        fwrite(final_result, sizeof(char), 1, stdout);
        final_count++;
        final_result++;         
    }

    //pthread_exit(NULL);
    return 0;
}
