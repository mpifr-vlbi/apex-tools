      subroutine gtnam(ias,ifc,iec,lnames,nnames,lproc1,nproc1, 
     .lproc2,nproc2,ierr,itype,index)
C 
C     GTNAM returns the index of the given word in one of 3 
C              lists, and which list. 
C 
C     INPUT VARIABLES:
C 
      integer*2 ias(1)
C               - input character string
C        IFC,IEC- first, end characters in input string IAS 
C                 Word to be checked lies here. 
      dimension lnames(13,1)
      integer*4 lproc1(4,1),lproc2(4,1) 
C               - 3 lists to be checked for the word
C               - word to be checked is word (1,I) in each array
C        NNAMES - number of entries in LNAMES 
C        NPROC1,2 - number of entries in LPROC1,2 (may be zero) 
C 
C     OUTPUT VARIABLES: 
C
C        IERR   - error return.  0=all ok
C        ITYPE  - which list word was found in, F (names) or
C                 P (procedure list1) or Q (procedure list 2)
C        INDEX  - index of word in above list
C
C     CALLING SUBROUTINES: SPARS, GETCM
C     CALLED SUBROUTINES: none
C
C   LOCAL VARIABLES
C
C        NCHAR  - number of characters in word
C        LMATCH - copy of input character string
      integer ichcm, ichmv, rack, drive(2)
      integer*2 lmatch(6)
C
C  HISTORY:
C  WHO  WHEN    WHAT
C  LAR  880105  CHECK NAME AGAINST ACTUAL PROCEDURE NAMES INSTEAD OF 
C               HASH CODES
C
C
C     1. Write blank-filled left-justified word into LMATCH.
C
      nchar = iec-ifc+1
      if (nchar.gt.12) then
        ierr = -3
        return
      endif
      call ifill_ch(lmatch,1,12,' ')
      idumb=ichmv(lmatch,1,ias,ifc,nchar)
C
C     2. Now search the first list.
C
C  The logic here is different for other lists because if we actually
c  find a low-level command, then the equipment must match. If there
c  isn't corresponding equipment, it's an error. You can't have
c  procedure names the same as low-level SNAP commands even if you
c  don't have that equipment.
C
      call fs_get_rack(rack)
      call fs_get_drive(drive)
      index=-1
      do i=1,nnames
         if (ichcm(lnames(1,i),1,lmatch,1,12).eq.0.) then
            if((and(rack,lnames(11,i)).eq.rack
     $           .and.rack.ne.0
     $           .or.65535.eq.lnames(11,i))
     $           .and.
     $           (and(drive(1),lnames(12,i)).eq.drive(1)
     $           .and.drive(1).ne.0
     $           .or.65535.eq.lnames(12,i))
     $           .and.
     $           (and(drive(2),lnames(13,i)).eq.drive(2)
     $           .and.drive(2).ne.0
     $           .or.65535.eq.lnames(13,i))
     $           .and.
     $           (lnames(8,i).eq.0.or.
     $           (lnames(8,i).eq.1.and.drive(1).ne.0.and.drive(2).eq.0)
     $           .or.
     $           (lnames(8,i).eq.2.and.drive(1).eq.0.and.drive(2).ne.0)
     $           .or.
     $           (lnames(8,i).eq.3.and.drive(1).ne.0.and.drive(2).ne.0)
     $           ))then
               index=i
               call char2hol('F',itype,2,2)
               return
            endif
            index=0
         endif
      enddo
      if(index.eq.0) then
         ierr=-13
         return
      endif
C
C     3.  No match found in first list.
C         Search the first procedure list.
C
      do 390 i=1,nproc1
        if (ichcm(lproc1(1,i),1,lmatch,1,12).ne.0) goto 390
        call char2hol('P',itype,2,2)
        index = i
        return
390   continue
C 
C     4. No match on the second list.  Try for the third. 
C 
      do 490 i=1,nproc2 
        if (ichcm(lproc2(1,i),1,lmatch,1,12).ne.0) goto 490
        call char2hol('Q',itype,2,2)
        index = i 
        return
490   continue
C 
C     9. No match was found in any of the lists.
C 
      ierr = -4
      return
      end 
