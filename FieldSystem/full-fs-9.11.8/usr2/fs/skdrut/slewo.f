      SUBROUTINE SLEWo(NSNOW,MJD,UT,NSNEW,ISTN,LWRCUR,LWRNEW,TSLEW,
     .lookah,trise)
C
C   SLEWT calculates the slew time and the cable wrap
C   ***NOTE*** This is the version as of 10/93 before the pre-calculated
C              rise/set times were available. Use this version with
C              drudg. Use the new version with sked.
C
C 040623  ZMM  removed trailing RETURN

      include '../skdrincl/skparm.ftni'
      include '../skdrincl/constants.ftni'
C
C     INPUT VARIABLES:
         integer nsnow,mjd,nsnew,istn,lookah
         integer*2 lwrcur
         real*8 ut
C        NSNOW  - current source index into DB arrays
C        NSNEW  - new source index, i.e. the one to slew to
C        MJD    - date of observation
C               - UT of observation
C        ISTN   - station index
C        LWRCUR - current wrap of telescope
C        LWRNEW - The wrap requested by the user for the new observation.
C                 " "=fastest, "W"=clockwise part of overlap, "C"=counter
C                 clockwise part of overlap.  Modified as necessary by
C                 this program.
C        lookah - number of seconds of lookahead time for checking rising
C
C     OUTPUT VARIABLES:
         real*4 tslew
         integer*2 lwrnew
C        TSLEW  - time for ISTN to slew from NSNOW to NSNEW, seconds
C                 -1 = not up
C                 -2 = slew does not converge
C    DISABLED:    -3 = not up now, rising within an hour
C                 -nnn = time until rise
C        trise  =  time until the source rises, seconds
C          (it this is > 0 then TSLEW includes this time)
C
C   COMMON BLOCKS USED
      include '../skdrincl/sourc.ftni'
      include '../skdrincl/statn.ftni'
C
C     CALLING SUBROUTINES: OBSCM,CHCMD, and others
C     CALLED SUBROUTINES: CVPOS,CABLW
C
C   LOCAL VARIABLES
       LOGICAL kcont
      REAL*4 RSTCON(2),tslewp,tslewc,delaz,delel,deldc,delha,
     .delx30,dely30,delx85,dely85,aznow,aznew,elnow,elnew,hanow,
     .hanew,decnow,decnew,x30now,x30new,y30now,y30new,x85now,x85new,
     .y85now,y85new,az1,az2,trise,elrate,tslew1,tslew2
      integer nloops,il
      integer*2 lwr1,lwr2,lwr2p
      LOGICAL KUP ! Returned from CVPOS, TRUE if source within limits
C        TSLEWP,TSLEWC - previous, current slew times.  For iterating.
C        DELAZ,DELEL,DELDC,DELHA,DELX30,DELY30,DELX85,DELY85
C        AZNOW,AZNEW,ELNOW,ELNEW,HANOW,HANEW,DECNOW,DECNEW
C        X30NOW,X30NEW,Y30NOW,Y30NEW,X85NOW,X85NEW,Y85NOW,Y85NEW
C               - Increments, current, next values of az,el,ha,x,y
C        CABLW  - Function to compute required az move.
C        NLOOPS - Number of iterations on slewing time
C        AZ1,AZ2,LWR1,LWR2
C               - current,new values of az,wrap
      real*4 cablw ! function
      real rme
C
C  History
C      DATE   WHO    CHANGES
C     811125  MAH    CHECK THAT SLEWING DOES CONVERGE FOR AZ-EL ANTENNAS
C     830423  NRV    ADD X,Y CALCULATIONS
C     830523  WEH    SATELLITES ADDED, DEC ADDED TO CVPOS CALL, SLEWING
C                    NOW USES RETURNED DECs NOT VALUES FROM COMMOM
C     880315  NRV    DE-COMPC'D
C     900425  NRV    Added check for axis type 6 (SEST)
C     900511  NRV         "       "      "     7 (ALGO)
C     930308  nrv    implicit none
C     931012  nrv    Add in the constants when calculating slew times for
C                    type 7 (ALGO)
!   2008Jun20 JMG. Changed arg list for kcontn.
C
C
C     1. First we find the position of the telescope at the end of
C        the current observation and the position of the new source
C        at that time also. Then we go into
C        a loop which calculates the required telescope move, the
C        time to get there, and the source position at the end of
C        the move.  The loop is terminated when the slewing time
C        does not change by more than 30sec, or when 5 tries have
C        been made.
C
      CALL CVPOS(NSNOW,ISTN,MJD,UT,AZNOW,ELNOW,HANOW,DECNOW,
     .X30NOW,Y30NOW,X85NOW,Y85NOW,KUP)
C                    this calculates the current telescope position
      trise=-1.0
      TSLEWC = 0.0
      NLOOPS = 0
100   NLOOPS = NLOOPS + 1
      TSLEWP = TSLEWC
      LWR2P = LWR2
C     This calculates the new source location:
      CALL CVPOS(NSNEW,ISTN,MJD,UT+TSLEWC,
     .AZNEW,ELNEW,HANEW,DECNEW,X30NEW,Y30NEW,X85NEW,Y85NEW,KUP)
      if (aznew.gt.100.or.aznew.lt.-100.d0) then
        write(7,*) 'SLEWT: bad aznew ',aznew
        stop
      endif
C
      if (.not.kup) then !Check for source being up within lookahead
        CALL CVPOS(NSNEW,ISTN,MJD,UT+lookah,
     .  AZNEW,ELNEW,HANEW,DECNEW,X30NEW,Y30NEW,X85NEW,Y85NEW,KUP)
C       Check for the minute it rises
        if (kup) then !rising within lookahead
          kup=.false.
          il=0

C 040623  ZMM  changed, was
C         do while ( kup.eq..false. .and. il.le.lookah/60 )

          do while ( .not.kup .and. il.le.lookah/60 )
            il=il+1
            CALL CVPOS(NSNEW,ISTN,MJD,UT+il*60.,
     .      AZNEW,ELNEW,HANEW,DECNEW,X30NEW,Y30NEW,X85NEW,Y85NEW,KUP)
          enddo
        trise = il*60.0
C       Now we have the time to rising and position at rise.
C       Compute slewing time to this position.
        endif !rising within lookahead
      endif !check within an hour
      IF (.NOT.KUP) GOTO 980
C
      AZ1=AZNOW
      LWR1=LWRCUR
      AZ2=AZNEW
      LWR2=LWRNEW
      DELAZ = CABLW(ISTN,AZ1,LWR1,AZ2,LWR2)
C                   Function to compute az move including cable wrap
      DELEL = ABS(ELNEW-ELNOW)
      DELHA = ABS(HANEW-HANOW)
      DELDC = DABS(0.D0+DECNEW-DECNOW)
      DELX30 = ABS(X30NEW-X30NOW)
      DELX85 = ABS(X85NEW-X85NOW)
      DELY30 = ABS(Y30NEW-Y30NOW)
      DELY85 = ABS(Y85NEW-Y85NOW)
C
      IF (IAXIS(ISTN).EQ.1.OR.IAXIS(ISTN).EQ.5)
     .  TSLEWC = AMAX1(ISTCON(1,ISTN)+DELHA/STNRAT(1,ISTN),
     .  ISTCON(2,ISTN)+DELDC/STNRAT(2,ISTN))
      IF (IAXIS(ISTN).EQ.2)
     .  TSLEWC = AMAX1(ISTCON(1,ISTN)+DELX30/STNRAT(1,ISTN),
     .  ISTCON(2,ISTN)+DELY30/STNRAT(2,ISTN))
      IF (IAXIS(ISTN).EQ.3.or.IAXIS(ISTN).eq.6)
     .  TSLEWC = AMAX1(ISTCON(1,ISTN)+DELAZ/STNRAT(1,ISTN),
     .  ISTCON(2,ISTN)+DELEL/STNRAT(2,ISTN))
      IF (IAXIS(ISTN).EQ.7) then
        elrate = stnrat(2,istn)
C       The Algonquin antenna is faster going down.
C       if (elnew.lt.elnow) elrate=elrate*1.333
C       First compute the az/el slewing rate
        TSLEW1 = AMAX1(istcon(1,istn)+DELAZ/STNRAT(1,ISTN),
     .                 istcon(1,istn)+DELEL/elrate)
C       Compute the ha/dec slewing rate for the master equatorial
C       NOTE: Rates are hard-coded here because they are not available
C             in the normal antenna info.  Rates are 24 deg/min.
        rme = 24.d0*PI/(180.d0*60.d0)
        TSLEW2 = AMAX1(istcon(1,istn)+DELHA/rme,
     .                 istcon(1,istn)+DELDC/rme)
        tslewc=amax1(tslew1,tslew2)
      endif
      IF (IAXIS(ISTN).EQ.4)
     .  TSLEWc = AMAX1(ISTCON(1,ISTN)+DELX85/STNRAT(1,ISTN),
     .  ISTCON(2,ISTN)+DELY85/STNRAT(2,ISTN))
C
      IF ((ABS(TSLEWC-TSLEWP).LT.10).OR.(NLOOPS.GE.5)) GOTO 110
      GOTO 100
C     We get here if the slew has converged OR we iterated 5 times.
110   IF  (kcont(mjd,UT+TSLEWC,TSLEWP-TSLEWC,NSNEW,ISTN,LWRCUR))
     .  THEN  !continuity OK
        TSLEW = TSLEWC
        RSTCON(1) = FLOAT(ISTCON(1,ISTN))
        RSTCON(2) = FLOAT(ISTCON(2,ISTN))
        IF(TSLEW.LE.(AMAX1(RSTCON(1),RSTCON(2))+5.))
     .      TSLEW=0.0
        LWRNEW = LWR2
C       Final slewing time is the larger of
C       "time to rise" (trise) and "slew to risen position" (tslew
C       calculated using az,el at UT+trise).
        if (trise.gt.0..and.tslew.gt.0.) tslew = amax1(tslew,trise)
        RETURN
      ELSE  !
        TSLEW = -2.0
        trise=-1.0
        RETURN
      END IF  !
980   TSLEW = -1.0
      trise=-1.0

      END
