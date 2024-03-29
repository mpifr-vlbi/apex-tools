      subroutine hdchk(ichecks,lwho,indxtp)
C
      include '../include/fscom.i'
C 
C  INPUT: 
      integer ichecks(1)
      integer*2 lwho
      integer indxtp
C 
C  SUBROUTINES CALLED:
C 
C     LOGIT - to log and display the error
C 
C  LOCAL VARIABLES: 
C 
      dimension ip(5)             ! - for RMPAR
      dimension poffx(2),pnow(2)
      real*4 scale,volt           ! - for Head Position Read-out
      integer inerr
      integer rn_take
      character*2 dev(2)
C
C  INITIALIZED:
C
      data dev/2hh1,2hh2/
c
      ierr=rn_take('fsctl',0)
      call lvdonn('lock',ip,indxtp)
      if (ip(3).ne.0) then
         call logit7(0,0,0,0,ip(3),ip(4),ip(5))
         call logit7ic(0,0,0,0,-199,lwho,dev(indxtp))
        goto 1092
      endif
      call fs_get_ipashd(ipashd,indxtp)
      call fs_get_posnhd(posnhd,indxtp)
      CALL fs_get_drive(drive)
      call fs_get_drive_type(drive_type)
      do ihd=1,2
        if(kposhd_fs(ihd,indxtp)) then
          inerr = 0
          call vlt_head(ihd,volt,ip,indxtp)
          if (ip(3).ne.0) then
             call logit7(0,0,0,0,ip(3),ip(4),ip(5))
             call logit7ic(0,0,0,0,-198,lwho,dev(indxtp))
             goto 1091
          endif
          call vlt2mic(ihd,ipashd(ihd,indxtp),kautohd_fs(indxtp),volt,
     $         pnow(ihd),ip,indxtp)
          if (ip(3).ne.0) then
             call logit7(0,0,0,0,ip(3),ip(4),ip(5))
             call logit7ic(0,0,0,0,-197,lwho,dev(indxtp))
             goto 1091
          endif
          poffx(ihd) = pnow(ihd) - posnhd(ihd,indxtp)
          if(volt.lt.-0.010) then
            scale=rslope(ihd,indxtp)
          else if(volt.gt.0.010)then
            scale=pslope(ihd,indxtp)
          else
            scale=max(pslope(ihd,indxtp),rslope(ihd,indxtp))
          endif
          if(.not.(
     &         (drive(indxtp).eq.VLBA.and.drive_type(indxtp).eq.VLBA2)
     &         .or.
     &         (drive(indxtp).eq.VLBA4.and.drive_type(indxtp).eq.VLBA42)
     &         ))then
             if (abs(poffx(ihd)).gt.
     $            ((ilvtl_fs(indxtp)+2)*0.0049+0.0026)*scale)
     &            inerr = inerr+1
          else
             if(abs(poffx(ihd)).gt.ilvtl_fs(indxtp)) inerr=inerr+1
          endif
          call fs_get_icheck(icheck(20+indxtp-1),20+indxtp-1)
          if(icheck(20+indxtp-1).gt.0.and.
     $         ichecks(20+indxtp-1).eq.icheck(20+indxtp-1)) then
            if (inerr.ge.1)
     $            call logit7ic(0,0,0,0,-350-ihd,lwho,dev(indxtp))
          endif
        endif
      enddo
C
C  Turn off LVDT Oscillator
C
1091  continue
      call lvdofn('unlock',ip,indxtp)
      call rn_put('fsctl')
      if (ip(3).lt.0) then
         call logit7(0,0,0,0,ip(3),ip(4),ip(5))
         call logit7ic(0,0,0,0,-196,lwho,dev(indxtp))
      endif
      return
C
 1092 continue
      call lvdofn('unlock',ip,indxtp)
      call rn_put('fsctl')
      return
      end
