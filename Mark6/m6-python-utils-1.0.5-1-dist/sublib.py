#!/usr/bin/python
#
# library of functions

import commands
import optparse
import socket
import string

                    # function to send command to cplane,
                    # read its response, and parse it
def cplane_command (kommand, s, opts):
    if opts.verbose:
        print 'cplane command:', kommand
    s.sendall (kommand)
    resp = s.recv (8192)
    if opts.verbose:
        print 'cplane response:', resp
                    # parse response
    fields = resp.split(':')
    first = fields[0]
    (x, rc) = first.split('?')
    if rc != '0':
        print 'cplane command failure', resp
        exit(1)
    return fields[2:]


def cplane_running ():
    response = commands.getoutput('ps -A')
    if 'cplane' in response:
        rc = 1
    else:
        rc = 0
    return rc
