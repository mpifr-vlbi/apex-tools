* Put site-specific programs here that should
* be started by the Field System. 
* antcn should not be here
*erchk n xterm -geom 99x16+0+518 -title ERRORS -e erchk &
&stqkr n stqkr &
moni2 n xterm -fn 8x16 -geom 81x6+20+200 -tit "System Status" -e monit2 &
erchk n xterm -fn 8x16 -sb -geom 96x6+0+732 -tit ERRORS -e erchk &
dbbccn n dbbccn &
