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

