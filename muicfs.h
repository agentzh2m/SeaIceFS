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
	int * block_pt[16]; //64
	int ** indir_pt;
	int f_size;
	int total_size;
	int total_blck;
	//Inode will 128 bytes
} Inode;

typedef struct BitMapStruct 
{
	int obj_num;
	char alloc; //1 is alloc, 0 is free
	//Bitmap will be 8 byte each
	
} Bmap;

typedef struct DirStruct
{
	char f_name[28];
	int inode_num;
	//each entry in dir is 32 byte therefore 1 Direcory will have max of 
	//16 files except if we link them together (do that later)
} Direcory;
