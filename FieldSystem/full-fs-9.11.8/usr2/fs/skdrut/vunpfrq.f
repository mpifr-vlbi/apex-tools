      SUBROUTINE vunpfrq(modef,stdef,ivexnum,iret,ierr,lu,
     .bitden,srate,csg,frf,csb,cchref,vbw,csw,cbbref,
     .cpcalref,nchandefs)
C
C     VUNPFRQ gets the channel def statements 
C     for station STDEF and mode MODEF and converts it.
C     It also gets bit density and sample rate.
C     All statements are gotten and checked before returning.
C     Any invalid values are not loaded into the returned
C     parameters.
C     Only generic error messages are written. The calling
C     routine should list the station name for clarity.
C
      include '../skdrincl/skparm.ftni'
C
C  History:
C 960520 nrv New.
C 960607 nrv Initialize band ID to '-', not blank.
C 970114 nrv Remove polarization, shift up all subsequent field numbers.
C            (pol was not read or stored anyway)
C 970124 nrv Move initialization to start.
C 971208 nrv Add phase cal ref.
C
C  INPUT:
      character*128 stdef ! station def to get
      character*128 modef ! mode def to get
      integer ivexnum ! vex file ref
      integer lu ! unit for writing error messages
C
C  OUTPUT:
      integer iret ! error return from vex routines, !=0 is error
      integer ierr ! error from this routine, >0 indicates the
C                    statement to which the VEX error refers,
C                    <0 indicates invalid value for a field
      double precision srate ! sample rate
      double precision bitden ! bit density
      character*2 csg(max_chan) ! subgroup
      double precision frf(max_chan) ! RF frequency
      character*2 csb(max_chan) ! net SB
      character*6 cchref(max_chan) ! channel ID
      character*6 cbbref(max_chan) ! BBC ref 
      character*6 cpcalref(max_chan) ! pcal ref 
      double precision vbw(max_chan) ! video bandwidth
      character*3 csw(max_chan) ! switching
      integer nchandefs ! number of channel defs found
C
C  LOCAL:
      character*128 cout,cunit
      double precision d
      character upper
      integer j,ic,nch
      integer fvex_double,fvex_len,fvex_int,fvex_field,
     .fvex_units,ptr_ch,fget_all_lowl
C
C  Initialize.
C
      nchandefs=0
      do ic=1,max_chan
        csg(ic)="-"
        frf(ic)=0.d0
        csb(ic)=" "
        cchref(ic)=''
        vbw(ic)=0.d0
        csw(ic)='   ' ! initialize to blank
        cbbref(ic)=''
      enddo
      bitden=0.d0
      srate=0.d0
C  1. Channel def statements
C
      ierr = 1
      iret = fget_all_lowl(ptr_ch(stdef),ptr_ch(modef),
     >  ptr_ch('chan_def'//char(0)),ptr_ch('FREQ'//char(0)),ivexnum)
      ic=0
      do while (ic.lt.max_chan.and.iret.eq.0) ! get all fanout defs
        ic=ic+1

C  1.1 Subgroup

        ierr = 11
        iret = fvex_field(1,ptr_ch(cout),len(cout)) ! get subgroup
        
        if (iret.ne.0) return
        NCH = fvex_len(cout)       
        if (nch.gt.1) then
          ierr = -1
          write(lu,'("VUNPFRQ02 - Band ID must be 1 character.")')
        else if (nch.eq.1) then
          csg(ic)=cout(1:1)
        endif
C
C  1.2 Polarization -- skip this for now
C  Removed in Vex 1.5. Shift up all field numbers.

C  1.2 RF frequency

        ierr = 12
        iret = fvex_field(2,ptr_ch(cout),len(cout)) ! get frequency
     
        if (iret.ne.0) return
        iret = fvex_units(ptr_ch(cunit),len(cunit))
        if (iret.ne.0) return
        iret = fvex_double(ptr_ch(cout),ptr_ch(cunit),d)
        IF  (d.lt.0.d0) then
          write(lu,'("VUNPFRQ03 - Invalid RF frequency < 0")')
          ierr=-2
        else
          frf(ic) = d/1.d6
        ENDIF 

C  1.3 Net SB

        ierr = 13
        iret = fvex_field(3,ptr_ch(cout),len(cout)) ! get sideband  
        if (iret.ne.0) return
        cout(1:1) = upper(cout(1:1))
        if (cout(1:1).ne.'U'.and.cout(1:1).ne.'L') then
          ierr = -3
          write(lu,'("VUNPFRQ04 - Invalid sideband field.")')
        else
          csb(ic)=cout(1:1)
        endif

C  1.4 Bandwidth

        ierr = 14
        iret = fvex_field(4,ptr_ch(cout),len(cout)) ! get bandwidth
        if (iret.ne.0) return
        iret = fvex_units(ptr_ch(cunit),len(cunit))
        if (iret.ne.0) return
        iret = fvex_double(ptr_ch(cout),ptr_ch(cunit),d) ! convert to binary
        if (iret.ne.0.or.d.lt.0.d0) then
          ierr = -4
          write(lu,'("VUNPFRQ05 - Invalid  bandwidth")')
        else
          vbw(ic) = d/1.d6
        endif

C  1.5 Channel ID

        ierr = 15
        iret = fvex_field(5,ptr_ch(cout),len(cout)) ! get channel ID
        if (iret.ne.0) return
        nch = fvex_len(cout)
        if (nch.gt.len(cchref(ic)).or.nch.le.0) then
          ierr=-5
          write(lu,'("VUNPFRQ06 - Channel ID too long")')
        else
          cchref(ic)=cout(1:nch)
        endif

C  1.6 BBC ref

        ierr = 16
        iret = fvex_field(6,ptr_ch(cout),len(cout)) ! get BBC ref
        if (iret.ne.0) return
        nch = fvex_len(cout)
        if (nch.gt.len(cbbref(ic)).or.nch.le.0) then
          ierr=-6
          write(lu,'("VUNPFRQ07 - BBC ref too long")')
        else
          cbbref(ic)=cout(1:nch)
        endif

C  1.7 Phase cal 

        ierr = 17
        iret = fvex_field(7,ptr_ch(cout),len(cout)) ! get pcal ref
        if (iret.eq.0) then ! some phase cal ref
          nch = fvex_len(cout)
          if (nch.gt.len(cpcalref(ic)).or.nch.le.0) then
            ierr=-7
            write(lu,'("VUNPFRQ08 - Pcal ref too long")')
          else
            cpcalref(ic)=cout(1:nch)
          endif
        endif ! some phase cal ref

C  1.8 Switching

        ierr = 18
        iret = fvex_field(8,ptr_ch(cout),len(cout)) ! get switch
        if (iret.eq.0) then ! some switching, 1st switch
          iret = fvex_int(ptr_ch(cout),j)
          if (iret.ne.0.or.j.ne.1.or.j.ne.2) then
            ierr=-8
            write(lu,'("VUNPFRQ09 - Switching cycle must be 0,1,2")')
          else
            csw(ic)(1:1)=cout(1:1)
          endif
          iret = fvex_field(10,ptr_ch(cout),len(cout)) ! get 2nd switch
          if (iret.eq.0) then ! second cycle
            iret = fvex_int(ptr_ch(cout),j)
            if (iret.ne.0.or.j.ne.1.or.j.ne.2) then
              ierr=-8
              write(lu,'("VUNPFRQ10 - Switching cycle must be 0,1,2")')
            else
              csw(ic)(2:2)=','
              csw(ic)(3:3)=cout(1:1)
            endif
          endif ! second cycle
          iret = fvex_field(11,ptr_ch(cout),len(cout)) ! get switch
          if (iret.eq.0)
     .    write(lu,'("VUNPFRQ11 - Too many switching cycles, ",
     .    "2 is max")')
        endif ! some switching

C       Get next channel def statement
        iret = fget_all_lowl(ptr_ch(stdef),ptr_ch(modef),
     .  ptr_ch('chan_def'//char(0)),
     .  ptr_ch('FREQ'//char(0)),0)
      enddo ! get all channel defs
      nchandefs = ic

C 2. Bit density

        ierr = 2
        iret = fget_all_lowl(ptr_ch(stdef),ptr_ch(modef),
     .  ptr_ch('record_density'//char(0)),
     .  ptr_ch('DAS'//char(0)),ivexnum)
        if (iret.eq.0) then
          iret = fvex_field(1,ptr_ch(cout),len(cout)) ! get number
          if (iret.ne.0) return
          iret = fvex_units(ptr_ch(cunit),len(cunit))
          if (iret.ne.0) return
          iret = fvex_double(ptr_ch(cout),ptr_ch(cunit),d)
          if (iret.ne.0.or.d.lt.0.d0) then
            ierr=-9
            write(lu,'("VUNPFRQ12 - Invalid bit density")')
          else
            bitden = d
          endif
        endif

C 3. Sample rate

        ierr = 3
        iret = fget_all_lowl(ptr_ch(stdef),ptr_ch(modef),
     .  ptr_ch('sample_rate'//char(0)),
     .  ptr_ch('FREQ'//char(0)),ivexnum)
        if (iret.eq.0) then
          iret = fvex_field(1,ptr_ch(cout),len(cout)) ! get number
          if (iret.ne.0) return
          iret = fvex_units(ptr_ch(cunit),len(cunit))
          if (iret.ne.0) return
          iret = fvex_double(ptr_ch(cout),ptr_ch(cunit),d)
          if (iret.ne.0.or.d.lt.0.d0) then
            ierr=-10
            write(lu,'("VUNPFRQ13 - Invalid sample rate")')
          else
            srate = d/1.d6
          endif
        endif

      if (ierr.gt.0) ierr=0
      return
      end
