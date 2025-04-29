import apexObsUtils
import socket
import sys
import subprocess
import datetime
import os

#############################################################################
# VLBI Calibration and Scan/VLBI Recording Helpers
#############################################################################

def vlbi_tuning():
    '''
    (Re)Configure SEPIA.

    This needs to be invoked regulary in case 1) operator interaction left apecs
    tuned to e.g. CO line instead of vlbi freq, or 2) backend for Tsys was left
    in continuum rather than in line mode.
    '''

    setup_sepia( fenames=['sepia660'],
        linenames=['vlbifreq690'],
        sidebands=[''], mode='spec', numchans=[65536], sbwidths=[8],
        cats='all',
        doppler='off' )

    sepia660.configure(doppler='off')  # prevent Doppler correction during VLBI scan on()
    tp()                               # cancel any wob() wobbler config persisting from operator line pointing (JPE: 2021-04-13)
    use_ref('OFF')                     # avoid going off-source during VLBI scan on()


def vlbi_tsys(mode_='COLD',time_s=10):
    '''
    Initiate Tsys measurement. Should ideally be called on-target before vlbi_scan().

    2022: calibrate(time=6,mode='HOT')    does Sky-Hot without Cold, takes ~40 sec in total
          calibrate(time=10,mode='COLD')  full Sky-Hot-Cold, takes ~85 sec in total
    '''

    vlbi_tuning()
    reference(x=-100.0, y=0.0, time=0.0, on2off=1, unit='arcsec', mode='REL', system='HO', epoch=2000.0)
    calibrate(mode=mode_,time=time_s)


def vlbi_reference_scan():
    '''
    Take an on() scan with duration of 20s (was:~1 minute) with an off-source reference.
    Prior to calling this function, must already be tracking a source.
    '''

    vlbi_tuning()

    # Set reference in Horizontal mode, so it is at the same elevation as the target source.
    use_ref('ON')
    reference(x=-100.0, y=0.0, time=0.0, on2off=1, unit='arcsec', mode='REL', system='HO', epoch=2000.0)
    tp()  # wobbler off

    # Do one on() subscan that has a reference
    repeat(1)
    # on(drift='no',time=30) # 2 x 30s = 1 minute   ; e21b09 : too long for 2-3min gaps!
    # on(drift='no',time=10) # 2 x 10s = 20 s
    on(drift='no',time=5) # EHT2022: reduced to 2 x 5sec, from 2 x 10sec/EHT2021, due e22b19 scan gaps being very short

    # Continue tracking VLBI target
    use_ref('OFF')
    repeat(1)
    track()


def vlbi_scan(t_mins=5,targetSource=''):
    '''
    Call at the start of a VLBI scan, possibly after vlbi_reference_scan(),
    while already tracking the VLBI target source.

    Uses a series of on() measurements, because a plain track() does not signal an active
    observation in APECS and leads to auto-standby of the telescope some minutes later. 
    The series of on-scans also collect amplitude calibration data for VLBI postprocessing.

    Turns Doppler off, Wobbler off, disables off-source reference position; the telescope
    must be ON source for the whole  duration of the VLBI scan (~t_mins minutes).
    '''

    vlbi_tuning()

    if targetSource:
        # If 'targetSource' arg is not empty, make sure we are on that source before starting VLBI scan
        # todo: query current APECS source somehow? to avoid possibly no-op commands?
        source(targetSource,cats='user')
        go()
        track()

    # Fill most of the VLBI scan time with series of on()
    repeat(t_mins)
    # on(drift='no',time=60) # EHT2017, EHT2018
    # on(drift='no',time=30) # EHT2021: changed to 50% of t_mins from middle of e21b09 due to overheads (30%) that are greater than before
    on(drift='no',time=30)   # EHT2022, EHT2023: assume same high overhead as found EHT2021. Worked out okay in e22b19 with 1-5min long scans.

    # Continue tracking for remainder of VLBI scan; ought to be less than auto-standby timeout time
    repeat(1)
    track()


#############################################################################
# probably unused functions
#############################################################################

def vlbi_wpoint(t=20,cal=1):
    '''Wobbler pointing for VLBI.'''
    sepia660.configure(doppler='on')
    if (cal):
        calibrate('cold')
    wob(amplitude=75, rate=1.5, mode='pos')
    #point(150, time=t)
    point(length=54, unit='arcsec', time=t, mode='ras', points=5, direction='x')
    wob(amplitude=75, rate=1, mode='sym')
    tp()
    sepia660.configure(doppler='off')


def vlbi_focus(axis='Z',t=6):
    '''Focus scan for VLBI.'''
    sepia660.configure(doppler='on')
    vlbi_focus.func_defaults = (axis,)
    wob(amplitude=75, rate=1.5, mode='pos')
    focus(amplitude=1, points=5, axis=axis, time=t)
    wob(amplitude=75, rate=1, mode='sym')
    tp()
    sepia660.configure(doppler='off')


def vlbi_get_calibration():
    '''Collect calibration results'''
    onlineCal = apexObsUtils.getApexCalibrator()
    try:
        calResult = onlineCal.getCalResult('PI230-PBE_C',1,0)
    except:
        print 'No calibration result available.'

