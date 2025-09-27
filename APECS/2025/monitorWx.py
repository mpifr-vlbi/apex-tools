#!/alma/ACS-2021DEC/pyenv/shims/python
'''
Shows an info feed from the APEX Weather Station (wx)
together with APEX Radiometer readings (pwv).
'''
import apexObsUtils
import datetime
import time
import apexOnlineWV

# apexWV = apexOnlineWV.onlineWV()

def getMCPoint_nx(name, defaultVal):

	rv = defaultVal
	try:
		rv = apexObsUtils.getMCPoint(name)
	except Exception as e:
		print("Error getting %s: %s" % (name, str(e)))
	finally:
		return rv


def readMeters(logfile, verbose=False):

	onlineCal = apexObsUtils.getApexCalibrator()
	offsetUTC = 37  # TODO: update automatically to most recent TAI (APECS computer) vs UTC time offset
	rxName = "N3AR"

	try:
		calResult = onlineCal.getCalResult('%s-PBE_A' % (rxName),1,0)
		# this data has no timestamp?
		calStr = 'tsys/%f,trx/%f,gainimage/%f,tausig/%f,tauima/%f,wvr/%f,wvrrm/%f,thot/%f,tcold/%f' % ( \
			calResult.tSys[0], \
			calResult.tRx[0], \
			calResult.gainImage[0], \
			calResult.tauSig[0], \
			calResult.tauIma[0], \
			calResult.water[0], \
			calResult.waterRM[0], \
			calResult.tHot[0], \
			calResult.tCold[0] )
	except:
		calStr = 'tsys/nodata'

	try:
		timestamp = apexObsUtils.getMCPointTS('APEX:WEATHERSTATION:temperature')
	except:
		timestamp = (-1, -1)

	temperature = getMCPoint_nx('APEX:WEATHERSTATION:temperature', -273)
	dewPoint = getMCPoint_nx('APEX:WEATHERSTATION:dewPoint', -273)
	humidity = getMCPoint_nx('APEX:WEATHERSTATION:humidity', -1)
	pressure = getMCPoint_nx('APEX:WEATHERSTATION:pressure', -1)
	windspeed = getMCPoint_nx('APEX:WEATHERSTATION:windSpeed', 0)
	winddir = getMCPoint_nx('APEX:WEATHERSTATION:windDirection', 0)
	pwv = getMCPoint_nx('APEX:RADIOMETER:RESULTS:pwv', -1)

	# pwvCalc = apexWV.getCurrentWaterVapour()
	# print(pwv, pwvCalc)

	wxStr = 'wx/%.2f,%.1f,%.1f,%.2f,%.0f,%.2f' % (temperature, pressure, humidity, windspeed, winddir, pwv)
	wxStr_alt = 'temp %.1f C, dew %.1f C, hum %.1f %%, p %.1f mbar, wind %.2f m/s, wind dir %.1f deg, pwv %.2f mm' % (temperature,  dewPoint,  humidity, pressure, windspeed, winddir, pwv)

	T = datetime.datetime.utcnow() + datetime.timedelta(seconds=-offsetUTC)
        T_snp = T.strftime('%Y.%j.%H:%M:%S')

	logfile.write('%s/%s\n' % (T_snp,calStr))
	logfile.write('%s/%s\n' % (T_snp,wxStr))
	logfile.write('%s;%s\n' % (T_snp,wxStr_alt))
	if verbose:
		print('%s/%s' % (T_snp,calStr))
		print('%s/%s' % (T_snp,wxStr))
		print('%s;%s' % (T_snp,wxStr_alt))

logfile = open('vlbi-cals.log', 'a')
while True:
	readMeters(logfile, verbose=True)
	time.sleep(30)
logfile.close()
