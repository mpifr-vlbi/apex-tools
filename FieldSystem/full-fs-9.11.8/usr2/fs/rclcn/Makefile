FFLAGS= -m32
CFLAGS= -m32
FFLAGS= -m32
#
#NIX= -DDEBUG 
LIBES = ../rclco/rcl/rcl_cmd.o ../rclco/rcl/rcl_pkt.o ../rclco/rcl/rcl_sysn.o\
../rclco/rcl/rcl_util.o ../poclb/poclb.a ../clib/clib.a ../rtelb/rtelb.a 
#
OBJECTS = rclcn.o rclcn_das.o
#
../bin/rclcn: $(OBJECTS)
	cc  -o ../bin/rclcn $(CFLAGS) $(OBJECTS) $(LIBES)
	strip ../bin/rclcn
#
rclcn:
	rm -f ../bin/rclcn
	make
