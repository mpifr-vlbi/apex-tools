#!/usr/bin/env python

import argparse
import subprocess
from dbbc3.DBBC3 import DBBC3
from dbbc3.DBBC3Config import DBBC3Config
from dbbc3.DBBC3Validation import ValidationFactory
import dbbc3.DBBC3Util as d3u
import re
import sys
import numpy as np
import traceback
from signal import signal, SIGINT

from time import sleep

def exitClean():
    if 'dbbc3' in vars() or 'dbbc3' in globals():
        # re-enable the calibration loop
        dbbc3.enableloop()

        dbbc3.disconnect()

    print ("Bye")
    sys.exit()

def signal_handler(sig, frame):
    exitClean()

# handle SIGINT (Ctrl-C)
signal(SIGINT, signal_handler)

if __name__ == "__main__":
        parser = argparse.ArgumentParser(description="Setup DBBC3 for 690G Test")

        parser.add_argument('ipaddress', default='10.0.2.91',  help="the IP address of the DBBC3 running the control software")

        args = parser.parse_args()

        try:

                print ("===Trying to connect to %s" % (args.ipaddress))
                dbbc3 = DBBC3(host=args.ipaddress)
                print ("===Connected")

                ver = dbbc3.version()
                print ("=== DBBC3 is running: mode=%s version=%s(%s)" % (ver['mode'], ver['majorVersion'], ver['minorVersion']))


                useBoards = []
                for board in range(dbbc3.config.numCoreBoards):
                    useBoards.append(dbbc3.boardToDigit(board))

#                print ("=== Using boards: %s" % str(useBoards))


                # load tap filters 
                print ("=== Loading tap filters " )
                dbbc3.tap(0,1, "2000-4000_64taps.flt")
                dbbc3.tap(0,2, "2000-4000_64taps.flt")
                dbbc3.tap(1,1, "2000-4000_64taps.flt")
                dbbc3.tap(1,2, "2000-4000_64taps.flt")
                dbbc3.tap(2,1, "2000-3000_64taps.flt")
                dbbc3.tap(2,2, "2000-3000_64taps.flt")
                dbbc3.tap(3,1, "2000-3000_64taps.flt")
                dbbc3.tap(3,2, "2000-3000_64taps.flt")

                # setting 2nd LO freq
                # board A&B (230 GHz of "690 GHz Track") LO = 9051 = 2*4525.5 MHz
                # board C&D (230 GHz of "690 GHz Track") LO = 9045 = 2*4522.5 MHz
                dbbc3.sendCommand("synth=1,source 1")
                dbbc3.sendCommand("synth=1,cw 4525.5")
                dbbc3.sendCommand("synth=1,source 2")
                dbbc3.sendCommand("synth=1,cw 4525.5")
                dbbc3.sendCommand("synth=2,source 1")
                dbbc3.sendCommand("synth=2,cw 4522.5")
                dbbc3.sendCommand("synth=2,source 2")
                dbbc3.sendCommand("synth=2,cw 4522.5")

                dbbc3.disconnect()
                print ("=== Done")

        except Exception as e:
               # make compatible with python 2 and 3
               if hasattr(e, 'message'):
                    print(e.message)
               else:
                    print(e)
                    
               exitClean() 

        
