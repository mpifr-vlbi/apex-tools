define  proc_library  00000000000x
" dbsx       EFLSBERG  Ef
" drudg version 2008Oct08 compiled under FS  9.10.04
"< Mark4    rack >< Mark5A   recorder 1>
enddef
define  exper_initi   00000000000x            
proc_library
sched_initi
mk5=DTS_id?
mk5=OS_rev1?
mk5=OS_rev2?
mk5=SS_rev1?
mk5=SS_rev2?
mk5=status?
enddef
define  setup01       00000000000x            
"Connect the Set 1 Mark 5A recorder input to"
"the headstack 1 output of the formatter"
pcalon
tpicd=stop
pcald=stop
trkf01      
pcalf01
tracks=v0,v1,v2,v3
pcald=
form=m,32.000,1:2
vc01d       
ifd01       
tpicd=no,0
mk5=play_rate=data:16;
mk5=mode=mark4:32;
bank_check
pcald
tpicd
mk5=mode?
enddef
define  vc01d         00000000000x            
vc02=134.99,16.000,u
vc04=150.99,16.000,u
vc06=166.99,16.000,u
vc08=182.99,16.000,u
vc01=284.99,16.000,u
vc03=300.99,16.000,u
vc05=316.99,16.000,u
vc07=332.99,16.000,u
!+1s
valarm
enddef
define  ifd01         00000000000x            
ifd=,,nor,nor 
lo=
lo=lo1,8100.00,usb,rcp,1.000
lo=lo2,2100.00,usb,rcp,1.000
patch=
patch=lo1,1h,3h,5h,7h
patch=lo2,2l,4l,6l,8l
enddef
define  trkf01        00000000000x            
trackform= 
trackform=2,2us,6,2um,10,4us,14,4um,18,6us,22,6um,26,8us,30,8um 
trackform=3,1us,7,1um,11,3us,15,3um,19,5us,23,5um,27,7us,31,7um 
enddef
define  pcalf01       00000000000x            
pcalform=
pcalform=2u,1,16
pcalform=4u,1,16
pcalform=6u,1,16
pcalform=8u,1,16
pcalform=1u,1,16
pcalform=3u,1,16
pcalform=5u,1,16
pcalform=7u,1,16
enddef
