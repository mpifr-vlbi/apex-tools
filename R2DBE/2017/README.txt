===========================================================================================
=== Running R2DBE
===========================================================================================

** First-time installation **

1) set up a computer to serve R2DBE(s) with a DHCP service such as 'dnsmasq' (it
should hand out IPs to R2DBEs, and advertise BOOTP/PXE network boot host), 

2) set up (same) computer with a TFTP server that serves a copy of 'uImage-r2borph3'
that you can download from https://github.com/sma-wideband/r2dbe/tree/master/support/boot/,
you can use 'dnsmasq's built-in TFTP server capability (easiest) or a dedicated server

3) set up (same) computer with NFS server that serves a root file system
that you can download from https://github.com/sma-wideband/r2dbe/tree/master/support/
as several r2dbe-debian-fs.tar.gz.part-** files

The settings for 'dnsmasq' conf are here in "dnsmasq.conf.r2dbe-APEX"

Basically:

   $ su -
   $ aptitude install dnsmasq-base dnsmasq 
   $ mkdir -p /srv/roach2_boot ; cd /srv/roach2_boot
   $ mkdir boot ; cd boot
   $ wget https://github.com/sma-wideband/r2dbe/raw/master/support/boot/uImage-r2borph3
   $ ln -s uImage-r2borph3 uImage
   $ cd /srv/roach2_boot/
   $ wget https://github.com/sma-wideband/r2dbe/raw/master/support/r2dbe-debian-fs.tar.gz.part-aa
   $ wget https://github.com/sma-wideband/r2dbe/raw/master/support/r2dbe-debian-fs.tar.gz.part-ab
   $ wget https://github.com/sma-wideband/r2dbe/raw/master/support/r2dbe-debian-fs.tar.gz.part-ac
   $ wget https://github.com/sma-wideband/r2dbe/raw/master/support/r2dbe-debian-fs.tar.gz.part-ad
   $ cat r2dbe-debian-fs.tar.gz.part-* > r2dbe-debian-fs.tar.gz
   $ tar xzvf r2dbe-debian-fs.tar.gz
   $ ls -alh
   oper@vlbi1 trunk software> ll /srv/roach2_boot
   total 305M
   drwxr-xr-x  6 root root 4.0K Jul 27  2015 .
   drwxr-xr-x  4 root root 4.0K Sep  6 14:27 ..
   drwxr-xr-x  2 root root 4.0K Dec  5  2014 boot
   drwxr-xr-x  6 root root 4.0K Dec  5  2014 build
   lrwxrwxrwx  1 root root   19 Dec  5  2014 current -> debian_stable_devel
   drwxr-xr-x 24 root root 4.0K Jul 27  2015 debian_stable_devel
   drwxr-xr-x 24 root root 4.0K Jul 24  2012 debian_stable_old
   -rw-r--r--  1 root root 305M Dec  5  2014 r2dbe-debian-fs.tar.gz

   $ cp <here>/dnsmasq.conf.r2dbe-APEX /etc/dnsmasq.conf
   $ /etc/init.d/dnsmasq restart

   $ cp <here>/exports.r2dbe-APEX /etc/exports
   $ /etc/init.d/nfs-kernel-server restart

** After rebooting R2DBE **



===========================================================================================
=== Utilities
===========================================================================================

R2DBE_config.py : based on VDIF_test.py from the SMA wideband correlator git repository,
   configures the R2DBE (clock phase calibration, network setup, time sync, VDIF setup

APEX_r2dbe1_setup.sh : invokes R2DBE_config.py with settings for EHT 2017 for r2dbe-1

APEX_r2dbe2_setup.sh : invokes R2DBE_config.py with settings for EHT 2017 for r2dbe-2

r2dbe_start.py : SMA's script r2dbe_start.py for configuring R2DBE, slightly modified
   such that the config file now also contains 'thread_id' as one of thesettings

eht-r2dbe-1.cfg : settings file for r2dbe_start.py with EHT 2017 config for r2dbe-1

eht-r2dbe-2.cfg : settings file for r2dbe_start.py with EHT 2017 config for r2dbe-2

dnsmasq.conf.r2dbe-APEX : settings for dnsmasq to respond to R2DBE's DHCP/BOOTP queries
   during the first stage of R2DBE network booting ; provides static IP and the location
   of the NFS-based root file system (Linux, PowerPC) for the R2DBE.

exports.r2dbe-APEX : export NFS root file system for R2DBE

m5specR2DBE.c : spectrometer identical to 'm5spec' from mark5access utilities, but
   with a bugfix to the numerical overflow in 'm5spec' that caused negative values
   for the frequency axis for 2048 MHz wide bands

mark6_inputstream.m6cmd : the Mark6 commands for VDIF produced by R2DBE (input_stream=...)

quickspecR2DBE.py : R2DBE spectrometer using direct snapshots from ADC (8-bit data, 
   with 256k samples x time integration)

requantizeR2DBE.py : R2DBE threshold adjustment for the 8-bit(ADC)-->2-bit(VDIF) sample
   requantization, uses direct snapshots from ADC, adjusts threshold to get near-Gaussian
   distribution for the 2-bit sample stream

