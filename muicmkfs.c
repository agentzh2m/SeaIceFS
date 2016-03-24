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
        BIG CHANGE!!!!
        change imap to use only 1 block 1 is alloc 0 is free
        don't use bitmap structure anymore inode num start @
        imap offset (read and then use ptr to find the thingy
    */ 
   char *MapPTR = (char*)malloc(sizeof(char) * 256);
   char *initMPTR = MapPTR;

   for(int i = 0; i < 256; i++){
	   if(i == 2){
		   *MapPTR = 1;
	   }else{
		   *MapPTR=0;
	   }
	   MapPTR++;
   }
   if ((dwrite(my_fd, IMAP_OFFSET, initMPTR)) < 0){
        printf("assigning imap fail\n");
        return -1;
   }
   free(initMPTR);

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
        and will start on blck 66 onwards
    */ 
    int total_entry = (size - (BLOCKSIZE * 70)) ;
    if (total_entry < 1){
      printf("Not enough space to use as FS\n");
      return -1;
    }
    int block_amt = total_entry/BLOCKSIZE;
    int total_blck = block_amt/ BLOCKSIZE;
    MapPTR = (char*)malloc(sizeof(char) * 512);
    initMPTR = MapPTR;
    if(total_blck < 1){
    	for(int i = 0; i < block_amt; i++){
    		*MapPTR = 0;
    		MapPTR++;
    	}
    	if(dwrite(my_fd, DMAP_OFFSET, initMPTR) < 0){
    		printf("assign dmap fail at DMAP 0 \n");
    	}
    }else{
    	for(int i = 0; i < total_blck; i++){
    		for(int j = 0; j < BLOCKSIZE; j++ ){
    			*MapPTR = 0;
    			MapPTR++;
    		}
    		if(dwrite(my_fd, DMAP_OFFSET + i, initMPTR) < 0){
    			printf("assign dmap fail at DMAP 0");
    		}
    	}

    }
    //add root directory to dblock #0 starting after all the dblck
    Directory * DirPTS = (Directory *) malloc(sizeof(Directory));
    char *Map = (char *)malloc(sizeof(char) * 512);
    if(dread(my_fd, DMAP_OFFSET, Map)){
    	printf("Read fail \n");
    	return -1;
    }
    *Map = 1; //make dmap zero alloc cated
    if(dwrite(my_fd, DMAP_OFFSET, Map)){
    	printf("Write fail \n");
    	return -1;
    }
    free(Map);
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
  strcpy(sblockPT->fs_name, "SeaIce Lovely FS");
  sblockPT->root_inode_num = 2;
  sblockPT->total_inodes = 256;
  sblockPT->fs_size = size;
  sblockPT->dblck_start = DMAP_OFFSET + block_amt;
  sblockPT->total_dblck = block_amt;
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
