/*
 * Copyright(c) 2005-2010 Alex C. Snoeren <snoeren@cs.ucsd.edu>
 * 2005 Calvin Hubble
 * 2006 Kiran Tati <ktati@cs.ucsd.edu>
 *
 * This program is intended to format your disk file, and should be executed
 * BEFORE any attempt is made to mount your file system.  It will not, however
 * be called before every mount.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "muicfs.h"


int
myformat(const char *filename, int size)
{
  /* You need to fill this code in*/
  int my_fd = creat(filename, 0777);
  if (my_fd < 0){
    printf("Formatting fail\n");
    return -1;
  }
 
  /*assign imap
        IMAP will be in block number 1 and 2 
        3 and 4
        total Inode is 256 numbered from 0 to 255
        0-63 in blck 1, 64-127 in blck 2, 128-191 in
        bclk 3 and 192-255 in bclk 4
    */ 
   Bmap * MapPTR;
   Bmap * initMPTR;
    for (int i = 0; i < 4; i++){
      MapPTR = (Bmap *) malloc(sizeof(Bmap) * 64);
      initMPTR = MapPTR;
      for (int j = i * 64; j < 64 + (i * 64); j++){
        if(j == 2){
          //root Inode assign
          MapPTR->obj_num = j;
          MapPTR->alloc = 1;
        }else {
          MapPTR->obj_num = j;
          MapPTR->alloc = 0;
        }
          
          MapPTR++;
      }
      if ((dwrite(my_fd, i+IMAP_OFFSET, initMPTR)) == -1){
        printf("assigning imap fail\n");
        return -1;
      }
      free(initMPTR);
    }


    /* Assigning Inode and make root directory
    start on blck 5 to blck 69 each blck have 4 Inode 
    entries

    */
    Inode * InodePTR;
    Inode * initIPTR;
    for(int i = 0; i < 64; i++){
      InodePTR = (Inode *)malloc(sizeof(Inode) * 4);
      initIPTR = InodePTR;
      for(int j = i * 4; j < 4 + (i * 4); j++){
        if(j==2){
          //assigning root directory
          InodePTR->inode_num = j;
          InodePTR->block_pt[0] = 0;
          InodePTR->total_blck = 1;
          InodePTR->f_type = 1;
          InodePTR->f_size = 512;
        }else {
          //assigning skeleton for other Inodes
          InodePTR->inode_num = j;
        }
        InodePTR++;
      }
      if ((dwrite(my_fd, i + INODE_OFFSET, initIPTR)) == -1){
        printf("assigning Inodes fail\n");
        return -1;
      }
      free(initIPTR);
    }

     /*assign dmap 
        depend on the FS size and how many DBlocks are left over
        and will start on blck 70 onwards 
    */ 
    int total_entry = (size - (BLOCKSIZE * 70)) / 8;
    if (total_entry < 1){
      printf("Not enough space to use as FS\n");
      return -1;
    }
    int block_amt = total_entry/64;
    for(int i = 0; i < block_amt; i++){
      MapPTR = (Bmap *) malloc(sizeof(Bmap) * 64);
      initMPTR = MapPTR;
      for(int j = i * 64; j < 64 + (i * 64); j++ ){
        if(j == 0){
          //assign root dir to this block
          MapPTR->obj_num = j;
          MapPTR->alloc = 1;
        }else {
          MapPTR->obj_num = j;
          MapPTR->alloc = 0;
        }
          MapPTR++;
      }
      if((dwrite(my_fd, i + DMAP_OFFSET, initMPTR)) == -1){
        printf("assigning Dmap fail\n");
        return -1;
      }
      free(initMPTR);
    }

    //add root directory to dblock #0 starting after all the dblck
    Directory * DirPTS = (Directory *) malloc(sizeof(Directory));
    strcpy(DirPTS->f_name, ".");
    DirPTS->inode_num = 2;
    if(dwrite(my_fd, 71 + block_amt, DirPTS) == -1) {
      printf("assigning root dir fail\n");
      return -1;
    }
    free(DirPTS);

  /*
    assigning sblock
    refer to sblck struct for more details
  */
  sblock * sblockPT = (sblock *) malloc(sizeof(sblock));
  strcpy(sblockPT->fs_name, "SeaIce Cutey FS");
  sblockPT->root_inode_num = 2;
  sblockPT->total_inodes = 256;
  sblockPT->fs_size = size;
  sblockPT->dblck_start = 71 + block_amt;
  if (dwrite(my_fd, 0, sblockPT) == -1) {
    printf("assigning super block fail\n");
    return -1;
  }
  free(sblockPT);
  

  if (close(my_fd) < 0 ) {
    printf("Cannot close the file\n");
    return - 1;
  }

  return 0;
}

void usage(char *prgname) {
    printf("usage: %s diskSizeInBlockSize\n",prgname);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Invalid number of arguments \n");
        usage(argv[0]);
        return 1;
    }

    unsigned long size = atoi(argv[1]);
    printf("Formatting the %s with size %lu \n",DISKFILE, size);
    myformat(DISKFILE, size);

   return 0;
}
