

## APECS observing scripts

# GMVA frequency setup
./vex2apecs.py -s N3AR90_GMVA86 vexfiles/triggered/c252a.vex
./vex2apecs.py -s N3AR90_GMVA86 vexfiles/triggered/c252b.vex
./vex2apecs.py -s N3AR90_GMVA94 vexfiles/triggered/c252c.vex

# FPT test (not GMVA)

./vex2apecs.py -s N3AR90xNFLASH260 vexfiles/triggered/fpt_test_2025.vex

## Source catalog

./vexGetSources2cat.py vexfiles/triggered/c252?.vex > gmva.cat
./vexGetSources2cat.py vexfiles/triggered/fpt_test_2025.vex > fpt.cat

# sort gmva.cat | uniq >> ~/Observation/t-0115.f-9996a-2025.cat  # done 19sept2025
# sort fpt.cat | uniq >> ~/Observation/t-0115.f-9996a-2025.cat

