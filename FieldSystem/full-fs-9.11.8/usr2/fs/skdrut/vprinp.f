      SUBROUTINE VPRINP(ivexnum,lu,ierr)
C
C     This routine gets the scheduling parameter literal block.
C NOT USED -- see sked/prread.f and drudg drprrd.f
C
      include '../skdrincl/skparm.ftni'
      include '../skdrincl/statn.ftni'
      include '../skdrincl/skobs.ftni'
C
C History:
C 020619 nrv New.
C
C INPUT:
      integer ivexnum ! vex file number 
      integer lu ! unit for writing error messages
C
C OUTPUT:
      integer ierr ! error number, non-zero is bad
C
C LOCAL:
      integer*2 skbuf(80)
      integer i
      integer fget_literal,iret,ptr_ch,fget_all_lowl

C
C     1. Get all lines of the literal.
C
c  /* find $SCHEDULING_PARAMS section */
      kgeo = .false.
      iret=fget_all_lowl(ptr_ch(char(0)),ptr_ch(char(0)),
     .ptr_ch('literals'//char(0)),
     .ptr_ch('SCHEDULING_PARAMS'//char(0)),ivexnum)
      if (iret.eq.0) then ! this is a sked VEX file
        kgeo = .true.
        do i=1,nstatn
          tape_motion_type(i)='START&STOP'
          itearl(i)=0
          itlate(i)=0
          itgap(i)=0
          itlate(i)=0
        enddo
        isettm = 20
        ipartm = 70
        itaptm = 1
        isortm = 5
        ihdtm = 6
        iret=0
        do while(iret.ne.-3)
           iret=fget_literal(skbuf)
           if(iret.ne.0) then
              print *,(skbuf(i),i=1,80)
           end if
        enddo
        ierr=0
      endif ! read sked parameters

      RETURN
      END
