#!/usr/bin/env python
#
# APEX - Atacama Pathfinder EXperiment Project
#
# Copyright (C) 2011
#
# Max-Planck-Institut fuer Radioastronomie, Bonn, Germany
#
# Internet email: dmuders@mpifr-bonn.mpg.de
#
# "@(#) $Id: vlbi_commands.apecs,v 0.0 2011/03/24 05:12:42 vltsccm Exp $"
#
# who          when        what
# ---------    ----------  -------------------------
# J. Wagner    2013-03-16  Several updates after input from Dirk Muders
# J. Wagner    2010-04-02  Added VLBI routines (sendStr)
# D. Muders    2010-03-29  Added VLBI meta data collect function.
# D. Muders    2010-03-24  Created.

import apexObsUtils
import socket
import sys
import subprocess
import datetime
import os


#############################################################################
'''Special apecs commands for VLBI.'''
#############################################################################

def vlbi_send_str(strout,dest,port,infostr=''):
    '''Send a string in UDP packet to host (typically FieldSystem)'''

    try:
        ss = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        ss.sendto(strout, (dest, int(port)) )
        ss.close()
    except:
        print 'Sending string %s to %s:%d failed!' % (str(strout),str(dest),int(port))

    homepath = os.getenv("HOME")
    now = str(datetime.datetime.now())
    try:
        fout = open(homepath + '/vlbi_sendstr.log', 'a')
        header = '--vlbi_sendstr @ %s => %s:%s' % (now,str(dest),str(port))
        fout.write(header + '\n')
        fout.write(str(infostr) + str(strout) + '\n')
        fout.close()
    except:
        print 'Writing to vlbi_sendstr.log file failed!'


#############################################################################
# VLBI Init and De-Init
#############################################################################

def vlbi_init(gain=0.1):
    '''Initialize APEX VLBI setup.'''

    print 'Configuring 1mm VLBI'

    vlbi_init.func_defaults = (gain,)

    # List of calibrators and sources for manual intervention from APECS
    sourcecats('t-091.f-0006-2013.cat')

    # List of line frequencies,  currently just 'vlbifreq' for VLBI PFB backend setup
    linecats('t-091.f-0006-2013.lin')

    # Acquire SHeFI commands (==/data/soft/apecs/APECS-3.0/share/apecs/shfi_commands.apecs)
    exec_apecs_script('shfi_commands')  

    # Frontend SHeFI-1 without Doppler correction
    # setup_shfi(fename='het230',linename='sio1j54',sideband='',mode='spec', cats='user')
    setup_shfi(fename='het230',linename='vlbifreq2',sideband='',mode='cont', cats='user')

    het230.configure(doppler='off')

    # Run in Total Power mode, no freq or position switching/wobbler
    tp()

    # Disable the Sky-Hot-Cold for now
    offset(0,0)       # not observing mosaicing pattern
    reference(0,0)    # no reference pos for On-Off observations
    use_ref('off')


def vlbi_close():
    '''Clean up after VLBI.'''

    print 'Cleaning up 1mm VLBI'

    frontends('')
    reference(-1000,0)
    use_ref('on')
    remote_control('off')


#############################################################################
# VLBI Calibration Helpers
#############################################################################


def vlbi_tpoint():
    '''Total power pointing for VLBI.'''

    tp()
    point(150, time=20)


def vlbi_tp_onsource():
    '''Total power pointing for VLBI. Taken while tracking source.'''

    track()
    on(drift='no',time=30)

def vlbi_wpoint():
    '''Wobbler pointing for VLBI.'''

    wob(75, 1)
    point(150, time=30)
    tp()


def vlbi_focus(axis='Z'):
    '''Focus scan for VLBI.'''

    vlbi_focus.func_defaults = (axis,)

    wob(75, 1)
    focus(amplitude=1, points=5, axis=axis)
    tp()


def vlbi_get_tracking_status(dest,port):
    '''Report the current ONSOURCE status.'''
    # Simulation mode: ABM0   
    # Observing mode : ABM1
    status = apexObsUtils.getMCPoint('ABM[1,0]:ANTMOUNT:mode')
    vlbi_send_str(status,dest,port,'ONSOURCE= ')

    # print 'Tracking status is %s, sending to %s:%s' % (status,dest,port)

# Called from FieldSystem. Results are later polled by FieldSystem. 
# TODO: Python Corba event listener to inject results into FieldSystem (remote-exec of 'inject_snap')
def vlbi_initiate_tsys():
    '''Start a calibration (Tsys measurement).'''

    reference (-1000,0)
    calibrate()
    reference (0,0)

def vlbi_get_calibration():
    '''Collect VLBI meta data for FS log and inject it via remote ssh execute (TODO: daemon)'''

    onlineCal = apexObsUtils.getApexCalibrator()
    calStr = 'tsys/nodata'
    try:
        calResult = onlineCal.getCalResult('HET230-PBE_A',1,0)
        # this data has no timestamp?
        calStr = 'tsys/%f,trx/%f,gainimage/%f,tausig/%f,tauima/%f,wvr/%f,wvrrm/%f,thot/%f,tcold/%f' % ( \
            calResult.tSys[0], \
            calResult.tRx[0], \
            calResult.gainImage[0], \
            calResult.tauSig[0], \
            calResult.tauIma[0], \
            calResult.water[0], \
            calResult.waterRM[0], \
            calResult.tHot[0], \
            calResult.tCold[0] )
    except:
        print 'No calibration result available.'

    try:
        temperature = apexObsUtils.getMCPoint('APEX:WEATHERSTATION:temperature')
        timestamp = apexObsUtils.getMCPointTS('APEX:WEATHERSTATION:temperature')
    except:
        temperature = -273
        timestamp = (-1, -1)

    try:
        dewPoint = apexObsUtils.getMCPoint('APEX:WEATHERSTATION:dewPoint')
    except:
        dewPoint = -273

    try:
        humidity = apexObsUtils.getMCPoint('APEX:WEATHERSTATION:humidity')
    except:
        humidity = 0
    try:
        pressure = apexObsUtils.getMCPoint('APEX:WEATHERSTATION:pressure')
    except:
        pressure = 0

    try:
        windSpeed = apexObsUtils.getMCPoint('APEX:WEATHERSTATION:windSpeed')
    except:
        windSpeed = -999

    try:
        windDirection = apexObsUtils.getMCPoint('APEX:WEATHERSTATION:windDirection')
    except:
        windDirection = -999

    try:
        pwv = apexObsUtils.getMCPoint('APEX:RADIOMETER:RESULTS:pwv')
    except:
        pwv = -1

    try:
        GPSMinusFMOUT = apexObsUtils.getMCPoint('APEX:COUNTERS:GPSMINUSFMOUT:GPSMinusFMOUT')
    except:
        GPSMinusFMOUT = -1

    try:
        GPSMinusMaser = apexObsUtils.getMCPoint('APEX:COUNTERS:GPSMINUSMASER:GPSMinusMaser')
    except:
        GPSMinusMaser = -1


    timeStr = 'timestamp/%d' % (timestamp[1])
    gpsFormStr = 'gps-fmout/%d' % (GPSMinusFMOUT)
    gpsMaserStr = 'gps-maser/%d' % (GPSMinusMaser)
    wxStr = 'wx/temp/%f,dew/%f,hum/%f,p/%f,windspeed/%f,winddir/%f,pwv/%f' % ( \
        temperature,  dewPoint,  humidity,
        pressure, windSpeed, windDirection, pwv)

    now = str(datetime.datetime.now())
    print now
    print timeStr
    print calStr
    print gpsFormStr
    print gpsMaserStr
    print wxStr


    homepath = os.getenv("HOME")
    try:
        fout = open(homepath + '/fieldsys_calibdata.log', 'a')
        fout.write('--vlbi_get_calibration @ ' + now + '\n')
        fout.write('\\"' + timeStr + '\n')
        fout.write('\\"' + calStr + '\n')
        fout.write('\\"' + gpsFormStr + '\n')
        fout.write('\\"' + gpsMaserStr + '\n')
        fout.write('\\"' + wxStr + '\n')
        fout.close()
    except:
        print 'Failed to write local log file'

    # Invoke inject_snap on the fieldsystem computer FS to write a comment into the schedule log
    # Note: you need to have the APECS account ssh key installed on FS in authorized_keys
    #
    # The following should get executed: inject_snap "\"a comment"
    # After some crazy escaping this ends up as: '"\\\"a comment\"'

    prefix = '"\\\"'
    postfix = '\"'

    fs_host = "oper@10.0.2.95" # 10.0.2.95 = mark5c2 since FieldSystem failed to work on Mark6
    # oper@10.0.2.95:.ssh/authorized_keys has key of t-091.f-0006-2013@observer3
    ret = subprocess.call(["ssh", fs_host, "/usr2/fs/bin/inject_snap", prefix + calStr + postfix]);
    ret = subprocess.call(["ssh", fs_host, "/usr2/fs/bin/inject_snap", prefix + gpsFormStr + postfix]);
    ret = subprocess.call(["ssh", fs_host, "/usr2/fs/bin/inject_snap", prefix + gpsMaserStr + postfix]);
    ret = subprocess.call(["ssh", fs_host, "/usr2/fs/bin/inject_snap", prefix + wxStr + postfix]);
