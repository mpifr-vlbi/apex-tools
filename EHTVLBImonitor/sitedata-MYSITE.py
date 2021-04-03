from __future__ import print_function

import time
import math
import apexObsUtils

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
        '''Query APECS database for new values of all measurement points. Retain valid-looking data in self.params[]'''
        self.params.clear()
        self.__populateValues()
        filtered_params = {}
        for k,v in self.params.items():
            if v is None:
                continue
            if len(v) != 2:
                print('Warning: data for ',k,' not in [x,y] format but rather ', v)
                continue
            if v[1] == -999:
                continue
            if v[1] == 999:
                continue
            filtered_params[k] = [v]
        self.params = filtered_params


    def __populateValues(self):
        params = {}
        self.__getWeather()
        self.__getRadiometer()
        self.__getTarget()
        self.__getTemperatures()
        self.__getEHTEquivalentTunings()
        self.__getMisc()


    def __getApexPoint(self, par):
        """get measured points"""
        try:
            return convertApexPoint(apexObsUtils.getMCPointTS(par))
        except Exception as e:
            print(self.nerrors, e)
            self.nerrors += 1
            return None


    def __getWeather(self):
        '''
        Fill in params[] with wind, humidity, temperature, dew point, and pressure data.
        Convert units from APECS database units into those expected by EHT VLBIMonitor.
        '''

        self.params['APEX:WEATHERSTATION:windSpeed'] = self.__getApexPoint('APEX:WEATHERSTATION:windSpeed')
        self.params['APEX:WEATHERSTATION:windDirection'] = self.__getApexPoint('APEX:WEATHERSTATION:windDirection')
        self.params['APEX:WEATHERSTATION:humidity'] = self.__getApexPoint('APEX:WEATHERSTATION:humidity')

        temperature = self.__getApexPoint('APEX:WEATHERSTATION:temperature')
        dewpoint = self.__getApexPoint('APEX:WEATHERSTATION:dewPoint')
        pressure = self.__getApexPoint('APEX:WEATHERSTATION:pressure')

        if temperature is not None:
            # convert Celsius to Kelvin
            self.params['APEX:WEATHERSTATION:temperature'] = [temperature[0], signif(5, 273.15 + temperature[1])]

        if dewpoint is not None:
            # convert Celsius to Kelvin
            self.params['APEX:WEATHERSTATION:dewPoint'] =  [dewpoint[0], signif(5, 273.15 + dewpoint[1])]

        if pressure is not None:
            # convert hPa to kPa
            self.params['APEX:WEATHERSTATION:pressure'] = [pressure[0], signif(5, .1 * pressure[1])]


    def __getRadiometer(self):

        pwv = self.__getApexPoint('APEX:RADIOMETER:RESULTS:pwv')
        if pwv is not None  and  int(pwv[1]) != -999:
            self.params['APEX:RADIOMETER:RESULTS:pwv'] = pwv
            self.params['derived:weather:tau225'] = [pwv[0], signif(5, .058*pwv[1] + .004)] #-- ATM07 model


    def __getTarget(self):

        az = self.__getApexPoint('ABM[1,0]:ANTMOUNT:actualAz')
        el = self.__getApexPoint('ABM[1,0]:ANTMOUNT:actualEl')
        ra = self.__getApexPoint('ABM[1,0]:ANTMOUNT:actualRA')
        dec = self.__getApexPoint('ABM[1,0]:ANTMOUNT:actualDec')
        epoch = self.__getApexPoint('ABM[1,0]:ANTMOUNT:JEpoch')

        if az is not None  and  el is not None:
            ndec = max(list(map(ndecimal, (az[1], el[1]))))
            t = az[0]
            el = el[1]/math.pi * 180
            az = az[1]/math.pi * 180
            azel = "{:.{ndec}f}{:+.{ndec}f}".format(az, el, ndec=max(ndec, 1))
            self.params['derived:telescope:actualAzEl'] = [t, azel]

        if ra is not None  and  dec is not None:
            ndec = max(list(map(ndecimal, (ra[1], dec[1]))))
            t = ra[0]
            ra = ra[1]/math.pi * 12
            dec = dec[1]/math.pi * 180
            radec = "{:.{ndec}f}{:+.{ndec}f}".format(ra, dec, ndec=max(ndec, 1))
            self.params['derived:telescope:actualRaDec'] = [t, radec]

        if epoch is not None:
            self.params['ABM[1,0]:ANTMOUNT:JEpoch'] = [epoch[0], "J{:4d}".format(int(epoch[1]))]


    def __getMisc(self):

        self.params['ABM[1,0]:ANTMOUNT:mode'] = self.__getApexPoint('ABM[1,0]:ANTMOUNT:mode')

        self.params['APEX:COUNTERS:GPSMINUSMASER:GPSMinusMaser'] = self.__getApexPoint('APEX:COUNTERS:GPSMINUSMASER:GPSMinusMaser')
        # self.params['APEX:COUNTERS:GPSMINUSFMOUT:GPSMinusFMOUT'] = self.__getApexPoint('APEX:COUNTERS:GPSMINUSFMOUT:GPSMinusFMOUT') # no vlbimon parameter counterpart due to vlbimon being r2dbe-centric

        #self.params['APEX:MASER:HOUSING:temperature'] = self.__getApexPoint('APEX:MASER:HOUSING:temperature')  # 03/2021 : measurement point not working, no data
        self.params['APEX:MASER:HOUSING:temperature'] = [currentUTC(), -123.4]


    def __getEHTEquivalentTunings(self):
        '''Get NFLASH sky frequency. Translate into EHT band1/2/3/4 frequencies'''

        skyfreq_GHz = self.__getApexPoint('APEX:NFLASH230:skyFrequency')
        lo1 = self.__getApexPoint('APEX:NFLASH230:LO1:frequency')

        lo1_timestamp = lo1[0]
        lo1_GHz = lo1[1]
        bdc_LO2_GHz = 7.000
        subband_bw = 2.048
        dbc3_LO2_GHz = 9.048  # == bdc_LO2 + 2.048; upper edge of 4.096G band sampled by DBBC3 in LSB 
        eht_band_centers = [
            lo1_GHz - bdc_LO2_GHz - subband_bw/2.0,
            lo1_GHz - bdc_LO2_GHz + subband_bw/2.0,
            lo1_GHz + bdc_LO2_GHz - subband_bw/2.0,
            lo1_GHz + bdc_LO2_GHz + subband_bw/2.0 ]

        self.params['APEX:NFLASH230:SKYFREQUENCY'] = skyfreq_GHz
        self.params['APEX:EHT:LO1'] = lo1
        #self.params['APEX:EHT:BAND1_MID'] = [lo1_timestamp, eht_band_centers[0]]
        #self.params['APEX:EHT:BAND2_MID'] = [lo1_timestamp, eht_band_centers[1]]
        #self.params['APEX:EHT:BAND3_MID'] = [lo1_timestamp, eht_band_centers[2]]
        #self.params['APEX:EHT:BAND4_MID'] = [lo1_timestamp, eht_band_centers[3]]


    def __getTemperatures(self):
        '''Get system temperatures from various FFTS subbands. Note that these do NOT match in frequency nor bandwidth with the EHT subbands.'''

        # Calibrator engine
        onlineCal = apexObsUtils.getApexCalibrator()

        # Docu:
        #   caldata = apexObsUtils.getApexCalibrator()::getCalResult(FeBe:str, baseband:int, 0)
        # Input:
        #   FeBe for nFLASH is 'Calibrator:NFLASH230-FFTS1:tSys'
        #   Baseline is 1 or 2 (4-8GHz IF sections), or 3 or 4 (8-12GHz IF sections)
        # Output:
        #   caldata.tSys  : [tSys feed X, tSys feed Y] : tsys are plain Tsys* values not adjusted by tau
        #   caldata.feeds : [id feed X, id feed Y]     : 1,2 are LSB sections, and 3,4 are USB sections

        # Docu from JP: tSys values are an average (over the IF section) Tsys value, and they are not tau-corrected.
        #               However, you can get the corresponding opacity values (also an average over IF) in the (returned)
        #               two-element arrays: tauSig and tauIma.

        basebands = [1,2,3,4]
        montargets = ['Calibrator:NFLASH230-FFTS1:1', 'Calibrator:NFLASH230-FFTS1:2', 'Calibrator:NFLASH230-FFTS1:3', 'Calibrator:NFLASH230-FFTS1:4']
        for (baseband,target) in zip(basebands,montargets):
            try:
                calResult = onlineCal.getCalResult('NFLASH230-FFTS1', baseband, 0)
            except:
                #-- instrument is not available, receiver is not currently in use
                print("Not available: FeBe'NFLASH230-FFTS1' baseband %d" % (baseband))
                pass
            else:
                #print(baseband, target, calResult)
                if baseband in [1,2]: ## currently limit to 1,2 - don't know in params.map where to stuff 'Calibrator:NFLASH230-FFTS1:3:tSys:X', 'Calibrator:NFLASH230-FFTS1:3:tSys:Y', 'Calibrator:NFLASH230-FFTS1:4:tSys:Y', 'Calibrator:NFLASH230-FFTS1:4:tSys:X'
                    self.params[target + ':tSys:X'] = convertApexPoint(calResult.tSys[0])
                    self.params[target + ':tSys:Y'] = convertApexPoint(calResult.tSys[1])
                self.params[target + ':tHot'] = convertApexPoint(calResult.tHot)
                self.params[target + ':tCold'] = convertApexPoint(calResult.tCold)
                # self.params[target + ':tAnt'] = convertApexPoint(calResult.tRx[polarization])  # no vlbimon server parameter counterpart
                # self.params[target + ':tCal'] = convertApexPoint(calResult.tCal[polarization]) # -"-
                # self.params[target + ':tSky'] = convertApexPoint(calResult.tSky[polarization]) # -"-
