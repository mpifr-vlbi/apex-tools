      integer function fnblnk(string,ipos)
c
c  Finds the first nonblank character in string at IPOS or after
c
      character*(*) string
      data iblank/32/
c
      ilen = len(string)
      if(ipos.gt.ilen) then
          fnblnk = 0
          return
      endif
      do 10 i = ipos,ilen
          if(ichar(string(i:i)).ne.iblank) then
              fnblnk = i
              return
          endif
 10   continue
c  Did not find it
      fnblnk = 0
      return
      end
