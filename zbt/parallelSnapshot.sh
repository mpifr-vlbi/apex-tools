#!/bin/bash

MK5C_IP_1=10.0.2.103 # attached to R2DBE
MK5C_IP_2=10.0.2.104 # attached to DBBC

set -m  # enable 'fg', 'bg'

# Clean up a bit first
ssh oper@$MK5C_IP_1 'rm -f /home/oper/vdif/r2dbe.vdif'
ssh oper@$MK5C_IP_2 'rm -f /home/oper/vdif/dbbc.vdif'
sleep 2
ssh oper@$MK5C_IP_1 '/bin/sync'
ssh oper@$MK5C_IP_2 '/bin/sync'
sleep 2

# Make snapshot
#  R2DBE sends 2 x 2048 MHz     = 4096 MHz (two identical 2048 MHz IFs via splitter)
#  DBBC  sends 2 x 16x62.5 MHz  = 2000 MHz (two different IFs, 0-1024MHz/1000MHz and 1200-1800MHz/1000MHz)
ssh oper@$MK5C_IP_1 '/usr/local/bin/vdifsnapshotUDP --cpu=2  --offset=8 4096 4001 /home/oper/vdif/r2dbe.vdif'  &
ssh oper@$MK5C_IP_2 '/usr/local/bin/vdifsnapshotUDP --cpu=2  --offset=8 2048 46227 /home/oper/vdif/dbbc.vdif'

# Split out threads from recorded data when recording dual threads (e.g. from two 10 GbE interfaces)
ssh oper@$MK5C_IP_1 '/usr/local/bin/modifyVDIF --extract=0 /home/oper/vdif/r2dbe.vdif /home/oper/vdif/r2dbe_t0.vdif' &
ssh oper@$MK5C_IP_1 '/usr/local/bin/modifyVDIF --extract=1 /home/oper/vdif/r2dbe.vdif /home/oper/vdif/r2dbe_t1.vdif'
ssh oper@$MK5C_IP_2 '/usr/local/bin/modifyVDIF --extract=0 /home/oper/vdif/dbbc.vdif /home/oper/vdif/dbbc_t0.vdif' &
ssh oper@$MK5C_IP_2 '/usr/local/bin/modifyVDIF --extract=1 /home/oper/vdif/dbbc.vdif /home/oper/vdif/dbbc_t1.vdif'

# Show the time stamps (can be copied into DiFX):
echo "Time info for /home/oper/vdif/r2dbe_t0.vdif:"
ssh oper@$MK5C_IP_1 '/usr/local/difx/bin/m5time VDIF_8192-8192-1-2 /home/oper/vdif/r2dbe_t0.vdif'
echo "Time info for /home/oper/vdif/r2dbe_t1.vdif:"
ssh oper@$MK5C_IP_1 '/usr/local/difx/bin/m5time VDIF_8192-8192-1-2 /home/oper/vdif/r2dbe_t1.vdif'

echo "Time info for /home/oper/vdif/dbbc_t0.vdif:"
ssh oper@$MK5C_IP_2 '/usr/local/difx/bin/m5time VDIF_8000-4000-16-2 /home/oper/vdif/dbbc_t0.vdif'
echo "Time info for /home/oper/vdif/dbbc_t1.vdif:"
ssh oper@$MK5C_IP_2 '/usr/local/difx/bin/m5time VDIF_8000-4000-16-2 /home/oper/vdif/dbbc_t1.vdif'
