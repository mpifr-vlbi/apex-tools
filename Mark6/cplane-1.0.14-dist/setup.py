#!/usr/bin/env python

from distutils.core import setup

setup(name='cplane',
      version='1.0.14',
      description='VLBI Data Recording Service Control plane for Mark6 system',
      author='Chet Ruszczyk',
      author_email='chester@haystack.mit.edu',
      url='http://www.haystack.edu/tech/vlbi/mark6/index.html',
      package_dir={'':''},
      packages=[''],
      scripts=['scripts/cplane']
)
