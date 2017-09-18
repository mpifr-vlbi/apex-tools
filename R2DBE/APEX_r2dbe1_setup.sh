#
# Configure the R2DBE-1 attached to Mark6 vlbi1
#

# If Mark6 (vlbi1) network cards change re-run this
#   /etc/r2dbe/R2DBE_config.py --ifaces eth3,eth5 --stationIDs AR,ar --src 192.168.1.13,192.168.1.15 --dst 192.168.1.3,192.168.1.5 r2dbe-1
# to determine the new MAC addresses to use here:
/etc/r2dbe/R2DBE_config.py --ifaces eth3,eth5 --stationIDs AR,ar --src 192.168.1.13,192.168.1.15 --dst 192.168.1.3,192.168.1.5 --arp 3,0060dd440b75 --arp 5,0060dd4446c0 r2dbe-1


