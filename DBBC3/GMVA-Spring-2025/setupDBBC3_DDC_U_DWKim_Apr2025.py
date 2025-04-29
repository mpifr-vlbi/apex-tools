#!/usr/bin/env python

import argparse
from dbbc3.DBBC3 import DBBC3  # v1.0.1 or at least 'develop' branch with https://github.com/mpifr-vlbi/dbbc3/commit/5b9dcf3
from dbbc3.DBBC3Config import DBBC3Config
from dbbc3.DBBC3Validation import ValidationFactory
import re
import sys
import numpy as np

from time import sleep

def reportResult(rep):

    if not rep:
        return

    for res in rep.result:
        if ("OK" in res.state):
            state = "\033[1;32m{0}\033[0m".format(res.state)
        elif ("FAIL" in res.state):
            state = "\033[1;31m{0}\033[0m".format(res.state)

        if ("ERROR" in res.level):
            level = "\033[1;31m{0}\033[0m".format(res.level)
        elif ("WARN" in res.level):
            level = "\033[1;35m{0}\033[0m".format(res.level)

        if "INFO" in res.level:
            print("[{0}] {1} - {2}".format(state,  res.action, res.message))
        elif ("WARN" in res.level ):
            print("[{0}]/[{1}] {2} - {3}".format(state, level, res.action, res.message))
        elif ("ERROR" in res.level ):
            print("[{0}]/[{1}] {2} - {3}".format(state, level, res.action, res.message))

        if len(res.resolution) > 0:
            print("\033[1;34m[{0}] {1}\033[0m".format("RESOLUTION",  res.resolution))

parser = argparse.ArgumentParser(description="Setup and validate DBBC3 in DDC_U mode")

parser.add_argument("-p", "--port", default=4000, type=int, help="The port of the control software socket (default: %(default)s)")
parser.add_argument("-n", "--num-coreboards", type=int, help="The number of activated core boards in the DBBC3")
#parser.add_argument("-i", "--if", dest='boards', nargs="+", help="A list of core boards to be used for setup and validation. (e.g. -i A C E). If not specified will use all activated core boards.")
parser.add_argument("-b", "--boards", dest='boards', type=lambda s: list(map(str, s.split(","))), help="A comma separated list of core boards to be used for setup and validation. Can be specified as 0,1 or A,B,.. (default: use all activated core boards)")
parser.add_argument("--use-version", dest='ver', default= "", help="The software version of the DBBC3 DDC_U mode to use. Will assume the latest release version if not specified")
parser.add_argument("--ignore-errors", dest='ignoreErrors', default=False, action='store_true', help="Ignore any errors and continue with the validation")
parser.add_argument("--check", dest='doCheck', default=False, action='store_true', help="Carry out a series of checks (power levels, sync ...)")
parser.add_argument("-s", "--ipaddress", dest='ipaddress', help="the IP address of the DBBC3 running the control software", default="192.168.0.91")
parser.add_argument("-m", "--mode", required=False, default="DDC_U", help="The current DBBC3 mode (default: %(default)s)")

args = parser.parse_args()

try:

        print ("===Trying to connect to %s:%d" % (args.ipaddress, args.port))
        dbbc3 = DBBC3(host=args.ipaddress, port=args.port, mode=args.mode)
        print ("===Connected")

        ver = dbbc3.version()
        print ("=== DBBC3 is running: mode=%s version=%s(%s)" % (ver['mode'], ver['majorVersion'], ver['minorVersion']))
        
        valFactory = ValidationFactory()
        val = valFactory.create(dbbc3, args.ignoreErrors)

        useBoards = []
        if args.boards:
            for board in args.boards:
                useBoards.append(dbbc3.boardToDigit(board))
        elif args.num_coreboards:
            for board in range(args.num_coreboards):
                useBoards.append(dbbc3.boardToDigit(board))
        else:
            for board in range(dbbc3.config.numCoreBoards):
                useBoards.append(dbbc3.boardToDigit(board))

        print ("=== Using boards: %s" % str(useBoards))

        print ("=== Setting BBC frequencies" )
        # bbc 1-8    board A
        # bbc 9-16   board B
        # bbc 17-24  board C
        # bbc 25-32  board D
        bbcFreq = {
             1: 128.0,  2: 256.0,  3:384.0,  4:512.0,
             9: 128.0, 10: 256.0, 11:384.0, 12:512.0,
            17: 128.0, 18: 256.0, 19:384.0, 20:512.0,
            25: 128.0, 26: 256.0, 27:384.0, 28:512.0
        } 

        print ("=== Current BBC frequencies" )
        for bbc in bbcFreq.keys():
            ret = dbbc3.dbbc(bbc)
            print ("BBC %s freq=%s bw=%s"%(bbc, ret['freq'], ret['bw']))

        print ("=== Updating BBC frequencies" )
        for bbc,freq in bbcFreq.items():
            print ("Setting BBC %d to %f" %(bbc, freq))
            dbbc3.dbbc(bbc, freq)

        print ("=== Post-update BBC frequencies" )
        for bbc in bbcFreq.keys():
            ret = dbbc3.dbbc(bbc)
            print ("BBC %s freq=%s bw=%s"%(bbc, ret['freq'], ret['bw']))

        if args.doCheck:
            print ("=== Checking sampler phases")
            reportResult(val.validateSamplerPhases())
            for board in useBoards:
                print ("=== Checking board %d" % (board))
                reportResult(val.validatePPS())
                reportResult(val.validateTimesync(board))
                reportResult(val.validateSynthesizerLock(board))
                reportResult(val.validateSynthesizerFreq(board))
                reportResult(val.validateIFLevel(board))
                reportResult(val.validateSamplerPower(board))
                # reportResult(val.validateSamplerOffsets(board))
                reportResult(val.validateBitStatistics(board))
        

except Exception as e:
        print (e)
       # make compatible with python 2 and 3
       #if hasattr(e, 'message'):
       #     print(e.message)
       #else:
       #     print(e)
finally:
       if 'dbbc3' in vars() or 'dbbc3' in globals():
               dbbc3.disconnect()
       print ("=== Done")
