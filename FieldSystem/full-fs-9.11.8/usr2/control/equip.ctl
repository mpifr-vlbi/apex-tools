********* equip.ctl Equipment Control File *********
* Please refer to the Control Files Manual in Volume 1 of the 
* Field System Documentation
* 
*  VLBI equipment
none     type of rack (mk3, vlba, vlbag, mk4, vlba4, mk5, vlba5
*                     k41, k41u, k41/k3, k41u/k3, k41/mk4, k41u/mk4,
*                     k42, k42a, k42b, k42bu, k42c, k42/k3, k42a/k3,
*                     k42bu/k3, k42/mk4, k42a/mk4, k42b/mk4, k42bu/mk4
*                     k42c/mk4, lba, lba4, s2, dbbc, dbbc/fila10g, vlbac,
*                     cdas, or none)
none     type of recorder 1 (mk3, vlba, vlba2, vlbab, vlba4, vlba42, mk4,
*		    mk4b, s2, k41, k41/dms, k42, k42/dms, mk5a, mk5a_bs,
*                   mk5b, mk5b_bs, mk5c, mk5c_bs, flexbuff, or none)
none     type of recorder 2 (mk3, vlba, vlba2, vlbab, vlba4, vba42, mk4,
*                           mk4b, or none)
none     type of decoder (mk3, dqa, mk4, or none)
*
* Mark III/IV rack parameters
 500.10 IF3 LO Frequency
   3    hex mask indicating which IF3 switches are installed, sw N ~ 2^(N-1)
*
* VLBA/4 rack parameters
  a/d   VLBA formatter cross-point switch (a/d or dsm)
101     Hardware ID for VLBA rack (assigned by GSFC)
*
* CDP S/X Receiver Parameters
60      Receiver 70K Stage Check Temperature
20      Receiver 20K Stage Check Temperature
* pcal control
none    type of phase cal control (if3 or none)
*mk iv fm firmware version
40      pre-40 versions have no barrel-rolling or data modulation
*
* LBA/4 rack parameters
   1    No of LBA DAS installed (up to MAX_DAS - see "params.h")
   in   160MHz IF input filters (in or out)
  8bit	Digital input setting (8bit internal sampler or 4bit external at ATCA)
* met sensor type
cdp     cdp or met3 server port, use cdp if you don't have either
* default mk4 form command synch test value
  3     off or 0, 1, ..., 16
*mk4 decoder transmission terminator
 return return, $, or %
*DBBC DDC version (v optional,then max 16 characters, no trailing _x until v105)
 v101   v100, v101, v102, v104, v105, v105e, v105f, ...
*DBBC PFB version
 v12    v12 only
*DBBC number of conditioning modules
  4     1-4 (a,...,d)
*DBBC IF power conversion factors, one for each module specified above, no trailing comments or extra fields
  15000 15000 15000 15000
*Mark5B clock rate (MHz): "nominal" is 32 for rack=Mark5 or VLBA5,
*                                             rack=DBBC DDC, letter ' '
*                                             rack=VLBAC
*                                      64 for rack=DBBC DDC, letters E/F,
*                                             rack-CDAS
*                                       0 otherwise except rack=none
*                         "nominal" is not allow for rack=none
* a clock rate value of "none" will suppress clock_set command in FMSET
*                                            and mk5c_mode command
   32   one of: none, nominal, 2, 4, 8, 16, 32, 64
*FiLa10G input select, one of: vsi1, vsi2, vsi1-2, vsi1-2-3-4, gps, tvg
vsi1
