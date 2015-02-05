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
Author:   chester@haystack.mit.edu
Date:     11/15/2012
Description:

The StatusRequestHandler is a class that looks to receive status from 
d-plane and provide the main routine with information on the state
of the d-plane application.  It will use a queue to pass information
back to the main c-server thread.

'''

import logging
import socket
import sys
import Queue
import SocketServer
import binascii
import Parser
import struct
import ctypes 
import subprocess
from ctypes import create_string_buffer

import State
from module_libs import write_list

log = logging.getLogger('M6')

GSW_FACTIVE_ERR = 0x0001
GSW_FOPEN_ERR    = 0x0002
GSW_NO_STREAMS_ERR = 0x0004
GSW_ACTIVE_STREAMS_ERR = 0x0008
GSW_BUFFER_OVERRUN_ERR = 0x0010
GSW_WRITE_ERROR  =      0x0020
GSW_M5PKT_SEQUENCE_ERR= 0x0040
GSW_NO_FILES_ERR      = 0x0080
GSW_STR0_SEQUENCE_ERR = 0x0100
GSW_STR1_SEQUENCE_ERR = 0x0200
GSW_STR2_SEQUENCE_ERR = 0x0400
GSW_STR3_SEQUENCE_ERR = 0x0800
GSW_STR0_SPOOLING     = 0x1000
GSW_STR1_SPOOLING     = 0x2000
GSW_STR2_SPOOLING     = 0x4000
GSW_STR3_SPOOLING     = 0x8000
GSW_FILES_OPEN        = 0x10000

hostname = socket.gethostname()

rec_flag = 0

class StatusRequestHandler(SocketServer.ThreadingMixIn, SocketServer.BaseRequestHandler):
    '''Implements d-plane status updates.

    Receives incoming d-plane messages from a UDP socket. Processess the 
    incoming messages and updates the appropriate status word for access
    from the outside VSI-S world.  The messages are passed via a queue
    to the main application.  If d-plane is not started the status will
    indicate this.  Likewise, if the status messages are not being transmitted
    it will notify of this state also.
    '''

    STATEMENT_TERMINATOR = ';'

    #def __init__(self, request, client_address, server):
    #    self._queue = Queue.Queue()
    #    SocketServer.BaseRequestHandler.__init__(self, request,
    #                                             client_address,
    #                                             server)

    def handle(self):
        global hostname
        global rec_flag

        '''Handle incoming TCP connection from an operator/field system.'''
        #log.debug('StatusRequestHandler:Handle incoming connection')
        received_data = []
        b = create_string_buffer(76)
        #v = Parser.StatusParser()
        res = subprocess.Popen(["pidof", "%s", 'dplane'],  stdout=subprocess.PIPE)
        pidof_dplane = res.communicate()[0]

        if pidof_dplane == None:
            log.warning("SRH: dplane is not operational, check later")            
        #else:
        #    log.debug("SRH: dplane is operational, continue")            
        #while True:
        #    try:
        b = self.request[0].strip()
        rcv = binascii.b2a_hex(b)
        (code,seqno) = struct.unpack_from('II',b, 0)
        if code == State.command_tag['status']:
            #log.debug("SRH: process information ")
            (tsys, hn, gsw, gsw_fileno, ns) = struct.unpack_from('d20siii',b, 8)
            if hostname in hn:
                #log.debug("code is %d, seq # %d " % (code,seqno))
                log.debug("SRH: time %f, hostname %s, gsw 0x%04x, gsw_fn  %d, num_streams=%d " % (tsys, hn, gsw ,gsw_fileno, ns ))
                (rb_MB1, vtime1, good_packets1, psn_err_cnt1, len_err_cnt1, nbytes1 ) = struct.unpack_from('iiqqqq',b, 48)
                (rb_MB2, vtime2, good_packets2, psn_err_cnt2, len_err_cnt2, nbytes2 ) = struct.unpack_from('iiqqqq',b, 88)
                (rb_MB3, vtime3, good_packets3, psn_err_cnt3, len_err_cnt3, nbytes3 ) = struct.unpack_from('iiqqqq',b, 128)
                (rb_MB4, vtime4, good_packets4, psn_err_cnt4, len_err_cnt4, nbytes4 ) = struct.unpack_from('iiqqqq',b, 168)
                #(rb_MB1, vtime1, rb_MB2, vtime2,rb_MB3, vtime3,rb_MB4, vtime4  ) = struct.unpack_from('iiiiiiii',b, 80)
                #log.info("SRH: rb_MB %d, %d, %d, %d, vdiftime %d, %d, %d, %d" % (rb_MB1, rb_MB2, rb_MB3, rb_MB4, vtime1, vtime2, vtime3, vtime4))
                if ns > 0:
                    log.debug("  #, Mem GB , VDIFtime , valid ,  psn OoO , len discards, bytes")
                    log.debug(" 1 , %d, %d, %d, %d, %d, %d" %  (rb_MB1, vtime1, good_packets1, psn_err_cnt1, len_err_cnt1,nbytes1))
                if ns == 2:
                    log.debug(" 2 , %d, %d, %d, %d, %d, %d" %  (rb_MB2, vtime2, good_packets2, psn_err_cnt2, len_err_cnt2,nbytes2))
                if ns == 3 :
                    log.debug(" 3 , %d, %d, %d, %d, %d, %d" %  (rb_MB3, vtime3, good_packets3, psn_err_cnt3, len_err_cnt3,nbytes3))
                if ns == 4:
                    log.debug(" 4 , %d, %d, %d, %d, %d, %d" %  (rb_MB4, vtime4, good_packets4, psn_err_cnt4, len_err_cnt4,nbytes4))

                # --- IF THE NUMBER OF STREAMS IS SET, UPDATE STATUS TO STREAMS CONFIGURED  
                # --- (CHET) NEED TO PASS THIS TO A QUEUE SO YOU DO NOT HAVE CONFLICTS
                #log.debug("SRH:rec_flag = %d " % rec_flag)
                if ns:
                    State.c_status |= 0x200
                    #log.info("SRH:streams have been defined for dplane 0x%8x" %(State.c_status) )

                if gsw & GSW_BUFFER_OVERRUN_ERR:
                    State.c_status |= 0x80
                    max_vtime = max(vtime1, vtime2, vtime3, vtime4)
                    msg = "Buffer overflow at %d for scan %s" % (max_vtime, State.record_session['scan_name'])
                    State.add_error_msg(msg)

                if gsw & 0x10000:
                    State.c_status |= 0x10
                    rec_flag = 1
                    #log.info("SRH: Files are open for recording 0x%8x" %(State.c_status))
                    # --- IF THE MEMORY HAS DATA, AND REC FLAG IS ON, RECORING IS ON.
                    MBinMEM = rb_MB1 + rb_MB2 + rb_MB3 + rb_MB4
                    if MBinMEM > 0 :
                        State.record_session['action'] = 'on'
                        # --- CHANGE THE STATUS OF THE SCAN IN FILE_DICT TO OFF
                    else:
                        State.record_session['action'] = 'time'
                    
                    
                if rec_flag and not (gsw & GSW_FILES_OPEN):
                    #log.info("SRH: rec_flag is set and !(gsw & 0x10000) recording off")
                    # --- CLEAR THE BUFFER OVERUN ERROR AND 
                    State.c_status &= 0xfffff6f 
                    rec_flag = 0
                    log.debug("SRH: reset rec_flag and c_status to record off 0x%8x" %(State.c_status))
                    State.record_session['action'] = 'off'
                    # --- CHANGE THE STATUS OF THE SCAN IN FILE_DICT TO OFF
                    State.file_dict[State.sn]['status'] = 'recorded'
                    # --- APPEND DATA TO ALL DISKS META DIRECTORY
                    # --- WRITE THE DIRECTORY LIST TO THE META DATA LOCATION
                    if State.sn != 1:
                        act = 'append'
                    else:
                        act = 'new'

                    if write_list(State.open_group['slot'], act, State.sn):
                        log.info("Scan listing was successfully written to disk module")
                    else:
                        log.warning("Scan listing was not successfully written")

                #    State.c_status | =
            #received_data.append(rcv)
            #log.debug("Status_RH received %s" % b)
            
            # Parse command and lookup handler.
            #command = v.parse(received_command)
                status = 'received_command'
            #log.debug("StatusRequestHandler:handle - returned from parse calling command.execute() command is %s" % (status))
            # --- SEND THE PARSED COMMAND TO THE MAIN APPLICATION
                response = gsw
            #log.debug("StatusRequestHandler:handle - the new status word is %s" % response)
            #lresp= len(response)
            #log.debug("StatusRequestHandler:handle - len of response is %d" % lresp)
            #ret = response.find("\n")
            #if ret < lresp:
            #    log.warning("StatusRequestHandler:handle - a <cr> was found in string at %d" % ret)
            # --- send it to main application
            #self.request.sendall(response)

        else:
            log.warning("SRH: received INVALID message code -> %d from d-plane with seqno -> %d" % (code,seqno))


            
