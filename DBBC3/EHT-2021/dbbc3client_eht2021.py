#!/usr/bin/env python

from dbbc3.DBBC3 import DBBC3

import argparse
import sys
from time import sleep

from threading import Thread, Lock
from prompt_toolkit import PromptSession
from prompt_toolkit.completion import WordCompleter

command_mutex = Lock()


class UserPrompt():

	def __init__(self):
		self.prompt_str = ">> "
		self.dbbc3_completer = WordCompleter([
			"dbbcifa", "dbbcifb", "dbbcifc", "dbbcifd", "dbbcife", "dbbciff",
			"core3hstats", "time", "checkphase", "reconfigure",
			"cal_offset", "cal_gain", "cal_delay", "enablecal", "enableloop", "disableloop",
			"adb3l", "core3h", "synth", "adb3linit", "core3hinit", "synthinit",
			"tap", "tap", "exit", "quit"
		])
		self.session = PromptSession(completer=self.dbbc3_completer)

	def getInput(self):
		txt = ""
		while len(txt) < 1:
			try:
				txt = self.session.prompt(self.prompt_str).strip()
			except Exception as e:
				# Ctrl-D pressed, or other exception
				txt = "exit"
		return txt


def execCommand(dbbc, command):
	'''Send a command to DBBC3 and return the response. Thread safe.'''
	command_mutex.acquire()
	dbbc.sendCommand(command)
	response = str(dbbc.lastResponse)
	command_mutex.release()	
	return response.strip()


def testPPSSync(useBoards):
    global resetPPS

    ppsState = []
    for board in useBoards:
        ret = dbbc3.pps_delay(board)
        ppsState.append("OK")

        if  (ret[0] > 2000):
            resetPPS = True
            ppsState[board] = "FAIL"
        line = "pps_sync on board %d: %d %d %s" % (board, ret[0], ret[1], ppsState[board])
        logger.debug(line)
    logger.info("pps states  {}".format(ppsState))


def monitoringInjector(dbbc, terminateFlag):
	seconds_count = 0 # count 0..59 and wrap
	while not terminateFlag():
		# TODO: invoke functions in DBBC3 Module
		try:
			if seconds_count % 10 == 0:
				result = execCommand(dbbc, "dbbcifa")
				print(result)
		except Exception as e:
			print("monitoringInjector: %s" % (str(e)))
		seconds_count = (seconds_count + 1) % 60
		sleep(1)


if __name__ == "__main__":

	parser = argparse.ArgumentParser(description="Simple client to send commands to the DBBC3")
	parser.add_argument("-p", "--port", default=4000, type=int, help="The port of the control software socket (default: 4000)")
	parser.add_argument('ipaddress',  help="the IP address of the DBBC3 running the control software")

	args = parser.parse_args()
	prompt = UserPrompt()

	try:
		print ("===Trying to connect to %s:%d" % (args.ipaddress, args.port))
		dbbc3 = DBBC3(host=args.ipaddress, port=args.port )
		print ("===Connected")
	except Exception as e:
		# make compatible with python 2 and 3
		if hasattr(e, 'message'):
			print(e.message)
		else:
			print(e)
		sys.exit(1)

	ver = dbbc3.version()
	print ("=== DBBC3 is running: mode=%s version=%s(%s)" % (ver['mode'], ver['majorVersion'], ver['minorVersion']))

	stop_monitor = False
	monitor = Thread(target=monitoringInjector, args=(dbbc3,lambda:stop_monitor))
	monitor.start()

	while True:
		command = prompt.getInput()
		if command.lower() in ['quit', 'exit']:
			break
		response = execCommand(dbbc3, command)
		print (response)

	stop_monitor = True
	monitor.join()

	dbbc3.disconnect()
