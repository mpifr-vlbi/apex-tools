#!/bin/bash
#
# Dual pol metadata can be produced with
# $ class @tau
#
# Alas, sorting in those metadata is such that all "RCP" of the
# day come first, followed by all "LCP" data last.
#
# In addition, datasets in .txt are not yet split by GMVA tracks.
#
# Trying to fix the above issues:
#

for fn in tau_2024-*.txt; do
	sort -k 2 -h -t"," $fn > ${fn}.sorted
done

for vexf in ~/src/apex-tools/APECS/2024/obs-logs/{f242,c242}*.vex ; do
        outname=$(basename $vexf)
        outname=${outname/.vex/}ax_tau.csv
        ./filterByVexTime.py --backend AP-N90 --output $outname $vexf  tau_2024-*.sorted
done

./filterByVexTime.py --backend AP-N90 --output fpt24ax_86G_tau.csv  ~/src/apex-tools/APECS/2024/obs-logs/fpt_test.vex  tau_2024-*.sorted
./filterByVexTime.py --backend AP-N20 --output fpt24ax_260G_tau.csv ~/src/apex-tools/APECS/2024/obs-logs/fpt_test.vex  tau_2024-*.sorted

