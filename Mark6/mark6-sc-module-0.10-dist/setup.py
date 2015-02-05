#
# $Id: m6scsetup.py 1726 2014-01-02 22:05:47Z gbc $
#
# build this module with:
#
#  python m6scsetup.py build
#

from distutils.core import setup, Extension

setup(name='mark6-sc-module',
      version = '0.10',
      description = 'Scan check utility module for c-plane', 
      author='Geoff Creq',
      author_email='gbc@haystack.mit.edu',
      ext_modules= [Extension('m6sc',
                              sources = ['m6scmodule.c', 'per_file.c', 'sg_access.c', 'sc_stats.c'],
                              ) ],
      )


#
# eof
#
