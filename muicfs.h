/*
 * Copyright(c) 2005-2007  Alex C. Snoeren <snoeren@cs.ucsd.edu>
 *
 */

#define BLOCKSIZE 512

/* This is the name of the file you will use for your disk.
   Currently, it looks in the current directory.  You may want
   to replace this with an absolute path. */
#define DISKFILE "disk.txt"

/* You must use the following two calls to read from your "disk" */

/* Read a block from disk */
int dread(int fd, int blocknum, char *buf);

/* Write a block to disk */
int dwrite(int fd, int blocknum, char *buf);

typedef struct InodeStruct {
	char f_name[28];
	int * block_pt[16];
	int ** indir_pt;

} Inode;

typedef struct BitMapStruct 
{
	int obj_num;
	char alloc;
	
} Bmap;
