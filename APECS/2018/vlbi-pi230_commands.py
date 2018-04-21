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

def vlbi_tp_onsource(src='M87',t=4):
    '''Total power monitoring for VLBI, taken while tracking source.
    
       t : integration time in MINUTES.
    '''
    execfile('pi230_setup.apecs')
    source(src)
    #track()   # probably should be changed to "point(<args>)"
    #on(drift='no',time=30)
    # Try this...JPE @ 2018-04-19   
    tp()    
    repeat(1)
    use_ref('ON')       
    reference(x=-100.0, y=0.0, time=0.0, on2off=1, unit='arcsec', mode='REL', system='EQ', epoch=2000.0)
    calibrate(time=5)
    use_ref('OFF')  
    pi230.configure(doppler='off') 
    repeat(t)
    on(drift='no',time=60)
    repeat(1)
    track()
    #use_ref('ON')    
    

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
