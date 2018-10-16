define  proc_library  18111030714x
" e18c21     APEX      Ax
" drudg version 2015Aug31 compiled under FS  0.00.00
"< none         rack >< none     recorder 1>
enddef
define  exper_initi   18111030704x            
proc_library
sched_initi
enddef
define  setup01       18111030714x            
pcaloff
tpicd=stop
" Channel assignments consistent with vsi4=astro3    
" Following command assumes VSi4/DBBC input
" Please check and change if necessary
bit_streams=ext,0x000000ff,,4096.000
bit_streams
"channel  sky       lo       video   sample video net  bits/
"         freq     freq      freq     rate    sb  sb  sample
"  01   ******** ********    16.00 4096.00    U   U     2
tpicd=no,0
tpicd
enddef
