
APEX_config.py : based on VDIF_test.py from the SMA wideband correlator git repository,
   configures the R2DBE (clock phase calibration, network setup, time sync, VDIF mode
   setup (hard-coded VDIF epoch 30 = 01/01/2015), ...)

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

