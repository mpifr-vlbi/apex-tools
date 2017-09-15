Extra notes
=======================================

The users .bashrc should contain

  export PATH=$PATH:/usr2/fs/bin:/usr2/st/bin

To properly initialize shared memory areas used by fs (st) the
startup file /etc/rc.local should contain

  /usr2/fs/bin/fsalloc
  /usr2/st/bin/stalloc

The compile of 'oprin' component of fs needs some extra effort,

  $ sudo aptitude install lib32readline-dev libreadline-dev  
  $ cp oprin-Makefile /usr2/fs/oprin/
      was fixed to use -m32 flags to compile as 32bit
  $ cd /usr2/fs/oprin/ ; make

Subdirectories here
=======================================

st/antcn : sends track() commands with source coordinates from FieldSystem over to APECS

st/apecs : some general UDO "connection" helpers to "connect" to the APECS system

libf2c-32bit : instructions for compiling&installing 32-bit libf2c without killing distros 64-bit libf2c


Field System on 64-bit machines (amd64)
=======================================

The field system normally runs on 32-bit systems. There are two ways to run
FS on 64-bit machines:
1)  Use VirtualBox and a 32-bit Linux virtual system.
    Runs slow, and can be starved of execution time.
2)  Keep running a 64-bit Debian/Ubuntu but add "multiarch"/"multilib" support
    to install 32-bit system libraries on the 64-bit system, and then
    compile FieldSystem with GCC/gfortran77 flags "-m32 -Wl,-melf-i386"

Option 2 looks better. This means changing many Makefiles because the
build system in FieldSystem is literally from the last century.

The script 'mk64.pl' partly the automates Makefile editing:

cp mk64.pl /usr2/fs/
cp mk64.pl /usr2/st/

cd /usr2/fs/ ; ./mk64.pl
sed -i "s/\$(FC)  -o/\$(FC) -Wl,-m32 -o/g" */?akefile
sed -i "s/\$(FC) -o/\$(FC) -Wl,-m32 -o/g" */?akefile
sed -i "s/\$(FC) -m32 -o/\$(FC) -m32 -Wl,-m32 -o/g" */?akefile

cd /usr2/st/ ; ./mk64.pl
sed -i "s/\$(FC)  -o/\$(FC) -Wl,-m32 -o/g" */?akefile
sed -i "s/\$(FC) -o/\$(FC) -Wl,-m32 -o/g" */?akefile
sed -i "s/\$(FC) -m32 -o/\$(FC) -m32 -Wl,-m32 -o/g" */?akefile

Before compiling:

apt-get install flex bison fortran77-compiler
dpkg --add-architecture i386
apt-get update
apt-get install libc6-i386 libc6-dev-i386 ncurses readline

Before compiling (2): need 32-bit libf2c

Go to the subdirectory ./libf2c-32bit/ of this git repository.
Follow the brief instructions there how to install 32-bit libf2c.

Compiling:

cd /usr2/fs/
make clean
make

cd /usr2/st/
make clean
make

Sometimes 'oprin/readline' directory fails to compile.
Fix 1: adding -DHAVE_STDLIB as compiler directive to Makefile.
Fix 2: edit oprin/readline/Makefile and remove the section about 'termcap'

Verify that none of the compiled files are 64-bit:

cd /usr2/fs/
file */*.o | grep 64   
file bin/* | grep 64

This should produce a blank list. If not, delete the
listed 64-bit files and run 'make' again.

Installing:

Make sure /etc/rc.local calls 'fsalloc' and 'stalloc', e.g.:

$ sudo nano /etc/rc.local
#!/bin/sh -e
/usr2/fs/bin/fsalloc
/usr2/st/bin/stalloc
exit 0
