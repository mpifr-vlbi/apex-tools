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
    (Re)Configure nflash230.

    This needs to be invoked regulary in case 1) operator interaction left apecs
    tuned to e.g. CO line instead of vlbi freq, or 2) backend for Tsys was left
    in continuum rather than in line mode.

    Note: In 2022 we verified CO line pointing works fine while tuned to 'vlbifreq7',
          nevertheless it remains prudent for the scripted schedule to hammer in the
          correct VLBI tuning at every opportunity.
    '''

    setup_nflash( fenames=['nflash230'],
        linenames=['vlbifreq7'],
        sidebands=[''], mode='spec', sbwidths=[8], numchans=[65536],
        cats='all',
        doppler='off' )

    nflash230.configure(doppler='off') # prevent Doppler correction during VLBI scan on()
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

    2022: on(drift='no',time=10) takes ~40 seconds
          on(drift='no',time=5)  takes ~30 seconds
    '''

    vlbi_tuning()

    # Set reference in Horizontal mode, so it is at the same elevation as the target source.
    use_ref('ON')
    reference(x=-100.0, y=0.0, time=0.0, on2off=1, unit='arcsec', mode='REL', system='HO', epoch=2000.0)
    tp()  # wobbler off

    # Do one on() subscan that has a reference
    repeat(1)
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
    # on(drift='no',time=30) # EHT2021, changed to 50% of t_mins from middle of e21b09 due to overheads (30%) that are greater than before
    on(drift='no',time=30)   # EHT2022 assume same high overhead of EHT2021. Worked out okay in e22b19 with 1-5min long scans.

    # Continue tracking for remainder of VLBI scan; ought to be less than auto-standby timeout time
    repeat(1)
    track()



#############################################################################
# TEST CODE / EXAMPLE CODE
#############################################################################

# ==EXAMPLE== function by JP below for Tsys measurement
# JP recommended this process:
#  do a 1min calibrate OFF-Source before VLBI scan, if time allows -- use_ref('ON'); reference(x=-100.0, ...); repeat(1); on(drift='no', time=60); 
#  do the VLBI scan                                                -- use_ref('OFF'); repeat(N_minutes); on(drift='no', time=60):
#  do a 20s..1min Tsys VLBI scan, if time allows                   -- calibrate(time=5);
#      move the Tsys to be done after the Off-Source reference cal and _before_ the VLBI scan rather than after
#
def vlbi_tp_onsource_JP(src='M87', t_mins=4):
    '''Total power monitoring for VLBI, taken while tracking source.
    
       t_mins : integration time on-source in MINUTES, not including other calibrations (1-2 minutes extra overhead)
    '''

    ## EXAMPLE
    # UNUSED CODE

    nflash230.configure(doppler='off')
    #
    # First on() subscan will have a reference
    #
    # Set reference in Horizontal mode, so it is at the same elevation as the target source.
    use_ref('ON')
    reference(x=-100.0, y=0.0, time=0.0, on2off=1, unit='arcsec', mode='REL', system='HO', epoch=2000.0)
    repeat(1)
    on(drift='no', time=60)
    
    #
    # The next on() subscans will not have a reference to stay on target.
    # 
    use_ref('OFF')
    repeat(t-1)
    on(drift='no', time=60)
    
    #
    # Just stay on target
    #
    repeat(1)
    track()

    # Now get Tsys
    calibrate(time=10)
    #use_ref('ON')


# ------------------- probably unused functions:

def vlbi_wpoint(t=20,cal=1):
    '''Wobbler pointing for VLBI.'''
    nflash230.configure(doppler='on')  # should be OFF
    if (cal):
        calibrate('cold')
    wob(amplitude=75, rate=1.5, mode='pos')
    #point(150, time=t)
    point(length=54, unit='arcsec', time=t, mode='ras', points=5, direction='x')
    wob(amplitude=75, rate=1, mode='sym')
    tp()
    nflash230.configure(doppler='off')  # This brings back the VLBI frequency for the next source (velocity=0)


def vlbi_focus(axis='Z',t=6):
    '''Focus scan for VLBI.'''
    nflash230.configure(doppler='on')
    vlbi_focus.func_defaults = (axis,)
    wob(amplitude=75, rate=1.5, mode='pos')
    focus(amplitude=1, points=5, axis=axis, time=t)
    wob(amplitude=75, rate=1, mode='sym')
    tp()
    nflash230.configure(doppler='off')  # This brings back the VLBI frequency for the next source (velocity=0)

def vlbi_get_calibration():
    '''Collect calibration results'''
    onlineCal = apexObsUtils.getApexCalibrator()
    try:
        calResult = onlineCal.getCalResult('PI230-PBE_C',1,0)
    except:
        print 'No calibration result available.'

def vwcpoint(t=24., l=[], cal=1, line='vlbifreq7', dopp='OFF', ptRun=False, dbpcorr=False):
    '''

    Continuum pointing cross scan in beam switching (wob) mode using pseudocontinumm.

    Parameters:   t: Integration time per subscan. Must be given.
                  l: Length of the arms of the cross
                     [] = use default value for current FE.
                cal: 1 = calibrate before the pointing
               line: 'vlbifreq7' = do pointing at vlbifreq7
                     '' = do pointing at current frequency
                     '*' = use standard line for current FE.
               dopp: 'ON' apply Doppler correction in the tuning frequency.
              ptRun: True = add "POINTING RUN" to the log comments.
            dbpcorr: True = apply pointing corrections from database.

    '''
    ask = 0
    doPoint(t, l, ask, cal, line, dopp, ptRun,
            dbpcorr, obsmode='wob', mode='otf')
