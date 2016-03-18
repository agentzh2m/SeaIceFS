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

#define DEBUG_ME printf("LINE number: %d\n", __LINE__ );
#define PRINTPTR(pt) printf("Line num %d, PTR adr is %x \n", __LINE__, pt);

int
myformat(const char *filename, int size)
{
  /* You need to fill this code in*/
  int my_fd = open(filename, O_CREAT | O_RDWR, 0x00700);
  if (my_fd < 0){
    printf("Formatting fail\n");
    return -1;
  }
  //sblock info
  /*
    byte 0-27 fs_name (char 28)
    byte 28-31 root_inode_num (int == 4 byte)
    byte 32-35 size (int == 4 byte)
    byte 36-39 total_indoes (int == 4 byte)

  */
  const char fs_name[28] = "SeaIceFS";
  int root_inode_num = 2;
  int total_inodes = 256; //can store maximum of 256 files
  

  //storing sblock info into fs image before init
  char *my_buf;
  my_buf =  (char *) malloc(64);
  char * buf_ptr = my_buf;
  for (int i = 0; i < sizeof(fs_name); i++){
    *my_buf = fs_name[i];
     my_buf++;
  }
  *my_buf = root_inode_num;
  my_buf += sizeof(int);
  *my_buf = size;
  my_buf += sizeof(int);
  *my_buf = total_inodes;
  if ( (dwrite(my_fd, 0, my_buf)) == -1) {
    printf("your disk image crashed\n");
  }
  free(buf_ptr);
  /*assign imap
        IMAP will be in block number 1 and 2 
        3 and 4
        total Inode is 256 numbered from 0 to 255
        0-63 in blck 1, 64-127 in blck 2, 128-191 in
        bclk 3 and 192-255 in bclk 4
    */ 
   Bmap * MapPTR;
   Bmap * initMPTR;
   Bmap myMap;
    //assigning blck 1
   printf("the size of Bmap is %d \n", sizeof(Bmap));
    for (int i = 0; i < 4; i++){
      MapPTR = (Bmap *) malloc(sizeof(Bmap) * 64);
      initMPTR = MapPTR;
      for (int j = i * 64; j < 64 + (i * 64); j++){
          myMap.obj_num = j;
          myMap.alloc = 0;
          *MapPTR = myMap;
          MapPTR++;
      }
      if ((dwrite(my_fd, i+1, MapPTR)) == -1){
        printf("assigning imap fail\n");
        return -1;
      }
      free(initMPTR);
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
          myMap.obj_num = j;
          myMap.alloc = 0;
          *MapPTR = myMap;
          MapPTR++;
      }
      if((dwrite(my_fd, i + 70, MapPTR)) == -1){
        printf("assigning Dmap fail\n");
        return -1;
      }
      free(initMPTR);
    }

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
