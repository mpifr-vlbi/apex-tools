#!/usr/bin/python
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
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Mark6 cplane.    If not, see <http://www.gnu.org/licenses/>.

'''
Original Author:   Dave Lapsley
Author: Chester Ruszczyk (  chester@haystack.mit.edu)
Date:         5/12/2011
Description:

Implements the Mark6 cplane serving functionality.
'''

import logging
import optparse
import socket
import sys
import threading
import SocketServer
import os
import subprocess
import string
import Server
import TCPRequestHandler as TCPRequestHandler
import StatusRequestHandler as StatusRequestHandler
import Utils
import State

# Default parameters.
DEFAULT_VSIS_PORT = 14242
DEFAULT_VSIS_HOST = ''
DEFAULT_LOG_LEVEL = '1' # --- LOGGING.INFO
DEFAULT_COMMAND_FILE = '/opt/mit/mark6/etc/m6_commands'
DEFAULT_CONFIG_FILE = '/opt/mit/mark6/etc/m6_config.xml'

log = logging.getLogger('M6')

def main():
    # --- original location, but not processing correctly
    # --- initialize state of input_stream and group from previous start of c-plane.
    #State.init()

    parser = optparse.OptionParser(version="cplane "+State.Version)
    parser.add_option(
        '-p', '--port', dest='port', help='VSIS TCP port.',
        default=DEFAULT_VSIS_PORT)
    parser.add_option(
        '-H', '--host', dest='host', help='VSIS host.',
        default=DEFAULT_VSIS_HOST)
    parser.add_option(
        '-l', '--log_level', dest='log_level', help='Log level(0-debug,1-info,2-warning,3-error,4-critical).',
        default=DEFAULT_LOG_LEVEL)
    parser.add_option(
        '-c', '--commands', dest='command_file', help='Execute the Mark6 commands contained in file at Startup.',
        default=DEFAULT_COMMAND_FILE)
    parser.add_option(
        '-f', '--file_config', dest='config_file', help='Use the default settings contained in the XML file at Startup.',
        default=DEFAULT_CONFIG_FILE)

    (o, a) = parser.parse_args()

    log.debug ("cplane : main() calling set_log_level with %s" % o.log_level)
    Utils.set_log_level(o.log_level)
    log.debug ( "cplane : main() return set_log_level")

    if os.path.isfile(o.config_file):
        log.info('Config file exists and will be now processed')
    else:
        log.info('No Config file exists proceeding')

    if os.path.isfile(o.command_file):
        log.info('VSI-S commands exist in %s and will be now processed' % o.command_file)
        if Utils.process_cf(o.command_file):
            log.info('Commands in %s processed successfully' % o.command_file)
        else:
            msg = 'Not all commands in %s processed successfully' % o.command_file
            State.add_err_msg(msg)
    else:
        log.info('No Command file exists for initialization proceeding')

    # --- initialize state of input_stream and group from previous start of c-plane.
    State.init()

    # --- CHECK TO SEE IF DPLANE IS RUNNING
    Utils.dplane_status()

    log.debug("cplane : main() Starting server")
    
    # --- STATUS Server
    s_port = 7214
    # --- GET HOSTNAME
    hn = socket.gethostbyname(socket.getfqdn(socket.gethostname()))
    print 'hn = ', hn
    #hn = '192.52.61.255'
    SocketServer.UDPServer.allow_reuse_address=True
    status_server = SocketServer.UDPServer(("0.0.0.0", s_port), StatusRequestHandler.StatusRequestHandler)
    status_thread = threading.Thread(target=status_server.serve_forever)
    #vsi_thread = threading.Thread(target=vsi_server.)
    status_thread.setDaemon(True)
    status_thread.start()
    log.debug("cplane : main() Started thread status_server")
    #status_thread.join()
    #log.debug("cplane : main() Joined status_thread")       

    # TODO
    # CONFIGURATION_FILE='dimino6.xml'
    # logging.info("%s"%get_pypath_resource(CONFIGURATION_FILE))

    try:
        # --- MAIN VSI-S INTERFACE
        vsi_server = Server.Server((o.host, o.port), TCPRequestHandler.TCPRequestHandler)
        vsi_thread = threading.Thread(target=vsi_server.serve_forever)
        #vsi_thread = threading.Thread(target=vsi_server.handle_request)
        vsi_thread.setDaemon(True)
        vsi_thread.start()
        log.debug("cplane : main() Started thread vsi_server")
        vsi_thread.join()
        log.debug("cplane : main() Joinded thread")

    except (KeyboardInterrupt, SystemExit):
        vsi_thread.cleanup_stop_thread()
        status_thread.cleanup_stop_thread()
        sys.exit(0)

    except Exception, e:
        log.error('Server error: %s'%e)


if __name__ == '__main__':
    main()
    sys.exit(0)
