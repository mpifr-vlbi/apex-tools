define  proc_library  00000000000x
" dbsx       EFF_DBBC  Ed
" drudg version 2008Oct08 compiled under FS  9.10.04
"< VLBA5    rack >< Mark5B   recorder 1>
enddef
define  exper_initi   00000000000x            
proc_library
sched_initi
mk5=DTS_id?
mk5=OS_rev?
mk5=SS_rev?
mk5=status?
enddef
define  setup01       00000000000x            
pcalon
tpicd=stop
pcald=stop
mk5b_mode=ext,0x0000FFFF,1
mk5b_mode
vsi4=geo
vsi4
bbc01d      
ifd01       
bank_check
tpicd
enddef
define  bbc01d        00000000000x            
bbc05=534.99,b,16.000,16.000
bbc06=550.99,b,16.000,16.000
bbc07=566.99,b,16.000,16.000
bbc08=582.99,b,16.000,16.000
bbc01=684.99,a,16.000,16.000
bbc02=700.99,a,16.000,16.000
bbc03=716.99,a,16.000,16.000
bbc04=732.99,a,16.000,16.000
enddef
define  ifd01         00000000000x            
ifdab=0,0,nor,nor
lo=
lo=loa,7700.00,usb,rcp,1.000
lo=lob,1700.00,usb,rcp,1.000
enddef
