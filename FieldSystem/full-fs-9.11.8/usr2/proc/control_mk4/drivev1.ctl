********* drivev1.ctl VLBA/4 Drive 1 Equipment Control File *********
* Please refer to the Control Files Manual in Volume 1 of the 
* Field System Documentation
* 
330     Max tape speed, ips
270     Schedule tape speed, ips
*
yes      vacuum switching control
*
mvme117 VLBA recorder CPU: mvme117 or mvme162
0       head motion delay (centiseconds)
*
11.0    vacuum level (inches) (if vacuum switching this is for thin tape ~5.0) 
1560    vacuum current offset (counts was 1373)
92.2     vacuum current scale  (counts/in was 92.2)
-4.8    vacuum sensor offset  (in)
.0139    vacuum sensor scale   (in/count)
152     tape thickness (kA)   (if vacuum switching this is for thin tape ~152) 
12.0    write voltage (V) (stack 1 for vlba4, for thin tape if switching)
54625   capstan size constant
* 3 additional values for vacuum switching only
15.0    thick tape vacuum level (inches)
268     thick tape thickness (kA)
15.0    thick tape write voltage (V) (stack 1 for vlba4)
* VLBA4 parameters
12.0    VLBA4 stack 2 write voltage (V) (thin tape if switching)
12.0    VLBA4 stack 2 write voltage (V) (only if switching then thick)
