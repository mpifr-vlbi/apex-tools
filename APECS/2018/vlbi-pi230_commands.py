import apexObsUtils
import socket
import sys
import subprocess
import datetime
import os

#############################################################################
# VLBI Calibration Helpers
#############################################################################

# vlbi_tpoint()              Total power pointing for VLBI
# vlbi_tp_onsource()         Total power pointing for VLBI, taken while tracking source
# vlbi_wpoint(t=20,cal=1)    Wobbler pointing for VLBI
# vlbi_focus(axis='Z')       Focus scan for VLBI
# vlbi_initiate_tsys()       Start a calibration (Tsys measurement)
# vlbi_get_calibration()     Collect calibration results

def vlbi_tpoint():
    '''Total power pointing for VLBI.'''
    tp()
    point(150, time=20)

def vlbi_tp_onsource():
    '''Total power pointing for VLBI, taken while tracking source.'''
    track()
    on(drift='no',time=30)

def vlbi_wpoint(t=20,cal=1):
    '''Wobbler pointing for VLBI.'''
    if (cal):
       calibrate('cold')

    wob(75, 1)
    #point(150, time=t)
    point(length=54, unit='arcsec', time=t, mode='ras', points=5, direction='x')
    tp()

def vlbi_focus(axis='Z'):
    '''Focus scan for VLBI.'''
    vlbi_focus.func_defaults = (axis,)
    wob(75, 1)
    focus(amplitude=1, points=5, axis=axis)
    tp()

def vlbi_initiate_tsys():
    '''Start a calibration (Tsys measurement).'''
    reference (-1000,0)
    calibrate()
    reference (0,0)

def vlbi_get_calibration():
    '''Collect calibration results'''
    onlineCal = apexObsUtils.getApexCalibrator()
    try:
        calResult = onlineCal.getCalResult('PI230-PBE_C',1,0)
    except:
        print 'No calibration result available.'


