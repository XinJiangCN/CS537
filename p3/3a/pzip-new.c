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
typedef struct _zip_arg{
    size_t size;
    char* start;
} zip_arg;
char last_char;
int last_num;
int IS_LAST_FILE;
void* proc_zip(void* args){
    zip_arg *arg = (zip_arg*)args;
    char* pointer = arg -> start;
    size_t size = arg -> size;
    char* result = malloc(10000000*sizeof(int));

    char curr;
    char last = '\0';
    int count = 1;
    int in = 0;
    for (int i = 0; i < size; i ++){
        curr = *pointer;
        if (curr == last)
            count++;
        else if (last != '\0') {
            *((int*) result + in) = count;
            in++;
            *(char*)((int*)result + in) = last;
            in++;
            count = 1;
        }

        last = curr;
        pointer++;
    }

    *((int*) result + in) = count;
    in++;
    *(char*)((int*)result + in) = last;
    in++;
    *(char*)((int*)result + in ) = '\377';

    return (void*)result;
}
void proc_print(char* args[]){
    //char* curr;
    char last_part = '\0';
    int last_count = 0;
    int IS_ONLY = 0;
    for(int i = 0; i < NUM_THREADS; i++){

        char* line = args[i];
        if (last_num != 0 && IS_ONLY == 0){
            if (*(line + 4) == last_char){
                last_count = last_num + *((int*)line);
                //line = line + 8;
                last_part = last_char;
            }
        }
        if (last_count != 0){
            if (*(line + 4) == last_part){
                if (*(line + 8) != '\377'){
                    last_count = last_count + *((int*)line);
                    fwrite(&last_count, 4, 1, stdout);
                    fwrite(&last_part, 1, 1, stdout);
                    line = line + 8;
                    last_count = 0;
                    last_part = '\0';

                } else {
                    if (IS_LAST_FILE == 1 && i == NUM_THREADS - 1){
                        last_count = last_count + *((int *) line);
                        fwrite(&last_count, 4, 1, stdout);
                        fwrite(&last_part, 1, 1, stdout);
                        break;
                    }
                    if (i != 0)
                        last_count =  last_count + *((int *) line);
                        //line = line + 8;
                    IS_ONLY = 1;
                    //if (i == NUM_THREADS - 1)
                    continue;
                }
            }

        }

        int j = 0;
        while(1){
            /*if (i == 0 && j == 0)
                break*/
            if (i == 0 && *(line + 8) == '\377');
                break;
            if (*(line+4) == '\377')
                break;

            fwrite(line, 4, 1, stdout);
            line = line + 4;

            fwrite(line, 1, 1, stdout);
            line = line + 4;
            j++;
        }
        last_count= *((int*)line);
        last_part = *(line + 4);
    }
    if(IS_ONLY == 1){
        last_num = last_count;
        last_char = last_count; 
    }
    last_num = last_count;
    last_char = last_part;
}
int main(int argc, char* argv[]){
    if (argc == 1){
        printf("pzip: file1 [file2 ...] \n");
        exit(1); 
    } 
    //Initialization
    IS_LAST_FILE = 0;
    last_char = '\0';
    last_num = 0;
    int fp;
    NUM_THREADS = get_nprocs_conf() * 10;
    NUM_FILES = argc - 1;
    pthread_t threads[NUM_THREADS];

    size_t size;
    char* data;
    for (int in = 1; in < argc; in++){
        if (in == argc - 1)
            IS_LAST_FILE = 1;
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
        /*if(data == MAP_FAILED){
            printf("map file fail");
            close(fp);
            exit(1);
        }*/
        close(fp);

        int rc;
        size_t part = size / NUM_THREADS;

        zip_arg args[NUM_THREADS];
        //char result[NUM_THREADS][1000000];
        //PTHREAD CREATE
        for (int i = 0; i < NUM_THREADS -1; i++){
            args[i] = (zip_arg){part, (data + i*part)};
            rc = pthread_create(&threads[i], NULL, proc_zip, &args[i]); assert(rc == 0);
        }
        args[NUM_THREADS - 1] = (zip_arg){(size - (NUM_THREADS - 1) * part), (data + (NUM_THREADS - 1)*part)};
        rc = pthread_create(&threads[NUM_THREADS - 1], NULL, proc_zip, &args[NUM_THREADS - 1]); assert(rc == 0);


        char* result[NUM_THREADS];

        //PTHREAD JOIN
        for (int j = 0; j < NUM_THREADS; j++) {
            if(threads[j] != 0)
                rc = pthread_join(threads[j], (void*)(result + j)); assert(rc == 0);
        }

        proc_print(result);

    }

}
