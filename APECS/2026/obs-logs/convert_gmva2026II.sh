

## APECS observing scripts

# GMVA frequency setup
./vex2apecs.py -s N3AR90_GMVA86 vexfiles/triggered/c262a.vex
./vex2apecs.py -s N3AR90_GMVA86 vexfiles/triggered/c262b.vex
./vex2apecs.py -s N3AR90_GMVA86 vexfiles/triggered/c262c.vex
# ./vex2apecs.py -s N3AR90_GMVA86 vexfiles/triggered/my008.vex # APEX not scheduled in this proj

# Special frequency setups
./vex2apecs.py -s N3AR90_mn006a vexfiles/triggered/mn006a.vex
./vex2apecs.py -s N3AR90_mn006b vexfiles/triggered/mn006b.vex
sed -i "s/vlbi_tone(enable=True)/vlbi_tone(enable=False)/g" mn006?.apecs.obs

## Source catalog

./vexGetSources2cat.py \
   vexfiles/triggered/c262?.vex \
   vexfiles/triggered/{mn006a,mn006b}.vex  > tmp.cat
sort tmp.cat | uniq > gmva.cat
rm tmp.cat
sdiff gmva.cat ~/Observation/t-0117.f-9996a-2026.cat | less -S

# grep -h "source" vexfiles/triggered/{c262a,c262b,c262c,mn006a,mn006b}.vex | grep mode | grep -e "86ghz\|hco\|hcn" | cut -d";" -f3 | sort | uniq
