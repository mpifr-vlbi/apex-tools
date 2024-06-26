      subroutine lvdonn(lock,ip,indxtp)

      character*(*) lock
      integer ip(5)
C
C  LVDONN: turn LVDT on & wait to stablize
C
C  INPUT:
C     LOCK: if "LOCK" then lock the LVDT RN before accessing
C           if "UNLOCK" then unlock the LVDT RN after accessing
C
C  OUTPUT:
C     IP - Field System Return Parameters
C     IP(3) - 0 if no error
C
      include '../include/fscom.i'
C
      integer*2 ibuf(6)
      integer iclass, nrec, rn_take
C
      call fs_get_drive(drive)
      call fs_get_drive_type(drive_type)
      if((drive(indxtp).eq.VLBA.and.drive_type(indxtp).eq.VLBA2).or.
     &     (drive(indxtp).eq.VLBA4.and.drive_type(indxtp).eq.VLBA42)
     &     ) then
        ip(3)=0
        return
      endif
c
      if(lock.eq.'lock') then
         if(indxtp.eq.1) then
            istat = rn_take('lvdt1',0)
         else if(indxtp.eq.2) then
            istat = rn_take('lvdt2',0)
         endif
         if (istat.eq.1) then
            ip(3)=-408
            call char2hol('q@',ip(4),1,2)
            return
         endif
      endif
C     
      if(VLBA.eq.drive(indxtp).or.VLBA4.eq.drive(indxtp)) then
        call fc_lvdonn_v(0,ip,indxtp)
      else
c
        nrec=0
        iclass=0
C
        ibuf(1)=0
        if(indxtp.eq.1) then
           call char2hol('h1',ibuf(2),1,2)
        else
           call char2hol('h2',ibuf(2),1,2)
        endif
        call ad2ma(ibuf(3),0,0,1)
        call add_class(ibuf,-12,iclass,nrec)
C
C   SEND STROBE
C
        ibuf(1)=5
        call char2hol('% ',ibuf(2),1,2)
        call add_class(ibuf,-3,iclass,nrec)
        klvdt_fs(indxtp)=.true.
        call fs_set_klvdt_fs(klvdt_fs,indxtp)
C
C   Now schedule MATCN
C
        call run_matcn(iclass,nrec)
        call rmpar(ip)
        call clrcl(ip(1))
        ip(2)=0
      endif
C
C  WAIT 0.05 SECONDS TO ALLOW the switch to settly and VLBA REC to resample
C
      if(ip(3).eq.0) then
        call susp(1,5)
      endif
C
      if(lock.eq.'unlock'.or.ip(3).ne.0) then
         if(indxtp.eq.1) then
            call rn_put('lvdt1')
         else if(indxtp.eq.2) then
            call rn_put('lvdt2')
         endif
      endif
C
      return
      end
