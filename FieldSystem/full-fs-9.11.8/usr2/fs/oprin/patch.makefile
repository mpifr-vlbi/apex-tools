FFLAGS= -m32
--- readline-2.0/Makefile	2016-07-12 15:01:23.000000000 +0000
+++ readline-2.0/Makefile	2016-07-12 15:05:47.000000000 +0000
@@ -36,8 +36,8 @@ RM = rm
 CP = cp
 MV = mv
 
-DEFS = -DHAVE_CONFIG_H
-CFLAGS = -m32  -pipe -O2 -fomit-frame-pointer
+DEFS = -DHAVE_CONFIG_H -DHAVE_STDLIB_H
+CFLAGS = -m32 -pipe -O2 -fomit-frame-pointer
 LDFLAGS = 
 
 # For libraries which include headers from other libraries.
