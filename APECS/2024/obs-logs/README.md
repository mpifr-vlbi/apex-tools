
# Logs and raw metadata for 2024

This directory contains various observation log and metadata files:

 - *.vex : copies of the observed VEX schedules

 - *.apecs.obs : timed APECS command sequence of the observation

 - *.apecs.obs.log : run log file of apecsVLBI.py execution of the above timed commands

 - *.wx.log : weather station data (WX) in FieldSystem-like format

 - _tau.csv : single dish metadata like tau and Tamb [K], extracted and converted with Gildas from observer.apex-telescope.org:~/scidata/*.apex day-based datasets


The order of the columns in the CSV files is:

 1. date of obs (dd-MMM-yyyy)
 2. time (UT hour)
 3. scan number
 4. source-name
 5. elevation
 6. rest freq [MHz]
 7. backend (e.g., 'AP-N901-F303')
 8. opacity (tau)
 9. opacity signal band (tausig; apparently identical to tau)
 10. Tsys system temperature [K]
 11. Tamb ambient temperature [K]

