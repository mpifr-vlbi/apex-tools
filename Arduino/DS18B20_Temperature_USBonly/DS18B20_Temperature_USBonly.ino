//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Readout of temperature (deg C) and 64-bit Unique ID from 1-8 x DS18B20 devices.
// The readings are outputted periodically on USB Serial (9600,n,8,1)
//
// The output is in JSON format, example:
//  {"numdevices":2, "devtempC":[29.00, 29.81], "devIDs":["bc000006d8119228", "8e000006d7e3d928"]}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Board
//   NodeMCU 1.0 ESP-12E  (AZ-Delivery,  large ESP8266 board)
//   Wemos D1 Mini clone (small ESP8266MOD board)
//     based on CH340 or CH341 USB-serial,
//     need driver ver 3.4.2014.8 on Windows 10+, newer ones do not work
//     port D4, gpioNr=2
//
//   Some D<x> pins do not support open-drain buses
//   and thus not 1-wire. At least pin D4 works.
//
// Config
//   CPU 80 MHz
//
// DS18B20
//   black=GND   red=3.3V  yellow=data=ESP8266_D4
//   with pull-up resistor 4.7k for data pin to 3.3V
//
// Libraries:
//  OneWire           2.3.7   by Paul Stoffregen
//  DallasTemperature 3.9.0   by Mike Burton        depends on OneWire
//

#include <OneWire.h>
#include <DallasTemperature.h>

#define SERIAL_BAUDRATE   9600   // 74880 matches rate of bootloader, while 9600 is more client friendly
#define ONE_WIRE_BUS_PIN  D4     // Wemos D1 board pin 'D4'
#define MAX_DS_DEV 8
#define READOUT_INTERVAL_MILLISEC 5000

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensorbus(&oneWire);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int num_ds18b20_devices = 0;
static float ds18b20_temperatures[MAX_DS_DEV] = {0,0,0,0, 0,0,0,0};
static uint64_t ds18b20_IDs[MAX_DS_DEV] = {0,0,0,0, 0,0,0,0};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// typedef uint8_t DeviceAddress[8];
float custom_getTempCByIndex(int index, uint64_t* addr)
{
    DeviceAddress deviceAddress;
    if (!sensorbus.getAddress(deviceAddress, index)) {
        return DEVICE_DISCONNECTED_C;
    }
    *addr = *((uint64_t*)&deviceAddress);
    return sensorbus.getTempC((uint8_t*)deviceAddress);
}

void measureTemperatures()
{
  num_ds18b20_devices = sensorbus.getDeviceCount();
  sensorbus.requestTemperatures();
  for (int dev=0; dev < num_ds18b20_devices && dev < MAX_DS_DEV; dev++) {
    //ds18b20_temperatures[dev] = sensorbus.getTempCByIndex(dev);
    ds18b20_temperatures[dev] = custom_getTempCByIndex(dev, &ds18b20_IDs[dev]);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printTemperatures()
{
  String json;
  char tmp[64] = { '\0' };

  sprintf(tmp, "{\"numdevices\":%d", num_ds18b20_devices);
  json += tmp;

  json += ", \"devtempC\":[";
  for (int dev=0; dev < num_ds18b20_devices; dev++) {
    sprintf(tmp, "%.2f", ds18b20_temperatures[dev]);
    json += tmp;
    if (dev == (num_ds18b20_devices-1)) { json += "]"; }
    else { json += ", "; }
  }

  json += ", \"devIDs\":[";
  for (int dev=0; dev < num_ds18b20_devices; dev++) {
    sprintf(tmp, "\"%llx\"", ds18b20_IDs[dev]);
    json += tmp;
    if (dev == (num_ds18b20_devices-1)) { json += "]"; }
    else { json += ", "; }
  }

  json += "}";

  Serial.println(json);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println("# JSON printout of measurements from up to 8 DS18B20 sensors (DS18B20_Temperature_USBonly.ino v1.0 JWagner)");

  sensorbus.begin();
  pinMode(ONE_WIRE_BUS_PIN, INPUT_PULLUP);
  num_ds18b20_devices = sensorbus.getDeviceCount();

  Serial.print("# Detected ");
  Serial.print(num_ds18b20_devices, DEC);
  Serial.println(" temperature sensors.");
}

void loop()
{
  unsigned long curr_ticks;
  static unsigned long prev_update_ticks = 0;
  bool update_temperatures = false;

  while(Serial.available()) {
    Serial.read();
  }

  curr_ticks = millis();
  update_temperatures = (curr_ticks - prev_update_ticks) >= READOUT_INTERVAL_MILLISEC;

  if (update_temperatures) {
    measureTemperatures();
    printTemperatures();
    prev_update_ticks = curr_ticks;
  }
}
