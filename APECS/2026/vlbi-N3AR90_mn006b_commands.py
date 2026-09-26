import apexObsUtils
import socket
import sys
import subprocess
import datetime
import os
import time
import requests

#############################################################################
# VLBI Calibration and Scan/VLBI Recording Helpers
#############################################################################

def vlbi_tuning():
    '''
    (Re)Configure n3ar90.

    This needs to be invoked regulary in case 1) operator interaction left apecs
    tuned to e.g. CO line instead of vlbi freq, or 2) backend for Tsys was left
    in continuum rather than in line mode.

    Note: In 2022 we verified CO line pointing works fine while tuned to 'vlbifreq230',
          nevertheless it remains prudent for the scripted schedule to hammer in the
          correct VLBI tuning at every opportunity.
    '''

    setup_n3ar90(
        linename='vlbifreq89',
        sideband='', mode='spec', sbwidth=8, numchan=65536,
        cats='all',
        doppler='off' )
    n3ar90.configure(doppler='off') # prevent Doppler correction during VLBI scan on()
    tp()                            # cancel any wob() wobbler config persisting from operator line pointing (JPE: 2021-04-13)
    use_ref('OFF')                  # avoid going off-source during VLBI scan on()


def vlbi_tsys(mode_='COLD',time_s=10,targetSource=''):
    '''
    Initiate Tsys measurement. Should ideally be called on-target before vlbi_scan().

    2022: calibrate(time=6,mode='HOT')    does Sky-Hot without Cold, takes ~40 sec in total
          calibrate(time=10,mode='COLD')  full Sky-Hot-Cold, takes ~85 sec in total
    '''

    if targetSource:
        source(targetSource,cats='user')

    vlbi_tuning()
    reference(x=-100.0, y=0.0, time=0.0, on2off=1, unit='arcsec', mode='REL', system='HO', epoch=2000.0)
    calibrate(mode=mode_,time=time_s)


def vlbi_reference_scan(targetSource=''):
    '''
    Take an on() scan with duration of 20s (was:~1 minute) with an off-source reference.
    Prior to calling this function, must already be tracking a source.

    2022: on(drift='no',time=10) takes ~40 seconds
          on(drift='no',time=5)  takes ~30 seconds
    '''

    if targetSource:
        source(targetSource,cats='user')

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
    ideally while already tracking the VLBI target source.

    t_mins: duration of VLBI scan
    targetSource: optionally name of the VLBI scan target

    Uses on() measurements rather than track(), since tracking does not signal an active
    observation under APECS and leads to auto-standby of the telescope some minutes later.

    The ON-scans also collect amplitude calibration data for VLBI postprocessing.

    Uses vlbi_tuning() prior to on() for standard setup: Doppler off, Wobbler off,
    off-source reference position disabled.
    '''

    # Make sure we are on target

    #if targetSource:
    #    # If 'targetSource' arg is not empty, make sure we are on that source before starting VLBI scan
    #    # todo: query current APECS source somehow? to avoid possibly no-op commands?
    #    # Overhead: 10-20 sec in 2026
    #    source(targetSource,cats='user')
    #    go()
    #    track()

    # Dirk Muders email 25.09.2026 11:xx:
    # - source() alone without extra go()+track() is faster and avoids 20sec overhead
    # - when already on-source, has little effect
    # - when accidentally still on different source, the next on() will trigger move to target
    if targetSource:
        source(targetSource,cats='user')
    vlbi_tuning()


    # Fill most of the VLBI scan duration with a series of on() measurements

    # EHT2022-2026 & GMVA 2026I:  repeat(n_rep) x on(30s):
    # n_rep = t_mins
    # repeat(n_rep)
    # #on(drift='no',time=60) # EHT2017, EHT2018
    # #on(drift='no',time=30) # EHT2021: changed to 50% of t_mins from middle of e21b09 due to overheads (30%) that are greater than before
    # on(drift='no',time=30)   # EHT2022, EHT2023: assume same high overhead of EHT2021. Worked out okay in e22b19 with 1-5min long scans.

    # EHT 2024:
    # Alternate method attempted for e24e07: single very long on()-scan, perhaps no phase jumps them, but perhaps no contiguous sub-integration data either?
    #on(drift='no',time=int(30*t_mins))
    # [[ e24e07 till e24d10 : used single on(drift='no',time=int(30*t_mins)) ]]
    # [[ e24g11: reverted back to the eht2023 known safe repeat(t_mins) x on(drift='no',time=30) ]]

    # GMVA 2026II : repeat(n_rep) x on(10s) with requested time shrunk by estimate of overhead
    # done like this until C262A 268-1445 inclusive
    # t_secs = int(t_mins * 60)
    # on_sec = 10
    # n_rep = int((t_secs*0.78) / on_sec) # 0.87: a bit too long, 0.82: too long
    # repeat(n_rep)
    # on(drift='no',time=on_sec)

    # GMVA 2026II : repeat(1) x on(seconds=60*t_mins*0.82) with requested time shrunk by estimate of overhead
    # based on Dirk Muders comments 25.09.2026 11:xx that N x on(10s) adjust hexapod between every 10 sec scan
    # Done like this from C262A 268-1801 onwards
    #
    # on_sec = int(60*t_mins*0.78)) # 0.78: from N x on(10s) but turned out too short here, 5min VEX -> 4min actual
    on_sec = int(60*t_mins) - 25 # use a fixed assumed worst case overhead of 25 sec
    if on_sec < 10:
        on_sec = 10
    repeat(1)
    on(drift='no',time=on_sec)

    # Continue tracking for remainder of VLBI scan; ought to be less than auto-standby timeout time
    repeat(1)
    track()


#############################################################################
# probably unused functions
#############################################################################

def vlbi_wpoint(t=20,cal=1):
    '''Wobbler pointing for VLBI.'''
    n3ar90.configure(doppler='on')
    if (cal):
        calibrate('cold')
    wob(amplitude=75, rate=1.5, mode='pos')
    #point(150, time=t)
    point(length=54, unit='arcsec', time=t, mode='ras', points=5, direction='x')
    wob(amplitude=75, rate=1, mode='sym')
    tp()
    n3ar90.configure(doppler='off')  # This brings back the VLBI frequency for the next source (velocity=0)


def vlbi_focus(axis='Z',t=6):
    '''Focus scan for VLBI.'''
    n3ar90.configure(doppler='on')
    vlbi_focus.func_defaults = (axis,)
    wob(amplitude=75, rate=1.5, mode='pos')
    focus(amplitude=1, points=5, axis=axis, time=t)
    wob(amplitude=75, rate=1, mode='sym')
    tp()
    n3ar90.configure(doppler='off')  # This brings back the VLBI frequency for the next source (velocity=0)


def vlbi_get_calibration():
    '''Collect calibration results'''
    onlineCal = apexObsUtils.getApexCalibrator()
    try:
        calResult = onlineCal.getCalResult('PI230-PBE_C',1,0) # FIXME: would N3AR90-PBE_C be correct?
    except:
        print 'No calibration result available.'


def vwcpoint(t=24., l=[], cal=1, line='vlbifreq230', dopp='OFF', ptRun=False, dbpcorr=False):
    '''

    Continuum pointing cross scan in beam switching (wob) mode using pseudocontinumm.

    Parameters:   t: Integration time per subscan. Must be given.
                  l: Length of the arms of the cross
                     [] = use default value for current FE.
                cal: 1 = calibrate before the pointing
               line: 'vlbifreq230' = do pointing at vlbifreq230
                     '' = do pointing at current frequency
                     '*' = use standard line for current FE.
               dopp: 'ON' apply Doppler correction in the tuning frequency.
              ptRun: True = add "POINTING RUN" to the log comments.
            dbpcorr: True = apply pointing corrections from database.

    '''
    ask = 0
    doPoint(t, l, ask, cal, line, dopp, ptRun,
            dbpcorr, obsmode='wob', mode='otf')


#############################################################################
# Tone Synthesizer Control
#############################################################################

def vlbi_tone(enable=False, freq_mhz=17819.0, pow_dbm=-6.0, scpi_url="http://10.0.6.66/scpi", simulate=False):
    '''
    Controls the output of an Agilent synthesizer via SCPI commands.
    Turn off, or softly turn on with a given freq by ramping power from -20 dBm to the target level.
    With simulate=True the commands will be shown but not actually sent.

    Examples:
    vlbi_tone(enable=False, simulate=True)
    vlbi_tone(enable=True, freq_mhz=17819.0, pow_dbm=1.0, simulate=True)
    '''

    powLimit_dbm = +4

    if pow_dbm > powLimit_dbm:
        print("Warning: vlbi_tone() requested power %.2f dbm exceeds safety limit of %.2f dbm. Ignoring command." % (pow_dbm, powLimit_dbm))
        return False

    if not enable:
        scpiCommands = ['output off', 'pow -20 dbm']
    else:
        scpiCommands = ['output off', 'pow -20 dbm', 'freq %.3f mhz' % (freq_mhz), 'output on']
        for ramp_dbm in range(-18, int(pow_dbm-1), 2):
            scpiCommands += ['pow %.2f dbm' % (ramp_dbm)]
        scpiCommands += ['pow %.2f dbm' % (pow_dbm)]

    # Encode commands, examples from Agilent web gui are:
    # http://10.0.6.66/scpi?s=output+off
    # http://10.0.6.66/scpi?s=pow+-4+dbm  = pow -4 dbm
    # http://10.0.6.66/scpi?s=freq+15315.00+mhz
    httpCommands = ['%s?s=%s' % (scpi_url,cmd.replace(' ','+')) for cmd in scpiCommands]
    for scmd,hcmd in zip(scpiCommands,httpCommands):
        if simulate:
            print("Tone synthesizer control (simulated), would send '%s' via %s" % (scmd,hcmd))
        else:
            print("Tone synthesizer control, sending '%s' via %s" % (scmd,hcmd))
            g = requests.get(hcmd)
        time.sleep(0.2)

