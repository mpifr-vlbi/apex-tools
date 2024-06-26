      subroutine lxopn
C
C LXOPN - Opens the current or specified log file & reads the station
C         name.
C
C MODIFICATIONS:
C
C    DATE     WHO  DESCRIPTION
C    820324   KNM  SUBROUTINE CREATED
C    820818   KNM  LOG AND STATION NAME ARE WRITTEN OUT BY CALLING
C                  LXWRT.
C
C RESTRICTIONS: Designed to open the Field System logs only.
C
C COMMON BLOCKS USED:
C
      include 'lxcom.i'
C
C SUBROUTINE INTERFACES:
C    CALLING SUBROUTINES:
C      LOGEX - Main program
C      LXPRC - Handles the processing of the static commands.
C
C    CALLED SUBROUTINES:
C      LXWRT - Writes out LOGEX data.
C      File manager package routines
C      LNFCH utilities
 
C LOCAL VARIABLES:
C
C
      integer answer, nchar, trimlen
      character*79 outbuf
      integer fmpread, ilen
      character*64 pathname
      character*79 logfc
      integer*2 logf(40)
      equivalence (logf(2),logfc)
      integer star(2)
      character*12 ibc1,ibc2
      character cjchar
      data star/1,'*'/
C
C  *****************************************************************
C
C  Let's open the current or specified log.  If we have a successful
C  open, then read and write out the log name and station name.
C
C  *****************************************************************
C
C
      ibc1=' '
      ibc2=' '
      ic1=iscn_ch(logna,1,20,'/')
      ic2=iscn_ch(logna,ic1+1,20,'/')
      if(ic2.gt.0) then
        il = ic2-ic1+1
        ibc1 = lognc(ic1:ic2)
      else
        ibc1 = '/usr2/log/'
        il = 10
      endif
C
      ic1 = iscn_ch(logna,1,20,'.')
      icl = iflch(logna,20)
      if(ic1.gt.0) ibc2 = lognc(ic1:icl)
C
C if the field system is running, the variable lognc will have
C the current log file name in it, less the extension. If the
C filename is shorter than 8 characters, null characters are the
C suffix.
C
      if(ic1.eq.0) then
        ic1=icl+1
        ibc2='.log'
      endif
C
      pathname=ibc1(1:il)//lognc(1:ic1-1)//ibc2
C
      do i=1,28
       if((ichar(pathname(i:i)).gt.64).and.(ichar(pathname(i:i)).lt.91))
     .    pathname(i:i)=char(ichar(pathname(i:i))+32)
      end do
C
      ierr=0
      call fmpopen(idcb,pathname,ierr,'r',5)
      nrec = 0
      iout=0
C
      if (ierr.lt.0) then
        outbuf='LXOPN - error '
        call ib2as(ierr,answer,1,4)
        call hol2char(answer,1,4,outbuf(15:))
        nchar = trimlen(outbuf) + 1
        outbuf(nchar:)=' opening log file '
        nchar = trimlen(outbuf) + 2
        call hol2char(logna,1,8,outbuf(nchar:))
        call po_put_c(outbuf)
        icode=-1
        goto 200
      end if
C
      ilen = fmpread(idcb,ierr,ibuf,iblen*2)
C
      if (ierr.lt.0) then
        outbuf='LXOPN - error '
        call ib2as(ierr,answer,1,4)
        call hol2char(answer,1,4,outbuf(15:))
        nchar = trimlen(outbuf) + 1
        outbuf(nchar:)=' reading log file '
        nchar = trimlen(outbuf) + 1
        call hol2char(logna,1,4,outbuf(nchar:))
        call po_put_c(outbuf)
        icode=-1
        goto 200
      end if
C
      do i=1,15
         if(index("0123456789",cjchar(ibuf,i)).eq.0) then
            if(i.eq.5) then
               logformat=3
            else if(i.eq.10) then
               logformat=1
C not Y10K compliant
            else if(i.eq.14) then
               logformat=2
            else
               logformat=-1
            endif
            goto 110
         endif
      enddo
      logformat=-1
c
 110  continue
      if(logformat.eq.-1) then
        outbuf='LXOPN - error '
        call ib2as(-999,answer,1,4)
        call hol2char(answer,1,4,outbuf(15:))
        nchar = trimlen(outbuf) + 1
        outbuf(nchar:)=' unknown log file format '
        nchar = trimlen(outbuf) + 1
        call hol2char(logna,1,4,outbuf(nchar:))
        call po_put_c(outbuf)
        icode=-1
        goto 200
      endif
c         
      ilen = fmpread(idcb,ierr,ibuf,iblen*2)
C
      if (ierr.lt.0) then
        outbuf='LXOPN - error '
        call ib2as(ierr,answer,1,4)
        call hol2char(answer,1,4,outbuf(15:))
        nchar = trimlen(outbuf) + 1
        outbuf(nchar:)=' reading log file '
        nchar = trimlen(outbuf) + 1
        call hol2char(logna,1,4,outbuf(nchar:))
        call po_put_c(outbuf)
        icode=-1
        goto 200
      end if
      if(logformat.eq.1) then
         ifc=10
         ilen = fmpread(idcb,ierr,ibuf,iblen*2)
C
         if (ierr.lt.0) then
            outbuf='LXOPN - error '
            call ib2as(ierr,answer,1,4)
            call hol2char(answer,1,4,outbuf(15:))
            nchar = trimlen(outbuf) + 1
            outbuf(nchar:)=' reading log file '
            nchar = trimlen(outbuf) + 1
            call hol2char(logna,1,4,outbuf(nchar:))
            call po_put_c(outbuf)
            icode=-1
            goto 200
         end if
      else if(logformat.eq.2) then
         ifc=15
      else if(logformat.eq.3) then
C not Y10K compliant
         ifc=21
      endif
C
      ic1=iscn_ch(ibuf,ifc,ilen,',')+1
      ic2=iscn_ch(ibuf,ic1,ilen,',')-1
C
      call ichmv_ch(lstatn,1,'        ')
C
      if (ic1.ne.1) call ichmv(lstatn,1,ibuf,ic1,min0(8,ic2-ic1+1))
      ierr=0
      call fmprewind(idcb,ierr)
      if (ierr.ne.0) then
        outbuf='LXOPN - error '
        call ib2as(ierr,answer,1,4)
        call hol2char(answer,1,4,outbuf(15:))
        nchar = trimlen(outbuf) + 1
        outbuf(nchar:)=' rewinding log file '
        nchar = trimlen(outbuf) + 1
        call hol2char(logna,1,4,outbuf(nchar:))
        call po_put_c(outbuf)
        icode=-1
        goto 200
      endif
      nchar = trimlen(pathname)
      logfc='log file ' // pathname(:nchar) // ' for station '
      nchar = trimlen(logfc) + 2
      call hol2char(lstatn,1,8,logfc(nchar:))
      idum = trimlen(logfc)
      call lxwrt(logf(2),idum)
      if (nintv.eq.0.or.iterm.eq.1) goto 200
        call lxwrt(star(2),1)
C
200   continue
      return
      end
















