      logical function kobot(lut,idcb,ierr,ibad,xlon,xlat,xlnoff,xltoff,
     +                 xlnsof,xltsof,lsourc,idoy,ih,im,elev,lst)
C
      integer*2 lsourc(1),ibuf(65)
      integer jr2as
      logical kpout
C
      inext=1
      call ifill_ch(ibuf,1,130,' ')
      if (ibad.ne.0) inext=ichmv_ch(ibuf,inext,'*bad ')
      if (ibad.eq.0) inext=ichmv_ch(ibuf,inext,'  1  ')
C
      inext=inext+jr2as(xlon,ibuf,inext,-9,5,50)
      inext=ichmv_ch(ibuf,inext,' ')
C
      inext=inext+jr2as(xlat,ibuf,inext,-9,5,50)
      inext=ichmv_ch(ibuf,inext,' ')
C
      inext=inext+jr2as(xlnoff,ibuf,inext,-8,5,50)
      inext=ichmv_ch(ibuf,inext,' ')
C
      inext=inext+jr2as(xltoff,ibuf,inext,-8,5,50)
      inext=ichmv_ch(ibuf,inext,' ')
C
      inext=inext+jr2as(xlnsof,ibuf,inext,-7,5,50)
      inext=ichmv_ch(ibuf,inext,' ')
C
      inext=inext+jr2as(xltsof,ibuf,inext,-7,5,50)
      inext=ichmv_ch(ibuf,inext,' ')
C
      inext=ichmv(ibuf,inext,lsourc,1,10)
      inext=ichmv_ch(ibuf,inext,' ')
C
      inext=inext+jr2as(elev,ibuf,inext,-5,1,50)
      inext=ichmv_ch(ibuf,inext,' ')
C
      if (0.eq.mod(inext,2)) inext=ichmv_ch(ibuf,inext,' ')
      kobot=kpout(lut,idcb,ibuf,inext,iobuf,lst)
C
      return
      end
