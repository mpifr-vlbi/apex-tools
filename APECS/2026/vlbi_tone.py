#!/alma/ACS-2021DEC/pyenv/versions/2.7.16/bin/python
# testing, to be copied into vlbi-commands (though a reusable module would be nicer...)

import time
import requests

#############################################################################
# Tone Synthesizer Control
#############################################################################

def vlbi_tone(enable=False, freq_mhz=15315.0, pow_dbm=-4, scpi_url="http://10.0.6.66/scpi", simulate=False):
    '''
    Controls the output of an Agilent synthesizer via SCPI commands.
    Turn off, or softly turn on with a given freq by ramping power from -20 dBm to the target level.
    With simulate=True the commands will be shown but not actually sent.

    Examples:
    vlbi_tone(enable=False, simulate=True)
    vlbi_tone(enable=True, freq_mhz=15315.0, pow_dbm=-3.5, simulate=True)
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

#############################################################################

vlbi_tone(enable=False, simulate=True)
vlbi_tone(enable=False, pow_dbm=22, simulate=True)

vlbi_tone(enable=False, simulate=True)
vlbi_tone(enable=True, freq_mhz=15315.0, pow_dbm=-3.5, simulate=True)

vlbi_tone(enable=False)
