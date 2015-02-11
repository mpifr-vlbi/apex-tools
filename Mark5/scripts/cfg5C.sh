#!/bin/bash
if [ "$1" == "" ]; then
        echo "Usage: $0 <remote IP>"
        IP=127.0.0.1
else
        IP=$1
fi
function send_cmd {
        echo "Mk5C: $1 ; "
        echo "$1 ; " | netcat -q 2 -w 2 -i 2 $IP 2620
}

echo "--- Setup of Mark5C at $IP ---"
send_cmd "personality=mark5c:bank"
sleep 1

send_cmd "fill_pattern=464374526"  # 0x1BADCAFE in decimal
#sleep 1

send_cmd "mode=mark5b:0xffff:1"
sleep 1

# Used to work, not supported any more:
send_cmd "MAC_list=ba.dc.af.e4.be.e0 : 01.00.5e.40.20.01 : 01.00.5e.40.20.02 : 00.60.DD.47.76.E5 : 00.60.DD.47.76.E6"

## Specify packet size as received from DBE-->FILA10G

# This worked in september 2010 firmware, no longer in Conduants "fixed"
# november 2010 firmware:
# send_cmd "packet=32:8:5016:1:4"

# This should hop over and ignore the 4+4byte PSN and not record it,
# but will also do NO packet reordering nor gap-over any lost packets:
send_cmd "packet=40:0:5008:0:0"
sleep 1

