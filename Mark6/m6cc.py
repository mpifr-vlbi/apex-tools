#!/usr/bin/python
## Copyright 2012 MIT Haystack Observatory
##
## This file is part of Mark6 command and control file.
##
## M6_CC is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, version 2 of the License.
##
## M6_CC is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with M6_CC.    If not, see <http://www.gnu.org/licenses/>.

'''
Author:     chester@haystack.mit.edu
Date:         9/07/2012
Description:

Implements the command and control of the Mark6 recording system
by issuing the appropriate VSI-S commands to the Mark6 cplane.
The methods used to invoke the M6_CC is to supply a file that
contains the XML description of the schedule to be executed.
'''
import socket
import os
import sys
import getopt
import string
import time
import calendar
import logging
import xml.etree.ElementTree as ET

logger = logging.getLogger("M6_CC")

def setup_logging(logfile):
    '''
    Setup the logging
    '''
    logger.setLevel(logging.DEBUG)
    # create file handler which logs even debug messages
    fh = logging.FileHandler(logfile)
    fh.setLevel(logging.DEBUG)
    # create console handler with a higher log level
    ch = logging.StreamHandler()
    ch.setLevel(logging.WARNING)
    # create formatter and add it to the handlers
    formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    ch.setFormatter(formatter)
    fh.setFormatter(formatter)
    # add the handlers to logger
    logger.addHandler(ch)
    logger.addHandler(fh)

def seconds_until(target):
    '''
    Returns seconds until the target time; >0 if target is in the future
    '''
    tim = calendar.timegm(time.strptime(target, '%Y%j%H%M%S'))
    now = calendar.timegm(time.gmtime())
    return tim - now

def args():
    '''
    Parse arguments and return xml file, mark6 host and port
    '''
    parms = {'-f':"test.xml",       # schedule file
             '-m':'127.0.0.1',      # m6 host ip address
             '-p':14242,            # m6 host port
             '-r':16064,            # data rate in Mbps
             }
    try:
        opts, pargs = getopt.getopt(sys.argv[1:], "f:m:p:r:")
    except getopt.GetoptError, msg:
        print msg
        sys.exit(1)

    for o,v in opts:
        parms[o] = v

    input_fn = str(parms['-f'])
    m6_host = str(parms['-m'])
    m6_port = int(parms['-p'])
    rate_mbps = int(parms['-r'])
    return input_fn, m6_host, m6_port, rate_mbps

def mark6_send(m6_s, st, m6cmd, where):
    '''
    This method sends commands to the mark6, with logging and
    error checking.  A null string on return implies success.
    '''
    global logger
    logger.info("%s - sent: %s", st, m6cmd.strip())
    try:
        rv = m6_s.send(m6cmd)
    except Exception, e:
        logger.error("%s - send exception %s", st, str(e))
        return where + ', ' + str(e)
    if rv <= 0:
        logger.error("%s - send failed (%d)", st, rv)
        return where + ', error'
    logger.info("%s - send success (%s)", st, where)
    return ''

def mark6_recv(m6_s, st, qore, where):
    '''
    This method receives commands from the mark6, with logging and
    error checking.  A null string on return implies success, and
    the retrieved string is cleared of nulls.
    '''
    global logger
    try:
        ret = m6_s.recv(1024)
        logger.info("%s - mk6 ret: %s", st, ret.strip())
    except Exception, e:
        logger.error("%s - recv exception %s", st, str(e))
        return where + ', ' + str(e), ''
    pos = ret.find(qore)
    if '0' not in ret[(pos+1):]:
        logger.error("%s - failed %s", st, ret.strip())
        return where + ', error', ret
    else:
         logger.info("%s - success %s (%s)", st, ret.strip(), where)
    rnn = ret.translate(None, '\0')
    return '', rnn

def vextime(jt):
    '''
    Convert YYYYDOYHHMMSS into vextime YYYYyDOYdHHhMMmSSs
    '''
    vx = jt[0:4] + 'y' + jt[4:7] + 'd' + \
        jt[7:9] + 'h' + jt[9:11] + 'm' + \
        jt[11:13] + 's'
    return vx

def main(input_fn, m6_host, m6_port, rate_mbps):
    '''
    Run the XML schedule from the input file at the specified host.
    '''
    global logger
    errmsg = ''
    proceed = False
    if os.path.isfile(input_fn):
        try:
            tree = ET.parse(input_fn)
            errmsg = ("Parsed input schedule file %s" % input_fn)
            proceed = True
        except Exception, e:
            errmsg = ("Unparsable %s: %s" % (input_fn,e))
    else:
        errmsg = ("Session XML file %s does not exist " % (input_fn))

    if proceed:
        root = tree.getroot()
        exp = root.get('name')
        st = root.get('station')
        exp_start = root.get('start')
        exp_end = root.get('end')
    else:
        st = 'xx'

    # --- start the log
    stamp = time.strftime('%Y.%j.%H.%M.%S',time.gmtime());
    logname = 'M6_CC-' + st + '-' + stamp + '.log'
    setup_logging(logname)
    logger.info('%s - log started', st)
    logger.info('%s - %s', st, errmsg)
    if not proceed:
        logger.info('%s - unable to continue', st)
        sys.exit(1)
    # --- number of scans in file
    numscans = len(tree.findall('scan'))
    logger.debug("%s - client %s port %d, scans %d",
        st, m6_host, m6_port, numscans)

    # --- connect to mk6
    m6_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    m6_s.settimeout(5.0) ### 5-sec timeout
    m6_s.connect((m6_host,m6_port))

    logger.info("%s - Exp %s, station %s, rate %d", st, exp, st, rate_mbps)
    logger.info("%s - starting %s, ending %s", st, exp_start, exp_end)

    # --- dboss status every second
    os.system('dboss s 1')
    logger.info("%s - boosted dplane status to every second")

    scan_no = 0
    # --- for the remainder of the list process the scan information
    for scan in root.findall('scan'):
        scan_no += 1 
        # --- parse scan information
        exp = scan.get('experiment')
        src = scan.get('source')
        sc = scan.get('station_code')
        start_time = scan.get('start_time')
        delta = scan.get('duration')
        sn = scan.get('scan_name')
        logger.debug("%s - source %s starting %s dur %s scan_name %s",
            st, src, start_time, delta, sn)
        logger.debug("%s - scan %d of %d", st, scan_no, numscans)

        # --- get time from system
        dot_time = time.strftime('%Y%j%H%M%S',time.gmtime());
        logger.info("%s - current time %s ", st, dot_time)
        logger.info("%s - start_time   %s ", st, start_time)

        diff = seconds_until(start_time)
        if diff > -int(delta):
            # --- there is some time left in the scan
            logger.info("%s - Mk6 record on (%d sec to go)", st, diff)
            dsize = ("%.0f" % (float(delta) * float(rate_mbps) / 8000.0))
            m6cmd = "record=" + vextime(start_time) + ":" + delta
            m6cmd = m6cmd + ":" + dsize + ":" + sn
            m6cmd = m6cmd + ":" + exp + ":" + st + ";\n"
            where = mark6_send(m6_s, st, m6cmd, "record=on send")
            if where != '':
                break
            where, ret = mark6_recv(m6_s, st, "=", "record=on resp")
            if where != '':
                break

            # --- sleep until after scan starts
            diff = seconds_until(start_time) + 2
            if diff > 0:
                logger.debug ("%s - sleeping for %d secs", st, diff)
                time.sleep(diff)

            # --- get recording status
            where = mark6_send(m6_s, st, "record?;\n", "record? at start")
            if where != '':
                break
            where, ret = mark6_recv(m6_s, st, "?", "record? query at start")
            if where != '':
                break

            # --- sleep until after done
            diff = seconds_until(start_time) + int(delta) + 1
            if diff > 0:
                logger.debug ("%s - sleeping for %d secs", st, diff)
                time.sleep(diff)
            else:
                logger.debug("%s - Not Sleeping during scan", st)

            # --- query state of record, it should be off
            where = mark6_send(m6_s, st, "record?;\n", "record? at end")
            if where != '':
                break
            where, ret = mark6_recv(m6_s, st, "?", "record? query at end")
            if where != '':
                break
            # --- verify it is not recording
            pos = ret.find("?")
            info = ret[(pos+1):].split(':')
            state = info[2]

            # --- if recording, issue off command
            if state != 'off':
                logger.info( "%s - Mk6 issue record off required.", st )

                m6cmd = "record=off;\n"
                where = mark6_send(m6_s, st, "record=off;\n", "record=off")
                if where != '':
                    break
                where, ret = mark6_recv(m6_s, st, "=", "record=off")
                if where != '':
                    break

            # --- notice time remaining
            where = mark6_send(m6_s, st,
                "rtime?"+str(rate_mbps)+";\n", "rtime?")
            if where != '':
                break
            where, ret = mark6_recv(m6_s, st, "?", "rtime?")
            if where != '':
                break

            # --- make some allowance for recorder data to drain
            logger.debug ("%s - sleeping for %d secs", st, 4)
            time.sleep(4)

            # --- notice scan status
            where = mark6_send(m6_s, st, "scan_check?;\n", "scan_check?")
            if where != '':
                break
            where, ret = mark6_recv(m6_s, st, "?", "scan_check?")
            if where != '':
                break

            # --- nothing more to do for this scan
            logger.debug ("%s - sleeping for %d secs", st, 21)
            time.sleep(21)
            where = 'done'

	# --- it was too late for that scan...
        else:
            logger.warning("%s - dot time %s passed the start time %s",
                st, dot_time, start_time) 
            logger.warning("%s - skipping this scan...trying the next", st)
            where = 'skip'

	# --- bail if input file now missing
	if os.path.isfile(input_fn):
	    logger.debug ("%s - still have input fn %s", st, input_fn)
	else:
	    where = 'input file ' + input_fn + ' removed'
	    break

    # --- dboss normal status rate
    os.system('dboss s 4')
    logger.info("%s - restored dplane status to every 4 secs")

    # --- parting words
    logger.info("%s - reached this place via '%s'", st, where)
    logger.info("%s - %s file processing complete ", st, input_fn)
    m6_s.close()

if __name__ == '__main__':
    fn, host, port, rate = args()
    main(fn, host, port, rate)
    sys.exit(0)


#####  ENDOF FILE
