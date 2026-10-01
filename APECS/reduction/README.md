
# Extraction of tau and other values

Input are the observational data in the timestamped files ~/scidata/*.apex

These can be read by Gildas https://www.iram.fr/IRAMFR/GILDAS/

Edit tau.class and modify the lines

  file in ../scidata/T-0117.F-9996A-2026-2026-09-29.apex
  sic output "tau_0929.txt"

Then run with

$ class @tau

The order of the columns in the output .txt file are:

  - source-name
  - date of obs
  - scan number
  - time(UT)
  - rest freq (MHz)
  - backends
  - opacity (tau)
  - opacity signal band
  - elevation
  - Tsys system temperature
  - Tamb ambient temperature (K)
