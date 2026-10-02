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

# 230 GHz: AP-N201 AP-N202 AP-N203 AP-N204
for vexf in ~/vlbisystem/vexfiles/triggered/e24*.vex; do
	outname=$(basename $vexf)
	outname=${outname/.vex/}ax_tau.csv
	./filterByVexTime.py --backend AP-N20 --output $outname $vexf  tau_2024-*.sorted
done

# 345 GHz: AP-S30
./filterByVexTime.py --backend AP-S30 --output e24c09ax_tau.csv ~/vlbisystem/vexfiles/triggered/e24c09.vex tau_2024-*.sorted

