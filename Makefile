# Copyright (C) 2007-10 Alex C. Snoeren <snoeren@cs.ucsd.edu>
# Root for OSXFUSE includes and libraries
OSXFUSE_ROOT = /usr/local
#OSXFUSE_ROOT = /opt/local

INCLUDE_DIR = $(OSXFUSE_ROOT)/include/osxfuse/fuse
LIBRARY_DIR = $(OSXFUSE_ROOT)/lib

CC ?= gcc

CFLAGS_OSXFUSE = -I$(INCLUDE_DIR) -L$(LIBRARY_DIR)
CFLAGS_OSXFUSE += -DFUSE_USE_VERSION=26
CFLAGS_OSXFUSE += -D_FILE_OFFSET_BITS=64
CFLAGS_OSXFUSE += -D_DARWIN_USE_64_BIT_INODE

CFLAGS_EXTRA = -Wall $(CFLAGS)

LIBS = -losxfuse

.c:
	$(CC) $(CFLAGS_OSXFUSE) $(CFLAGS_EXTRA) -o $@ $< $(LIBS)

all:	rawdisk.o muicfs muicmkfs

muicfs:		muicfs.c
#	gcc $(CFLAGS_OSXFUSE) $(CFLAGS_EXTRA) $< $(LIBS) -lfuse -D_FILE_OFFSET_BITS=64 -o $@
	gcc $(CFLAGS_OSXFUSE) $(CFLAGS_EXTRA) $< $(LIBS) -o $@ rawdisk.o

muicmkfs:	muicmkfs.c
	gcc $< -o muicmkfs rawdisk.o

rawdisk.o:	rawdisk.c
	gcc -c -o rawdisk.o rawdisk.c

clean:
	rm -f muicfs muicmkfs rawdisk.o
