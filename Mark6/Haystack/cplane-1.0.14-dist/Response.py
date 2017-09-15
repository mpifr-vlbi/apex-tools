## Copyright 2011 MIT Haystack Observatory
##
## This file is part of Mark6 VDAS.
##
## Mark6 VDAS is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, version 2 of the License.
##
## Mark6 VDAS is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Mark6 VDAS.  If not, see <http://www.gnu.org/licenses/>.

'''
Original Author:   del@haystack.mit.edu
Author:   chester@haystack.mit.edu
Date:     5/12/2011
Description:
'''

import logging

log = logging.getLogger('M6')

#-----------------------------------------------------------
class Response(object):

    def __init__(self, **kwargs):
        log.debug("Response:Response.__init__ entered")

        # Return parameters.
        self._params = []
        
        # List of return parameters.
        self._list_params = []
        try:
            for n in self.__class__.PARAMS:
                self.append_param(value=kwargs[n])
                #log.debug("Response:__init__ - value %s %s" % (n, kwargs[n]))
        except Exception, e:
            log.error('Response exception: %s'%e)

        try:
            for l in kwargs['list']:
                #list_param = [ '\n' ]
                list_param = []
                for k, v in zip(self.__class__.LIST_PARAMS, l):
                    list_param.append(v)
                    #log.debug("Response:__init__ list_param.append %s" % v)

                # NEED TO LOOK INTO THE LOGIC HERE, NOT FORMATTING CORRECTLY. (SYS_INFO? EXAMPLE)
                # --- originally here
                list_param_string = ":"+(':'.join(list_param))
                #log.debug("Response:__init__ list_param_string %s" % list_param_string)

                self.append_list_param(list_param_string)
                    # TODO: More checking.
        except Exception, e:
            log.error('Response exception: %s'%e)

    def execute(self):
        return str(self)

    def append_param(self, value):
        self._params.append(value)
        
    def append_list_param(self, value):
        self._list_params.append(value)
        
    def __str__(self):

        # --- ORIGINALLY ONLY RETURNED ? FOR COMMANDS AND QUERIES.  CORRECTED SYNTAX
        #     BYE ADDING A TYPE FIELD TO EACH CLASS SPECIFYING SET / GET TO INJECT THE CORRECT
        #     RETURN SYMBOL

        if "set" == self.__class__.TYPE:
            ret_sym = '='
        else:
            ret_sym ='?'

        resp = ''.join([
            '!',
            self.__class__.NAME,
            ret_sym,
            ':'.join(self._params)
            ])
        list_resp = ''
        if len(self._list_params):
            list_resp = ''.join(self._list_params)
        
        # --- original
        #return resp + list_resp[:-2] + ';'
        return resp + list_resp + ';'

#-----------------------------------------------------------
class DiskInfo(Response):

    NAME = 'disk_info'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'type',
        ]
    LIST_PARAMS = [
        'volref', 'msn', 'dsvd_disks', 'reg_disks', 'num_disks', 'd1','d2','d3','d4','d5','d6','d7','d8',
        ]
    TYPE = 'get'

#-----------------------------------------------------------
class Msg(Response):

    NAME = 'msg'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'error_msg',
        ]
    TYPE = 'get'

#-----------------------------------------------------------
class SetInputStream(Response):

    NAME = 'input_stream'
    #LIST_PARAMS = ['action','stream_label','data_format','interface_ID','filter_address',  ]
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'

class GetInputStream(Response):

    NAME = 'input_stream'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        # TODO: Correct VSIS spec - no nested optional parameters
        ]
    LIST_PARAMS = [
        'stream_label', 'data_format', 'payload_size', 'payload_offset', 'psn_offset', 'interface_id', 'filter_address', 'port', 'subgrp_ref'
        ]
    TYPE = 'get'


#-----------------------------------------------------------
class SetModInit(Response):

    NAME = 'mod_init'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'

class GetModInit(Response):

    NAME = 'mod_init'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    LIST_PARAMS = [
        'slot_num', 'num_disks', 'eMSN'
        ]
    TYPE = 'get'


#-----------------------------------------------------------
class SetRecord(Response):

    NAME = 'record'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'

class GetRecord(Response):

    NAME = 'record'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'status',
        'scan_number',
        'scan_name',
        ]
    TYPE = 'get'


#-----------------------------------------------------------
class GetRtime(Response):

    NAME = 'rtime'
    PARAMS = [
        'return_code',
        'cplane_return_code',]
    LIST_PARAMS = [
        'group',
        'data_rate',
        'remaining_time',
        'remaining_GB',
        'available_GB', ]
    TYPE = 'get'

class SetRtime(Response):

    NAME = 'rtime'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'

#-----------------------------------------------------------
class GetScanCheck(Response):

    NAME = 'scan_check'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'groupref',
        'scan_num',
        'scan_label',
        'num_streams',
        ]
    LIST_PARAMS = [
        'stream_label', 'status', 'data_format', 'start_time',
        'duration', 'datasize', 'stream_rate', 'missing_bytes'
        ]
    TYPE = 'get'


#---------------- scan_info -------------------------------------------
class SetScanInfo(Response):

    NAME = 'mstat'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'

class GetScanInfo(Response):

    NAME = 'scan_info'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'groupref',
        'scan_number',
        'scan_label',
        'status',
        'start_time',
        'duration',
        'num_data_streams',
        'sys_perf_code',
        ]
    TYPE = 'get'


#-----------------------------------------------------------
class Status(Response):

    NAME = 'status'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'status_word',
        ]
    TYPE = 'get'


#-----------------------------------------------------------
class SysInfo(Response):

    NAME = 'sys_info'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'system_type',
        'mark6_sn',
        'os_type_rev',
        'cplane_version_number',
        'command_set_revision',
        'available_ram',
        'num_data_disks_supported',
        'num_ethernet_input_ports',
        ]
    LIST_PARAMS = [
        'portref', 'ip', 'state', 'portspeed',
        ]
    TYPE = 'get'


#-----------------------------------------------------------
class SetGroupCmd(Response):

    NAME = 'group'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'group_ref',
        ]
    TYPE = 'set'

class GetGroupCmd(Response):

    NAME = 'group'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'group_ref',
        ]
    TYPE = 'get'


#-----------------------------------------------------------
class SetVolStack(Response):

    NAME = 'vol_stack'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'message'
        ]
    TYPE = 'set'

class GetVolStack(Response):

    NAME = 'vol_stack'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    LIST_PARAMS = [
        'volref', 'ex_msn', 'num_disks_nominal', 'num_disks_discovered',
        'percent_full', 'module_status',
        ]
    TYPE = 'get'

#-----------------------------------------------------------
class SetGSM(Response):

    NAME = 'gsm'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'GSM',
        ]
    TYPE = 'set'

class GetGSM(Response):

    NAME = 'gsm'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    LIST_PARAMS = [
        'volref', 'msn', 'vsm',
        ]
    TYPE = 'get'

#------------------------- gsm_mask ----------------------------------
class SetGsmMask(Response):

    NAME = 'gsm_mask'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'

class GetGsmMask(Response):

    NAME = 'gsm_mask'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    LIST_PARAMS = [    
        'erase_mask_enable',
        'play_mask_enable',
        'record_mask_enable',
        ]
    TYPE = 'get'

###########################################################
class SetDterm(Response):

    NAME = 'dterm'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'

###########################################################
class SetCterm(Response):

    NAME = 'cterm'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'
#------------------------ delete -----------------------------------
class SetDelete(Response):

    NAME = 'delete'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'

class GetDelete(Response):

    NAME = 'delete'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'scan_name',
        ]
    TYPE = 'get'


#------------------------- execute ----------------------------------
class SetExecute(Response):

    NAME = 'execute'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'

class GetExecute(Response):

    NAME = 'execute'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'action',
        'file_name',
        'output',
        ]
    TYPE = 'get'

#------------------------- list  ----------------------------------
class SetList(Response):

    NAME = 'list'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'

class GetList(Response):

    NAME = 'list'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'group_ref',
        ]
    LIST_PARAMS = [
        'num', 'sname', 'size', 'ctime',
        ]
    TYPE = 'get'

#-----------------------  m6cc ------------------------------------
class SetM6cc(Response):

    NAME = 'm6cc'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'

class GetM6cc(Response):

    NAME = 'm6cc'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'config_file',
        'state',
        ]
    TYPE = 'get'

#-------------------------- mstat ---------------------------------
class SetMstat(Response):

    NAME = 'mstat'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'

class GetMstat(Response):

    NAME = 'mstat'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]

    LIST_PARAMS = [
        'grp_ref', 'slot', 'emsn', 'num_disks', 'reg_disks', 'util', 'rem', 'cap', 's1', 's2', 'type',
        ]
    TYPE = 'get'

############################### DTS-id Response #################################
class GetDtsId(Response):

    NAME = 'dts_id'
    PARAMS = [
        'return_code',
        'mark6_sn', 
        'sw_version_number', 
        'command_set_revision',
        ]
    TYPE = 'get'


#-------------------------- group_members ---------------------------------
class SetGroupMembers(Response):

    NAME = 'group_members'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'

class GetGroupMembers(Response):

    NAME = 'group_members'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        'slot'
        ]

    LIST_PARAMS = [
        'eMSN', 
        ]
    TYPE = 'get'

#-------------------------- test response ---------------------------------
class SetTest(Response):

    NAME = 'test'
    PARAMS = [
        'return_code',
        'cplane_return_code',
        ]
    TYPE = 'set'
