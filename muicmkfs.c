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
  int my_fd = open(filename, O_WRONLY | O_TRUNC);
  if (my_fd < 0){
    return -1;
  }
  //sblock info
  const char fs_name[28] = "SeaIceFS";
  int root_inode_num = 2;
  int total_inodes = 256 //can store maximum of 256 files
  

  //storing sblock info into fs image before init
  char *my_buf =  (char *) malloc(64);
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
  free(my_buf);

  if (close(my_fd) < 0 ) {
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
