#!/bin/bash
#
# $Id: m6sensors.sh 1832 2014-02-25 18:25:06Z gbc $
#
# Simple self-test loopback script
#
VERSION='$Id: m6sensors.sh 1832 2014-02-25 18:25:06Z gbc $'

#
# Defaults for option variables
#
verb=false
# principal work packages
sensors=false
procfs=false
pscmd=false
dfcmd=false
lscsi=false
ntp=false
raw=false
zip=false
alma=false
full=false
# mode of operation
command='--help'
# options
log=/dev/null
# should have done this in perl
sensgrep='in.:|fan2|SYSTIN|CPUTIN|AUXTIN|PECI|cpu'
# sed script to extract name / value
sensed='
s/ALARM//g
s/$/%/
y/:/\//
s/[ ]*(.*)//
'
# followed by tr to turn newlines into %
sensfr=' %\012'
sensto=' %%'

#
USAGE="$0 command [options]

reports on Mark6 physical state.  The command should be one of:

    sensors     provides report based on lm-sensors
    procfs      provides a report based on /proc
    pscmd       provides a report based on the ps cmd
    dfcmd       provides a report based on the df cmd
    lscsi       provides a report based on the lsscsi cmd
    ntp		provides a report based on ntpq
    raw         provides a raw combined report
    zip         provides a terse combined report
    alma        provides a terse combined report (frozen for ALMA)
    full        provides a verbose combined report

and the options are (defaults are in parentheses)

    verb=<true|false>   controlling the amount of verbiage ($verb)
    log=stderr          pass the log to stderr ($log)

In the normal mode of operations, the result of the script is
a report to stdout and errors directed to a logfile.  The zip
response format is

    time:sensors:procfs:pscmd:dfcmd:lscsi:ntp

where internal newlines in each field have been turned in to % characters.
Since this format is hard-coded in the ALMA support software, this output
is also available as 'alma' and in the future the two forms may diverge.
"

# standard option handling
[ $# -lt 1 -o "$1" = "--help" ] && { echo "$USAGE" ; exit 0 ; }
[ "$1" = "--version" ] && { echo "$VERSION" ; exit 0 ; }
args="$@" ; cmd=$1 ; shift 1
# Parse command line options...
while [ $# -gt 0 ] ; do eval "$1" ; shift ; done

#
# Main command check
#
common="sensors=true procfs=true pscmd=true dfcmd=true lscsi=true ntp=true"
case ${cmd-'--help'} in
--help) echo "$USAGE" ; exit 0 ;;
sensors) sensors=true ;;
procfs)  procfs=true ;;
pscmd)   pscmd=true ;;
dfcmd)   dfcmd=true ;;
lscsi)   lscsi=true ;;
ntp)     ntp=true ;;
raw)     eval $common raw=true ;;
zip)     eval $common zip=true ;;
alma)    eval $common zip=true ;;
full)    eval $common full=true ;;
*)       echo "Command $cmd not recognized" 1>&2 ; exit 1 ;;
esac

timestamp="time/ "$(date +%Y%m%dT%H%M%S.%N)

#
# NOTE TO DEVELOPERS: If you change the zip output below,
# provide the original output for backward compatibility
# if the zip output was requested via the alma command.
#
# Otherwise something upstream will break.
#

#
# Sensors
#
sensors_raw="$(sensors 2>&1)"
sensors_full=$(echo "$sensors_raw" | egrep "$sensgrep")
sensors_zip="sens/ "$(echo "$sensors_full" |\
    sed "$sensed" | tr -s "$sensfr" "$sensto")
[ -n "$sensors_zip" ] || sensors_zip=-

#
# Use /proc
#
proc_raw="$(grep MemTotal /proc/meminfo)"
proc_full="$proc_raw"
proc_zip=$(echo "$proc_raw" | tr -s ': ' '/ ' | sed s/MemTotal/mem/)
[ -n "$proc_zip" ] || proc_zip=-

#
# ps -eo pid,ruser,%mem,%cpu,comm | grep plane
#
pscmd_raw="$(ps -eo pid,ruser,%mem,%cpu,comm | grep plane)"
pscmd_full="$pscmd_raw"
pscmd_zip="ps/ "$(echo $pscmd_raw | tr -s ' ' , | sed -e 's/^,//')
[ -n "$pscmd_zip" ] || pscmd_zip=-

#
# df -h on main disk
#
dfcmd_raw="$(df -h / | tail -1 | tr -s ' ')"
dfcmd_full="$dfcmd_raw"
dfcmd_zip="df/ "$(echo "$dfcmd_raw" | cut -d' ' -f4,5)
[ -n "$dfcmd_zip" ] || dfcmd_zip=-

#
# lscsi list of disks
#
lscsi_raw="$(lsscsi | grep ATA | grep disk)"
lscsi_full="$lscsi_raw"
lscsi_zip="scsi/ "$(echo "$lscsi_raw" | wc -l)
[ -n "$lscsi_zip" ] || lscsi_zip=-

#
# NTP status
#
ntp_raw="$(ntpq -n -c 'timeout 100' -c peers)"
ntp_full="$ntp_raw"
ntp_zip="ntp/ "$(echo "$ntp_raw" | grep '*' | tr -s ' ' , | cut -f8-10 -d,)
[ -n "$ntp_zip" ] || ntp_zip=-

#
# Construct the responses
#
response=''

$raw && response="\
# date
$timestamp

# sensors
$sensors_raw

# proc
$proc_raw

# pscmd
$pscmd_raw

# dfcmd
$dfcmd_raw

# lscsi
$lscsi_raw

# ntp
$ntp_raw

"

$full && response="\
# date
$timestamp

# sensors
$sensors_full

# proc
$proc_full

# pscmd
$pscmd_full

# dfcmd
$dfcmd_full

# lscsi
$lscsi_full

# ntp
$ntp_full

"

# one-liner
$zip &&
response="$timestamp":"$sensors_zip":"$proc_zip":"$pscmd_zip" &&
response="$response":"$dfcmd_zip":"$lscsi_zip":"$ntp_zip"

$raw || $full || $zip || {
    response="$timestamp"
    nl="
"
    $sensors && response="$response$nl$sensors_raw"
    $procfs && response="$response$nl$proc_raw"
    $pscmd && response="$response$nl$pscmd_raw"
    $dfcmd && response="$response$nl$dfcmd_raw"
    $lscsi && response="$response$nl$lscsi_raw"
    $ntp && response="$response$nl$ntp_raw"
}

# provide the response -- cplane appends ';\n'
echo -n "$response"

#
# eof
#
