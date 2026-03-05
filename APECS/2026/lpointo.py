#!/usr/bin/env python
# coding: utf-8

from __future__ import print_function

import json
import pandas as pd
import datetime as dt
import os
import mysql.connector
import sys
import time
from time import sleep
import numpy as np
import matplotlib.pyplot as plt
#import apexObsUtils

import apexObsUtils
# Defines function in charge of getting frontend name from APECS
def frontCheck():
#  '''
#  Get the FrontEnd information from scanStatus
#  '''
#
  engine   = apexObsUtils.getObsEngine('')
  pScan    = engine.getScan()
  scan     = apexObsUtils.unpackScanObject(pScan)
  feName   = ''
  # Next line changed to new syntax (FMA, 2009-01-15)
  cobNames     = [cob.name for cob in apexObsUtils.sc.availableComponents()]
  frontEndList = ['NFLASH230','NFLASH460','SEPIA345','SEPIA180','SEPIA660','HET230','HET345','HET460','FLASH345','CHAMP690','SUPERCAM','HOLO','CONCERTO','GARD180', 'LABOCA', 'NOVA660','PI230','AMKID870','ARTEMIS350','ARTEMIS450','LASMA345','n3ar90']
  feNames,sbNames = [],[]
  for frontend in scan.spectralSetup.frontendSetups.values():
    for cobName in cobNames:
          if (cobName.find('APEX/%s' % (frontend.name)) != -1):
                   #print (frontEndList.count(frontend.name),frontend.name)
                   status = int(frontEndList.count(frontend.name))
                   if status==1:
                        feName=frontend.name
			feNames.append(feName)
			sbNames.append(frontend.sideBandSetup.signalBand)

                   else:
		   	feName = 'AMKID350' # To be verify ...
                        pass
    return feName


frontCheck()
feName = frontCheck() # Gets the frontend name into variable
receiver = feName.lower() # lowercases the name




load()
# Connect
cnx = mysql.connector.connect(user='apexReader', password='apecs4op',
                              host='argon',
                              port='3307',
                              database='apexobs')

mycursor = cnx.cursor()

#cnx1 = mysql.connector.connect(user='apexReader', password='apecs4op',
#                              host='argon',
#                              port='3306',

#                              database='SciopsDB')

#mycursor1 = cnx1.cursor()

#SELECT * FROM Table ORDER BY ID DESC LIMIT 1
#
#receiver = 'nflash230'
#receiver = 'sepia345'
#receiver = 'nflash460'
#receiver = 'sepia660'
#receiver = 'sepia180'
print("")
print ("\033[92;1m Optimizing pointing integration time for", receiver,"...\033[0m")
print("")
if receiver == 'nflash230':
    receiver = 1
elif receiver == 'sepia345':
    receiver = 2
elif receiver == 'lasma345':
    receiver = 2
elif receiver == 'nflash460':
    receiver = 3
elif receiver == 'sepia660':
    receiver = 4
elif receiver == 'sepia180':
    receiver = 5
else:
    receiver = 6

query = "SELECT scan_water, source_name FROM scans ORDER BY scan_timemark DESC LIMIT 1"
#query = "SELECT * FROM RefSpecPoint WHERE src = 'o-Ceti'"

#print ("Would you like to calibrate?")
#answer = raw_input('y/n? ')
#if answer == "y":
#    calibrate()
#    sleep(1)
#else:
#    load()
#    sleep(0.5)
#    print("")
#    print("\033[91;1mContinue without calibration...\033[0m")
#load()
sleep(1)
print("")
df1 = pd.read_sql_query(query,cnx)
#print (df1)
pwv = df1['scan_water'].loc[0]
src = df1['source_name'].loc[0]
#src = ('IRC+10216')
#src = ('RAFGL1235')
print("")
print('For',src,'with a pwv of:',pwv)
#print('current src is:',src)
#print(receiver)

if receiver == 5:
    df = pd.read_csv('B5.edb', sep=" |,|", engine = 'python')
elif receiver == 1:
    df = pd.read_csv('B6.edb', sep=" |,|", engine = 'python')
elif receiver == 2:
    df = pd.read_csv('B7.edb', sep=" |,|", engine = 'python')
elif receiver == 3:
    df = pd.read_csv('B8.edb', sep=" |,|", engine = 'python')
elif receiver == 4:
    df = pd.read_csv('B9.edb', sep=" |,|", engine = 'python')
else:
    print('')
    print('Receiver not detected')
    sys.exit()

df = df.loc[df['Source'] == src]
df = df[['Source', 'Flux']]
if df.empty:
    print(src,'is not a pointing source!')
    sys.exit()
else:
    print ("")
#df_max = round(df['Flux'].max())
#df_min = round(df['Flux'].min())
#df_mean = round(df['Flux'].mean())
#df_median = round(df['Flux'].median())
#df_mode = df['Flux'].mode()
#print('')
#print ('Max Flux is:',df_max)
#print ('Min Flux is: ',df_min)
#print ('Mean Flux is:',df_mean)
#print ('Median Flux is:',df_median)
#print ('Mode Flux is:',df_mode)

#print('')
#print (df)
#df = df.loc[df['src'] == src ]
df = df['Flux']
Flux = df.to_string(index=False)
Flux = float(Flux)
#load()
#sleep(0.5)
print (Flux)

if Flux >= 130 or src == 'Orion-H2O':
    to = 5
    print("Intense source, applying minimum integration time.")
else:
    if ((receiver == 1) and (Flux > 22)): # Conditions for NFLASH230
        if 2.5 <= pwv <= 7.01:
            to = 20
            print('\033[95;1m Degraded conditions, the standard 20 seconds of integration will be applied\033[0m')
        elif 2.0 <= pwv <= 2.49:
            to = 10
            print('\033[93;1m Normal conditions, 10 seconds of integration will be applied\033[0m')
        elif 0.2 <= pwv <= 1.99:
            to = 7
            print('\033[92;1m Optimal conditions, 7 seconds of integration will be applied\033[0m')
    elif ((receiver == 2) & (Flux > 30)): # Conditions for SEPIA/LASMA 345
        if 1.75 <= pwv <= 4.0:
            to = 20
            print('Degraded conditions, the standard 20 seconds of integration will be applied')
        elif 1.5 <= pwv <= 1.74:
            to = 10
            print('Normal conditions, 10 seconds of integration will be applied')
        elif 0.2 <= pwv <= 1.49:
            to = 7
            print('Optimal condition, 7 seconds of integration will be applied')
    elif ((receiver == 3) & (Flux > 31)): # Conditions for NFLASH460
        if 0.85 <= pwv <= 3.0:
            to = 20
            print('Degraded conditions, the standard 20 seconds of integration will be applied')
        elif 0.8 <= pwv <= 0.84:
            to = 10
            print('Normal conditions, 10 seconds of integration will be applied')
        elif 0.2 <= pwv <= 0.79:
            to = 7
            print('Optimal conditions, 7 seconds of integration will be applied')
    elif ((receiver == 4) & (Flux > 54)): # Conditions for SEPIA660 & CHAMP690
        if 0.55 <= pwv <= 2.0:
            to = 20
            print('Degraded conditions, the standard 20 seconds of integration will be applied')
        elif 0.5 <= pwv <= 0.54:
            to = 10
            print('Normal conditions, 10 seconds of integration will be applied')
        elif 0.1 <= pwv <= 0.49:
            to = 7
            print('Optimal conditions, 7 seconds of integration will be applied')
    elif ((receiver == 5) & (Flux > 9)): # Conditions for SEPIA180
        if 1.25 <= pwv <= 3.00:
            to = 20
            print('Degraded conditions, the standard 20 seconds of integration will be applied')
        elif 1.0 <= pwv <= 1.24:
            to = 10
            print('Normal conditions, 10 seconds of integration will be applied')
        elif 0.2 <= pwv <= 0.99:
            to = 7
            print('Optimal conditions, 7 seconds of integration will be applied')
    else:
        print ("No optimization to be done. Using wlpoint(t=20) instead")
        to = 20
#print(to)
#wl = ("wlpoint(t={})".format(to))
#print (wl)
f = open("wlpo.py","w")
f.write("wlpoint(t={},cal=1)".format(to))
f.close()
print("")
print ("would you like to apply the suggested integration time?")
print("")
answer = raw_input('y/n? ')
print("")
if answer == "y":
    execfile('wlpo.py')
else:
    print("you probably have a better estimation... sure")


# Close the connection
cnx.close()
#cnx1.close()
