      SUBROUTINE LSPIN(iDIR,ISPM,SPS,IBUF2,NCH,crec,nrec)
C LSPIN  forms a buffer with the SNAP command FASTx=nnMmmS or SFASTx
C
      include '../skdrincl/skparm.ftni'
C
C Input:
      integer idir,ispm,nch,nrec
      real sps
      integer*2 IBUF2(ibuf_len)
      character*1 crec 
C
C MODIFICATIONS - 880411 NRV DE-COMPC'D
C nrv 930407 implicit none
C 970721 nrv Replace LDIR in call with idir
C 991102 nrv Add crec to call.
C 991123 nrv Add nrec to call.
C 021010 nrv Add option for SFASTx (super fast)
C 021014 nrv Change seconds to real, to get fractional times.

	integer Z8000
        integer ic
        integer ir2as,ib2as,ichmv_ch ! functions
	DATA Z8000/Z'8000'/
C
      nch=0
      if (idir.eq.0) return
      if (idir.eq.+2.or.idir.eq.-2) nch = ichmv_ch(IBUF2,1,'SFAST')
      if (idir.eq.+1.or.idir.eq.-1) nch = ichmv_ch(IBUF2,1,'FAST')
      if (idir.eq.+1.or.idir.eq.+2) nch = ICHMV_ch(IBUF2,nch,'f')
      if (idir.eq.-1.or.idir.eq.-2) nch = ICHMV_ch(IBUF2,nch,'r')

      if (nrec.gt.1) nch = ichmv_ch(ibuf2,nch,crec)
      nch = ichmv_ch(IBUF2,nch,'=')
      IC=Z8000+2
      NCH = nch+IB2AS(ISPM,IBUF2,nch,ic)
      NCH = ichmv_ch(IBUF2,NCH,'M')
      NCH = NCH + Ir2AS(SPS,IBUF2,NCH,-5,-2)
      NCH = ichmv_ch(IBUF2,NCH,'S ')-1
      RETURN
      END
