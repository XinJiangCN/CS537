#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "fs.h"

void perr(char* msg){
    fprintf(stderr,"%s\n", msg);
    exit(1);
}
int main(int argc, char* argv[]){
    if (argc == 1)
        perr("Usage: xcheck <file_system_image>");
    int fsfd;
    fsfd = open(argv[1], O_RDONLY);
    if (fsfd < 0)
        perr("image not found.");


    struct stat fs_stat;
    fstat(fsfd, &fs_stat);
    void* fs_ptr = mmap(NULL, fs_stat.st_size, PROT_READ, MAP_PRIVATE, fsfd, 0);
    struct superblock *sb = (struct superblock*)(fs_ptr+BSIZE);

    struct dinode *curr = (struct dinode*)(fs_ptr+2*BSIZE);
    struct dinode *first_ind = curr;
    //bit map
    int bmap[sb->size];
    int counter = 0;
    int num_blk = (sb->ninodes *sizeof(struct dinode))/BSIZE + 1;
    char* bmap_ptr = fs_ptr + (2*BSIZE) + (num_blk*BSIZE);
    //bmap[0] = 1;
    for (int i = 0; i < sb->size; i++){
        int div = 1 << counter;
        bmap[i] = div & *bmap_ptr;
        //printf("offset %d, buf %d\n", counter,bmap[i] );
        if (counter >= 7){
            counter = 0;
            bmap_ptr++;
            continue;
        }
        counter++;
    }
/*
    //block  status
    unsigned int* blk_stat = calloc(sb->size, sizeof(unsigned int));
    //used blocks
    unsigned int* blk_used = calloc(sb->size, sizeof(unsigned int));
    //used inodes
    unsigned int* ind_used = calloc(sb->ninodes, sizeof(unsigned int));
    //referred inodes
    unsigned int* ind_ref = calloc(sb->ninodes, sizeof(unsigned int));
    //referred counts
    unsigned int* ind_count = calloc(sb->ninodes, sizeof(unsigned int));
    //dir ref count
    unsigned int* dir_count = calloc(sb->ninodes, sizeof(unsigned int));
*/
    unsigned int blk_stat[sb->size];
    unsigned int blk_used[sb->size];
    unsigned int ind_used[sb->ninodes];
    unsigned int ind_ref[sb->ninodes];
    unsigned int ind_count[sb->ninodes];
    unsigned int dir_count[sb->ninodes];
    for (int i = 0; i<sb->size; i++){
        blk_stat[i] = 0;
        blk_used[i] = 0;
    }
    for (int i = 0; i < sb->ninodes; i++){
        ind_used[i] = 0;
        ind_ref[i] = 0;
        ind_count[i] = 0;
        dir_count[i] = 0;
    }
    //iterate the inodes to check
    for (int i = 0; i < sb->ninodes; i++){
        //CHECK 1
        //Each inode is either unallocated or one of the valid types 
        //(T_FILE, T_DIR, T_DEV)
        if (curr->type != 0
                && curr->type != 1
                && curr->type != 2
                && curr->type != 3)
            perr("ERROR: bad inode.");

        //CHECK 2 direct
        if(curr->type != 0){
            ind_used[i] = 1;
            for (int j = 0; j < NDIRECT+1; j++){
                //BAD DIRECT in CHECK 2
                if (curr->addrs[j] < 0 ||
                        curr->addrs[j] > 1023
                        /*TODO add exceeding boundry*/)
                    perr("ERROR: bad direct address in inode.");

                //CHECK 5
                if (bmap[curr->addrs[j]] <= 0)
                    perr("ERROR: address used by inode but marked free in bitmap.");
                blk_stat[curr->addrs[j]] = 1;

                //CHECK 7
                if (blk_used[curr->addrs[j]] == 1
                        && curr->addrs[j] != 0)
                    perr("ERROR: direct address used more than once.");
                blk_used[curr->addrs[j]] = 1;
            }

            //CHECK 2 indirect
            unsigned int* blk_ptr =(unsigned int*)(fs_ptr + (BSIZE*(curr->addrs[NDIRECT])));
            for(int j = 0; j < NINDIRECT; j++){
                if (*blk_ptr < 0 ||
                        *blk_ptr > 1023/*TODO add exceeding boundry*/)
                    perr("ERROR: bad indirect address in inode.");
                blk_stat[*blk_ptr] = 1;
                //CHECK 8
                if (blk_used[*blk_ptr] == 1
                        && *blk_ptr != 0)
                    perr("ERROR: indirect address used more than once.");
                if (bmap[*blk_ptr] <= 0)
                    perr("ERROR: address used by inode but marked free in bitmap.");
                blk_used[*blk_ptr] = 1;
                blk_ptr++;
            }
            //CHECK 3
            if (i == ROOTINO){
                if (curr->type != 1)
                    perr("ERROR: root directory does not exist.");
                struct dirent* dir = fs_ptr + (BSIZE*curr->addrs[0]);
                if (dir->inum == 1){
                    if ((dir+1)->inum != 1)
                        perr("ERROR: root directory does not exist.");
                }

            }
            if (curr->type == 1){
                struct dirent* first_dir = fs_ptr+(BSIZE*curr->addrs[0]);
                //CHECK 4
                if (strcmp(first_dir->name, "."))
                    perr("ERROR: directory not properly formatted.");
                else if (strcmp((first_dir+1)->name, ".."))
                    perr("ERROR: directory not properly formatted.");
                //Check direct 
                struct dirent* dir_curr;
                
                for (int j = 0; j < NDIRECT; j++) {
                    dir_curr = fs_ptr + (BSIZE*curr->addrs[j]);
                  
                    for (int k = 0; k < (BSIZE/sizeof(struct dirent)); k++){
                        if (dir_curr->inum != 0){
                            //printf("%d\n",dir_curr->inum);
                            ind_ref[dir_curr->inum] = 1;
                            struct dinode *n = (struct dinode*)(first_ind + dir_curr->inum);
                            if (n->type == 2)
                                ind_count[dir_curr->inum]++;
                            if (n->type == 1)
                                if (strcmp(dir_curr->name, ".") != 0
                                        && strcmp(dir_curr->name, "..") != 0)
                                    dir_count[dir_curr->inum]++;   
                        }
                        dir_curr++;
                    }

                }
           
                //check indirect
                blk_ptr =(unsigned int*)(fs_ptr + (BSIZE*(curr->addrs[NDIRECT])));
                for (int j = 0; j < NINDIRECT; j++) {
                    dir_curr = fs_ptr + ((*blk_ptr)*BSIZE);
                    for (int k = 0; k < (BSIZE/sizeof(struct dirent)); k++){
                        if (dir_curr->inum != 0){
                            ind_ref[dir_curr->inum] = 1;
                            struct dinode *n = (struct dinode*)(first_ind + dir_curr->inum);
                            if (n->type == 2)
                                ind_count[dir_curr->inum]++;
                            if (n->type == 1)
                                if (strcmp(dir_curr->name, ".") != 0
                                        && strcmp(dir_curr->name, "..") != 0)
                                    dir_count[dir_curr->inum]++;   
                        }
                        dir_curr++;
                    }
                    blk_ptr++;
                }

            }
        }
        curr++;
    }
/*for (int i = 0; i < sb->ninodes; i++){
    printf("ind_count[%d]: %d \n",i ,ind_count[i]);

}
*/
    for (int i = 0; i < sb->size; i++){
        //CHECK 6
        if (i > num_blk+2)
            if (bmap[i] > 0 && blk_stat[i] == 0)
                perr("ERROR: bitmap marks block in use but it is not in use.");
    }
    //iterate the inodes to check
    curr = first_ind;
    for (int i = 0; i < sb->ninodes; i++){
        //CHECK 9
        if (ind_used[i] == 1
                && ind_ref[i] == 0)
            perr("ERROR: inode marked use but not found in a directory.");
        //CHECK 10
        if (ind_used[i] == 0
                && ind_ref[i] == 1)
            perr("ERROR: inode referred to in directory but marked free.");
        //CHECK 11
        if (curr->type == 2
                && ind_count[i]!=curr->nlink)
            perr("ERROR: bad reference count for file.");
        //CHECK 12
        if (curr->type == 1
                && dir_count[i] >= 2)
            perr("ERROR: directory appears more than once in file system.");
        curr++;
    }
/*
    //free
    free(blk_stat);
    free(blk_used);
    free(dir_count);
    free(ind_ref);
    free(ind_used);
    free(ind_count);
    blk_stat = NULL;
    blk_used = NULL;
    dir_count = NULL;
    ind_ref = NULL;
    ind_used = NULL;
    ind_count = NULL;
    */
    exit(0);
}
