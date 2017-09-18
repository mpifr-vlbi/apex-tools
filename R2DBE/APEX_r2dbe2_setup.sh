#
# Configure the R2DBE-2 attached to Mark6 vlbi2
#

# If Mark6 (vlbi2) network cards change re-run this
#   /etc/r2dbe/R2DBE_config.py --ifaces eth3,eth5 --src 192.168.1.13,192.168.1.15 --dst 192.168.1.3,192.168.1.5 r2dbe-2
# to determine the new MAC addresses to use here:
/etc/r2dbe/R2DBE_config.py --ifaces eth3,eth5 --stationIDs AR,ar --src 192.168.1.13,192.168.1.15 --dst 192.168.1.3,192.168.1.5 --arp 3,0060dd444639 --arp 5,0060dd444643 --pols 1,1 r2dbe-2


