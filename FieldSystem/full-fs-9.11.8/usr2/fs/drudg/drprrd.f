      subroutine drprrd(ivexnum)

C   DRPRRD reads the lines in the $PARAM section needed by drudg.
      include '../skdrincl/skparm.ftni'
      include 'drcom.ftni'
      include '../skdrincl/statn.ftni'
C History
C 020713 nrv copied from sked

C Input
      integer ivexnum
! functions
      integer ichmv,i2long
      integer fget_literal,iret,ptr_ch,fget_all_lowl

C Local
      integer nch,ilen,ic1,ic2,ich,idummy,ierr
      integer*2 ibufq(100)
      logical kmore

      if (.not.kvex) then ! find $PARAM section
        rewind(lu_infile)
        ibufq(1) = 0
        DO WHILE (ibufq(1).ne.-1.and.cbuf(1:6).ne.'$PARAM')
          CALL READS(lu_infile,ierr,IBUF,isklen,ilen,2) ! get next line
          ibufq(1) = ilen
        enddo
      else ! find SCHEDULING_PARAMS literal
        iret=fget_all_lowl(ptr_ch(char(0)),ptr_ch(char(0)),
     .  ptr_ch('literals'//char(0)),
     .  ptr_ch('SCHEDULING_PARAMS'//char(0)),ivexnum)
        if (iret.lt.0) return
        kgeo = .true.
      endif ! $PARAM or SCHEDULING_PARAMS

C  Get the initial line of parameters
      if (.not.kvex) then ! read sk file first line
        CALL READS(lu_infile,ierr,IBUF,isklen,ilen,2)
        ibufq(1) = ilen
      else ! get first literal line
        iret=fget_literal(ibuf) ! first fget is null
        iret=fget_literal(ibuf)
        ibufq(1) = iret
      endif ! sk/vex
      kmore = .true.

C  Loop on parameter section lines
      DO WHILE (kmore) !decode an entry
        ICH=1
        CALL GTFLD(IBUF,ICH,i2long(IBUFQ(1)),IC1,IC2)
        nch=ibufq(1)-ic2
        IF  (    cbuf(1:6).eq. 'SUBNET') THEN  !SUB line
        ELSE IF (cbuf(1:4).eq. 'SCAN') THEN !SCAN line
        ELSE IF (cbuf(1:5).eq. 'WEIGHT') THEN
        ELSE if(cbuf(1:5) .eq. 'TAPE_') then
          ibufq(1) = nch
          idummy=ichmv(ibufq(2),1,ibuf,ic2+1,nch)
          IF      (cbuf(1:9).eq. 'TAPE_TYPE') THEN
            CALL TTAPE(IBUFQ,luscn,luscn)
          ELSE IF (cbuf(1:11) .eq.'TAPE_MOTION') THEN
            CALL STAPE(IBUFQ,luscn,luscn)
          ELSE IF (cbuf(1:15).eq.'TAPE_ALLOCATION') THEN
            CALL ATAPE(IBUFQ,luscn,luscn)
          ENDIF
        ELSE IF (cbuf(1:9) .eq.'ELEVATION') THEN
          ibufq(1) = nch
          idummy=ichmv(ibufq(2),1,ibuf,ic2+1,nch)
          CALL SELEV(IBUFQ,luscn,luscn)
        ELSE IF (cbuf(1:11).eq. 'EARLY_START') THEN
          ibufq(1) = nch
          idummy=ichmv(ibufq(2),1,ibuf,ic2+1,nch)
          CALL SEARL(IBUFQ,luscn,luscn)
        else if(cbuf(1:3) .eq. 'SNR') then !SNR or SNR_1
        ELSE
          idummy=ichmv(ibufq(2),1,ibuf,1,i2long(ibufq(1)))
          CALL drSET(IBUFQ)
        ENDIF
        if (.not.kvex) then ! read sk file first line
          CALL READS(lu_infile,ierr,IBUF,isklen,ilen,2)
          ibufq(1) = ilen
          kmore = cbuf(1:1) .ne. "$" .and. ibufQ(1).NE.-1
        else ! get first literal line
          iret=fget_literal(ibuf)
          ibufq(1) = iret
          kmore = iret.gt.0
        endif ! sk/vex
      enddo

      return
      end
