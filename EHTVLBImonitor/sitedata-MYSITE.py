from __future__ import print_function

import time
import math
import apexObsUtils

polarization = 0  # which polarization to report Tsys etc for; 0=RCP, 1=LCP?

def signif(n, x):
    """Print float to specified number of significant figures"""
    return float("%.*g" % (n, x))

def nsignif(x):
    """Number of significant figures"""
    return len(str(x).replace(".", ""))

def ndecimal(x):
    """Number of decimals"""
    words = str(x).split(".")
    if len(words) < 2: return 0
    return len(words[1])

def currentUTC():
    return int(time.time()) - 37 #-- TAI to UTC

def convertApexPoint(point):
    #-- On APEX timestamp format Dirk Muders says:
    #-- This is the POSIX format counting the number of 100ns intervals since
    #-- 1582-10-15 00:00:00 (start of the Gregorian calendar).
    #-- Convert from TIA (APEX) to UTC (VLBImonitor).

    #-- no data
    if point is None:
        print("apex data point is 'None': skip")
        return None
    if not isinstance(point,tuple):
        point = (point,)
    if len(point) == 0:
        print("apex data point has len(0): skip")
        return None

    #-- calResult values have len(1)
    if len(point) == 1:
        t = currentUTC()
    else:
        posix2unix = lambda t: int((t - 0x01b21dd213814000)*1e-7) - 37
        t = posix2unix(point[1])

    return [t, point[0]]


class Getter():
    def __init__(self, sitelist):
        self.params = {}
        self.sitelist = sitelist
        self.nerrors = 0

    def update_values(self):
        self.params.clear()
        params = self.insertValues()
        for k,v in params.items():
            if v is None: continue
            if len(v) != 2:
                print('Warning: data for ',k,' not in [x,y] format but rather ', v)
                continue
            if v[1] == -999: continue
            self.params[k] = [v]

    def insertValues(self):
        params = {}
        def getApexPoint(par):
            """get measured points"""
            try:
                return convertApexPoint(apexObsUtils.getMCPointTS(par))
            except Exception as e:
                if self.nerrors == nerrors0: print(self.nerrors, e)
                self.nerrors += 1
                return None

        nerrors0 = self.nerrors
        params['APEX:WEATHERSTATION:humidity'] = getApexPoint('APEX:WEATHERSTATION:humidity')

        point = getApexPoint('APEX:WEATHERSTATION:windSpeed')
        #print('windSpeed: ',point)
        if point is not None  and  int(point[1]) != 999: params['APEX:WEATHERSTATION:windSpeed'] = point

        point = getApexPoint('APEX:WEATHERSTATION:windDirection')
        if point is not None  and  int(point[1]) != 999: params['APEX:WEATHERSTATION:windDirection'] = point

        #-- pwv and tau
        point = getApexPoint('APEX:RADIOMETER:RESULTS:pwv')
        if point is not None  and  int(point[1]) != -999:
            params['APEX:RADIOMETER:RESULTS:pwv'] = point
            params['derived:weather:tau225'] = [point[0], signif(5, .058*point[1] + .004)] #-- ATM07 model

        #-- degrees C to K
        point = getApexPoint('APEX:WEATHERSTATION:temperature')
        if point is not None: params['APEX:WEATHERSTATION:temperature'] = [point[0], signif(5, 273.15 + point[1])]

        #-- degrees C to K
        point = getApexPoint('APEX:WEATHERSTATION:dewPoint')
        if point is not None: params['APEX:WEATHERSTATION:dewPoint'] =  [point[0], signif(5, 273.15 + point[1])]

        #-- hPa to kPa
        point = getApexPoint('APEX:WEATHERSTATION:pressure')
        if point is not None: params['APEX:WEATHERSTATION:pressure'] = [point[0], signif(5, .1 * point[1])]

        params['ABM[1,0]:ANTMOUNT:mode'] = getApexPoint('ABM[1,0]:ANTMOUNT:mode')

        params['APEX:COUNTERS:GPSMINUSFMOUT:GPSMinusFMOUT'] = getApexPoint('APEX:COUNTERS:GPSMINUSFMOUT:GPSMinusFMOUT')
        params['APEX:COUNTERS:GPSMINUSMASER:GPSMinusMaser'] = getApexPoint('APEX:COUNTERS:GPSMINUSMASER:GPSMinusMaser')
        params['APEX:MASER:HOUSING:temperature'] = getApexPoint('APEX:MASER:HOUSING:temperature')

        az = getApexPoint('ABM[1,0]:ANTMOUNT:actualAz')
        el = getApexPoint('ABM[1,0]:ANTMOUNT:actualEl')
        if az is not None  and  el is not None:
            ndec = max(list(map(ndecimal, (az[1], el[1]))))
            t = az[0]
            el = el[1]/math.pi * 180
            az = az[1]/math.pi * 180
            azel = "{:.{ndec}f}{:+.{ndec}f}".format(az, el, ndec=max(ndec, 1))
            params['derived:telescope:actualAzEl'] = [t, azel]

        ra = getApexPoint('ABM[1,0]:ANTMOUNT:actualRA')
        dec = getApexPoint('ABM[1,0]:ANTMOUNT:actualDec')
        if ra is not None  and  dec is not None:
            ndec = max(list(map(ndecimal, (ra[1], dec[1]))))
            t = ra[0]
            ra = ra[1]/math.pi * 12
            dec = dec[1]/math.pi * 180
            radec = "{:.{ndec}f}{:+.{ndec}f}".format(ra, dec, ndec=max(ndec, 1))
            params['derived:telescope:actualRaDec'] = [t, radec]

        point = getApexPoint('ABM[1,0]:ANTMOUNT:JEpoch')
        if point is not None: params['ABM[1,0]:ANTMOUNT:JEpoch'] = [point[0], "J{:4d}".format(int(point[1]))]

        #-- calibration results
        onlineCal = apexObsUtils.getApexCalibrator()
        try:
            calResult = onlineCal.getCalResult('NFLASH230-FFTS1',1,0)
        except:
            #-- instrument is not available, receiver is not currently in use
            pass
        else:
            #print(calResult)
            params['Calibrator:NFLASH230-FFTS1:tSys'] = convertApexPoint(calResult.tSys[polarization])  # tSys=[95.0042874479913, 85.13580861018505]
            params['Calibrator:NFLASH230-FFTS1:tHot'] = convertApexPoint(calResult.tHot)
            params['Calibrator:NFLASH230-FFTS1:tCold'] = convertApexPoint(calResult.tCold)
            params['Calibrator:NFLASH230-FFTS1:tAnt'] = convertApexPoint(calResult.tRx[polarization])
            params['Calibrator:NFLASH230-FFTS1:tCal'] = convertApexPoint(calResult.tCal[polarization])
            params['Calibrator:NFLASH230-FFTS1:tSky'] = convertApexPoint(calResult.tSky[polarization])

        return params
