#!/usr/bin/env python
# $Id: create-schedule.py 1951 2014-04-25 21:12:15Z gbc $
#
# ALMA - Atacama Large Millimeter Array
# (c) Associated Universities Inc., 2013
# (c) Massachusetts Institute of Technology, 2013
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
#
###
#
# A script to exercise Mark6 recorders for testing purposes.
#

import base64
import calendar
import optparse
import os
import random
import sys
import textwrap
import time
import xml.etree.ElementTree

#
# handle the command line
#
def deal_with_opts():
    cmd='%prog [options]'
    desc='Mark 6 testing script.'
    epi=textwrap.dedent(
    '''\
    This script generates a test schedule from its arguments suitable
    for recorder testing.
    ''')
    vers="%prog $Id: create-schedule.py 1951 2014-04-25 21:12:15Z gbc $"
    parser = optparse.OptionParser(usage=cmd, description=desc,
                                   version=vers, epilog=epi)
    parser.add_option(
        '-v', '--verbose', dest='verb', help='provide some verbosity',
        action='store_true', default=False)
    parser.add_option(
        '-t', '--time', dest='time', help='length of run (hours)',
        default="1.0")
    parser.add_option(
        '-d', '--delay', dest='delay', help='before first scan (secs)',
        default="0")
    parser.add_option(
        '-s', '--scan', dest='scan', help='scan parameters: min,max (secs)',
        default="60,300")
    parser.add_option(
        '-i', '--idle', dest='idle', help='idle parameters: min,max (secs)',
        default="20,120")
    parser.add_option(
        '-e', '--exp', dest='expn', help='experiment name',
        default="test")
    parser.add_option(
        '-c', '--code', dest='code', help='station two-letter code',
        default="Aa")
    parser.add_option(
        '-a', '--aux', dest='aux', help='scan name aux info (e.g. pnX_bb1)',
        default="noaux")
    parser.add_option(
        '-n', '--numb', dest='numb', help='use DOY-HHMM for scan name',
        action='store_false', default=True)
    parser.add_option(
        '-f', '--full', dest='full', help='scan name is full name',
        action='store_true', default=False)
    parser.add_option(
        '-w', '--wid', dest='wid', help='width of upload field',
        default="55")
    parser.add_option(
        '-u', '--name', dest='name', help='upload file name (or help)',
        default="session")
    parser.add_option(
        '-x', '--xml', dest='save', help='save a copy of the xml',
        action='store_true', default=False)
    (o, a) = parser.parse_args()
    return(o)

# Take option parameters and generate a set of scans start/stop pairs
def create_scans(duration, idle, scan, delay):
    scans = []
    (imin, imax) = map(int, idle.split(','))
    (smin, smax) = map(int, scan.split(','))
    now = delay
    duration = duration + delay
    while now < duration:
        now = now + int(random.uniform(imin, imax)+0.5)
        dur = int(random.uniform(smin, smax)+0.5)
        end = now + dur
        if end < duration:
            scans.append([now, dur])
        now = end
    return scans

# Generate the xml scrap for one scan
def xmlscan(now, scan, expn, code, no, numb, aux, full, verb):
    xml_head = '<scan experiment="' + expn + '" source="dunno"'
    xml_head = xml_head + ' station_code="' + code + '" start_time="'
    xml_midd = '" duration="'
    xml_name = '" scan_name="'
    fn = expn + '_' + code + '_' 
    xml_tail = '" />'
    when = time.strftime('%Y%j%H%M%S', time.gmtime(now + scan[0]))
    xml = xml_head + when + xml_midd + str(scan[1]) + xml_name
    if no:
        sn = ('No%04d' % numb) # actually up to 16 chars are legal
    else:
        sn = time.strftime('%j-%H%M', time.gmtime(now + scan[0]))
    if aux != 'noaux':
        sn = sn + '_' + aux
    fn = fn + sn
    if full:
        xml = xml + fn + xml_tail
    else:
        xml = xml + sn + xml_tail
    if verb:
        print 'scan-%04d [%s] at %s (%d) for %d s' % (
            numb, fn, when, now + scan[0], scan[1])
    return xml

# Turn the scanlist into an XML schedule -- Cf. fakescans of m6sim
def create_schedule(o, scanlist):
    now = calendar.timegm(time.gmtime())
    end = now
    start = time.strftime('%Y%j%H%M%S', time.gmtime(now))
    data = ''
    numb = 0
    for s in scanlist:
        numb = numb + 1
        end = now + s[0] + s[1]
        if o.numb:
            d = xmlscan(now, s, o.expn, o.code, True,
                numb, o.aux, o.full, o.verb)
        else:
            d = xmlscan(now, s, o.expn, o.code, False,
                numb, o.aux, o.full, o.verb)
        data = data + d
    finish = time.strftime('%Y%j%H%M%S', time.gmtime(end))
    preamble = '<experiment name="%s" station="%s" start="%s" end="%s">'
    postscript = '</experiment>'
    xml = (preamble % (o.expn, o.code, start, finish)) + data + postscript
    return xml

# repackage the xml into M6 execute= commands -- Cf. formatscans of m6sim
def formatscans(name, wid, data):
    mess = ''
    act = 'upload'
    while len(data) > wid:
        ecmd = 'execute=' + act + ":" + name + ":" + data[0:wid] + ';\n'
        mess = mess + ecmd
        data = data[wid:]
        act = 'append'
    ecmd = 'execute=finish:' + name + ":" + data + ';\n'
    mess = mess + ecmd
    return mess

# convert start time to VEX time
def vex_time(ts):
    year = ts[0:4]
    doy = ts[4:7]
    hr = ts[7:9]
    mn = ts[9:11]
    sc = ts[11:13]
    return year + 'y' + doy + 'd' + hr + 'h' + mn + 'm' + sc + 's'

# generate a set of record commands and check xml while at it.
def checkxml(data, slist, full, verb):
    tree = xml.etree.ElementTree.fromstring(data)
    cmds = ''
    index = 0
    now = 0
    for scan in tree.findall('scan'):
        exp = scan.get('experiment')
        sc = scan.get('station_code')
        scan_name = scan.get('scan_name')
        if full:
            nm = scan_name.lstrip( exp + '_' + sc + '_' )
        else:
            nm = scan_name
        st = scan.get('start_time')
        dur = int(scan.get('duration'))
        bs=dur # 8 Gbps
        # build a comparison string
        if now == 0:
            tim = calendar.timegm(time.strptime(st, '%Y%j%H%M%S'))
            now = tim - slist[0][0]
        start = time.strftime('%Y%j%H%M%S',time.gmtime(now + slist[index][0]))
        stvex = vex_time(start)
        compare = 'record=' + stvex + ':' + str(slist[index][1])
        compare = compare + (':%d:%s:%s:%s;\n' % (bs,nm,exp,sc))
        stv = vex_time(st)
        recline = "record=%s:%s:%d:%s:%s:%s;\n" % (stv,dur,bs,nm,exp,sc)
        if verb:
            print exp,sc,nm,st,dur
            print 'check: ',recline,
            print 'check: ',compare,
        cmds = cmds + recline
        index = index + 1
    return cmds

# this would re-assemble the upload scraps and do the whole nine yards.
def checkfmt(fmt, xml):
    xxx = ''
    lines = fmt.split(';')
    for l in lines:
        if len(l) > 1:
            xxx = xxx + l.split(':')[2].strip(';\n')
    if xxx == xml:
        return 'consistent'
    return 'inconsistent'

#
# do the requested work of creating a schedule based the option arguments
#
def cs_main(o):
    sess = o.name.split(':')
    if sess[0] == 'random':
        sess[0] = base64.b64encode(os.urandom(3), '+-')
        o.expn = sess[0]
    sess.append(sess[0])

    scanlist = create_scans(float(o.time)*3600, o.idle, o.scan, int(o.delay))
    xml = create_schedule(o, scanlist)
    if o.verb:
        print '#' + '--'*39
    if o.verb:
        print xml
        print '#' + '--'*39
    rec = checkxml(xml, scanlist, o.full, o.verb);
    if o.verb:
        print rec,
        print '#' + '--'*39

    fmt = formatscans(sess[1], int(o.wid), xml)
    if o.verb:
        print fmt,
        print '#' + '--'*39
    chk = checkfmt(fmt, xml);
    # provide the output
    if sess[0] == 'makexml':
        print xml
    elif sess[0] == 'record':
        print rec,
    elif sess[0] == 'session':
        print chk
    else:
        print fmt,
    if o.save:
        f = open(sess[1] + '.xml', 'w')
        f.write(xml)
        f.close
        f = open(sess[1] + '.rec', 'w')
        f.write(rec)
        f.close
        f = open(sess[1] + '.exe', 'w')
        f.write(fmt)
        f.close

#
# utility widget
#
def tw_main(o):
    unow = calendar.timegm(time.gmtime())
    unix = calendar.timegm(time.strptime(o.time, '%Yy%jd%Hh%Mm%Ss'))
    print o.time,unix,unow,unix-unow

# enter here
if __name__ == '__main__':
    o = deal_with_opts()
    if o.name == 'help':
        print 'wait           tells how long a wait it will be until the time'
        print '               specified by the -t vex time argument'
        print ''
        print 'makexml[:name] prints out an xml schedule file'
        print 'record[:name]  prints out a list of record=start_time commands'
        print 'session[:name] checks that execute= commands will assemble'
        print '               properly, and if a name is supplied it will'
        print '               be used, instead of the default "session"'
        print 'random         generates a random schedule file name and'
        print '               uses the same name for the experiment'
        print ''
        print 'anything else prints out execute= upload commands for mark6'
        print 'using the supplied name for the upload session name.'
        print ''
        print 'If you set the -x flag, then the xml will be saved in a file'
        print 'with the session name (.xml) as will the schedule (.rec)'
        print 'and the execute= sequence of commands (.exe).'
    elif o.name == 'wait':
        tw_main(o)
    else:
        cs_main(o)
    sys.exit(0)

#
# eof
#
