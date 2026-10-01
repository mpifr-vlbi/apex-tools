
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
