# This file is part of the VLBImonitor client.  This client is released under the terms of the MIT Licence.
# Copyright (c) 2016 Radboud RadioLab.  All rights reserved
import os
import sys

#-- VLBImonitor servers
servers = [
	"https://vlbimon1.science.ru.nl",
	"https://vlbimon2.science.ru.nl"]

#-- proxy servers: leave empty if you don't need to use a proxy
#-- the system will attempt to connect using the proxy first
#-- if it fails using the proxy, it will attempt to connect directly
proxies = {
#   'http': 'http://your.http.proxy.name:3128',
#   'https': 'http://your.https.proxy.name:3128',
}

#-- login credentials: as used on observer3 in 2017/2018
#username = "APEX central"
#password = "4pex"
#facility = "APEX"

#-- login credentials: as used on observer3 in 2024
username = "-------------"
password = "-------------"
facility = "APEX"

#-- root dir: directory containing client.py
rootdir = sys.argv[0]
rootdir = os.path.abspath(rootdir)
rootdir = os.path.dirname(rootdir)

#-- local storage directory
storagedir = os.path.abspath(rootdir + "/local_storage")

#-- create storage dir if not exists
if not os.path.exists(storagedir):
    print ("creating directory: %s" % (storagedir))
    os.makedirs(storagedir)
