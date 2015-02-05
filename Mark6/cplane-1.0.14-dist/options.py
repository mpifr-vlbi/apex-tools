#
# options.py
#
# CREATED BY CHESTER RUSZCZYK ON Feb. 1 2013.
#
#
# THIS MODULE IS A DEFAULT MODULE.  IT PROCESSES THE COMMAND LINE OPTIONS
# IF IT THERE IS NOT A -h RUNS NORMALLY.  IF A -H EXISTS IN THE STRING
# THEN IT PRINTS THE OPTIONS AND RETURNS 0


##  This software is released under the terms of the MIT License(included below).
##  Copyright (c) 2009 MIT Haystack Observatory

##  Permission is hereby granted, free of charge, to any person obtaining a
##  copy of this software and associated documentation files (the "Software"),
##  to deal in the Software without restriction, including without limitation
##  the rights to use, copy, modify, merge, publish, distribute, sublicense,
##  and/or sell copies of the Software, and to permit persons to whom the
##  Software is furnished to do so, subject to the following conditions:

##  The above copyright notice and this permission notice shall be included in
##  all copies or substantial portions of the Software.

##  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
##  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
##  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
##  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
##  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
##  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
##  SOFTWARE.

import drsState
import string
import drsParser
import os
import logging
import logfunct

#logging.basicConfig(level=logging.warning, format="%(created)-15s %(msecs)d %(levelname)8s %(thread)d %(name)s %(message)s")
log = logging.getLogger("drs")

BACKLOG = 5

params = ['-e', '-f', '-h', '-m', '-s', '-b', '-p']

usage = ''' Usage:\t mark5c [options]
\t mark5c [-h| --help] [-v| --version]

 Options:
\t -e \t [command] \t execute command immediately after starting
\t -f \t [file] \t execute the mark5c commands contained in file upon start
\t -m \t [level] \t set output message level to level (between 0 and 4), default 1 
\t -s \t [number] \t set maximum number of simultaneous connections to number, default 5
\t -b \t [path] \t set STREAMSTOR_BIB_PATH to path
\t -p \t [path] \t set the path were system programs can live
'''

def get_args (input_args):
    global log
    log.debug("get_args: input arguments are %s" % (input_args))
    #assume all arguments are valid
    ret_val = 1

    #IF THE INPUT_ARGS ARE NOT MEMBERS EXIT
    if '-h' in input_args:
        print usage
        ret_val = 0
    elif '-v' in input_args:
        print "DRS version %s, (%s)" % (drsState.VERSION, drsState.DATE)
        ret_val = 0

    if ret_val == 1:
        for arg in input_args:
            if not arg in params:
                # --- IF THE ARGUMENT DOES NOT CONTAIN A HYPHEN, SKIP IT
                if string.find(arg, "-") < 0:
                    log.debug("a hyphen was not found in %s" % arg)
                else:
                    log.debug(" -arg found not in the supported list %s" % arg)
                    ret_val = -1
                break

            print "continue to process the arguments"
            if '-b' in input_args:
                log.debug("options:get_args - updating streamstor bibpath function not supported yet")
                loc = input_args.index('-b')
                print "value is %s" % input_args[loc+1]
		if os.environ.has_key('STREAMSTOR_BIB_PATH'):
                    print 'Changing STREAMSTOR_BIB_PATH from %s to %s' % (os.environ['STREAMSTOR_BIB_PATH'], input_args[loc+1])
                # --- SET STREAMSTOR_BIB_PATH
		os.environ['STREAMSTOR_BIB_PATH'] = input_args[loc+1]
                ret_val &= 1

            if '-m' in input_args:
                log.debug("options:get_args updating message level from %d" % drsState.app['msg_level'])
                loc = input_args.index('-m')
                drsState.app['msg_level'] = int(input_args[loc+1])
                level = int(drsState.app['msg_level'])
                log.debug("options:to %d" % drsState.app['msg_level'])
                # --- DETERMINE THE EQUIVALENT WORD LEVEL

                if level == 4:
                    #eq_level = "CRITICAL"
                    eq_level=50                    
                elif level == 3:
                    #eq_level = "ERROR"
                    eq_level=40                    
                elif level == 2:
                    #eq_level = "WARNING"
                    eq_level=30                    
                elif level == 1:
                    #eq_level = "INFO"
                    eq_level=20                    
                elif level == 0:
                    #eq_level = "DEBUG"
                    eq_level=10
                #elif level == 0:
                #    eq_level = 0 # NOSET
                else:
                    msg = "Unsupported option -m %d" % level
                    logfunct.err("1", msg)
                    log.warning("since the msg is unsupported, setting the level to info")
                    eq_level = 20
                    #ret_val = 0

                    #return ret_val

                log.info("Level passed for logging is %s" % eq_level)
                logging.getLogger().setLevel(eq_level)
                # --- read the level that was set
                log.warning("logging set to %s" % logging.getLogger().getEffectiveLevel())
                ret_val &= 1;

            if '-e' in input_args:
                log.info("execute command -e")
                loc = input_args.index('-e')
                log.debug("value is %s" % input_args[loc+1])

                opt_results = drsParser.cmd_process(input_args[loc+1])
                log.info("options.get_args - options result is %s" % opt_results)
                ret_val &= opt_results[0]
                log.info("options.get_args ret_val %d" % ret_val)

            if '-f' in input_args:
                log.info("commands in file -f")
                loc = input_args.index('-f')
                log.info("value is %s" % input_args[loc+1])
                file_name = input_args[loc+1]
                # --- OPEN FILE AND READ COMMANDS
                cmd_file = open(file_name, 'r')
                # --- WE ASSUME THE COMMAND CAN ALL BE IN ONE LINE OR A COMMAND / LINE
                for line in cmd_file:
                    opt_results = drsParser.cmd_process(line)
                    log.info("options.get_args - options result is %s" % opt_results)
                    ret_val &= opt_results[0]
                    log.info("options.get_args ret_val %d" % ret_val)

                # --- CLOSE THE FILE
                cmd_file.close()

            if '-s' in input_args:
                log.debug("options:get_args setting max simult. connections allowed from %d" % drsState.app['max_conn'])
                loc = input_args.index('-s')
                drsState.app['max_conn'] = int(input_args[loc+1])
                log.debug("to %d" % drsState.app['max_conn'])

            if '-p' in input_args:
                log.debug("options:get_args setting prog exe from %s" % drsState.app['prog_exe'])
                loc = input_args.index('-p')
                log.debug("value is %s" % input_args[loc+1])
            # --- UPDATE SYSTEM PROG_ROOT
                drsState.app['prog_exe'] = input_args[loc+1]
                log.debug("to %s" % drsState.app['prog_exe'])
                ret_val &= 1

    return ret_val




