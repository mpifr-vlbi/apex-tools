"ap station procedure file for master field system
"handles apecs (tsys, wx) and controls mark5c2
"---------------------------------------------------

define  initi         18111025015x
"Welcome to the field system on sunny apex
"Please check that you have frontend pi230 selected in apecs
"The Mark6 recorders are controlled outside of FieldSystem
!+5s
"sy=run setcl offset &
enddef

define  sched_initi   18111030714x
"- - - - starting schedule - - - -
enddef

define  sched_end     00000000000x
enddef

define  preob         18111031950x
"mk5=bank_set?;
enddef

define  midob         18111032000x
onsource
enddef

define  postob        18111032400x
!+2s
sy=/usr2/proc/apecscmd.sh 'cancel()' &
!+2s
caltsys
enddef

define  midtp         00000000000
enddef

define  calon         00000000000
cal=on
enddef

define  caloff        00000000000x
cal=off
enddef

define  pcalon        00000000000x
xdisp=on
"phase cal not available at apex
xdisp=off
enddef
define  pcaloff       18111030714x
xdisp=on
"phase cal not available at apex
xdisp=off
enddef

define  vread         00000000000
dbbc=dbbc01
dbbc=dbbc02
dbbc=dbbc03
dbbc=dbbc04
dbbc=dbbc05
dbbc=dbbc06
dbbc=dbbc07
dbbc=dbbc08
enddef

define  caltsys       18111032404x
"invokes tsys measurement via apecs vlbi_calibrate.apecs and gets results via vlbi_calibrateget.apecs
sy=/usr2/proc/tsysapex.sh &
!+3s
enddef

define  ifdsx         00000000000
"old sx not available at apex
enddef

define  ifdwb         00000000000
"wideband sx not available at apex
enddef

define  initvcs       00000000000x
"no video converters at apex
enddef
define  vcsx4         00000000000
"no video converters at apex
enddef
define  vcwb2         00000000000
"no video converters at apex
enddef

define  ready_disc    00000000000x
"mk5close
xdisp=on
"mount the mark5 discs for this experiment now
"recording will begin at current position
"enter 'mk5relink' when ready or
"if you can't get the mk5 going then
"enter 'cont' to continue without the mk5
xdisp=off
halt
"disc_serial
"mk5=vsn?;
"disc_pos
"mk5=dir_info?;
"mk5=rtime?;
enddef
define  change_pack   00000000000x
"changed disk pack!
xdisp=on
"the disk module that is not selected can now be replaced (or so you wish...)
wakeup
xdisp=off
"disc_serial
"mk5=vsn?;
"disc_pos
"mk5=dir_info?;
"mk5=rtime?;
enddef
define  calofffp      00000000000
"note this must be done manually via apecs
"caloff
"!+2s
"sy=go fivpt &
enddef
define  calonfp     00000000000
"note this must be done manually via apecs
"calon
"!+1s
"sy=go fivpt &
enddef
define  caloffnf      00000000000
"note this must be done manually via apecs
"caloff
"!+2s
"sy=go onoff &
enddef
define  calonnf       00000000000x
"note this must be done manually via apecs
"calon
"!+2s
"sy=go onoff &
enddef
define  fvpt          00000000000
"note this must be done manually via apecs
" ein versuch, 5 punkte, abstand 0.5 beams, det i1
"fivept=azel,1,5,0.5,1,i1,120
"fivept
enddef
define  onof          00000000000
onoff=2,2,60,5,120,v1,v2,v3,v4,v5,v6,v7,v8
onoff
enddef

define  onbore        00000000000x
azeloff=0d,0d
enddef
define  test8mhz      00000000000x27x
"no video converters at apex
enddef
define  mk5panic      00000000000x
"please panic
"disc_end
"mk5=bank_set=inc;
!+10s
"disc_serial
"mk5=bank_set?;
"mk5=vsn?;
enddef
define  dbbcread      00000000000x
"this will work only with the dbbc tunable downconverters firmware
dbbc01
dbbc02
dbbc03
dbbc04
dbbc05
dbbc06
dbbc07
dbbc08
dbbc09
dbbc10
dbbc11
dbbc12
dbbc13
dbbc14
dbbc15
dbbc16
enddef
define  dbbcifread    00000000000x
dbbcifa
dbbcifb
dbbcifc
dbbcifd
enddef
define  dbbcset4     000000000000x
"this will work only with the dbbc tunable downconverter firmware
dbbcifa=1,agc,2
dbbc01=138.99,a,4,4,1,agc
dbbc02=146.99,a,4,4,1,agc
dbbc03=154.99,a,4,4,1,agc
dbbc04=162.99,a,4,4,1,agc
dbbc05=170.99,a,4,4,1,agc
dbbc06=178.99,a,4,4,1,agc
dbbc07=186.99,a,4,4,1,agc
dbbc08=192.99,a,4,4,1,agc
dbbc_direct=pps_sync
!+1s
dbbcmon=01u
enddef
define  dbbcset8      00000000000x
"this will work only with the dbbc tunable downconverter firmware
dbbcifa=1,agc,2
dbbcifb=1,agc,2
dbbcifc=1,agc,1
dbbcifd=1,agc,1
dbbc01=130.99,a,8,8,1,man,2,2
dbbc02=146.99,a,8,8,1,man,2,2
dbbc03=162.99,a,8,8,1,man,2,2
dbbc04=178.99,a,8,8,1,man,2,2
dbbc05=192.99,b,8,8,1,man,2,2
dbbc06=208.99,b,8,8,1,man,2,2
dbbc07=224.99,b,8,8,1,man,2,2
dbbc08=240.99,b,8,8,1,man,2,2
dbbc09=510.99,c,8,8,1,man,2,2
dbbc10=530.99,c,8,8,1,man,2,2
dbbc11=550.99,c,8,8,1,man,2,2
dbbc12=570.99,c,8,8,1,man,2,2
dbbc13=640.99,d,8,8,1,man,2,2
dbbc14=660.99,d,8,8,1,man,2,2
dbbc15=680.99,d,8,8,1,man,2,2
dbbc16=700.99,d,8,8,1,man,2,2
dbbc_direct=pps_sync
!+1s
dbbcmon=01u
enddef
define  dbbcset16     00000000000x
"this will work only with the dbbc tunable downconverter firmware
dbbcifa=1,agc,2
dbbc01=100.99,a,16,16,1
dbbc02=132.99,a,16,16,1
dbbc03=164.99,a,16,16,1
dbbc04=196.99,a,16,16,1
dbbc05=228.99,b,16,16,1
dbbc06=260.99,b,16,16,1
dbbc07=292.99,b,16,16,1
dbbc08=324.99,b,16,16,1
dbbc_direct=pps_sync
!+1s
dbbcmon=01u
enddef
define  setsame       00000000000x
"this will work only with the dbbc tunable downconverter firmware
dbbcifa=1,agc,2
dbbcifb=1,agc,2
dbbcifc=1,agc,1
dbbcifd=1,agc,1
dbbc01=150.00,a,8,8,1,5,5
dbbc02=150.00,a,8,8,1,5,5
dbbc03=150.00,a,8,8,1,5,5
dbbc04=150.00,a,8,8,1,5,5
dbbc05=150.00,b,8,8,1,5,5
dbbc06=150.00,b,8,8,1,5,5
dbbc07=150.00,b,8,8,1,5,5
dbbc08=150.00,b,8,8,1,5,5
dbbc_direct=pps_sync
!+1s
dbbcmon=01u
enddef
define  listreg       00000000000x
dbbc_direct=r_reg=1,0
dbbc_direct=r_reg=1,1
dbbc_direct=r_reg=1,2
dbbc_direct=r_reg=1,3
dbbc_direct=r_reg=1,4
dbbc_direct=r_reg=1,5
dbbc_direct=r_reg=1,6
dbbc_direct=r_reg=1,7
dbbc_direct=r_reg=1,8
dbbc_direct=r_reg=1,9
dbbc_direct=r_reg=1,10
dbbc_direct=r_reg=1,11
dbbc_direct=r_reg=1,12
dbbc_direct=r_reg=1,13
dbbc_direct=r_reg=1,14
dbbc_direct=r_reg=1,15
dbbc_direct=r_reg=1,16
dbbc_direct=r_reg=1,17
dbbc_direct=r_reg=1,18
dbbc_direct=r_reg=1,19
dbbc_direct=r_reg=1,20
dbbc_direct=r_reg=1,21
dbbc_direct=r_reg=1,22
dbbc_direct=r_reg=1,23
dbbc_direct=r_reg=1,24
dbbc_direct=r_reg=1,25
dbbc_direct=r_reg=1,26
dbbc_direct=r_reg=1,27
dbbc_direct=r_reg=1,28
dbbc_direct=r_reg=1,29
dbbc_direct=r_reg=1,30
dbbc_direct=r_reg=1,31
dbbc_direct=r_reg=1,32
dbbc_direct=r_reg=1,33
dbbc_direct=r_reg=1,34
dbbc_direct=r_reg=1,35
dbbc_direct=r_reg=1,36
dbbc_direct=r_reg=1,37
dbbc_direct=r_reg=1,38
dbbc_direct=r_reg=1,39
dbbc_direct=r_reg=1,40
dbbc_direct=r_reg=1,41
dbbc_direct=r_reg=1,42
enddef
