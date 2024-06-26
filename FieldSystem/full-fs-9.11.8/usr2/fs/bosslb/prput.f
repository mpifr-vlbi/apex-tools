      subroutine prput(istack,ientry,nwords,ierr)
C
C     PRPUT - pushes entry onto stack
C
C
C  INPUT:
C
      dimension istack(1)
C       Stack holding procedure names and record numbers
      dimension ientry(1)
C       Entry to be pushed onto stack.  May be a procedure
C       name or number of records.
C     NWORDS - number of words to push
C
C
C  OUTPUT:
C
C     IERR - error return.  -1 means no more room.
C
C
C  LOCAL:
C
C     INDEX - index in stack array
C
C
C  INITIALIZED:
C
C  PROGRAMMER: NRV
C  LAST MODIFIED:  CREATED 790912
C# LAST COMPC'ED  870115:04:22  #
C
C
C     1. First check that there is enough room at the inn.
C
      if (istack(2)+nwords.le.istack(1)) goto 200
      ierr = -1
      goto 900
C
C
C     2. Now push down the number of words.
C
200   do 210 i=1,nwords
      index = istack(2) + 1
      istack(2) = index
      istack(index) = ientry(i)
210   continue
      ierr = 0
C 
900   return
      end 
