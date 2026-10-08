
# Extraction of tau and other values

Input are the observational datasets found under ~/scidata/*.apex

These can be read by Gildas https://www.iram.fr/IRAMFR/GILDAS/

Check the content of the tau.class script, make modifications if needed.
By default it will process all datasets (all ~/scidata/*.apex files).
Run the script with:

$ class @tau

The order of the columns in the generated tau_<dataset>.txt file(s) are:

  - date of obs (dd-mm-yyyy)
  - time (UT hour)
  - scan number
  - source-name
  - elevation
  - rest freq (MHz)
  - backend (e.g., 'AP-N901-F303')
  - opacity (tau)
  - opacity signal band
  - Tsys system temperature
  - Tamb ambient temperature (K)

## FFTS Mappings

Names of FFTS channels that correspond roughly to the VLBI-covered parts of the spectrum are

 - AP-N901-F303    N3AR 3mm VLBI, 4-8G, RCP(?)
 - AP-N902-F303    N3AR 3mm VLBI, 4-8G, LCP(?)
 - AP-N201-F101    NFLASH 1mm VLBI in LSB (EHT b1+b2), 4-8G, RCP(?)
 - AP-N202-F101    NFLASH 1mm VLBI in LSB (EHT b1+b2), 4-8G, LCP(?)
 - AP-N203-F102    NFLASH 1mm VLBI in USB (EHT b3+b4), 4-8G, RCP(?)
 - AP-N204-F102    NFLASH 1mm VLBI in USB (EHT b3+b4), 4-8G, LCP(?)
 - AP-S301-F101    SEPIA345 0.8mm VLBI in LSB (EHT b1+b2), 4-8G, RCP(?)
 - AP-S302-F101    SEPIA345 0.8mm VLBI in LSB (EHT b1+b2), 4-8G, LCP(?)
 - AP-S303-F102    SEPIA345 0.8mm VLBI in USB (EHT b3+b4), 4-8G, RCP(?)
 - AP-S304-F102    SEPIA345 0.8mm VLBI in USB (EHT b3+b4), 4-8G, LCP(?)

## FFTS Details

The APEX FFTS spectrometer backend produces tau data grouped by 4 GHz wide dual-pol
spectral windows; FFTS windows are 4-8 GHz and 8-12 GHz on either receiver sideband.

For GMVA 86G the VLBI spectral window is narrow (512 MHz) and falls entirely within
a single 4-8 GHz FFTS spectral window.

The EHT 345G observations record VLBI spectral windows at 4-6 GHz (b2/b3) and
6-8 GHz (b1/b4). Both fall within a single 4-8 GHz FFTS window. Consequently
the metadata of each 4-6G+6-8G band pair (b1+b2, b3+b4) are identical.

The EHT 230G/260G observations record VLBI spectral windows at 5-7 GHz (b2/b3) and
7-9 GHz (b1/b4). These are mosly within the same 4-8 GHz FFTS spectral window, but notice
the 1 GHz shift which leads the outer (b1, b4) to overlap into the higher 8-12 GHz
FFTS spectral window. There is no workaround. A weighted average of metadata
might be possible. The easier choice, however, is to take all metadata from just
the 4-8 GHz FFTS spectral window.



