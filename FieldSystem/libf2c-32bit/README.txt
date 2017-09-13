
Debian multilib (simultaneous 32/64-bit) support does not extend to libf2c.

However, FieldSystem compile with f77 in 32-bit mode requires a 32-bit 
libf2c library to be available.

Can achieve this by getting the sources of libf2c and
compiling and installing this as 32-bit to a different
location than where Debian installs its 64-bit binaries.

cd /usr/local/src/

apt-get source libf2c2

cd libf2c2-20090411

vim debian/make_lib

  should use "gcc -m32 -shared ..."

vim debian/rules

  relace references to /lib by /lib32

vim makefile.u

  should use "ld -melf_i386 -r ..."

  and "$(CC) -m32 -shared ..."

make -f debian/rules

sudo make -f debian/rules binary  # does the install but into a local subdir

sudo cp -a debian/libf2c2/usr/lib32/* /usr/lib32/

sudo cp -a debian/libf2c2-dev/usr/lib32/*.a /usr/lib32/

