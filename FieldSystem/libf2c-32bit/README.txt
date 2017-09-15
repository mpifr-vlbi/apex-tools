
Debian multilib support (simultaneous 32/64-bit) does not cover the libf2c library.

However, FieldSystem compile with f77 in 32-bit mode requires a 32-bit libf2c library.

Can achieve this by getting the sources of libf2c and
compiling and installing this as 32-bit to a different
location than where Debian installs its 64-bit binaries.

Get sources:

   cd /usr/local/src/

   apt-get source libf2c2

   cd libf2c2-20090411

Switch to a 32-bit compile:

  either edit manually

   vim debian/make_lib  # change places to use "gcc -m32 -shared ..."

   vim debian/rules     # change /lib to /lib32 in all places

   vim makefile.u       # use "ld -melf_i386 -r ..." and "$(CC) -m32 -shared ..."

  or use the pre-edited files from this git repository

   cp make_lib /usr/local/src/libf2c2-*/debian/
   cp rules /usr/local/src/libf2c2-*/debian/
   cp makefile.u /usr/local/src/libf2c2-*/

Compile:

   cd /usr/local/src/libf2c2-*/

   make -f debian/rules

   sudo make -f debian/rules binary  # does the install but into a local subdir

Install:

   sudo mkdir -p /usr/lib32/
   sudo cp -a debian/libf2c2/usr/lib32/* /usr/lib32/
   sudo cp -a debian/libf2c2-dev/usr/lib32/*.a /usr/lib32/

   vim /etc/ld.so.conf.d/i?86-linux-gnu.conf    # make sure /usr/lib32/ is in the list

   ldconfig


