#!/bin/bash
#
# Dual pol metadata can be produced with
# $ class @tau
#
# Alas, sorting in those metadata is such that all "RCP" of the
# day come first, followed by all "LCP" data last.
#
# In addition, datasets in .txt are not yet split (renamed)
# according to EHT track names.
#
# Trying to fix the above issues:
#

for fn in tau_*.txt; do
	sort -k 2 -h -t"," $fn > ${fn}.sorted
done

## Method 1
## while filterByVexTime.py did not exist yet
#cp -av tau_2026-03-10.txt.sorted e26m10ax_tau.csv
#cp -av tau_2026-03-13.txt.sorted e26m13ax_tau.csv
#cp -av tau_2026-03-17.txt.sorted e26m17ax_tau.csv
#cp -av tau_2026-03-20.txt.sorted e26m20ax_tau.csv
#cp -av tau_2026-03-24.txt.sorted e26m24ax_tau.csv
#cp -av tau_2026-03-27.txt.sorted e26m27ax_tau.csv
#
#cp -av tau_2026-04-03.txt.sorted e26a03ax_tau.csv
#cp -av tau_2026-04-07.txt.sorted e26a07ax_tau.csv
#cp -av tau_2026-04-10.txt.sorted e26a10ax_tau.csv
#cp -av tau_2026-04-14.txt.sorted e26a14ax_tau.csv
#
# Days with EHT and GMVA, drop entries from backends other than AP-N20<x>
#grep "AP-N20" tau_2026-04-17.txt.sorted > e26a17ax_tau.csv
#grep "AP-N20" tau_2026-04-20.txt.sorted > e26a20ax_tau.csv
#
#cp -av tau_2026-04-24.txt.sorted e26a24ax_tau.csv
#cp -av tau_2026-04-28.txt.sorted e26a28ax_tau.csv
#
#cp -av tau_2026-05-01.txt.sorted e26y01ax_tau.csv
#
# EHT tracks that crossed UT day boundaries
#cat tau_2026-05-04.txt.sorted tau_2026-05-05.txt.sorted >  e26y05ax_tau.csv
#cat tau_2026-05-07.txt.sorted tau_2026-05-08.txt.sorted >  e26y08ax_tau.csv

## Method 2
# 230 GHz: AP-N201 AP-N202 AP-N203 AP-N204
for vexf in ~/vlbisystem/vexfiles/triggered/e26*.vex; do
	outname=$(basename $vexf)
	outname=${outname/.vex/}ax_tau.csv
	./filterByVexTime.py --backend AP-N20 --output $outname $vexf  tau_2026-*.sorted
done

