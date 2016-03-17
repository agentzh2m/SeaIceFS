/*
 * DO NOT MODIFY THIS FILE
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "muicfs.h"

/* Read a block from disk */
int dread(int fd, int blocknum, char *buf) {
  if(lseek(fd, blocknum*BLOCKSIZE, SEEK_SET)<0)
    return -1;

  if (read(fd, buf, BLOCKSIZE) != BLOCKSIZE) {
    perror("dread");
    return -1;
  }
  return BLOCKSIZE;
}

/* Write a block to disk */
int dwrite(int fd, int blocknum, char *buf)
{
  if(lseek(fd, blocknum*BLOCKSIZE, SEEK_SET)<0)
    return -1;
  if (write(fd, buf, BLOCKSIZE) != BLOCKSIZE) {
    perror("dwrite");
    return -1;
  }
  return BLOCKSIZE;
}
