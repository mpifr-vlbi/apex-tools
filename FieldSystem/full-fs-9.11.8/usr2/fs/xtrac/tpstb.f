      subroutine tpstb(lut,idcb,avglon,avglat,rmslon,rmslat,
     .dirms,inp,igp,iobuf,lst,ibuf,il)
C
      integer*2 ibuf(1)
      integer*2 idcb(1)
      integer lut,inp,igp,lst,il
      integer ichmv_ch
      integer jr2as,ib2as
      real avglon,avglat,rmslon,rmslat,dirms
      character*(*) iobuf
      logical kpout,kpret
C
      call ifill_ch(ibuf,1,100,' ')
      inext=1
      inext=ichmv_ch(ibuf,inext,'     ')
C
      inext=inext+jr2as(avglon,ibuf,inext,-8,4,il)
      inext=ichmv_ch(ibuf,inext,' ')
C 
      inext=inext+jr2as(rmslon,ibuf,inext,-8,4,il)  
      inext=ichmv_ch(ibuf,inext,' ')
C 
      inext=inext+jr2as(avglat,ibuf,inext,-7,4,il)  
      inext=ichmv_ch(ibuf,inext,' ')
C 
      inext=inext+jr2as(rmslat,ibuf,inext,-7,4,il)  
      inext=ichmv_ch(ibuf,inext,' ')
C 
      inext=inext+jr2as(dirms,ibuf,inext,-6,4,il) 
      inext=ichmv_ch(ibuf,inext,' ')
C 
      inext=inext+ib2as(igp,ibuf,inext,3) 
      inext=ichmv_ch(ibuf,inext,' ')
C 
      inext=inext+ib2as(inp,ibuf,inext,3) 
      inext=ichmv_ch(ibuf,inext,' ')
C
      if (0.eq.mod(inext,2)) inext=ichmv_ch(ibuf,inext,' ')
      kpret=kpout(lut,idcb,ibuf,inext,iobuf,lst)
C
      return
      end
