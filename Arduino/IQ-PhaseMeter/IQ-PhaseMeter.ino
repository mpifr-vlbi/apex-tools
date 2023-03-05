/*

  Measures output voltages of an I/Q mixer and reports the phase angle

  Accumulates readings on two (/three) analog input pins for ~1 sec.
  Calculates phase angle between the two inputs.

  Prints the results onto usb serial in plain numeric format,
  e.g., directly plottable with the Serial Monitor in Arduino IDE.

  Board:  Arduino UNO / ATmega328P 8-bit AVR, no hw float
  Pins:   A0 for "I"
          A5 for "Q"
          A4 optionally for a third analog signal
  Serial: 9600,8,n,1

  Mixer:  Hittite 109996-1 eval board, EVAL-HMC525A
          LO abs max 25 dBm
          RF abs max 20 dBm
          supplies ADCs with quadrature "I" and "Q" voltages

  Output: as below for two or three signals
    <adc0 avg V> <adc1 avg V> <iq phase deg> [<adc0 raw reading> <adc1 raw reading>]
    <adc0 avg V> <adc1 avg V> <iq phase deg> [<adc0 raw reading> <adc1 raw reading>]
    <adc0 avg V> <adc1 avg V> <iq phase deg> [<adc0 raw reading> <adc1 raw reading>]
    ...

  Created 18 Feb 2023, Alan Roy
  Modified March 2023, Jan Wagner

*/

#include <math.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#define N_SIGS 3
const int analogInPins[N_SIGS] = { /*I*/ A0, /*Q*/ A5, /*aux*/ A4 };
const int ledPin = 13;

const int Nsamples = 200;         // samples to average for a new result (default: 425)
const int sampleSleep_msec = 1;   // millisec to sleep after all ADC sampled; chosen to yield approx one result per second

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ADC Averaging
/////////////////////////////////////////////////////////////////////////////////////////////////////////

long accus[N_SIGS] = { 0 };                         // for averaging
float results[N_SIGS] = { 0.0f };                   // for post-avg normalized results
const int adc_offsets[N_SIGS] = { 480, 480, 180 };  // ADC values for 0 V at inputs I, Q, aux/10MHz

const float norm = (float)Nsamples * 1023.0f; // normalization factor, 10 bit ADC, 0-5 V = 0-1023

int show_raw = 0;
int show_volts = 1;
int show_phase = 1;
const int voltage_in_milliV = 1;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
  Send data column labels for the Arduino IDE "Serial Plotter" utility
  Provides a data-label header for upcoming data on the serial connection.
  Note, the plotter has a shortcoming, does not re-detect label header if we change data fields on the run...
*/
void sendColumnLabels()
{
    String columnLabels;
    int i;

    if (show_raw) {
        for (i = 0; i < N_SIGS; i++) {
          columnLabels += "R" + String(i, DEC) + " ";
        }
    }
    if (show_volts) {
        for (i = 0; i < N_SIGS; i++) {
          columnLabels += "V" + String(i, DEC) + " ";
        }
    }
    if (show_phase) {
      columnLabels += "Phi";
    }

    Serial.println(columnLabels);
}


void handleSerialKey()
{
    if (Serial.available()) {
        switch(Serial.read()) {
          case 'r':
            show_raw = !show_raw;
            break;
          case 'v':
            show_volts = !show_volts;
            break;
          case 'p':
            show_phase = !show_phase;
            break;
          default:
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
    Serial.end();
    Serial.flush();
    Serial.begin(9600);

    pinMode(ledPin, OUTPUT);

    // Serial.println("IQ Phase Meter - toggle commands are: 'r' for raw ADC values, 'v' for voltages, 'p' for I,Q phase");
    if (1) {
        sendColumnLabels();
    }
}

void loop()
{
    int i, n;

    // Commands

    handleSerialKey();

    // Accumulate ADC readings

    for (i = 0; i < N_SIGS; i++) {
      accus[i] = 0;
    }

    for (n = 0; n < Nsamples; n++) {
      for (i = 0; i < N_SIGS; i++) {
        accus[i] += analogRead(analogInPins[i]) - adc_offsets[i];
      }
      delay(sampleSleep_msec);
    }

    // Normalize

    for (i = 0; i < N_SIGS; i++) {
      results[i] = accus[i] / norm;
    }

    // Calculate phase angle from the two quadrature voltages

    const double iq_phase = 180.0 / M_PI * atan2(results[1], results[0]);

    // Serial logging

    digitalWrite(ledPin, HIGH);

    if (show_raw) {
      for (i = 0; i < N_SIGS; i++) {
        Serial.print(analogRead(analogInPins[i]));
        Serial.print(" ");
      }
    }
    if (show_volts) {
      for (i = 0; i < N_SIGS; i++) {
        if (voltage_in_milliV) {
          Serial.print(1000*results[i], 5);
        } else {
          Serial.print(results[i], 5);
        }
        Serial.print(" ");
      }
    }
    if (show_phase) {
      Serial.print(iq_phase, 3);
      Serial.print(" ");
    }
    Serial.println();

    digitalWrite(ledPin, LOW);
}
