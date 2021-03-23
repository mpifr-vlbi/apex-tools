#-- VLBImonitor servers
servers = [
	"https://vlbimon1.science.ru.nl/rpc",
	"https://vlbimon2.science.ru.nl/rpc"]

#-- proxy servers: leave empty if you don't need to use a proxy
#-- the system will attempt to connect using the proxy first
#-- if it fails using the proxy, it will attempt to connect directly
proxies = {
#    'http': 'http://your.http.proxy.name:3128',
#    'https': 'http://your.https.proxy.name:3128',
}

#-- login credentials
username = "APEX central"
password = "4pex"
facility = "APEX"

#-- root dir: directory containing client.py
import sys
startdir = sys.path[0]
rootdir = startdir

#-- local storage directory
storagedir = rootdir + "/local_storage"

#-- create storage dir if not exists
import os
try:
    os.mkdir(storagedir)
    print "creating directory: " + storagedir
except OSError:
    pass
