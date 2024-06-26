      SUBROUTINE vunpant(stdef,ivexnum,iret,ierr,lu,
     > cNAANT,cAXIS,AXISOF,SLRATE,ANLIM1,ANLIM2,DIAMAN,ISLCON)
C
C     VUNPANT gets the antenna information for station
C     STDEF and converts it.
C     All statements are gotten and checked before returning.
C     Any invalid values are not loaded into the returned
C     parameters.
C     Only generic error messages are written. The calling
C     routine should list the station name for clarity.
C
      include '../skdrincl/skparm.ftni'
      include '../skdrincl/constants.ftni'
C
C  History:
C 960516 nrv New.
C 970116 nrv Change "ant_motion" to "antenna_motion" for Vex 1.5
C 970123 nrv Move initialization to front.
C 970303 nrv When matching up the axis for antenna_motion, change
C            "dec" to "dc" to keep it to the 2 letters sk/dr use.
C 970306 nrv Handle XY axis types.
C 970718 nrv Incorrect error returns for axis limits fixed.
C 970930 nrv For XY axis types check the matching axis names as they
C            appear in the VEX file, not as sk/dr keeps them.
C 990630 nrv Axis offset has only 1 parameter. This is an error in 
C            the VEX parameter tables, but correct in VEX example
C            document.
C 990913 nrv Change spelling from "ant_diam" to "antenna_diam"
C 011107 nrv Set antenna limits to wide values if none are found.
C
C  INPUT:
      character*128 stdef ! station def to get
      integer ivexnum ! vex file ref
      integer lu ! unit for writing error messages
C
C  OUTPUT:
      integer iret ! error return from vex routines, !=0 is error
      integer ierr ! error from this routine, >0 indicates the
C                    statement to which the VEX error refers,
C                    <0 indicates invalid value for a field
      character*8 cNAANT   ! name of the antenna
      character*4 cAXIS    ! axis type
      integer islcon(2)    ! slewing constant
      double precision AXISOF ! axis offset, meters
      real SLRATE(2),ANLIM1(2),ANLIM2(2),diaman
C            - antenna slew rates for axis 1 and 2, degrees/minute
C            - antenna upper,lower limits for axis 1, degrees
C            - antenna upper,lower limits for axis 2, degrees
C     DIAMAN - diameter of antenna, in m
C
C  LOCAL:
      character*128 cout,cout2,cunit
      character*2 cax,cax1,cax2
      double precision R
      real almin1,almax1,almin2,almax2
      character*4 cdum
      integer i,i1,i2,iv,nch1,nch2,nl,nch,iax
      character upper
      integer fvex_double,fvex_units,fvex_len,fget_station_lowl,
     .ptr_ch,fvex_field
C
C
C  Initialize at start in case we have to leave early.

      cnaant=" "
      caxis=" "
      axisof = 0.d0
      slrate(1)=0.0
      slrate(2)=0.0
      islcon(1)=0
      islcon(2)=0
      anlim1(1)=+999.9*deg2rad
      anlim1(2)=-999.9*deg2rad
      anlim2(1)=+999.9*deg2rad
      anlim2(2)=-999.9*deg2rad
      diaman=0.0

C  1. The antenna name.
C
      ierr = 1
      iret = fget_station_lowl(ptr_ch(stdef),
     .ptr_ch('ant_name'//char(0)),
     .ptr_ch('ANTENNA'//char(0)),ivexnum)
      if (iret.eq.0) then 
        iret = fvex_field(1,ptr_ch(cout),len(cout))
        NCH = fvex_len(cout)
        IF  (NCH.GT.8.or.NCH.le.0) THEN  !
          write(lu,'("VUNPANT01 - Antenna name too long")')
          iret=-1
        else
          cnaant=cout(1:nch)
        ENDIF 
      endif
C
C  2. Axis type. Get two fields.
C
      ierr = 2
      iret = fget_station_lowl(ptr_ch(stdef),
     .ptr_ch('axis_type'//char(0)),
     .ptr_ch('ANTENNA'//char(0)),ivexnum)
      if (iret.ne.0) return
      iret = fvex_field(1,ptr_ch(cout),len(cout))
      if (iret.ne.0) return
      NCH1 = fvex_len(cout)
      iret = fvex_field(2,ptr_ch(cout2),len(cout2))
      if (iret.ne.0) return
      NCH2 = fvex_len(cout2)
      cdum=cout(1:2)//cout2(1:2)
      if (cout2(1:3).eq.'dec') then ! dec -> dc
        cdum(3:4)="dc"
      elseif (cout2(1:3).eq.'yns') then ! XYNS
        cdum="xyns"
      else if(cout2(1:3).eq.'yew') then ! XYEW
        cdum="xyew"
      endif
      call capitalize(cdum) 
      call axtyp(cdum,iax,1) ! check for legal sked axis type
      if (iax.eq.0) then
        ierr=-2
        write(lu,'("VUNPANT02 - Axis type not recognized: ",a,":",a)') 
     .  cout(1:nch1),cout2(1:nch2)
      else
        caxis=cdum
        cax1(1:2)=upper(cout(1:2))
        cax2(1:2)=upper(cout2(1:2))
      endif
C
C  3. Axis offset.

      ierr = 3
      iret = fget_station_lowl(ptr_ch(stdef),
     .ptr_ch('axis_offset'//char(0)),
     .ptr_ch('ANTENNA'//char(0)),ivexnum)
      if (iret.eq.0) then ! got one
C       iret = fvex_field(1,ptr_ch(cout),len(cout)) ! get axis type
C       if (cout(1:2).ne.'el') then
C         ierr=-3
C         write(lu,'("VUNPANT03 - Only elevation axis offsets.")')
C       else
          iret = fvex_field(1,ptr_ch(cout),len(cout)) ! get offset value
          if (iret.ne.0) return
          iret = fvex_units(ptr_ch(cunit),len(cunit))  ! offset units
          if (iret.ne.0) return
          iret = fvex_double(ptr_ch(cout),ptr_ch(cunit),r) ! convert to binary
          IF  (Iret.ne.0) THEN
            Ierr = -3
            write(lu,'("VUNPANT04 - Invalid axis offset value.")')
          else
            AXISOF = R
          endif
C       endif
      endif

C  4. Slewing rates and constants.

      ierr = 4
      do i=1,2 ! two slewing rates
        iv=0
        if (i.eq.1) iv=ivexnum
        iret = fget_station_lowl(ptr_ch(stdef),
     .  ptr_ch('antenna_motion'//char(0)),
     .  ptr_ch('ANTENNA'//char(0)),iv)
        if (iret.ne.0) return
        iret = fvex_field(1,ptr_ch(cout),len(cout)) ! get axis
        if (iret.ne.0) return
        cax=cout(1:2)
        if (cax.eq.'de') then ! kludg for dec -> dc
          cax='dc'
        endif
C       Match up the axis for the motion with the stored axis type
C       Compare the first letter only.
        cax(1:1)=upper(cax(1:1))
        cax(2:2)=upper(cax(2:2))
C       i1=1
C       if (caxis(1:3) .eq. cax) i1=2
        i1=0
        if (cax(1:1).eq.cax1(1:1)) i1=1
        if (cax(1:1).eq.cax2(1:1)) i1=2
        if (i1.eq.0) then
          ierr=-4
          write(lu,
     >     '("VUNPANT05 - Unmatched axis types ",a," and ",a," or ",a)')
     >      cax,cax1,cax2
        else
        iret = fvex_field(2,ptr_ch(cout),len(cout)) ! get slewing rate
        if (iret.ne.0) return
        iret = fvex_units(ptr_ch(cunit),len(cunit)) ! slewing rate units
        if (iret.ne.0) return
        iret = fvex_double(ptr_ch(cout),ptr_ch(cunit),r) 
        if (iret.ne.0) then
          ierr=-4
          write(lu,
     >     '("VUNPANT06 - Invalid first axis constant ", i2)') i
        else
          SLRATE(i1) = R
        endif
        endif
        iret = fvex_field(3,ptr_ch(cout),len(cout)) ! get slewing constant
        if (iret.ne.0) return
        iret = fvex_units(ptr_ch(cunit),len(cunit)) ! slewing constant units
        if (iret.ne.0) return
        iret = fvex_double(ptr_ch(cout),ptr_ch(cunit),r)
        if (ierr.lt.0) then
          iret=-7
          write(lu,
     >     '("VUNPANT06 - Invalid first axis constant ", i2)') i
        else
          ISLCON(i1) = R
        endif
      enddo ! two slewing rates

C  5. Antenna limits
C   cout field#    1   2   3       4      5   6      7      
C       sample   &ccw:az:-90 deg: 90 deg:el: 0 deg: 88 deg;
C       sample   &n  :az: 90 deg:270 deg:el: 0 deg: 88 deg;
C       sample   &cw :az:270 deg:450 deg:el: 0 deg: 88 deg;

      nl=0 ! count number of pointing sector lines
      i1=0
      i2=0
      ierr = 5
      iret = fget_station_lowl(ptr_ch(stdef),
     .ptr_ch('pointing_sector'//char(0)),
     .ptr_ch('ANTENNA'//char(0)),ivexnum)
      do while (iret.eq.0)
        nl=nl+1 ! increment number of lines found
        iret = fvex_field(2,ptr_ch(cout),len(cout)) ! get axis type
        if (iret.ne.0) return
        cax=cout(1:2)
        cax(1:1)=upper(cax(1:1))
        if (caxis(1:1) .eq. cax(1:1)) then ! first axis is first 
          i1=3
          i2=6
        else ! second axis is first
          i1=6
          i2=3
        endif
        iret = fvex_field(i1,ptr_ch(cout),len(cout)) ! get lower limit first axis
        if (iret.ne.0) return
        iret = fvex_units(ptr_ch(cunit),len(cunit))
        if (iret.ne.0) return
        iret = fvex_double(ptr_ch(cout),ptr_ch(cunit),r) 
        almin1 = r
        if (iret.ne.0) then
          ierr=-5
          write(lu,9909) nl,i1
9909      format("VUNPANT09 - Invalid antenna limit on line",
     .    i3,", field ",i3)
        endif
        iret = fvex_field(i1+1,ptr_ch(cout),len(cout)) ! get upper limit first axis
        if (iret.ne.0) return
        iret = fvex_units(ptr_ch(cunit),len(cunit))
        if (iret.ne.0) return
        iret = fvex_double(ptr_ch(cout),ptr_ch(cunit),r) 
        almax1 = r
        if (iret.ne.0) then
          ierr=-5
          write(lu,9909) nl,i1
        endif
        iret = fvex_field(i2,ptr_ch(cout),len(cout)) ! get lower limit second axis
        if (iret.ne.0) return
        iret = fvex_units(ptr_ch(cunit),len(cunit))
        if (iret.ne.0) return
        iret = fvex_double(ptr_ch(cout),ptr_ch(cunit),r) 
        almin2 = r
        if (iret.ne.0) then
          ierr=-5
          write(lu,9909) nl,i1
        endif
        iret = fvex_field(i2+1,ptr_ch(cout),len(cout)) ! get lower limit second axis
        if (iret.ne.0) return
        iret = fvex_units(ptr_ch(cunit),len(cunit))
        if (iret.ne.0) return
        iret = fvex_double(ptr_ch(cout),ptr_ch(cunit),r)
        almax2 = r 
        if (iret.ne.0) then
          ierr=-5
          write(lu,9909) nl,i1
        endif
C         Select the max and min of all sectors for now.
        if (almin1.lt.anlim1(1)) anlim1(1)=almin1
        if (almax1.gt.anlim1(2)) anlim1(2)=almax1
        if (almin2.lt.anlim2(1)) anlim2(1)=almin2
        if (almax2.gt.anlim2(2)) anlim2(2)=almax2
        iv=0
        iret = fget_station_lowl(ptr_ch(stdef),
     .  ptr_ch('pointing_sector'//char(0)),
     .  ptr_ch('ANTENNA'//char(0)),iv)
      enddo
      if (nl.eq.0) then ! no sectors found
        anlim1(1)=-999.9*deg2rad
        anlim1(2)=+999.9*deg2rad
        anlim2(1)=-999.9*deg2rad
        anlim2(2)=+999.9*deg2rad
      endif ! no sectors found

C  6. Antenna diameter

      ierr = 6
      iret = fget_station_lowl(ptr_ch(stdef),
     .ptr_ch('antenna_diam'//char(0)),
     .ptr_ch('ANTENNA'//char(0)),ivexnum)
      if (iret.eq.0) then ! got one
        iret = fvex_field(1,ptr_ch(cout),len(cout)) ! get diameter
        if (iret.ne.0) return
        iret = fvex_units(ptr_ch(cunit),len(cunit))
        if (iret.ne.0) return
        iret = fvex_double(ptr_ch(cout),ptr_ch(cunit),r) 
        IF  (IERR.LT.0) THEN
          Ierr = -6
          write(lu,'("VUNPANT04 - Invalid antenna diameter value.")')
        else
          diaman = R
        endif
      endif

      if (ierr.gt.0) then
        ierr=0
        iret=0
      endif
      return
      end
