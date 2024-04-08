import os
import re
import numpy as np
from six.moves import urllib
import datetime
import time

__all__ = ["get_maser_data"]

key_map = {
    "U batt.A[V]": "batteryVoltageA",
    "IB heater[V]": "heaterInternalBottom",
    "C field[V]": "cfieldVoltage",
    "U OCXO[V]": "ocxoVoltage",
    "I batt.A[A]": "batteryCurrentA",
    "IS heater[V]": "heaterInternalSide",
    "U varactor[V]": "varactorVoltage",
    "+24 VDC[V]": "supplyP24V",
    "U batt.B[V]": "batteryVoltageB",
    "UTC heater[V]": "heaterThermalControlUnit",
    "U HT ext.[kV]": "externalHighVoltageValue",
    "+15 VDC[V]": "supplyP15V",
    "I batt.B[A]": "batteryCurrentB",
    "ES heater[V]": "heaterExternalSide",
    "I HT ext[uA]": "externalHighVoltageCurrent",
    "-15 VDC[V]": "supplyM15V",
    "Set H[V]": "hydrogenPressureSetting",
    "EB heater[V]": "heaterExternalBottom",
    "U HT int.[kV]": "internalHighVoltageValue",
    "+5 VDC[V]": "supplyP5V",
    "Meas. H[V]": "hydrogenPressureMeasurement",
    "I heater[V]": "heaterIsolator",
    "I HT int.[uA]": "internalHighVoltageCurrent",
    "-5 VDC[V]": "supplyM5V",
    "I pur.[A]": "purifierCurrent",
    "T heater[V]": "heaterTube",
    "H st.pres.[bar]": "hydrogenStoragePressure",
    "+8 VDC[V]": "supplyP8V",
    "I diss.[A]": "dissociatorCurrent",
    "Boxes temp[C]": "boxesTemperature",
    "H st. heat[V]": "hydrogenStorageHeater",
    "+18 VDC[V]": "supplyP18V",
    "H light[V]": "dissociatorLight",
    "I boxes[A]": "boxesCurrent",
    "Pirani heat.[V]": "piraniHeater",
    "Lock": "lock",
    "IT heater[V]": "heaterInternalTop",
    "Amb.temp.[c]": "ambientTemperature",
    "U 405kHz[V]": "405kHzAmplitude",
    "Freq Synth": None,
}

def py27_timestamp(dtime):
    "Return POSIX timestamp as float"
    return time.mktime((dtime.year, dtime.month, dtime.day,
                         dtime.hour, dtime.minute, dtime.second,
                        -1, -1, -1)) + dtime.microsecond / 1e6

def get_maser_data(url="http://10.7.114.7/monit.htm"):

    #now = datetime.datetime.now().timestamp()
    now = py27_timestamp(datetime.datetime.now())
    try:
        response = urllib.request.urlopen(url)
        data = response.read()
        data = data.replace(b"\xb0", b"").decode().strip()
        data = re.split("<table[^<>]+>", data)[1]
        data = [d.strip() for d in re.split("<[^<>]+>", data) if d.strip()]
        data = dict(np.array(data).reshape(-1, 2))
    except:
        return {"maser_status": [now, False]}

    params = {"maser_status": [now, True]}

    for k, v in data.items():
        nk = key_map.get(k, None)
        if nk is None:
            continue

        try:
            v = float(v)
            if "temperature" in nk.lower():
                v = 273.15 + v
        except ValueError:
            pass

        if nk == "lock":
            v = bool(v)

        params["maser_" + nk] = [now, v]

    return dict(sorted(params.items()))


if __name__ == "__main__":

    maser_data = get_maser_data("http://10.0.2.96/monit.htm")
    for k, v in maser_data.items():
        print("{}: {}".format(repr(k), repr(v)))
