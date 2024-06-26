"PV rdbe station procedure file
"--------------------------------------
define  initi         11082142422x
"welcome to the rdbe field system on sunny PV
"mk5close
mk5relink
!+5s
mk5=personality=mark5c:bank;
mk5=fill_pattern=464374526;
mk5=mode=mark5b:0xffff:1;
"o not use, FS will mangle MC to mac, not recognised
"mk5=MAC_list=ba.dc.af.e4.be.e0 : 01.00.5e.40.10.01 : ba.dc.af.e4.be.e1 : 01.00.5e.40.20.01;
"mk5=MAC_list=00.00.00.00.00.00;
mk5=packet=40:0:5008:0:0 ;  
!+2s
"sy=run setcl offset &
antenna=frontends('het230')
enddef
define  sched_initi   11082142434x
"mk5=status?
!+2s
"- - - - starting schedule - - - -
enddef
define  sched_end   01110821425
"mk5=status?
"- - - - end of schedule - - - -
enddef
define  midob         11082142446x
"onsource
"mk5=dot?
"disc_pos
!sy=run setcl &
"pcald=stop
enddef
define  postob        11082142503x
mk5=pointers?;
mk5=rtime?;
mk5=scan_check?;
mk5=dir_info?;
"mk5=vsn?
mk5=disk_serial?;
mk5=get_stats?;
"mk5=bank_set?
"checkcrc
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
"switch on phasecal now!
"phasecal=on
xdisp=off
enddef
define  pcaloff       00000000000x
xdisp=on
"switch off phasecal now!
"phasecal=off
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
define  caltsys       00000000000x
caltemp=formvc,formif
onsource
tpi=formvc,formif
ifd=max,max,*,*
!+2s
tpzero=formvc,formif
ifd=old,old,*,*
calon
!+2s
onsource
tpical=formvc,formif
tpdiff=formvc,formif
caloff
tsys=formvc,formif
enddef
define  ifdsx         00000000000
ifd=33,30,nor,nor
lo=lo1,8110.00,usb,rcp,1
lo=lo2,2100.00,usb,rcp,1
enddef
define  ifdwb         00000000000
ifd=33,30,nor,nor
lo=8110.00,2100.00,8110.0
upconv=0,0,0
patch=lo1,1l,2l,3h,4h
patch=lo2,9l,10l,11h,12h,13h,14h
enddef
define  initvcs       00000000000x
vc01=123.99,2,u,5,5
vc02=123.99,2,u,5,5
vc03=123.99,2,u,5,5
vc04=123.99,2,u,5,5
vc05=123.99,2,u,5,5
vc06=123.99,2,u,5,5
vc07=123.99,2,u,5,5
vc08=123.99,2,u,5,5
vc09=123.99,2,u,5,5
vc10=123.99,2,u,5,5
vc11=123.99,2,u,5,5
vc12=123.99,2,u,5,5
vc13=123.99,2,u,5,5
vc14=123.99,2,u,5,5
vc15=498.99,2,u,5,5
!+1s
valarm
enddef
define  vcsx4         00000000000
vc01=100.99,4.000
vc02=110.99,4.000
vc03=140.99,4.000
vc04=200.99,4.000
vc05=310.99,4.000
vc06=390.99,4.000
vc07=440.99,4.000
vc08=460.99,4.000
vc09=117.99,4.000
vc10=122.99,4.000
vc11=137.99,4.000
vc12=167.99,4.000
vc13=192.99,4.000
vc14=202.99,4.000
!+1s
valarm
enddef
define  vcwb2         00000000000
vc01=132.99,2.000
vc02=172.99,2.000
vc03=272.99,2.000
vc04=432.99,2.000
vc05=152.89,2.000
vc06=272.89,2.000
vc07=332.89,2.000
vc08=352.89,2.000
vc09=200.99,2.000
vc10=210.99,2.000
vc11=230.99,2.000
vc12=285.99,2.000
vc13=320.99,2.000
vc14=325.99,2.000
!+1s
valarm
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
disc_serial
mk5=vsn?
disc_pos
mk5=dir_info?
mk5=rtime?
enddef
define  change_pack   00000000000x
"changed disc pack!
xdisp=on
"the disk module that this is not selected can now be replaced.
wakeup
xdisp=off
disc_serial
mk5=vsn?
disc_pos
mk5=dir_info?
mk5=rtime?
enddef
define  calofffp      00000000000
caloff
!+2s
sy=go fivpt &
enddef
define  calonfp     00000000000
calon
!+1s
sy=go fivpt &
enddef
define  caloffnf      00000000000
caloff
!+2s
sy=go onoff &
enddef
define  calonnf       00000000000x
calon
!+2s
sy=go onoff &
enddef
define  fvpt          00000000000
" ein versuch, 5 punkte, abstand 0.5 beams, det i1
fivept=azel,1,5,0.5,1,i1,120
fivept
enddef
define  onof          00000000000
onoff=2,2,60,5,120,v1,v2,v3,v4,v5,v6,v7,v8
onoff
enddef
define  3c84          00000000000
source=3c84,031948.16,+413042.1,2000.
enddef
define  3c454d3       00000000000
source=3c454.3,225357.75,+160853.6,2000.
enddef
define  3c273b        00000000000
source=3c273b,122906.70,+020308.6,2000.0
enddef
define  1921m293      00000000000
source=1921-293,192451.06,-291430.1,2000.
enddef
define  3c345         00000000000
source=3c345,164258.81,+394837.0,2000.
enddef
define  3c353         00000000000
source=3c353,172028.2,-005848.,2000.
enddef
define  2134p004      00000000000
source=2134+004,213638.59,+004154.2,2000.
enddef
define  3c279         00000000000
source=3c279,125611.17,-054721.5,2000.
enddef
define  3c48         00000000000
source=3c48,013741.3,+330935.4,2000
enddef
define  3c123         00000000000
source=3c123,043704.4,+294015.0,2000
enddef
define  3c147         00000000000
source=3c147,054236.14,+495107.2,2000.
enddef
define  3c161         00000000000
source=3c161,062710.10,-055304.8,2000.
enddef
define  3c218         00000000000
source=3c218,091805.7,-120544.,2000.
enddef
define  3c286         00000000000
source=3c286,133108.29,+303033.0,2000.
enddef
define  3c295         00000000000
source=3c295,141120.65,+521209.1,2000.
enddef
define  3c348         00000000000
source=3c348,165108.2,+045933.,2000.
enddef
define  0552p398      00000000000
source=0552+398,055530.8,+394849.,2000.
enddef
define  ngc7027       00000000000
source=ngc7027,210701.59,+421410.2,2000.
enddef
define  onbore        00000000000x
azeloff=0d,0d
enddef
define  test8mhz      00000000000x27x
vc01=102.99,8.000
vc02=110.99,8.000
vc03=118.99,8.000
vc04=126.99,8.000
vc05=134.99,8.000
vc06=142.99,8.000
vc07=150.99,8.000
vc08=158.99,8.000
vc09=166.99,8.000
vc10=174.99,8.000
vc11=182.99,8.000
vc12=190.99,8.000
vc13=198.99,8.000
vc14=206.99,8.000
!+1s
valarm
enddef
define  mk5panic      00000000000x
disc_end
mk5=bank_set=inc;
!+10s
disc_serial
mk5=bank_set?
mk5=vsn?
enddef
define  dbbcread      00000000000x
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
define  preob         11082142436x
"do nothing at all
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
