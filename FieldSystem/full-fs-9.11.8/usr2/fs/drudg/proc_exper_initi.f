      subroutine proc_exper_initi(lufil,luscn,kin2net_on)
      include 'hardware.ftni'
! passed
      integer lufil,luscn
      logical kin2net_on

! functions

! local
      character*12 lname
! History
! 2007May28 JMGipson.  Modified to add Mark5B support.
! 2014Dec06 JMG. Added Mark5C support    
! 2015Jun05 JMG.  A.) Don't output 'mk5=ss_rev?';  B.) Lowercase all output text.         


      lname="exper_initi"

      call proc_write_define(luFil,luscn,lname)
      write(luFile,'(a)') "proc_library"
      write(luFile,'(a)') "sched_initi"

      if(kin2net_on .and. (km5A .or. km5a_piggy .or. km5B)) then
         write(lufile,'("mk5=net_protocol=tcp:4194304:2097152;")')
      endif     

      if(km5A .or. km5A_piggy) then
        write(lufile,'(a)')   "mk5=dts_id?"
        write(lufile,'(a)')   "mk5=os_rev1?"
        write(lufile,'(a)')   "mk5=os_rev2?"
        write(lufile,'(a)')   "mk5=ss_rev1?"
        write(lufile,'(a)')   "mk5=ss_rev2?"
        write(lufile,'(a)')   "mk5=status?"
      else if(km5B .or. Km5C) then
        write(lufile,'(a)')   "mk5=dts_id?"
        write(lufile,'(a)')   "mk5=os_rev?"
        if(kflexbuff) then 
! Moved to local_shed_initi....
!          write(lufile,'("jive5ab=version?")') 
        else
          write(lufile,'(a)')   "mk5=ss_rev?"
        endif 
        write(lufile,'(a)')   "mk5=status?"
      endif
      if(kdbbc_rack)     write(lufile,'("dbbc=version")') 
      if(kfila10g_rack)  write(lufile,'("fila10g=version")')     

      write(lufile,'(a)') "enddef"

      return
      end

