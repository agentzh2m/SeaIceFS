# SeaIceFS
the lovely stupid and wonderful filesystem
how to use easy?
	run start_fs to start in debug mode to execute in unix "./start_fs"
where you can cd into /tmp/`whoami`/test to test the filesystem
However you can do the following
	"mkdir test" and "./muicfs -d test" in the same directory
This fs is modeled after the VSFS system but with some twist 
to make it easier to be implement
you also need the FUSE Api
before running start_fs you should Make by typing make
but before "make" do the following
	1. If you are on Linux type "mv Makefile_linux Makefile"
	2. If you are on Mac type "mv Makefile_mac Makefile"

After that type "make" to make the binaries necessary to run the fs
if you are on mac you should have macfuseosx install if you are on linux
you should have libfuse-dev if not you can "apt get libfuse-dev"