      subroutine phcal(pcal,itrk,ivc)

C   calculates phase cal freq c#870115:05:14# 
C 
C     This routine determines the phase cal freq in a video converter 
C     given the track number. 
C 
C     MAH 19820222
C 
C     INPUT:
C         ITRK - track number 
C 
C     OUTPUT: 
C         IVC - video converter number for ITRK 
      double precision pcal,sum,flo,fvc 
      double precision rflo
C         PCAL - phase cal freq in IVC
C 
      include '../include/fscom.i'
C 
      lsb = 0 
      call fs_get_imodfm(imodfm)
      ivc = itr2vc(itrk,imodfm+1) 
      if (ivc.lt.0) lsb = 1 
      ivc = iabs(ivc) 
      call fs_get_ifp2vc(ifp2vc)
      i1dex = iabs(ifp2vc(ivc)) 
      call fs_get_freqlo(rflo,i1dex-1)
      flo=rflo
      call fs_get_freqif3(freqif3)
      call fs_get_imixif3(imixif3)
      if(i1dex.eq.3.and.imixif3.eq.1) flo=flo+freqif3*0.01d0
      call fs_get_freqvc(freqvc)
      fvc = freqvc(ivc)+5.d-3 
      fvc = fvc*100.d0
      fvc = (aint(fvc))/100.d0 
      sum = flo+fvc 
      if (lsb.eq.0) pcal = aint(sum+1.d0)-sum
      if (lsb.eq.1) then
        pcal = dmod(sum,1.d0) 
      endif
      pcal = pcal*1.d6
c
      return
      end 
