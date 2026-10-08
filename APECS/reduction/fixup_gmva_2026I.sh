#!/bin/bash
#
# Dual pol metadata can be produced with
# $ class @tau
#
# Alas, sorting in those metadata is such that all "RCP" of the
# day come first, followed by all "LCP" data last.
#
# Some days had both GMVA and EHT on the same UT day.
#
# In addition, datasets in .txt are not yet split by GMVA tracks.
#
# c261a 17.04.2026 09:00 UT to 15:40 UT
# c261b 17.04.2026 22:30 UT to 18.04. (+1d) 15:40 UT
# c261c 18.04.2026 22:40 UT to 19.04. (+1d) 15:40 UT
# c261d
# mj008b 21.04.2026 07:00 UT to 14:00 UT
#
# Trying to fix the above issues:
#

for fn in tau_*.txt; do
	sort -k 2 $fn > ${fn}.sorted
done

grep "AP-N90" tau_2026-04-21.txt.sorted > mj008bax_tau.csv

./filterByVexTime.py --backend AP-N90 --output c261bax_tau.csv  ~/vlbisystem/vexfiles/triggered/c261b.vex  tau_2026-04-*.sorted
./filterByVexTime.py --backend AP-N90 --output c261cax_tau.csv  ~/vlbisystem/vexfiles/triggered/c261c.vex  tau_2026-04-*.sorted
./filterByVexTime.py --backend AP-N90 --output c261dax_tau.csv  ~/vlbisystem/vexfiles/triggered/c261d.vex  tau_2026-04-*.sorted
./filterByVexTime.py --backend AP-N90 --output c261eax_tau.csv  ~/vlbisystem/vexfiles/triggered/c261e.vex  tau_2026-04-*.sorted
./filterByVexTime.py --backend AP-N90 --output c261fax_tau.csv  ~/vlbisystem/vexfiles/triggered/c261f.vex  tau_2026-04-*.sorted

# c261a not observed
# mk036 and mk037a/b/c/d also not observed
