#************************************************************************
# APEX project
#
# "@(#) $Id: Makefile 6317 2013-06-27 11:22:03Z dmuders $"
#
# Makefile of apexVLBI
#
# who       when        what
# --------  ----------  ---------------------------------------------------
# aroy      2010/01/13  Adapted for apexMaserHousing.
# dmuders   2010/03/18  Adapted yacc setup for further SCPI servers to be
#                       included in apexVLBI.
# dmuders   2010/04/16  Added apexMaserHousing start/stop script.
# aroy      2010/05/18  Added VLBI downconverter files.
# aroy      2010/10/05  Added Counters files.
#

#************************************************************************
# This Makefile follows VLT Standards (see Makefile(5) for more).
#************************************************************************
# REMARKS
#    None
#------------------------------------------------------------------------

YACC = yacc

#
# user definable C-compilation flags
USER_CFLAGS = -Wno-deprecated
# USER_CFLAGS = -Wno-deprecated -O0 -g

#
# additional include and library search paths
#USER_INC =
USER_LIB = -lACE

#
# MODULE CODE DESCRIPTION:
# ------------------------
# As a general rule:  public file are "cleaned" and "installed"  
#                     local (_L) are not "installed".

#
# C programs (public and local)
# -----------------------------
EXECUTABLES     = apexMaserHousingControl apexVLBIDownConverterControl apexCountersControl
EXECUTABLES_L   = 

#
# <brief description of xxxxx program>
apexMaserHousingControl_OBJECTS	= cid_maserhousing udpio_maserhousing yacc_maserhousing \
				  encode_maserhousing keyword_maserhousing \
				  reset_maserhousing strsplit_maserhousing queues \
				  maserhousing version usage_maserhousing

apexVLBIDownConverterControl_OBJECTS	= cid_downconverter udpio_downconverter yacc_downconverter \
				  encode_downconverter keyword_downconverter \
				  reset_downconverter strsplit_downconverter queues \
				  downconverter version usage_downconverter

apexCountersControl_OBJECTS	= cid_counters udpio_counters yacc_counters \
				  encode_counters keyword_counters \
				  reset_counters strsplit_counters queues \
				  counters version usage_counters
#
# special compilation flags for single c sources
#yyyyy_CFLAGS   = 

#
# Includes (.h) files (public only)
# ---------------------------------
INCLUDES        =

#
# Libraries (public and local)
# ----------------------------
LIBRARIES       =
LIBRARIES_L     =

#
# <brief description of lllll library>
lllll_OBJECTS   =

#
# Scripts (public and local)
# ----------------------------
SCRIPTS         = apexMaserHousing apexVLBIDownConverter apexCounters
SCRIPTS_L       =

#
# TCL scripts (public and local)
# ------------------------------
TCL_SCRIPTS     =
TCL_SCRIPTS_L   =

#
# Python stuff (public and local)
# ----------------------------
PY_SCRIPTS         = 
PY_SCRIPTS_L       =

PY_MODULES         = 
PY_MODULES_L       =

PY_PACKAGES        =
PY_PACKAGES_L      =
pppppp_MODULES	   =

#
# <brief description of tttttt tcl-script>
tttttt_OBJECTS  =
tttttt_TCLSH    = 
tttttt_LIBS     = 

#
# TCL libraries (public and local)
# ------------------------------
TCL_LIBRARIES   =
TCL_LIBRARIES_L =

#
# <brief description of tttlll library>
tttlll_OBJECTS  = 

#
# UIF panels (public and local)
# ----------------------------
PANELS   =
PANELS_L = 

#
# On-Line Database Files
# ----------------------

DBL_CLASSES = 
DBL_BCF =
DBL_FLAGS =
xxxx_DBL_FLAGS =

# 
# IDL Files and flags
# 
IDL_FILES =
IDL_TAO_FLAGS =
USER_IDL =

#
# man pages to be done
# --------------------
MANSECTIONS =
MAN1 =
MAN3 =
MAN5 =
MAN7 =
MAN8 =

#
# local man pages
# ---------------
MANl =

#
# ASCII file to be converted into Framemaker-MIF
# --------------------
ASCII_TO_MIF = 

#
# INS_ROOT files to be installed
#-------------------------------
INS_ROOT_FILES =
INS_ROOT_DIR   =

#
# other files to be installed
#----------------------------
INSTALL_FILES =

#
# list of all possible C-sources (used to create automatic dependencies)
# ------------------------------
CSOURCENAMES = \
	$(foreach exe, $(EXECUTABLES) $(EXECUTABLES_L), $($(exe)_OBJECTS)) \
	$(foreach lib, $(LIBRARIES) $(LIBRARIES_L), $($(lib)_OBJECTS))

#
#>>>>> END OF standard rules

#
# INCLUDE STANDARDS
# -----------------
ifdef ACSROOT
    MAKEDIR  = $(shell if [ -f $(INTROOT)/include/acsMakefile ];  then \
                     echo $(INTROOT)/include; \
	           else \
	             echo $(ACSROOT)/include; \
		   fi;)
    include $(MAKEDIR)/acsMakefile
else
    MAKEDIR  = $(shell if [ -f $(INTROOT)/include/vltMakefile ];  then \
                     echo $(INTROOT)/include; \
	           else \
	             echo $(VLTROOT)/include; \
		   fi;)
    include $(MAKEDIR)/vltMakefile
endif

#
# TARGETS
# -------
all:	clean_yaccs yacc_maserhousing.cpp yacc_downconverter.cpp yacc_counters.cpp do_all
	@echo " . . . 'all' done" 

clean: clean_all 
	@rm -rf ../lib/python/site-packages/*
	@rm -f *.pyc
	@echo " . . . clean done"

clean_dist: clean_all clean_dist_all 
	@echo " . . . clean_dist done"

man: do_man 
	@echo " . . . man page(s) done"

install: install_all
	@echo " . . . installation done"

db: db_all
	@echo " . . . ../DB done"

clean_yaccs:
	rm -f yacc_*.cpp
	rm -f ../include/yacc_*.h

yacc_maserhousing.cpp:
	$(YACC) -vdl ../include/yacc_maserhousing.y
	mv y.tab.h ../include/yacc_maserhousing.h
	rm -f yacc_maserhousing.cpp
	grep -v yacc_maserhousing.y y.tab.c > yacc_maserhousing.cpp
	rm y.tab.c y.output

yacc_downconverter.cpp:
	$(YACC) -vdl ../include/yacc_downconverter.y
	mv y.tab.h ../include/yacc_downconverter.h
	rm -f yacc_downconverter.cpp
	grep -v yacc_downconverter.y y.tab.c > yacc_downconverter.cpp
	rm y.tab.c y.output

yacc_counters.cpp:
	$(YACC) -vdl ../include/yacc_counters.y
	mv y.tab.h ../include/yacc_counters.h
	rm -f yacc_counters.cpp
	grep -v yacc_counters.y y.tab.c > yacc_counters.cpp
	rm y.tab.c y.output

#___oOo___
