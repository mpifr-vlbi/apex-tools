define  as4c2         00000000000
vcas4
form=c2,8.000
ifdas
tapeformc
pass=$,same
enable=s1
tape=low
repro=byp,6,18,4
decode=a,crc
decode
enddef
define  as4c1         00000000000
vcas4
form=c1,8.000
ifdas
tapeformc
pass=$,same
enable=s1
tape=low
repro=byp,5,17,4
decode=a,crc
decode
enddef
define  as4a1         00000000000
vcas4
form=a,8.000
ifdas
tapeforma
pass=$,same,none
enable=s1
tape=low
repro=byp,5,17,4
decode=a,crc
decode
enddef
define  as2c2         00000000000
vcas2
form=c2,4.000
ifdas
tapeformc
pass=$,same
enable=s1
tape=low
repro=byp,6,18
decode=a,crc
decode
enddef
define  as2c1         00000000000
vcas2
form=c1,4.000
ifdas
tapeformc
pass=$,same
enable=s1
tape=low
repro=byp,5,17
decode=a,crc
decode
enddef
define  ifdas         00000000000 
ifd=33,30,nor,nor
if3=20,out,1
if3=alarm
lo=8080.00,2020.00,8080.0
upconv=0,0,0
patch=lo1,1l,2l,3l,4h
patch=lo2,9l,10l,11h,12h,13h,14h
patch=lo3,5h,6h,7h,8h
enddef
define  vcas4         00000000000
vc01=130.99,4.000
vc02=140.99,4.000
vc03=170.99,4.000
vc04=220.99,4.000
vc05=340.99,4.000
vc06=460.99,4.000
vc07=470.99,4.000
vc08=490.99,4.000
vc09=200.99,4.000
vc10=210.99,4.000
vc11=230.99,4.000
vc12=290.99,4.000
vc13=320.99,4.000
vc14=325.99,4.000
!+1s
valarm
enddef
define  vcas2         00000000000
vc01=130.99,2.000
vc02=140.99,2.000
vc03=170.99,2.000
vc04=220.99,2.000
vc05=340.99,2.000
vc06=460.99,2.000
vc07=470.99,2.000
vc08=490.99,2.000
vc09=200.99,2.000
vc10=210.99,2.000
vc11=230.99,2.000
vc12=290.99,2.000
vc13=320.99,2.000
vc14=325.99,2.000
!+1s
valarm
enddef
define  as2a1         00000000000
vcas2
form=a,4.000
ifdas
tapeforma
pass=$,same,none
enable=s1
tape=low
repro=byp,5,17
decode=a,crc
decode
enddef
define  wb4a1         00000000000
vcwb4
form=a,8.000
ifdwb
tapeforma
pass=$,same,none
enable=s1
tape=low
repro=byp,5,17,4
decode=a,crc
decode
enddef
define  wb2a1         00000000000
vcwb2
form=a,4.000
ifdwb
tapeforma
pass=$,same,none
enable=s1
tape=low
repro=byp,5,17
decode=a,crc
decode
enddef
define  caloff        00000000000
rx=*,*,*,*,*,*,off
enddef
define  calon         00000000000
rx=*,*,*,*,*,*,on
enddef
define  caltemps      00000000000
caltemp1=20.8
caltemp2=21.4
caltemp3=20.8
enddef
define  check2a1      00000000000
check2c1
enddef
define  check2a2      00000000000
check2c2
enddef
define  check2c1      00000000000
check=*,-tp,-hd
enable=
decode=a,crc,byte
parity=,,ab,on,g1,g3
fastr=15s
!+6s
!*
st=for,135,off
!+4s
repro=raw,1,3
parity
repro=byp,0,0
!*+29.20s
et
!+3s
check=*,tp,hd
enddef
define  check2c2      00000000000
check=*,-tp,-hd
enable=
decode=a,crc,byte
parity=,,ab,on,g2,g4
fastf=15s
!+6s
!*
st=rev,135,off
!+4s
repro=raw,2,4
parity
repro=byp,0,0
!*+29.20s
et
!+3s
check=*,tp,hd
enddef
define  dat           00000000000
vc15=200
vcsx2
vc15=alarm
ifdsx
form=c1,4
form=alarm
enddef
define  fastf         00000000000
ff
!+$
et
enddef
define  fastr         00000000000
rw
!+$
et
enddef
define  ifdsx         00000000000
ifd=33,30,nor,nor
if3=20,out,1
if3=alarm
lo=8080.00,2020.00,8080.0
upconv=0,0,0
patch=lo1,1l,2l,3l,4h
patch=lo2,9l,10l,11l,12h,13h,14h
patch=lo3,5h,6h,7h,8h
enddef
define  ifdwb         00000000000
ifd=33,30,nor,nor
if3=20,in,2
lo=8080.00,2020.00,8080.0
upconv=0,0,0
patch=lo1,1l,2l,3h,4h
patch=lo2,9l,10l,11h,12h,13h,14h
patch=lo3,5h,6h,7h,8h
enddef
define  initi         00000000000
"welcome to the pc field system
sy=run setcl &
enddef
define  midob         00000000000
onsource
wx
cable
ifd
if3
vc02
vc06
vc11
tpi=v1,v2,v3,v4,v5,v6,v7,v8,if1,if3
tpi=v9,v10,v11,v12,v13,v14,if2
caltemps
tsys1=v1,v2,v3,v4,v5,v6,v7,v8,if1,if3
tsys2=v9,v10,v11,v12,v13,v14,if2
enddef
define  midtp         00000000000
ifd=max,max,*,*
if3=max,*,*,*,*,*
!+2s
tpzero=v1,v2,v3,v4,v5,v6,v7,v8,if1,if3
tpzero=v9,v10,v11,v12,v13,v14,if2
ifd=old,old,*,*
if3=old,*,*,*,*,*
rxmon
enddef
define  min15         00000000000
rxall
wx
sy=brk pcalr &
!+15s
cable
sxcts
pcal
enddef
define  overnite      00000000000
log=overnite
setup
check=*,-tp
min15@!,15m
rxmon@!+2m30s,5m
pcal=0,60,by,25,0,5,11
pcal
enddef
define  postob        00000000000
enddef
define  precond       00000000000
schedule=prepass,#1
enddef
define  preob         00000000000
onsource
calon
!+2s
tpical=v1,v2,v3,v4,v5,v6,v7,v8,if1,if3
tpical=v9,v10,v11,v12,v13,v14,if2
caloff
enddef
define  prepass       00000000000
wakeup
xdisp=on
" mount the next tape without cleaning the tape drive.
" use the cont command when finished.
halt
xdisp=off
check=*,-tp,-hd
tape=low
sff
!+5m27s
et
!+9s
wakeup
xdisp=on
"drop vacuum loop, clean the tape drive thoroughly.
"re-thread the tape, establish vacuum.
"type cont when finished.
halt
xdisp=off
srw
!+5m28s
et
!+9s
enddef
define  ready         00000000000
sxcts
rxmon
newtape
loader
label
check=*,tp
enddef
define  loader        00000000000
st=for,135,off
!+11s
et
!+3s
enddef
define  rxall         00000000000
rx=00,*,*,*,*,*,*
rx
rx=01,*,*,*,*,*,*
rx
rx=02,*,*,*,*,*,*
rx
rx=03,*,*,*,*,*,*
rx
rx=04,*,*,*,*,*,*
rx
rx=05,*,*,*,*,*,*
rx
rx=06,*,*,*,*,*,*
rx
rx=07,*,*,*,*,*,*
rx
rx=08,*,*,*,*,*,*
rx
rx=09,*,*,*,*,*,*
rx
rx=0a,*,*,*,*,*,*
rx
rx=0b,*,*,*,*,*,*
rx
rx=0c,*,*,*,*,*,*
rx
rx=0d,*,*,*,*,*,*
rx
rx=0e,*,*,*,*,*,*
rx
rx=0f,*,*,*,*,*,*
rx
rx=10,*,*,*,*,*,*
rx
rx=11,*,*,*,*,*,*
rx
rx=12,*,*,*,*,*,*
rx
rx=13,*,*,*,*,*,*
rx
rx=14,*,*,*,*,*,*
rx
rx=15,*,*,*,*,*,*
rx
rx=16,*,*,*,*,*,*
rx
rx=17,*,*,*,*,*,*
rx
rx=18,*,*,*,*,*,*
rx
rx=19,*,*,*,*,*,*
rx
rx=1a,*,*,*,*,*,*
rx
rx=1b,*,*,*,*,*,*
rx
rx=1c,*,*,*,*,*,*
rx
rx=1d,*,*,*,*,*,*
rx
rx=1e,*,*,*,*,*,*
rx
enddef
define  rxmon         00000000000
rx=lo,*,*,*,*,*,*
rx
rx=dcal,*,*,*,*,*,*
rx
rx=lo5mhz,*,*,*,*,*,*
rx
rx=pres,*,*,*,*,*,*
rx
rx=20k,*,*,*,*,*,*
rx
rx=70k,*,*,*,*,*,*
rx
enddef
define  rxx           00000000000
rx=$,*,*,*,*,*,*
enddef
define  setup         00000000000
dat
et
enable=
tape=alarm
repro=byp,1,3
enddef
define  sfastf        00000000000
sff
!+$
et
enddef
define  sfastr        00000000000
srw
!+$
et
enddef
define  sx2a1         00000000000
vcsx2
form=a,4.000
ifdsx
tapeforma
pass=$,same,none
enable=s1
tape=low
repro=byp,5,17
decode=a,crc
decode
enddef
define  sx2c1         00000000000
vcsx2
form=c1,4.000
ifdsx
tapeformc
pass=$,same
enable=s1
tape=low
repro=byp,5,17
decode=a,crc
decode
enddef
define  sx2c2         00000000000
vcsx2
form=c2,4.000
ifdsx
tapeformc
pass=$,same
enable=s1
tape=low
repro=byp,6,18
decode=a,crc
decode
enddef
define  sx4a1         00000000000
vcsx4
form=a,8.000
ifdsx
tapeforma
pass=$,same,none
enable=s1
tape=low
repro=byp,5,17,4
decode=a,crc
decode
enddef
define  sx4c1         00000000000
vcsx4
form=c1,8.000
ifdsx
tapeformc
pass=$,same
enable=s1
tape=low
repro=byp,5,17,4
decode=a,crc
decode
enddef
define  sx4c2         00000000000
vcsx4
form=c2,8.000
ifdsx
tapeformc
pass=$,same
enable=s1
tape=low
repro=byp,6,18,4
decode=a,crc
decode
enddef
define  sxcts         00000000000
tpi=v1,v2,v3,v4,v5,v6,v7,v8,if1,if3
tpi=v9,v10,v11,v12,v13,v14,if2
ifd=max,max,*,*
if3=max,*,*,*,*,*
!+2s
tpzero=v1,v2,v3,v4,v5,v6,v7,v8,if1,if3
tpzero=v9,v10,v11,v12,v13,v14,if2
ifd=old,old,*,*
if3=old,*,*,*,*,*
calon
!+2s
tpical=v1,v2,v3,v4,v5,v6,v7,v8,if1,if3
tpical=v9,v10,v11,v12,v13,v14,if2
caloff
caltemps
tsys1=v1,v2,v3,v4,v5,v6,v7,v8,if1,if3
tsys2=v9,v10,v11,v12,v13,v14,if2
enddef
define  tapeforma     00000000000
tapeform=1,-350,2,  0,3,-295, 4, 55, 5,-240, 6,110
tapeform=7,-185,8,165,9,-130,10,220,11, -75,12,275
enddef
define  tapeformc     00000000000
tapeform=1,-330,2,-330,3,-275,4,-275,5,-220,6,-220
tapeform=7,-165,8,-165,9,-110,10,-110,11,-55,12,-55
tapeform=13,  0,14,  0,15, 55,16, 55,17,110,18,110
tapeform=19,165,20,165,21,220,22,220,23,275,24,275
enddef
define  unlod         00000000000
check=*,-tp
unloader
xdisp=on
"**************dismount this tape now************"
wakeup
xdisp=off
enddef
define  unloader      00000000000
!+5s
enable=,
tape=off
st=rev,135,off
enddef
define  valarm        00000000000
vc01=alarm
vc02=alarm
vc03=alarm
vc04=alarm
vc05=alarm
vc06=alarm
vc07=alarm
vc08=alarm
vc09=alarm
vc10=alarm
vc11=alarm
vc12=alarm
vc13=alarm
vc14=alarm
enddef
define  vcsx2         00000000000
vc01=130.99,2.000
vc02=140.99,2.000
vc03=170.99,2.000
vc04=230.99,2.000
vc05=340.99,2.000
vc06=420.99,2.000
vc07=470.99,2.000
vc08=490.99,2.000
vc09=197.99,2.000
vc10=202.99,2.000
vc11=217.99,2.000
vc12=247.99,2.000
vc13=272.99,2.000
vc14=282.99,2.000
!+1s
valarm
enddef
define  vcsx4         00000000000
vc01=130.99,4.000
vc02=140.99,4.000
vc03=170.99,4.000
vc04=230.99,4.000
vc05=340.99,4.000
vc06=420.99,4.000
vc07=470.99,4.000
vc08=490.99,4.000
vc09=197.99,4.000
vc10=202.99,4.000
vc11=217.99,4.000
vc12=247.99,4.000
vc13=272.99,4.000
vc14=282.99,4.000
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
define  vcwb4         00000000000
vc01=132.99,4.000
vc02=172.99,4.000
vc03=272.99,4.000
vc04=432.99,4.000
vc05=152.89,4.000
vc06=272.89,4.000
vc07=332.89,4.000
vc08=352.89,4.000
vc09=200.99,4.000
vc10=210.99,4.000
vc11=230.99,4.000
vc12=285.99,4.000
vc13=320.99,4.000
vc14=325.99,4.000
!+1s
valarm
enddef
define  vread         00000000000
vc01
vc02
vc03
vc04
vc05
vc06
vc07
vc08
vc09
vc10
vc11
vc12
vc13
vc14
enddef
define  wb2c1         00000000000
vcwb2
form=c1,4.000
ifdwb
tapeformc
pass=$,same
enable=s1
tape=low
repro=byp,5,17
decode=a,crc
decode
enddef
define  wb2c2         00000000000
vcwb2
form=c2,4.000
ifdwb
tapeformc
pass=$,same
enable=s1
tape=low
repro=byp,6,18
decode=a,crc
decode
enddef
define  wb4c1         00000000000
vcwb4
form=c1,8.000
ifdwb
tapeformc
pass=$,same
enable=s1
tape=low
repro=byp,5,17,4
decode=a,crc
decode
enddef
define  wb4c2         00000000000
vcwb4
form=c2,8.000
ifdwb
tapeformc
pass=$,same
enable=s1
tape=low
repro=byp,6,18,4
decode=a,crc
decode
enddef
