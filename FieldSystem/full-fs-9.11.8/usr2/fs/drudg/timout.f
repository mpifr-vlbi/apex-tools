      subroutine timout(ibuf2,itlen,iyear,idayr,ihr,imin,isc)
      include '../skdrincl/skparm.ftni'
C Format time commands for the SNAP file.
C 980910 nrv Extracted from snap.f
C 980910 nrv Change output format to use punctuation.

C Input
      integer*2 ibuf2(*)
      integer iyear,idayr,ihr,imin,isc
C Output
      integer itlen ! length of time field
C     ibuf2 has 10 characters on output

C Local
      integer Z4000,Z100,iblen
      integer ichmv_ch,ib2as
C Initialized
      data Z4000/Z'4000'/
      data Z100/Z'100'/

      iblen = ibuf_len*2
      CALL IFILL(IBUF2,1,iblen,32)
      itlen = ichmv_ch(IBUF2,1,'!')
      itlen = itlen + IB2AS(IYEAR,IBUF2,itlen,Z4000+4*Z100+4)
      itlen = ichmv_ch(ibuf2,itlen,'.')
      itlen = itlen + IB2AS(IDAYR,IBUF2,itlen,Z4000+3*Z100+3)
      itlen = ichmv_ch(ibuf2,itlen,'.')
      itlen = itlen + IB2AS(IHR,IBUF2,itlen,Z4000+2*Z100+2)
      itlen = ichmv_ch(ibuf2,itlen,':')
      itlen = itlen + IB2AS(iMIN,IBUF2,itlen,Z4000+2*Z100+2)
      itlen = ichmv_ch(ibuf2,itlen,':')
      itlen = itlen + IB2AS(ISC,IBUF2,itlen,Z4000+2*Z100+2)
      call hol2lower(ibuf2,(itlen+1))

      return
      end
