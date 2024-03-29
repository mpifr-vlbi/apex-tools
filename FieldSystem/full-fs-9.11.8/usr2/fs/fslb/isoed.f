      function isoed(iwhat,index,ias,ic1,ic2)

C  This routine encodes or decodes information relating
C  to the on-source indicator
C 
C  INPUT: 
C 
C     IWHAT - code for type of conversion, <0 encode, >0 decode 
      integer*2 ias(1)
C      - string with ASCII characters 
C 
C 
C  If encode
C 
C     INDEX - input index of quantity 
C     IAS - string to hold ASCII characters 
C     IC1 - first char available in IAS 
C     IC2 - last char available in IAS
C     ISOED - next available char in IAS after encoding 
C 
C  If decode: 
C 
C     INDEX - returned index of quantity, error if zero 
C     IAS - string containing ASCII characters to be decoded
C     IC1 - first char to use in IAS
C     IC2 - last char to use in IAS 
C 
C 
C  SUBROUTINES called: character manipulation 
C 
C 
C  LOCAL: 
C 
      integer*2 ltrack(8) 
      dimension ntrack(2) 
C 
C  INITIALIZED: 
C 
      data ltrack /2HSL,2HEW,2HIN,2HG ,2HTR,2HAC,2HKI,2HNG/ 
      data ntrack/7,8/
C 
C 
C  Initialize returned parameter in case we have to quit early. 
C 
      isoed = ic1 
      if (iwhat.gt.0) index = -1
C 
      goto (201) iwhat+2
C 
C  Code -1, on-source or not. 
C 
201   continue
      if (ic1-1+ntrack(index+1).gt.ic2) return
      isoed = ichmv(ias,ic1,ltrack,index*8+1,ntrack(index+1)) 

990   return
      end 
