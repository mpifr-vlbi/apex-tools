'''
R2DE manual ADC core offsets tuning, indended for the case
where an ADC-chip registers can be written to but the readout
returns garbage and prevents the normal start_backend.py from
properly calibrating the ADC.

Usag: manual_core_tune.py <r2dbe hostname>

'''
from mandc.r2dbe import R2dbe
from mandc.r2dbe.adc import get_core_offsets, set_core_offsets
from numpy import array
from time import sleep
import sys

if len(sys.argv) != 2:
    print (__doc__)
    sys.exit(1)

r2 = R2dbe(sys.argv[1])

# reset the core offsets for ADC1
set_core_offsets(r2.roach2, 1, [0,0,0,0])

# check that the core offsets are zero (with the broken ADC1 of course there will be variation)
get_core_offsets(r2.roach2, 1)

# 8-bit state counts integrate over 1s and the buffer holds values for 16s
# t - timestamps
# m - means (single value per 1s and per core)
# v - variances (single value per 1s and per core)
t,m,v = r2._dump_8bit_counts_mean_variance(1)
# m is (16,4), see above
m.shape
# calculate the mean value for each core over the last 16s
m.mean(axis=0)

# the units of core offsets are least-significant bit in the 8-bit sample, i.e. 1 on the ADC -128 to 127 range, so adjust the core offsets by the negative of means
set_core_offsets(r2.roach2, 1, -m.mean(axis=0))

# check that the core offsets are close to those requested (they are discrete values and will only approximate the values passed)
get_core_offsets(r2.roach2, 1)

# wait 16s for the new 8-bit state counts in buffer after new core settings applied
sleep(16)

# check again the means
t,m,v = r2._dump_8bit_counts_mean_variance(1)
m.mean(axis=0)

# adjust the core offsets by the negative of the means
set_core_offsets(r2.roach2, 1,array([ 2.35294118,  0.78431373,  2.35294118, -0.39215686])-m.mean(axis=0))

# read back the new core offsets and check they are as requested
get_core_offsets(r2.roach2, 1)

# wait 16s for the new 8-bit state counts in buffer after new core settings applied
sleep(16)

# check again the means
t,m,v = r2._dump_8bit_counts_mean_variance(1)
m.mean(axis=0)
