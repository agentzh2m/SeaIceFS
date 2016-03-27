/*
 * Copyright(c) 2005-2007  Alex C. Snoeren <snoeren@cs.ucsd.edu>
 *
 */

#define BLOCKSIZE 512

/* This is the name of the file you will use for your disk.
   Currently, it looks in the current directory.  You may want
   to replace this with an absolute path. */
#define DISKFILE "disk.txt"
#define MAXFILE 256
#define INODE_OFFSET (IMAP_OFFSET + 1) //after imap and sblock
#define DMAP_OFFSET (INODE_OFFSET + TOTAL_IBLCK + 1) //after sblock, imap and Inode
#define IMAP_OFFSET 1
#define DATA_OFFSET (DMAP_OFFSET + 1) //after everything

#define DIR_AMT (BLOCKSIZE/sizeof(Directory))
#define INODE_AMT (BLOCKSIZE/sizeof(Inode))

#define TOTAL_IBLCK ((MAXFILE * sizeof(Inode))/BLOCKSIZE)


/* You must use the following two calls to read from your "disk" */

/* Read a block from disk */
int dread(int fd, int blocknum, char *buf);

/* Write a block to disk */
int dwrite(int fd, int blocknum, char *buf);

#define DEBUG_ME printf("LINE number: %d\n", __LINE__ );
#define PRINTPTR(pt) printf("Line num %d, PTR adr is %x \n", __LINE__, pt);
#define RFAIL printf("Read fail at line %d \n", __LINE__);
#define WFAIL printf("Write fail at line %d \n", __LINE__);

typedef struct InodeStruct {
	int inode_num; //the number to identify this Inode
	int block_pt[16]; //64 byte, the block number that contain the data 
	int indir_pt; //4 byte, the block number that contain more block number
	int f_size; //total filesize
	int total_blck; //total number of block assign
	char f_type; //0 is file 1 is directory
	//Inode will 128 bytes
} Inode;

/*Remove the use of bitmap struct now each bitmap is 1 byte each */

typedef struct DirStruct
{
	char f_name[28];
	int inode_num;
	//each entry in dir is 32 byte therefore 1 Directory will have max of
	//16 files except if we link them together (do that later)
} Directory;

typedef struct sblckStruct
{
  char fs_name[28];
  int root_inode_num;
  int total_inodes;
  int fs_size;
  int dblck_start;
  int total_dblck;
  
}sblock;
