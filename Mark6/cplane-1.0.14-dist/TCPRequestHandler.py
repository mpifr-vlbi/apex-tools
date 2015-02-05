## Copyright 2011 MIT Haystack Observatory
##
## This file is part of Mark6 cplane.
##
## Mark6 cplane is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, version 2 of the License.
##
## Mark6 cplane is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Mark6 cplane.  If not, see <http://www.gnu.org/licenses/>.

'''
Author:   del@haystack.mit.edu
Date:     5/12/2011
Description:

'''

import logging
import socket
import sys
import Queue
import SocketServer
import errno 
import Parser
import State
import logging
from time import time
from Utils import linux_cmd

log = logging.getLogger('M6')


class TCPRequestHandler(SocketServer.BaseRequestHandler):
    '''Implements Command Pattern.

    Receives incoming commands from a socket. Accumulates characters
    into full command strings. Passes strings on to Parser which generates
    an executable Command object.  Executes the command object and then
    returns response across TCP socket.
    '''

    STATEMENT_TERMINATOR = ';'

    def __init__(self, request, client_address, server):
        self._queue = Queue.Queue()
        SocketServer.BaseRequestHandler.__init__(self, request,
                                                 client_address,
                                                 server)

    def handle(self):
        '''Handle incoming TCP connection from an operator/field system.'''
        log.debug('TCPRequestHandler:Handle incoming connection')
        ST = TCPRequestHandler.STATEMENT_TERMINATOR
        
        received_data = []
        v = Parser.Parser()

        while True:
            #print "TCPRH in while"
            try:
                #b = self.request.recv(1).strip()
                b = self.request.recv(1)
                if not len(b):
                    break


                # Accumulate characters until statement terminator found.
                # --- SINCE SPACES WERE REPLACED HERE, AND CAUSED PROBLEMS WITH EXECUTE=UPLOAD, MOVED FUNCTION TO 
                #     IF NOT EXECUTE TO PERFORM THIS.
                #received_data.append(b.strip())
                received_data.append(b)
                if cmp(b, ST) == 0:
                    #print "-%s-" % received_data
                    received_command = ''.join(received_data).strip()
                    #print "-%s-" % received_command

                    if not "execute" in received_command:
                        received_command = received_command.replace(' ','')
                        #print "new -%s-" % received_command

                    if len(received_command) == 1:
                        continue

                    if received_command == 'exit;':
                        break

                    try:
                        # --- CHECK MODULE STATUS IF 6 SECS HAS PASSED. 
                        disk_check_t = time()
                        #print "time ", disk_check_t
                        disk_check_td = disk_check_t - State.disk_check_prev

                        # Parse command and lookup handler.
                        log.debug("TCPRequestHandler:handle - calling v.parse with %s" % received_command) 
                        #command = v.prase(received_command)
                        command = v.parse(received_command)

                        log.debug("TCPRequestHandler:handle - returned from parse calling command.execute() command is %s" % (command))
                        #log.debug("TCPRequestHandler:handle - calling command.execute() ")
                        response = command.execute()
                        log.debug("TCPRequestHandler:handle - returned from command.execute with %s" % response)

                        #lresp= len(response)
                        #log.debug("TCPRequestHandler:handle - len of response is %d" % lresp)
                        # --- CLIENTS EXPECT A TERMINATING \N ON THE RETURN STRING, ADDING
                        ret = response.find("\n")
                        if ret < 0:
                            response=response + "\n"

                        self.request.sendall(response)

                        ''' --- THIS SECTION OF CODE LOOKS FOR A INTERVAL AND CHECKS THE MD5SUM OF /PROC/SCSI/SCSI TO SEE
                             IF A MODULE WAS ADDED OR REMOVED.  THE CHECK_MODULE FLAG IS SET TO NOTIFY THE NEXT COMMAND
                             TO UPDATE THE MODULES STATE BEFORE PROCESSING.  THIS ONLY OCCURS IF NOT RECORDING '''

                        ''' --- IF A RESTART COMMAND WAS ISSUED, AND THE STATE.RESTART FLAG IS SET, AFTER THE REPONSE
                                IS SENT, RESTART CPLANE '''
                        if State.restart == 1: 
                            sys.stdout.flush()                            
                            python = sys.executable
                            os.execl(python, python, 'cplane')
                        
                        elif State.restart == 6:
                            sys.stdout.flush()                            
                            os.system('shutdown -r -t 3')
                        
                        if disk_check_td >= State.disk_poll_int and State.record_session['action'] != 'on':
                            State.disk_check_prev = disk_check_t
                            log.debug("Prev time to check the state of modules %d ",State.disk_check_prev)
                            # --- get the md5sum of /proc/scsi/scsi
                            md5sum_rv = linux_cmd(["/usr/bin/md5sum", "/proc/scsi/scsi"])
                            #print "md5sum_rv is, ", md5sum_rv
                            if md5sum_rv[0] == 1:
                                #print "=1"
                                md5sum_now = md5sum_rv[1].split()[0]
                                #print md5sum_now
                                #print "smp ", State.md5sum_prev
                                # --- if the md5sum_now != md5sum_prev then disks have changed
                                if md5sum_now != State.md5sum_prev:
                                    log.debug("md5sum_now != md5sum_prev, set check modules flag and reset md5sum_prev value")
                                    State.check_disks = 1
                                    # --- check_disk flag!
                                    State.md5sum_prev = md5sum_now
                            else:
                                log.warning("md5sum of /proc/scsi/scsi is not available")


                    except Exception, e:
                        log.info("sending exception %s"% e)
                        self.request.sendall(str(e))

                    received_data = []

            except KeyboardInterrupt: # #1755388 #926423
                raise

            except socket.timeout, e:
                received_data = []
                log.error('handle: Received timeout')

            except Exception, e:
                received_data = []
                log.error('handle: Other exception: %s'%e)

            #except BrokenPipe:
            except socket.error, e:
                if isinstance(e.args, tuple):
                    log.error("errno is %d" % e[0])
                    if e[0] == errno.EPIPE:
                        log.error("Detected remote disconnect")
		break

            except KeyboardInterrupt:
                log.debug("server: keyboard interrupt exception")
                break

