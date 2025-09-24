#!/usr/bin/env python3
import serial
import json

serDevice = '/dev/ttyUSB0'

# Mapeo de IDs a nombres amigables
sensor_names = {
    "bc000006d8119228": "Hanging_Mid_Rack",
    "8e000006d7e3d928": "Top_Rack",
    "6fa96c441f64ff28": "Bottom_Rack"
}

def getJSON():
    d = ''
    try:
        ser = serial.Serial(port=serDevice, timeout=15)
        d = ser.readline()
        d = d.decode('utf-8').strip()
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
    return d

def getTemperatures():
    try:
        j = getJSON()
        v = json.loads(j)
        temps = v['devtempC']
        ids = v['devIDs']

        # Generar line protocol para InfluxDB
        lines = []
        for i in range(len(ids)):
            sensor_id = ids[i]
            temp = temps[i]
            sensor_name = sensor_names.get(sensor_id, sensor_id)
            # measurement=room_temperature, tag=sensor_name, field=value
            line = f"rec-room_temperature,sensor={sensor_name} value={temp}"
            lines.append(line)
        return lines
    except Exception as e:
        # Si hay error, no romper, solo devuelve vacío
        return []

if __name__ == "__main__":
    for line in getTemperatures():
        print(line)
