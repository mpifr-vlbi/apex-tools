      subroutine snap_setup(ipas,istnsk,icod,iobs,kerr)
! include files.
      include 'hardware.ftni'           !This contains info only about the recorders.
      include 'drcom.ftni'
      include '../skdrincl/statn.ftni'
      include '../skdrincl/sourc.ftni'
      include '../skdrincl/freqs.ftni'
      include '../skdrincl/skobs.ftni'
! History
!  2006Nov30. JMG. Code type is 1 for no recorder.
!  2007Jul27  JMG  Made Mark5 no recorder.
!  2012Oct09  JMG. If no recorder don't issue pass number etc. 
! 2015Jun05 JMG. Repalced squeezewrite by drudg_write. 
! passed variables
      integer istnsk    !index #.
      integer ipas(*)   !Pass number
      integer icod
      integer iobs
      logical kerr

! local variables.
      integer ndx
      character*12 cnamep

      character*80 ldum

! start of code

      if(knopass) then
        continue
      else ! mnemonic proc names
        if (ipas(istnsk).le.0) then ! invalid pass
          write(luscn,9912) ipas(istnsk),icod
          return
        endif ! invalid pass
        ndx = ihddir(1,ipas(istnsk),istn,icod) 	! subpass
        if (ndx.le.0) then 			! invalid head position
          write(luscn,9912) ipas(istnsk),icod,iobs
9912      format(/'SNAP_SETUP - Illegal head position or pass',
     .    ' for pass ',i3,' in mode ',i2, ' scan ',i3)
          return
        endif
      endif

      call setup_name(icod,cnamep)     

C     Don't use the pass number for Mk5-only OR for no recorder.
      if(km5disk .or. knorec(1)) then 
         write(lufile,"(a)") cnamep
      else
         write(ldum,"(a,'=',i3)") cnamep,ipas(istnsk)
         call drudg_write(lufile,ldum)
      endif
      return
      end
