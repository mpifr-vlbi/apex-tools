

## APECS observing scripts

# GMVA frequency setup
./vex2apecs.py -s N3AR90 vexfiles/triggered/c251a.vex
./vex2apecs.py -s N3AR90 vexfiles/triggered/c251b.vex
./vex2apecs.py -s N3AR90 vexfiles/triggered/c251c.vex
./vex2apecs.py -s N3AR90 vexfiles/triggered/c251d.vex
./vex2apecs.py -s N3AR90 vexfiles/triggered/f251a.vex
./vex2apecs.py -s N3AR90 vexfiles/triggered/mj008.vex

# Special frequency setup for DW Kim c251z.vex
# Original c251z has mode=104ghz for scans No0001-No0012,
# then switches to 114ghz for the remainin No0013-No0024
#
# No0013;
#  *     Note a COMMENT was inserted during scheduling:
#  *       change LO to 114 GHz, allow 45mins for setup
#     start=2025y118d03h15m00s; mode=114ghz; source=1308+326;
#
# For Mark6 the schedule remains called 'c251z.vex'
#
# For APECS made two copies of c251z.vex with scans of just
# the specific freq setup retained
#  APECS c251z104.apecs.obs = 104G - operator must load() N3AR90_DWKim1044_setup.apecs
#  APECS c251z114.apecs.obs = 114G - note operator must switch to N3AR90_DWKim114_setup.apecs and load()
#
./vex2apecs.py -s N3AR90_DWKim104 vexfiles/triggered/c251z104.vex
./vex2apecs.py -s N3AR90_DWKim114 vexfiles/triggered/c251z114.vex
#


## Source catalog

./vexGetSources2cat.py vexfiles/triggered/c251?.vex vexfiles/triggered/mj008.vex vexfiles/triggered/c251z104.vex vexfiles/triggered/c251z114.vex > gmva.cat
# sort gmva.cat | uniq >> ~/Observation/t-0115.f-9996a-2025.cat  # done 16apr2025


