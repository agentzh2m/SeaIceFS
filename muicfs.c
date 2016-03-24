/*
 *   Copyright (C) 2006-2007  Alex C. Snoeren <snoeren@cs.ucsd.edu>
 *
 *   FUSE: Filesystem in Userspace
 *   Copyright (C) 2001-2005  Miklos Szeredi <miklos@szeredi.hu>
 *
 *   This program can be distributed under the terms of the GNU GPL.
 *   See the file COPYING.
 */

#define _POSIX_C_SOURCE 199309
#define FUSE_USE_VERSION 26

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include "muicfs.h"

/**
 * Initialize filesystem
 *
 * You need the filename that you are using as disk to be passed to this
 * function, since it is not taking any parameters, hard code the file name
 * (we defined this file name in muicfs.h as DISKFILE). This function will
 * use that file as the disk image to mount the application-level filesystem.
 * You also need to open this disk file and store the file discriptor in a
 * global variable or super block to read and write from this file later (or
 * you can open this file whenever you needed and close it after that).
 * Once a mount occurs, that file will be used as the disk for all further
 * file system commands you will implement (until an xmp_unmount occurs).
 * Remember that this is an application level file system.  From the point
 * of view of a user of the application, it will appear like a structured
 * file system.  However, in reality the disk image is merely one contiguous
 * file in the operating system.
 *
 * This is also when you will need to check the integrity of your file
 * system.  For example, if a crash occurred during a file create after an
 * inode was allocated but before a directory entry is updated, such an error
 * should be found and fixed in xmp_mount.
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * xmp_unmount() method.
 *
 */
static int global_fd = -1;
static int st_block = 0;
static int my_root_num = -1;
static int total_blck = -1;

static void* xmp_mount(struct fuse_conn_info *conn) {
    global_fd = open(DISKFILE, O_RDWR);
    if (global_fd < 0){
        printf("Mount fail Sea is not happy!!\n");
        return -1;
    }
    //read and retreive superblock 
    sblock * s_holder = (sblock * ) malloc(sizeof(sblock));
    if((dread(global_fd, 0, s_holder)) == -1) {
        printf("Reading sblock fail maybe Sea don't like the name\n");
    }

    printf("-------Filesystem image information-------\n");
    printf("FS Name: %s\n", s_holder->fs_name);
    printf("FS size: %d\n", s_holder->fs_size);
    printf("Data Block Start At Block number: %d \n", s_holder-> dblck_start);
    st_block = s_holder-> dblck_start;
    my_root_num = s_holder -> root_inode_num;
    total_blck = s_holder->total_dblck;
    free(s_holder);
    return 1;
    

    /*inode
        will have a total of 64 blocks to accomodate maximum of 256 files 
        no need to initialize but will accomodate blck 5 to 69   
    */

}

/* return Inode number if the file exist in the data
    blck (if it is a directory) if not return -1  */
static int search_dir(char* filename, int data_blck){
    Directory* dir_buf = (Directory*) malloc(sizeof(Directory) * 16);
    Directory* init_buf = dir_buf;
    if(dread(global_fd, data_blck + st_block, dir_buf) < 0){
    	RFAIL;
    }
    for (int i = 0; i < 16; i++){
    	printf("Searching through dir of %s, chk %s right now \n", filename, dir_buf->f_name);
        if(!strcmp(filename, dir_buf->f_name)){
        	free(init_buf);
            return dir_buf->inode_num;
        }
        dir_buf++;
    }
    //free(init_buf);
    return -1;
}

/* Find free bitmap and return the free inode num or data blck num */
static int assign_bitmap(int offset){
	char *my_bbuf = (char*)malloc(sizeof(char) * 512);
	char *init_bbuf = my_bbuf;
	int free_obj = -1;
	//search for the free block
	if (offset == IMAP_OFFSET){
		if(dread(global_fd, IMAP_OFFSET, my_bbuf) < 0){
			printf("Read data blck fail at %d \n", IMAP_OFFSET);
			free(init_bbuf);
			return -1;
		}
		for(int i = 0; i < BLOCKSIZE; i++){
			if(!*my_bbuf){
				*my_bbuf = 1;
				free(init_bbuf);
				return i;
			}
			my_bbuf++;
		}
	}else {
		for(int i = 0; i < total_blck; i++){
			if(dread(global_fd, DMAP_OFFSET + i, init_bbuf) < 0){
				printf("Read data blck fail at %d \n", i);
				return -1;
			}
			for(int j = 0; j < BLOCKSIZE; j++){
				if(!*my_bbuf){
					*my_bbuf = 1;
					free(init_bbuf);
					return (i * BLOCKSIZE) + j;
				}
				my_bbuf++;
			}
		}
	}
	return -1;
}
/* return 0 if create file successful if return 0 or more return inode number that
 * store the newly created directory */
int static create_dir_file(char* path, int type){
	 char* tok_pt;
	    int inum = -1;
	    int datanum = 0;

	    int s_counter = 0;
	    int t_counter = 0;

	    //find total slash
	    for(int i = 0; i < strlen(path); i++){
	    	if(path[i] == '/'){
	    		s_counter++;
	    	}
	    }

	    for(tok_pt = strtok(path, "/"); tok_pt!=NULL; tok_pt = strtok("NULL", "/")){
	    	//traversing the dir to get dir before
	    	if((inum = search_dir(tok_pt, datanum)) < 0){
	    		printf("Cannot find dir in inode %s \n" ,tok_pt);
	    	}
	    	Inode *ibuf = (Inode*)malloc(sizeof(Inode) * 4);
	    	Inode *init_buf = ibuf;
	    	if(dread(global_fd, INODE_OFFSET+ inum/4, ibuf)){
	    		printf("Read inode fail at %d \n", inum);
	    	}
	    	ibuf+= inum%4;
	    	if(ibuf->f_type){
	    		datanum = ibuf->block_pt[0];
	    	}else{
	    		printf("Not a directory \n");
	    		return -1;
	    	}
	    	//find dir before the file and assign last val to tokpt
	    	if(t_counter>=s_counter-1){
	    		tok_pt = strtok("NULL", "/");
	    	    break;
	    	}
	    }

	    //find free_imap
	    int free_inode = assign_bitmap(IMAP_OFFSET);

	    //pull the correct inode num
	    Inode *ibuf = (Inode*)malloc(sizeof(Inode) * 4);
	    Inode *init_ibuf = ibuf;
	    if (dread(global_fd, INODE_OFFSET + free_inode/4, ibuf) < 0){
	    	 printf("Read the inode @ %d \n fail", free_inode);
	    }
	    ibuf+= free_inode%4;
	    //assign inode values
	    ibuf->total_blck = 0;
	    if(type){
	    	ibuf->f_type = 1;
	    }else{
	    	ibuf->f_type = 0;
	    }
	    if(free_inode != ibuf->inode_num){
	    	printf("Incorrect inode num assignment free_inode: %d, inode_num: %d", free_inode, ibuf->inode_num);
	    }

	    //update the directory
	    Directory *dbuf = (Directory*)malloc(sizeof(Directory) * 16);
	    Directory *init_dbuf = dbuf;

	    if(dread(global_fd, st_block + datanum, init_dbuf) < 0){
	        	RFAIL;
	        	return -1;
	     }

	     for(int i = 0; i < 16; i++){
	    	 if(strlen(dbuf->f_name) < 1){
	    		 strcpy(dbuf->f_name, tok_pt);
	    		 dbuf->inode_num = free_inode;
	    		 break;
	    	 }
	    	 dbuf++;
	     }

	    if(dwrite(global_fd, st_block + datanum, init_dbuf) < 0){
	    	printf("Write the directory entry");
	    }

	    if(type){
	    	 //assign free data block to host dir
	    	    int free_dmap = assign_bitmap(DMAP_OFFSET);
	    	    Inode *ibuf = (Inode*)malloc(sizeof(Inode) * 16);
	    	    Inode *init_ibuf = ibuf;
	    	    if(dread(global_fd, INODE_OFFSET + inum/4, ibuf) < 0){
	    	    	RFAIL;
	    	    	return -1;
	    	    }
	    	    ibuf += inum%4;
	    	    ibuf ->block_pt[0] = free_dmap;
	    	    ibuf->total_blck = 1;
	    	    ibuf->f_size = 512;

	    	    //write Inode and free
	    	    Directory *dirbuf = (Directory *)malloc(sizeof(Directory) * 16);
	    	    Directory *init_dirbuf = dirbuf;
	    	    strcpy(dirbuf->f_name, ".");
	    	    dirbuf ->inode_num = free_inode;
	    	    dirbuf++;
	    	    strcpy(dirbuf->f_name, "..");
	    	    dirbuf->inode_num = inum;
	    	    if(dwrite(global_fd, INODE_OFFSET + inum/4, dirbuf) < 0){
	    	    	WFAIL;
	    	    	return -1;
	    	    }
	    	    return 0;
	    }else{
	    	return 0;
	    }

}



/*
 *
 * Unmount is responsible for unmounting the file system. In addition to
 * closing the disk image file descriptor, unmount will need to write out
 * any necessary meta data that might be required the next time the file
 * system is mounted. For instance, it might note that the filesystem was
 * cleanly unmounted, speeding up the integrity check the next time the
 * file system is mounted.
 *
 */
static void xmp_unmount (void *private_data) {
    if (close(global_fd) < 0) {
        printf("Mount fail because Sea is angry\n");
        //return -1;
    }
    printf("unmount successful\n");
}

/**
 *
 * Given an absolute path to a file/directory
 * (i.e., /foo/bar ---all paths will start with the root directory, "/"),
 * you need to return the file attributes that is similar stat system call.
 *
 * However you don't need to fill all the attributes except file size,
 * file size in blocks and block size of file system.
 *
 * You can ignore the 'st_dev', 'st_blksize' and 'st_ino' fields
 * (FUSE ignore these values and for more information look into fuse.h file).
 *
 * For st_mode you have to send the S_IFDIR if the entry is directory
 * or S_IFREG if it is a simple file
 *
 * Since we are not implementing permissions,
 * we pass fixed values to st_uid (5), st_gid(500) entries.
 *
 * We are also not implementing hard links,
 * so set the st_nlinks to 3 (just as safe side).
 *
 * Its up to you to implemet the time fields (st_atime, st_mtime, st_ctime),
 * if didn't impement them just return current time.
 *
 * The only thing you would be returning correctly is the file size and
 * size in blocks (st_size, st_blocks) and file system block size (st_blksize)
 *
 * The following are common steps to implement this function:
 * 1. Resolve the directory path -- this
 *    will probably involve reading each directory in the path to get
 *    the location of the file entry (or location of data block with
 *    the file's inode) of the specified file.
 *
 * 2. Fill the file size, file size in blocks, block size information
 *
 * You should return 0 on success or -1 on error (e.g., the path doesn't
 * exist).
 */
static int xmp_getattr(const char *path, struct stat *stbuf)
{
    /*
     * I am setting some pluasible values but you can change the following to
     * other values.
     */
    stbuf->st_nlink = 3;
    stbuf->st_uid   = 5;
    stbuf->st_gid   = 500;
    stbuf->st_rdev  = 0;
    stbuf->st_atime = time(NULL);
    stbuf->st_mtime = time(NULL);
    stbuf->st_ctime = time(NULL);
    stbuf-> st_blksize = BLOCKSIZE;

    /* you have to implement the following fields correctly

    if (The path represents the directory)
        stbuf->st_mode  = 0777 | S_IFDIR;
    else
        stbuf->st_mode  = 0777 | S_IFREG;

    stbuf->st_size    =  // file size
    stbuf->st_blocks  =  // file size in blocks
    stbuf->st_blksize =  // block size of you file system

    */
    char *tok_pt;
    int inum = -1;
    int datanum = 0; //root at data blck zero

    //if root dir
    if(strlen(path) < 2){
    	Inode *root_data = (Inode*) malloc(sizeof(Inode) * 4);
    	Inode *initptr = root_data;

    	if(dread(global_fd, INODE_OFFSET, root_data) < 0){
    		printf("Read fail at Line %d \n",__LINE__);
    	}

    	root_data+=2;
    	stbuf->st_size = root_data->f_size;
    	stbuf->st_blocks = root_data->total_blck;
    	stbuf->st_blksize = 512;
    	stbuf->st_mode = 0777 | S_IFDIR;
    	datanum = root_data->block_pt[0];
    	//free(initptr);
    	printf("finish fetching from root dir \n");
    	return 0;

    }else { //if dir is not root or some other dir
    	DEBUG_ME;
    	for(tok_pt = strtok("path", "/"); tok_pt!=NULL; tok_pt=(NULL, "/")){
    		printf("going through file %s of the path: %s \n", tok_pt, path);
    		Directory *my_dir = (Directory*)malloc(sizeof(Directory) * 16);
    		Directory *init_dir = my_dir;
    		printf("Reading at blck %d \n", st_block + datanum);
    		if(dread(global_fd, st_block + datanum, my_dir) < 0){
    			printf("Read fail at datablck %d \n", datanum);
    			return -1;
    		}
    		//search the directory
    		if((inum = search_dir(path, datanum)) < 0){
    			printf("Unable to find the file in dir\n");
    			return -2;
    		}

    		Inode *ibuf = (Inode*)malloc(sizeof(Inode) * 64);
    		Inode *init_ibuf = ibuf;
    		if(dread(global_fd, INODE_OFFSET + inum, ibuf)){
    			printf("Read inode fail at inum: %d, @Line: %d \n", inum, __LINE__ );
    			return -1;
   			}
   			stbuf->st_size = ibuf->f_size;
   			stbuf->st_blocks = ibuf->total_blck;
   			stbuf->st_blksize = 512;
   			if(ibuf->f_type){
    			stbuf->st_mode = 0777 | S_IFDIR;
    			datanum = ibuf->block_pt[0];
   			}else {
    				stbuf->st_mode = 0777 | S_IFREG;
    			}
    		}
    	}
    printf("finish fetching from %s \n", path);
    return 0;
}

/**
 * Given an absolute path to a directory (which may or may not end in
 * '/'), xmp_mkdir will create a new directory named dirname in that
 * directory. Ignore the mode parameter as we are not implementing
 * the permissions. The steps your implementation will do to make a new
 * directory will be the following:
 *
 * 1. Resolve the directory path
 *
 * 2. Allocate and initialize a new directory block
 *
 * 3. Update the directory block for 'path' by adding an entry for
 *    the newly created directory.
 *
 * 4. You also need to create appropriate entries for "." and ".." in the
 *    New directory
 *
 * You should return 0 on success or -1 on error (e.g., the path doesn't
 * exist, or dirname already does).
 *
 */
static int xmp_mkdir(const char *path, mode_t mode)
{
   return create_dir_file(path, 1);
}

/** Read directory
 *
 * Given an absolute path to a directory, xmp_readdir will return
 * all the files and directories in that directory. The steps you
 * will take will be the following:
 *
 * 1. Resolve the directory entry from its absolute path to get
 *    the address of the directory block that corresponds to "path"
 *
 * 2. For each valid entry in the directory block, copy the file
 *    name into an array (buf) using the function filler. The filler
 *    is already implemeted in fuse and its sample use is shown in
 *    fusexmp.c file and you should pass the zero to the offest
 *    paramter of filler function. fuse.h file has some more infomration
 *    about how to implement this function and another way of implementing it.
 *
 * For time being ignore the fuse_file_info parameter. If you would
 * like to use please see the open function in fuse.h file for more
 * detials.
 *
 * You should return 0 on success or -1 on error (e.g., the directory
 * does not exist).
 */
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi)
{
	printf("Reading dir from path %s" ,path);
    //find root dir in the inodes
    char *tok_pt;
    int inum;
    int datanum = 0;
    int valid_dir_data = -1;
    Directory *dir_buf = (Directory *)malloc(sizeof(Directory) * 16);
    Directory *init_dbuf = dir_buf;

    if(strlen(path) < 2){ //if path is root
    	printf("Starting to read from root dir ");
    	if(dread(global_fd, st_block + datanum, init_dbuf) < 0){
    		printf("Read fail at blck %d, @ line num: %d \n", st_block + datanum, __LINE__);
    		return -1;
    	}
    	printf("Adding dir entry \n");
    	for(int i = 0; i < 16; i++){
    		if(strlen(dir_buf->f_name) > 0){
    			filler(buf, dir_buf->f_name, NULL, 0);
    		}
    	}
    }else{ //if path is not root
    	Inode *ibuf = (Inode*)malloc(sizeof(Inode) * 4);
    	Inode *init_ibuf = ibuf;
    	for(tok_pt = strtok(path, "/"); tok_pt!=NULL; tok_pt=strtok(NULL, "/")){
    		//read inode to find
    		if((inum = search_dir(path, datanum)) < 0){
    			printf("Cannot find path in dir \n");
    			return -1;
    		}
    		if(dread(global_fd, INODE_OFFSET + inum/4, ibuf) < 0){
    			printf("Read fail at inode num %d: @ Line: %d \n", inum, __LINE__);
    			return -1;
    		}
    		ibuf+=inum%4;
    		datanum = ibuf->block_pt[0];
    	}
    	if(dread(global_fd, st_block + datanum, init_dbuf)){
    		printf("Read fail at data blck %d @ Line: %d \n", datanum, __LINE__);
    		return -1;
    	}
    	printf("Adding dir entry \n");
    	for(int i = 0; i < 16; i++){
    		if(strlen(dir_buf->f_name) > 0){
    			filler(buf, dir_buf->f_name, NULL, 0);
    		}
    	}
    	//free(init_ibuf);
    }
    //free(init_dbuf);
    return 0;
}

/*
 * Given an absolute path to a file (for example /a/b/myFile), xmp_create
 * will create a new file named myFile in the /a/b directory.
 * Ignore the mode parameter as we are not implementing
 * the permissions. Also ignore the rdev parameter.
 * The steps your implementation will do to make a new
 * file will be the following: (similar to the xmp_mkdir except the foruth step)
 *
 * 1. Resolve the directory path
 *
 * 2. Allocate and initialize a new directory entry
 *
 * 3. Update the directory entry for 'path' by adding an entry for
 *    the newly created file.
 *
 *
 * You should return 0 on success or -1 on error (e.g., the path doesn't
 * exist, or file name already exists).
 */
static int xmp_create(const char *path, mode_t mode, dev_t rdev)
{
	return create_dir_file(path, 0);

}

/**
 * The function xmp_read provides the ability to read data from
 * an absolute path 'path,' which should specify an existing file.
 * It will attempt to read 'size' bytes from the specified file
 * on your filesystem into the memory address 'buf'. The return
 * value is the amount of bytes actually read; if the file is
 * smaller than size, xmp_read will simply return the most amount
 * of bytes it could read.
 *
 * For time being ignore the fuse_file_info parameter. If you would
 * like to use please see the open function in fuse.h file for more
 * detials.
 *
 * On error, xmp_read will return -1.  The actual implementation
 * of xmp_read is dependent on how files are allocated.
 *
 */
static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{
    return 0;
}

/**
 * The function xmp_write will attempt to write 'size' bytes from
 * memory address 'buf' into a file specified by an absolute 'path'
 *
 * For time being ignore the fuse_file_info parameter. If you would
 * like to use please see the open function in fuse.h file for more
 * detials.
 *
 * On error (e.g., the path does not exist) xmp_write will return -1,
 * otherwise xmp_write should return the number of bytes written.
 *
 * Note size is not necessarily an integral number of blocks.
 * Similar to xmp_read, the actual implementation of xmp_write will
 * depend on how you decide to allocate file space.
 *
 */
static int xmp_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
    return 0;
}

/**
 * This function deletes the last component of the path (e.g., /a/b/c you
 * need to remove the file 'c' from the directory /a/b). The steps you
 * will take will be the following:
 *
 * 1. Locate the directory entry for the file by resolving the path - this
 *    will probably involve reading each directory in the path to get
 *    the location of the first data block (or location of data block with
 *    the file's inode) of the specified file.
 *
 * 2. Update the directory entry for the deleted file
 *
 * 3. Free any data blocks used by the file
 *
 * Again, these steps are very general and the actual logistics of how
 * to locate data blocks of a file, how to free data blocks and how to update
 * directory entries are dependent on your implementation of directories
 * and file allocation.
 *
 * For time being ignore the fuse_file_info parameter. If you would
 * like to use please see the open function in fuse.h file for more
 * detials.
 *
 * On error (e.g., the path does not exist) xmp_delete will return -1,
 * otherwise xmp_delete should return the number of bytes written.
 *
 */
static int xmp_delete(const char *path)
{
    return 0;
}

/**
 * The function rename will rename a file or directory named by the
 * string 'oldpath' and rename it to the file name specified by 'newpath'.
 *
 * As usual, return 0 on success, -1 on failure.
 *
 */
static int xmp_rename(const char *from, const char *to)
{
    return 0;
}

static struct fuse_operations xmp_oper = {
    .init    = xmp_mount,
    .destroy = xmp_unmount,
    .getattr = xmp_getattr,
    .mkdir	 = xmp_mkdir,
    .readdir = xmp_readdir,
    .mknod	 = xmp_create,
    .read	 = xmp_read,
    .write	 = xmp_write,
    .unlink	 = xmp_delete,
    .rename	 = xmp_rename,
};

int main(int argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
