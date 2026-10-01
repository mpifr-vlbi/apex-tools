#!/bin/bash
#
# Dual pol metadata can be produced with
# $ class @tau
#
# Alas, sorting in those metadata is such that all "RCP" of the
# day come first, followed by all "LCP" data last.
#
# In addition, datasets in .txt are not yet split by GMVA tracks:
#
# tau_2026-09-24.txt  C262A starting from ~17:50 UT
# tau_2026-09-25.txt  C262A continued
# tau_2026-09-26.txt  C262A continued till end at ~08:57 UT, then C262B
# tau_2026-09-27.txt  C262B continued till end at ~07:30 UT, then C262C
# tau_2026-09-28.txt  C262C continued till end at ~03:00 UT, then MN006A till ~12:00 UT
# tau_2026-09-29.txt  MN006B starting from ~06:30 UT until ~12:00 UT
#
# Trying to fix the above issues:
#

for fn in *.txt; do
	sort -k 2 $fn > ${fn}.sorted
done

cat tau_2026-09-24.txt.sorted > c262a_tau.txt
cat tau_2026-09-25.txt.sorted >> c262a_tau.txt
grep -E "26-SEP-2026\,[0-8]\." tau_2026-09-26.txt.sorted >> c262a_tau.txt

grep -v -E "26-SEP-2026\,[0-8]\." tau_2026-09-26.txt.sorted > c262b_tau.txt
grep -E "27-SEP-2026\,[0-8]\." tau_2026-09-27.txt.sorted   >> c262b_tau.txt

grep -v -E "27-SEP-2026\,[0-8]\." tau_2026-09-27.txt.sorted > c262c_tau.txt
grep -E "28-SEP-2026\,[0-2]\." tau_2026-09-28.txt.sorted   >> c262c_tau.txt

grep -v -E "28-SEP-2026\,[0-2]\." tau_2026-09-28.txt.sorted > mn006a_tau.txt

cat tau_2026-09-29.txt.sorted > mn006b_tau.txt
