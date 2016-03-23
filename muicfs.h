/*
 * Copyright(c) 2005-2007  Alex C. Snoeren <snoeren@cs.ucsd.edu>
 *
 */

#define BLOCKSIZE 512

/* This is the name of the file you will use for your disk.
   Currently, it looks in the current directory.  You may want
   to replace this with an absolute path. */
#define DISKFILE "disk.txt"

#define INODE_OFFSET 5
#define DMAP_OFFSET 70
#define IMAP_OFFSET 1
/* You must use the following two calls to read from your "disk" */

/* Read a block from disk */
int dread(int fd, int blocknum, char *buf);

/* Write a block to disk */
int dwrite(int fd, int blocknum, char *buf);

#define DEBUG_ME printf("LINE number: %d\n", __LINE__ );
#define PRINTPTR(pt) printf("Line num %d, PTR adr is %x \n", __LINE__, pt);

typedef struct InodeStruct {
	int inode_num; //the number to identify this Inode
	int block_pt[16]; //64 byte, the block number that contain the data 
	int indir_pt; //4 byte, the block number that contain more block number
	int f_size; //total filesize
	int total_blck; //total number of block assign
	char f_type; //0 is file 1 is directory
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
} Directory;

typedef struct sblckStruct
{
  char fs_name[28];
  int root_inode_num;
  int total_inodes;
  int fs_size;
  int dblck_start;
  
}sblock;
