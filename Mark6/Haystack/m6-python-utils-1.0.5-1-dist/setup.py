#!/usr/bin/env python

from distutils.core import setup

setup(name='m6-python-utils',
      version='1.0.5-1',
      description='Mark6 client and python and shell utilities',
      author='Chet Ruszczyk',
      author_email='chester@haystack.mit.edu',
      url='http://www.haystack.edu/tech/vlbi/mark6/index.html',
      scripts=['scripts/da-client', 'scripts/ecplane', 'scripts/modspeed', 'scripts/gator', 'sublib.py', 'scripts/m6setup', 'scripts/m6sensors.sh']
)
