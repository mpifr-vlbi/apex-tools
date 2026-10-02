

This directory contains various observation log and metadata files:

 - *.vex : copies of the observed VEX schedules

 - *.apecs.obs : timed APECS command sequence of the observation

 - *.apecs.obs.log : run log file of apecsVLBI.py execution of the above timed commands

 - *.wx.log : weather station data (WX) in FieldSystem-like format

 - *.clock.log : gps-fmout and gps-maser time offset measurements in FieldSystem-like format

 - *.fslog : live APECS Online Calibrator data logged during the observations, incl. Tsys, Trec, and since Autumn 2026 also Tamb, tausig (tau in main typ. LSB sideband), tauima (tau in opposite sideband)

 - *_tau.csv : single dish metadata like tau and Tamb [K], extracted and converted with Gildas from observer.apex-telescope.org:~/scidata/*.apex day-based datasets



The order of the columns in the CSV files is:

  - date of obs (dd-mm-yyyy)
  - time (UT hour)
  - scan number
  - source-name
  - elevation
  - rest freq (MHz)
  - backend (e.g., 'AP-N901-F303')
  - opacity (tau)
  - opacity signal band (tausig; apparently identical to tau)
  - Tsys system temperature
  - Tamb ambient temperature (K)

