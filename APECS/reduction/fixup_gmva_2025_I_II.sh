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

for fn in tau_2025-*.txt; do
	sort -k 2 -h -t"," $fn > ${fn}.sorted
done

for vexf in ~/src/apex-tools/APECS/2025/obs-logs/c25*.vex; do
	outname=$(basename $vexf)
	outname=${outname/.vex/}ax_tau.csv
	./filterByVexTime.py --backend AP-N90 --output $outname $vexf  tau_2025-*.sorted
done

./filterByVexTime.py --backend AP-N90 --output mj008ax_tau.csv ~/src/apex-tools/APECS/2025/obs-logs/mj008.vex  tau_2025-*.sorted

./filterByVexTime.py --backend AP-N90 --output fpt25ax_86G_tau.csv ~/src/apex-tools/APECS/2025/obs-logs/fpt_test_2025.vex  tau_2025-*.sorted
./filterByVexTime.py --backend AP-N20 --output fpt25ax_260G_tau.csv ~/src/apex-tools/APECS/2025/obs-logs/fpt_test_2025.vex  tau_2025-*.sorted
