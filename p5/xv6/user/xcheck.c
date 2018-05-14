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

int fsfd;
int fs_sz;

void perr(char* msg){
    fprintf(stderr,"%s\n", msg);
    exit(1);
}
int main(int argc, char* argv[]){
    if (argc == 1)
        perr("Usage: xcheck <file_system_image>");
    fsfd = open(argv[1], O_RDONLY);
    if (fsfd < 0)
        perr("image not found.");

    void* fs_ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    struct superblock *sb = (superblock*)(fs_ptr+BSIZE);

    struct dinode *curr = (dinode*)(fs_ptr+2*BSIZE);
    struct dinode *first_ind = curr;
    //bit map
    int bmap[sb->size];
    int counter = 1;
    int num_blk = (sb->ninodes / sizeof(struct dinode))/BSIZE + 1;
    char* bmap_ptr = fs_ptr + (2*BSIZE + num_blk*BSIZE);
    for (int i = 0; i < sb->size; i++){
        bmap[i] = counter & *bmap_ptr;
        counter << 1;
        if (counter > 7){
            counter = 1;
            bmap_ptr++;
        }
    }

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
    //iterate the inodes to check
    for (int i = 0; i < sb->ninodes; i++){
        //CHECK 1
        //Each inode is either unallocated or one of the valid types 
        //(T_FILE, T_DIR, T_DEV)
        if (curr->type != 0
                && curr->type != T_FILE
                && curr->type !=  T_DIR
                && curr->type != T_DEV)
            perr("ERROR: bad inode.");

        //CHECK 2 direct
        if(curr->type != 0){
            ind_used[i] = 1;
            for (int j = 0; j < NDIRECT+1; j++){
                //BAD DIRECT in CHECK 2
                if (curr->addrs[j] < 0
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
                blk_used[curr->addrs[j]] == 1;
            }
        }
        //CHECK 2 indirect
        unsigned int* blk_ptr =(unsigned int*)(fs_ptr + (BSIZE*(curr->addrs[NDIRECT])));
        for(int j = 0; j < NINDIRECT; j++){
            if (*blk_ptr < 0
                    /*TODO add exceeding boundry*/)
                perr("ERROR: bad indirect address in inode");

            blk_stat[*blk_ptr] = 1;
            //CHECK 8
            if (blk_used[*blk_ptr] == 1
                    && *blk_ptr != 0)
                perr("RROR: indirect address used more than once.");
            blk_used[*blk_ptr] = 1;
            blk_ptr++;
        }
        //CHECK 3
        if (i == ROOTINO){
            if (curr->type != T_DIR)
                perr("ERROR: root directory does not exist.");
            //TODO CHECK THE PARENT OF ROOT
        }
        if (curr->type = T_DIR){
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
                if (dir_curr->inum != 0)
                    ind_ref[dir_curr->inum] = 1;

                for (int k = 0; k < (BSIZE/sizeof(struct dirent)); k++){
                    if (dir_curr->inum != 0){
                        struct dinode *n = (struct dinode*)(first_ind + dir_curr->inum);
                        if (n->type == T_FILE)
                            ind_count[dir_curr->inum]++;
                        if (n->type == T_DIR)
                            if (strcmp(dir_curr->name, ".") != 0
                                    && strcmp(dir_curr->name, "..") != 0)
                                dir_count[dir_curr->inum]++;   
                        
                    }
                    dir_curr++;
                }

            }

            //check indirect
            blk_ptr =(unsigned int*)(fs_ptr + (BSIZE*(curr->addrs[NDIRECT])));

            for (int j = 0; j < NDIRECT; j++) {
                dir_curr = fs_ptr + ((*blk_ptr)*BSIZE);
                if (dir_curr->inum != 0)
                    ind_ref[dir_curr->inum] = 1;
                for (int k = 0; k < (BSIZE/sizeof(struct dirent)); k++){
                    if (dir_curr->inum != 0){
                        struct dinode *n = (struct dinode*)(first_ind + dir_curr->inum);
                        if (n->type == T_FILE)
                            ind_count[dir_curr->inum]++;
                         if (n->type == T_DIR)
                            if (strcmp(dir_curr->name, ".") != 0
                                    && strcmp(dir_curr->name, "..") != 0)
                                dir_count[dir_curr->inum]++;   

                    }
                    dir_curr++;
                }

            }

        }








        curr++
    }

    //iterate the bitmap to check
    for (int i = 0; i < sb->size; i++){   
        //CHECK 6
        if (bmap[i] > 0 && blk_stat[i] == 0)
            perr("ERROR: bitmap marks block in use but it is not in use.");

    }
    //iterate the inodes to check
    curr = first_ind;
    for (int i = 0; i < sb->size; i++){
        //CHECK 9
        if (ind_used[i] == 1
                && ind_ref[i] == 0)
            perr("ERROR: inode marked use but not found in a directory.");
        //CHECK 10
        if (ind_used[i] == 0
                && ind_ref == 1)
            perr("ERROR: inode referred to in directory but marked free.");
        if (curr->type = T_FILE
                && ind_count[i]!=curr->nlink)
            perr("ERROR: bad reference count for file.");
        if (curr->type = T_DIR
                && dir_count[i] >= 2)
            perr("ERROR: directory appears more than once in file system.");
        curr++;
    }

    //free
    free(blk_stat);
    free(blk_used);
    return 0;
}
