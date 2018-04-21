* Put site-specific programs here that should
* be started by the Field System. 
* antcn should not be here
&stqkr n stqkr &
* Eff receives from VAX with WX and telescope data
* this should now be started at bootup
* May2002 yes do start it
sdhr n /usr2/st/bin/askvax &
* Eff when using reflection software, make sure Monitor starts
*moni1 n xterm -name monit1 -geometry 50x1 -e monit1 &
*moni2 n xterm -name monit2 -geometry 85x8 -e monit2 &
*moni3 n xterm -name monit3 -geometry 40x20 -e monit3 &
* Eff interface to OBSE (now called in .xsession)
*obse n /usr2/prog/rxvt/eff_rxvt -geometry 80x45+560+20  &
******bis auf weiteres*****dbget n dbget &
** next two lines included by AK Jul 2001
*moni2 n xterm -title "System Status Monitor" -geometry 81x6+200+200 -e monit2 &
moni2 n xterm -fn 8x16 -geom 81x6+20+200 -tit "System Status" -e monit2 &
erchk n xterm -fn 8x16 -sb -geom 96x6+0+732 -tit ERRORS -e erchk &
