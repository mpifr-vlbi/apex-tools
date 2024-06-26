C
C  COMMON STATEMENT FOR LOGEX
C
      CHARACTER*20 LOGNC
      character*64 LSKNC,NAMCMC
      CHARACTER*20 NAMFC
      CHARACTER*100 IBC,JBC
      integer logformat,iyear,isly
      INTEGER*2 IBUF(256),JBUF(256),NAMCM(32),NAMF(10),LOGNA(10)
      integer*2 LSKNA(32)
      INTEGER*2 LTAPEN(4), LSOURN(8), LSFT, LEFT, LSOUON(4), ltype(5)
      integer*2 i2dum
      EQUIVALENCE (IBUF,IBC),(JBUF,JBC),(NAMCM,NAMCMC),(NAMF,NAMFC)
      EQUIVALENCE (LOGNA,LOGNC),(LSKNA,LSKNC)
C
      COMMON /LXCOM/
     .SMAX(5),SMIN(5),IDCB(2),JDCB(2),IDCBSK(2),IBUF,
     .JBUF,LCOMND(6,5),LSTRNG(6,5),LSTATN(4),LLOGX(5),LTYPE,i2dum,
     .NCOMND(5),NPARM(5),NSTRNG(5),LSOUON(4),LSOURN(8),LTAPEN(4),
     .LOGNA,LSKNA,IBLEN,IEQ,IKEY,ILEN,ITE1,ITE2,ITE3,ITL1,
     .ITL2,ITL3,ITS1,ITS2,ITS3,LUUSR,LUDSP,L6,NCHAR,NCMD,NLINES,
     .NLOUT,NSTR,NTYPE,NINTV,NUMP,ITERM,ISLD,ISLHR,ISLMIN,ISLSEC,
     .IELD,IELHR,IELMIN,IELSEC,LSFT,LEFT,ILRDAY,ILRHRS,ILRMIN,
     .NAMF,ICODE,IRTN1,IRTN2,IRTN4,IRTN5,IRTN6,ILXGET,ITCNTL,
     .LSTAT(3),IWIDTH,IHGT,LTAPN(4),IOUT,IDCBCM(2),IFTYPE,ISKED,
     .NAMCM,IL,ICMD,IT3,LSTEND,NSCALE(5),ITSK1,ITSK2,ITSK3,ITSKE1,ITSKE2,
     .ITSKE3,SDELTA(5),logformat,iyear,isly
C
C
C     IBLEN - Buffer length in words.
C     IBUF - Buffer for input & log reading.
C     ICMD - A flag which indicates that a command file is being used.
C     ICODE - Error flag.
C     ICR - Logical unit where the log resides.
C     IDCB - DCB for the log file.
C     IDCBCM - DCB for command file.
C     IELD - SUMMARY stop time (the day) of an observation.
C     IELHR - SUMMARY stop time (the hour) of an observation.
C     IELMIN - SUMMARY stop time (the minutes) of an observation.
C     IELSEC - SUMMARY stop time (the seconds) of an observation.
C     IEQ - Character number of equals sign.
C     IFTYPE - Type of log file, 2-char or namr
C     IHGT - Height of the plot in lines.
C     IKEY - The command number determined in IGTCM.
C     IL - Number of words in a command file.
C     ILEN - Number of words in a log record.
C     ILRDAY,ILRHRS,ILRMIN - Time control command in day, hrs & mins
C     ILSEC - Log file security code. 
C     ILXGET - Flag which indicates whether the log file requires re- 
C              winding. 
C     IOUT - A flag that indicates whether the Output file processing 
C            statement has been written.
C     IRTN1,IRTN2,IRTN4,IRTN5,IRTN6 - Variables that define the return
C     return points for segments. 
C     ISCR - Logical unit where the schedule file resides.
C     ISKSEC - Schedule file security code. 
C     ISKED - Indicates whether a SKED file has been specified. 
C     ISLD - SUMMARY start time (the day) of an observation.
C     ISLHR - SUMMARY start time (the hour) of an observation.
C     ISLMIN - SUMMARY start time (the minutes) of an observation.
C     ISLSEC - SUMMARY start time (the seconds) of an observation.
C     ITCNTL - A flag that indicates whether we have a time control 
C              command for the SUMMARY. 
C     ITERM - A flag that indicates whether an output file name 
C             has been specified in the OUTPUT command. 
C     IT3 - Seconds in a log record.
C     ITE1,ITE2 - Stop day, min.
C     ITL1,ITL2 - Log record day, min.
C     ITS1,ITS2 - Start day, min. 
C     ITSK1,ITSK2 - Specified start day, minutes in schedule file 
C     ITSKE1,ITSKE2 - Specified end day, minutes in schedule file 
C     IWIDTH - Width of the plot in characters. 
C     JBUF - Output file buffer.
C     JDCB - DCB for output file. 
C     IDCBSK - DCB for schedule file. 
C     LCOMND - This array stores a maximum of five commands to search 
C              for with each command having a maximum of 12 characters. 
C     LEFT - Stop time footage count
C     LLOGX - Llogx (dB scale) plotting scale of each parameter.
C     LOGNA - File name of log. 
C     LSFT - Start time footage count.
C     LSKNA - Schedule file name. 
C     LSOUON - Contains the log tracking status.
C     LSOURN - Contains the source name found in the log entry. 
C     LSTAT - Contains the status codes for the SKSUMMAY & SUMMARY cmds.
C     LSTATN - Contains the station name. 
C     LSTEND - A flag that indicates whether requested listing has been 
C              reached. 
C     LSTRNG - This array stores five commands to search for after the
C              time field to end of the log entry.
C     LTAPEN - Contains the log tape number.
C     LTAPN - Contains the previous tape number.
C     LTYPE - This array stores five special characters to search for 
C             in column 10 of a log entry.
C     LUDSP - Display LU. 
C     LUUSR - User LU.
C     L6 - Carriage control.
C     NAMCM - Command file name.
C     NAMF - Output file name.
C     NCHAR - Total number of characters in a log entry.
C     NCMD - Total number of COMMAND commands specified.
C     NCOMND - Actual number of characters in each COMMAND command. 
C     NERROR - Number of CHEKR errors. A maximum of five is written out.
C     NINTV - A flag which indicates whether LOGEX is in the non- 
C             interactive mode. 
C     NLINES - A second parameter of the LIST, PLOT, or SUMMARY 
C              commands that specifies a limited number of lines to 
C              listed or plotted. 
C     NLOUT - Number of lines in the log outputted. 
C     NPARM - Contains the parameters specified in the PARM command 
C             that corresponds to the parameters in the log entry.
C     NSCALE - The parameter the SCALE command applies to.
C     NSTR - Number of STRING commands specified. 
C     NSTRNG - Actual number of characters in each STRING command.
C     NTYPE - Number of special characters specified in a TYPE command. 
C     NUMP - Total number of parameters specified.
C     SMAX - Maximum value of the plotting scale for each parameter.
C     SMIN - Minimum value of the plotting scale for each parameter.
