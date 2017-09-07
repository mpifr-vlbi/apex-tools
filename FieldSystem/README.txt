Directories here
=======================================

st/antcn : sends track() commands with source coordinates from FieldSystem over to APECS

st/apecs : some general UDO "connection" helpers to "connect" to the APECS system


Field System on 64-bit machines (amd64)
=======================================

The field system normally runs on 32-bit systems. There are two ways to run
FS on 64-bit machines:
1)  Install Oracle VM and install a 32-bit virtual PC. Runs slow, and can
    be starved of execution time.
2)  Debian and Ubuntu are "multiarch" since end of 2012: run gcc with "-m32" flag
    and load some 32-bit libraries.

Option 2 looks better. This means changing many Makefiles.

The script 'mk64.pl' automates Makefile edit. Navigate to (for instance) /usr2/fs-9.11.7
and run script. It reads all files matching */*akefile , adds '-m32' and other
flags and rewrites in place.

Install modified f77 (calls f2c and gcc) which accepts -m32 directive.
Make sure flex and bison are present.

Various 32-bit libraries:
dpkg --add-architecture i386
apt-get update
apt-get install libc6-i386
apt-get install libc6-dev-i386

Install 32-bit f2c (this will probably zap 64-bit f2c, doesn't matter)
Install 32-bit libfl.a (needed for flex)
For instance the  libraries for Ubuntu 14.10 can be found at:
http://launchpadlibrarian.net/174898048/libfl-dev_2.5.39-6_i386.deb
http://launchpadlibrarian.net/103690191/f2c_20100827-1_i386.deb
Then two architectures are visible, for instance:
-rw-r--r-- 1 root root 2020 Sep 17 14:08 /usr/lib/i386-linux-gnu/libfl.a
-rw-r--r-- 1 root root 2804 Jun 21  2014 /usr/lib/x86_64-linux-gnu/libfl.a

Call make. Sometimes 'oprin/readline' directory fails to compile. Fix by adding
-DHAVE_STDLIB as compiler directive to Makefile.

Check that all compiled files are 32-bit using 'file */*.o | grep 64'. This shows
any remaining 64-bit files.

