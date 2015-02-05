## Copyright 2011 MIT Haystack Observatory
##
## This file is part of Mark6 VDAS.
##
## Mark6 VDAS is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, version 3 of the License.
##
## Mark6 VDAS is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Mark6 VDAS.  If not, see <http://www.gnu.org/licenses/>.

'''
Author:   del@haystack.mit.edu
Date:     5/12/2011
Description:

'''

import logging
import re
import string

import Command
import Response
import State

# Globals
#CMD_RE_STRING = "(\w+)=((\w+)(:\w+)*)*;"
CMD_RE_STRING = "(\w+)=(\w+.*)(:\w+.*)*;"
QRY_RE_STRING = "(\w+)\\?((\w+)(:\w+)*)*;"
#QRY_RE_STRING = "(\w+)\?(\w+)(:\w+.*)*;"
EXIT_RE_STRING = "exit;"

# In re form.
CMD_RE = re.compile(CMD_RE_STRING)
QRY_RE = re.compile(QRY_RE_STRING)
EXIT_RE = re.compile(EXIT_RE_STRING)

log = logging.getLogger('M6')

class Parser(object):
    '''Implements command pattern.

    Parses an incoming query string and generates an executable command
    object based on this. Returns the object to the caller for execution.
    '''
    
    def __init__(self):
        pass

    def re_match(self, s, regexp, re_type):
 
        log.debug("Parser:re_match s %s, re_type %s" %(s, re_type))
        s = s.strip()
        #print "s is ", s
        m = re.match(regexp, s)
        #print m
        result = {}
        if m is not None:
            log.debug("there was a match ")
            groups = list(m.groups())
            result = {
                'type': re_type,
                'name': groups[0].lower(),
                'params': [] }

            if len(groups) > 1 and groups[1]:
                result['params'].extend(groups[1].split(':'))
        
        log.debug("returning result %s" % result)              
        return result

    def parse(self, s):
        log.debug("Parser:parse - received %s to parse" % s)
        parsed = self.re_match(s, CMD_RE, 'CMD')
        log.debug("Parser:parse - parsed is %s" % parsed)
        
        ldict = len(parsed)
        #print "length of parsed is ", ldict

        if ldict < 1:
            log.debug("Parser:parse - not a command - is it query?")
            parsed = self.re_match(s, QRY_RE, 'QRY')
            ldict = len(parsed)

        if ldict < 1 :
            log.error("Parser:parse - neither cmd / qry %s " % parsed)
            s = s[:s.rfind(';')]
            s += '?'
            e_mess = s + ":1:7;  neither cmd / query;"
            # --- IF UNPROTECT FLAG WAS SET, RESET IT.
            if State.unprotect_flag == 1: 
                State.unprotect_flag = 0
                log.info("resetting unprotect, since exception occurred in next command")
            raise Exception(e_mess)


        # Get the appropriate command object.
        if parsed['name'] not in Command.COMMANDS:
            e_mess = parsed['name'] + ":1:7;  no such keyword;"
            if State.unprotect_flag == 1: 
                State.unprotect_flag = 0
                log.info("resetting unprotect, since exception occurred in next command. no keyword")

            raise Exception(e_mess)

        else:
            # --- CHECK IF UNPROTECT_FLAG IS SET AND IF NOT GROUP COMMAND RESET IT, GROUP COMMAND WILL RESET IN EXECUTE
            if State.unprotect_flag == 1 and parsed['name'] != "group":
                log.info("resetting unprotect, since group command was not issued immediately")
                State.unprotect_flag = 0

            # e.g., input_stream -> InputStreamQuery || InputStreamCommand
            parsed_name = ''.join(string.capwords(parsed['name'], '_').split('_'))
            if cmp(parsed['type'], 'QRY') == 0:
                command = parsed_name + 'Query'
            elif cmp(parsed['type'], 'CMD') == 0:
                command = parsed_name + 'Command'
            else:
                raise Exception('invalid command type: %s', parsed['type'])

            log.debug("Parsed.parse - command to execute -%s-" % command)

        # Retrieve the command class from the Command module. 
        log.debug("Parsed.parse - retrieve the command class from Command Module")
        command = getattr(Command, command)
        log.debug("Parsed.parse - exit")
        return command(parsed)

