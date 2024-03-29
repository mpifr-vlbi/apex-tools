#
# Configure the R2DBE-2 attached to Mark6 vlbi2
#

# If Mark6 (vlbi2) network cards change re-run this
#   /etc/r2dbe/R2DBE_config.py --ifaces eth3,eth5 --src 192.168.1.13,192.168.1.15 --dst 192.168.1.3,192.168.1.5 r2dbe-2
# to determine the new MAC addresses to use here:

/etc/r2dbe/R2DBE_config.py --ifaces eth3,eth5 \
	--stationIDs Ap,Ap --threadIDs 1,3 --pols 0,1 \
        --rxsbands 1,1 --besbands 1,1 \
	--src 192.168.1.13,192.168.1.15 \
	--dst 192.168.1.3,192.168.1.5 \
	--arp 3,0060dd444639 \
	--arp 5,0060dd444643 \
	r2dbe-2

# equivalent to "r2dbe_start.py -f eht-r2dbe-2.cfg -v  r2dbe-2" but also configures the R2DBE network settings

