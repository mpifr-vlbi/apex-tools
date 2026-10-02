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
# Trying to fix the above issues:
#

for fn in tau_*.txt; do
	sort -k 2 $fn > ${fn}.sorted
done

for vexf in ~/vlbisystem/vexfiles/triggered/c25*.vex; do
	outname=$(basename $vexf)
	outname=${outname/.vex/}ax_tau.csv
	./filterByVexTime.py --backend AP-N90 --output $outname $vexf  tau_2025-*.sorted
done

./filterByVexTime.py --backend AP-N90 --output mj008ax_tau.csv ~/vlbisystem/vexfiles/triggered/mj008.vex  tau_2025-*.sorted

./filterByVexTime.py --backend AP-N90 --output fpt25ax_86G_tau.csv ~/vlbisystem/vexfiles/triggered/fpt_test_2025.vex  tau_2025-*.sorted
./filterByVexTime.py --backend AP-N20 --output fpt25ax_260G_tau.csv ~/vlbisystem/vexfiles/triggered/fpt_test_2025.vex  tau_2025-*.sorted
