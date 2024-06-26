      subroutine proc_disk_tracks(lu_outfile,istn,icode,
     >         kignore_mark5b_bad_mask)
      include 'hardware.ftni'
      include '../skdrincl/freqs.ftni'
      include '../skdrincl/statn.ftni'
     
     
! Write out Mark5B mode command.
! functions
      integer itras
! passed
      integer lu_outfile,istn,icode
      logical kignore_mark5b_bad_mask
! function
      integer iwhere_in_string_list

! History
!  2007Jul27 JMGipson.  First working version.
!  2012Aug29 JMGipson.  Modified to reduce minimum number of tracks.
!  2012Sep04 JMGipson.  Fixed bug in checking whether GEO or VLBA mode. 
!  2012Sep13 JMGipson.  Modified to handle DBBC rack 
!  2014Dec06 JMG.       Added Mark5C
!  2014Dec10 JMG.       Fixed error with sample rate. 
!  2015Jan13 JFHQ.      Support for DBBC E/F 'astro3' track layout.
!  2015Jan17 JFHQ.      Changed odd BBC exceptions to 1, 5, 9, 1+5, 1+9 (and 5+9)
!                       but only for bandwidths <= 16 MHz.
! 2015Jan20  JMG        Print sample rate to 3 places. Also put in line 900 for abort situations to branch to.
! 2015Jan22  JFHQ       Only allow (DBBC) 32 MHz bandwidth with E/F 'astro3'.
! 2015Jan30  JMG.       Function ITRAS returns values assuming that LO is normal. If it isn't, you need to swap U<-->L 
!           Also added suport for 'ignore_mark5b_bad_mask'.  If this flag is set, it will write a warning message
! 2015Feb04  JMG.       Minor typographic change if not DBBC. 
! 2015May08  JMG.       Support for DBBC/Fila10G
! 2015May19  JMG.       Suppport for flexbuf 
! 2015Jun05 JMG. Repalced squeezewrite by drudg_write. 
! 2015Jun10 JMG. Added support chinese VLBAC and CDAS racks.  Put checking of tracks in routine check_csb_list.f
! 2015Jul06 JMG. In output changed "geo-r" to geo_r, etc. 
! 2015Jul28 JMG. Emit 'fila10g_mode' after 'fila10g_mode=...."

! local
      integer ipass
      integer isb,ibit,ihd,ic
      integer isb_out
      integer ib
      integer num_tracks
      integer nchan_rec
    
      integer*4 itemp
      integer*4 imask
      integer ierr 
    
      character*10 lbit_mask_mode
      
      integer idiv
      character*80 cbuf
      character*20 lmode_cmd
      character*6 lext_vdif
      logical kcomment_only         !only put out comments.
   
      character*1 lul(2)            !ASCII "U","L"
      character*1 lsm(2)            !ASCII "S","M"
      integer max_csb
      parameter (max_csb=32)
      logical kdebug

      character*4 lsked_csb(max_csb)     !Channel,sideband,bit in sked file

      logical kastro3_mode               !True if valid astro3 mode. 

! Have many possible mappings.

      character*4 lvlba_csb(max_csb),    lgeo_csb(max_csb)
! Below added 2012Sep07.  
      character*4  lwastro_csb(max_csb), llba_csb(max_csb)
! Below added 2013Jun17   
      character*4 lastro2_csb(max_csb)
! Below added 2015Jan13   
      character*4 lastro3_csb(max_csb)
      character*4 lbbc159_csb(12)
! Below added 2015Jun05 for chinese stations.   
      character*4 lvlbac_geo(max_csb)      !Chinese analog for R1/T2
      character*4 lcdas_geo_t(max_csb)      !Chinese CDAS geo_t mode
      character*4 lcdas_geo_r(max_csb)      !Chinese CDAS geo_r mode   
! Below added 2015Jun17
      character*4 lcdas_vlba4(max_csb)
      character*4 lcdas_vlba_l(max_csb)
      character*4 lcdas_vlba_u(max_csb)       

! The order determines what bit is set.
! NOTE: This is the same astro mode.
      data lvlba_csb/
     >   "01US","01UM","02US","02UM","03US","03UM","04US","04UM",
     >   "05US","05UM","06US","06UM","07US","07UM","08US","08UM",
     >   "01LS","01LM","02LS","02LM","03LS","03LM","04LS","04LM",
     >   "05LS","05LM","06LS","06LM","07LS","07LM","08LS","08LM"/

! The order determines what bit is set.
      data lgeo_csb/
     >   "01US","01UM","02US","02UM","03US","03UM","04US","04UM",
     >   "05US","05UM","06US","06UM","07US","07UM","08US","08UM",
     >   "01LS","01LM","08LS","08LM","09US","09UM","10US","10UM",
     >   "11US","11UM","12US","12UM","13US","13UM","14US","14UM"/

! Note: 1st half of WASTRO is the same as ASTRO. This is for second 
      data lwastro_csb/
     >   "09US","09UM","10US","10UM","11US","11UM","12US","12UM",
     >   "13US","13UM","14US","14UM","15US","15UM","16US","16UM",
     >   "09LS","09LM","10LS","10LM","11LS","11LM","12LS","12LM",
     >   "13LS","13LM","14LS","14LM","15LS","15LM","16LS","16LM"/

! Note: This is for LBA.  Second set of 32 is same as first set. 
        data llba_csb/
     >   "01US","01UM","02US","02UM","05US","05UM","06US","06UM",
     >   "03US","03UM","04US","04UM","07US","07UM","08US","08UM",
     >   "01LS","01LM","02LS","02LM","05LS","05LM","06LS","06LM",
     >   "03LS","03LM","04LS","04LM","07LS","07LM","06LS","06LM"/

! ASTRO2  Added 2013Jun17
      data lastro2_csb/
     >   "01US","01UM","02US","02UM","03US","03UM","04US","04UM",
     >   "09US","09UM","10US","10UM","11US","11UM","12US","12UM",
     >   "01LS","01LM","02LS","02LM","03LS","03LM","04LS","04LM",
     >   "09LS","09LM","10LS","10LM","11LS","11LM","12LS","12LM"/

! ASTRO3  Added 2015Jan13
      data lastro3_csb/
     >   "01US","01UM","03US","03UM","05US","05UM","07US","07UM",
     >   "09US","09UM","11US","11UM","13US","13UM","15US","15UM",
     >   "01LS","01LM","03LS","03LM","05LS","05LM","07LS","07LM",
     >   "09LS","09LM","11LS","11LM","13LS","13LM","15LS","15LM"/

! BBC01/05/09   Added 2015Jan17
! - order not relevant here
      data lbbc159_csb/
     >   "01US","01UM","01LS","01LM","05US","05UM","05LS","05LM",
     >   "09US","09UM","09LS","09LM"/

! Added 2015Jun05. Chinese Bit stuff
      data lvlbac_geo/
     >   "01US","02US","01LS","02LS","09US","10US","09LS","10LS",
     >   "05US","06US","05LS","06LS","12US","13US","12LS","13LS",
     >   "03US","04US","03LS","04LS","11US","----","11LS","----",
     >   "07US","08US","07LS","08LS","14US","----","14LS","----"/

      data lcdas_geo_t/
     >   "01LS","01LM","02LS","02LM","03LS","03LM","04LS","04LM",
     >   "05LS","05LM","06LS","06LM","07LS","07LM","08LS","08LM",  
     >   "01US","01UM","08US","08UM","09US","09UM","10US","10UM",    
     >   "11US","11UM","12US","12UM","13US","13UM","14US","14UM"/

      data lcdas_geo_r/
     >   "01LS","01LM","02LS","02LM","03LS","03LM","04LS","04LM",
     >   "05US","05UM","06US","06UM","07US","07UM","08US","08UM",
     >   "01US","01UM","08LS","08LM","09US","09UM","10US","10UM",    
     >   "11US","11UM","12US","12UM","13US","13UM","14US","14UM"/   
  
! Added 2015Jun17. 
! Don't know what the "N,T,Q"  mean. 
      data lcdas_vlba4/
     >  "01US","01UN","01UT","01UQ","02US","02UN","02UT","02UQ",
     >  "03US","03UN","03UT","03UQ","04US","04UN","04UT","04UQ",
     >  "01LS","01LN","01LT","01LQ","02LS","02LN","02LT","02LQ",
     >  "03LS","03LN","03LT","03LQ","04LS","04LN","04LT","04LQ"/   

      data lcdas_vlba_l/  
     >   "01LS","01LM","02LS","02LM","03LS","03LM","04LS","04LM",
     >   "05LS","05LM","06LS","06LM","07LS","07LM","08LS","08LM",
     >   "09LS","09LM","10LS","10LM","11LS","11LM","12LS","12LM",
     >   "13LS","13LM","14LS","14LM","15LS","15LM","16LS","16LM"/
  
      data lcdas_vlba_u/  
     >   "01US","01UM","02US","02UM","03US","03UM","04US","04UM",
     >   "05US","05UM","06US","06UM","07US","07UM","08US","08UM",
     >   "09US","09UM","10US","10UM","11US","11UM","12US","12UM",
     >   "13US","13UM","14US","14UM","15US","15UM","16US","16UM"/ 


      data lul/"U","L"/
      data lsm/"S","M"/

!      kdebug=.true.  
      kdebug=.false.   
      kastro3_mode=.false. 

      if(km5brec(1)) then
         lmode_cmd="mk5b_mode"
      else if(km5Crec(1)) then 
         lmode_cmd="mk5c_mode"
      else
         lmode_cmd="bit_streams"
      endif 
      lext_vdif="ext"
      if(kfila10g_rack.and.km5crec(1)) then
        lext_vdif="vdif" 
       endif        

! If we don't have a VSI4 formatter or a DBBC write out comments.
      kcomment_only=.false.   
      if(.not.(km5rack .or. kv5rack .or. kdbbc_rack .or.
     >         kvlbac_rack.or.kcdas_rack)) then
        kcomment_only=.true.  
!        write(lu_outfile,'(a,/,a)')
!     >  '"The following mode command assumes a VSi4/DBBC input',
!     >  '"Please check and change if necessary'
!         cbuf=lmode_cmd//'=ext,0xffffffff,,1.000'
!         call drudg_write(lu_outfile,cbuf)
!         call drudg_write(lu_outfile,lmode_cmd)     
      endif

      idiv = 32/nint(samprate(istn,icode))

! Remainder of code assumes that we have VSI4 formatter.
      ipass=1            !only 1 pass for Mark5B (or any disk)

! 
!******************************************************************************
! Make list containing tracks we use, and keep track of the number.
!     
      write(*,*)
      if(kdebug) then              
        write(*,'(a)') "  sb   sbo   bit  hd  chan pass stn  code  CSB"
      endif
      num_tracks=0
      do ic=1,max_chan
         do isb=1,2    
               isb_out=isb 
            if(abs(freqrf(ic,istn,icode)).lt.freqlo(ic,istn,icode)) then 
              isb_out=3-isb    !swap the sidebands
            endif ! reverse sidebands
            do ihd=1,max_headstack
            do ibit=1,2    
              if (itras(isb,ibit,ihd,ic,ipass,istn,icode).ne.-99) then         !number of tracks set.                         
                ib=ibbcx(ic,istn,icode)   !this is the BBC#
                num_tracks=num_tracks+1    
                write(lsked_csb(num_tracks),'(i2.2,a1,a1)')
     >            ib, lul(isb_out), lsm(ibit)    
                if(kdebug) then 
                  write(*,'(8i5,1x,a)')  isb,isb_out,ibit,ihd,ic,
     >                   ipass,istn,icode,lsked_csb(num_tracks) 
                endif 
              endif
            enddo
          enddo
        enddo
      enddo

      if(num_tracks .gt. max_csb) then
         write(*,"('Proc_disk_tracks Error! max_tracks is: ',i3)")
     >        max_csb
         write(*,"('But specfied ', i3)") num_tracks
         goto 900
!         if(kcomment_only) then
!           return
!         else
!          stop
!         endif 
       endif

       write(*,'(a,$)') "Checking bit_masks: "

! ************************END OF GENERATING TRACK LIST****************************

! ***********Chinese VLBAC RACK****************************   
      if(kvlbac_rack) then
! Check vlba mode.    
        lbit_mask_mode="vlba" 
        write(*,'(1x,a,$)') lbit_mask_mode
        call check_csb_list(lvlba_csb,max_csb,  
     >                      lsked_csb,num_tracks,imask,ierr)
        if(ierr .eq. 0) goto 300   
     
! check vlbac_geo mode. 
        lbit_mask_mode="geo"
        write(*,'(1x,a,$)') lbit_mask_mode
        call check_csb_list(lvlbac_geo,max_csb,  
     >                      lsked_csb,num_tracks,imask,ierr)
        if(ierr .eq. 0) goto 300 
            
        goto 900 
      end if 

!******* Chinese CDAS RACK********************************************
      if(kcdas_rack) then 
        lbit_mask_mode="geo"
        write(*,'(1x,a,$)') lbit_mask_mode
        call check_csb_list(lgeo_csb,max_csb,  
     >                      lsked_csb,num_tracks,imask,ierr)
        if(ierr .eq. 0) goto 300       


        lbit_mask_mode="vlba"
        write(*,'(1x,a,$)') lbit_mask_mode
        call check_csb_list(lvlba_csb,max_csb,  
     >                      lsked_csb,num_tracks,imask,ierr)
        if(ierr .eq. 0) goto 300       
      

        lbit_mask_mode="geo_t"
        write(*,'(1x,a,$)') lbit_mask_mode
        call check_csb_list(lcdas_geo_t,max_csb,  
     >                      lsked_csb,num_tracks,imask,ierr)
        if(ierr .eq. 0) goto 300 
      
        lbit_mask_mode="geo_r"
        write(*,'(1x,a,$)') lbit_mask_mode
        call check_csb_list(lcdas_geo_r,max_csb,  
     >                      lsked_csb,num_tracks,imask,ierr)
        if(ierr .eq. 0) goto 300 
        
    
        lbit_mask_mode="vlba_l"
        write(*,'(1x,a,$)') lbit_mask_mode
        call check_csb_list(lcdas_vlba_l,max_csb,  
     >                      lsked_csb,num_tracks,imask,ierr)
        if(ierr .eq. 0) goto 300        
  

        lbit_mask_mode="vlba_u" 
        write(*,'(1x,a,$)') lbit_mask_mode
        call check_csb_list(lcdas_vlba_u,max_csb,  
     >                      lsked_csb,num_tracks,imask,ierr)
        if(ierr .eq. 0) goto 300           
        goto 900 
      end if 

! Ckeck DBBC racks....
     
100   continue
! Pre-check to see if a valid astro3 mode, but only for DBBC/Unknown racks
      if(.not.(kdbbc_rack .or. kcomment_only)) goto 110
      lbit_mask_mode="astro3"
      write(*,'(1x,a,$)') lbit_mask_mode
      call check_csb_list(lastro3_csb,max_csb,  
     >                    lsked_csb,  num_tracks,imask,ierr)
      if(ierr .ne. 0) goto 110
      kastro3_mode=.true. 
      if(idiv .lt. 1) goto 300          !Valid astro3 mode & 32 MHz channels. 
                                        !Other possible modes do not support 32 MHz. 
    
!     But don't use astro3 mode for BBC01/05/09 only or in combination
      do ic=1,num_tracks
        ibit=iwhere_in_string_list(lbbc159_csb,12,lsked_csb(ic))
        if(ibit .eq. 0) goto 300     !Found a track that was not in 01/05/09.  
      end do 
! At this point all of lsked_csb use BBCs 01, 05, or 09.  
! See if consistent with astro or astro2
      goto 200      
 
110   continue
      if(kdbbc_rack .and. (idiv.lt.1)) then
          write(*,*) "Only astro3 mode supports 32 MHz channels."
          goto 900
      endif

! Check to see if a valid geo mode.
      lbit_mask_mode="geo" 
      write(*,'(1x,a,$)') lbit_mask_mode
      call check_csb_list(lgeo_csb, max_csb,
     >                    lsked_csb,num_tracks,imask,ierr)
      if(ierr .eq. 0) goto 300 
           
      
200   continue 
      lbit_mask_mode="astro"
      write(*,'(1x,a,$)') lbit_mask_mode
      call check_csb_list(lvlba_csb, max_csb,
     >                    lsked_csb,num_tracks,imask,ierr)
      if(ierr .eq. 0) goto 300         
      if(.not.(kdbbc_rack .or. kcomment_only)) goto 900        
    
! Check to see if a valid astro2 mode.
      lbit_mask_mode="astro2" 
      write(*,'(1x,a,$)') lbit_mask_mode
      call check_csb_list(lastro2_csb, max_csb,
     >                    lsked_csb,num_tracks,imask,ierr)   
      if(ierr .eq. 0) goto 300             
      if(kastro3_mode) then
         lbit_mask_mode="astro3"
         goto 300
      endif 
      if(.not.kcomment_only) goto 900         
  
!
! A little bit of cleanup.
300   continue
      write(*,'(a)') " Success! mode="//lbit_mask_mode 
!      write(*,'(a)') " Using: ",lbit_mask_mode

! Previously checked that num_tracks <= max_csb so we don't need to check that here.
! Mark5A record a minimum of 8 channels.
! Mark5B record a minimum of 1 channel.    
      if(km5b .or. km5c)  then
         nchan_rec=1
      else
         nchan_rec=8
      endif 

      do while(nchan_rec .lt. num_tracks)
        nchan_rec=nchan_rec*2
      end do      
	
! If we need to, turn on extra bits until we get to 1,2, 4, 8, 16, or 32 channels.
      itemp=1
      do while(num_tracks .lt. nchan_rec)
        if(iand(itemp,imask) .eq. 0) then
          imask=ior(itemp,imask)             !bit not set. Set it.
          num_tracks=num_tracks+1
        endif
        itemp=ishft(itemp,1)            !shift the bit.
      end do

      if(kcomment_only) then             
        write(lu_outfile,'(a)')
     >   '" Channel assignments consistent with vsi4='//lbit_mask_mode
        write(lu_outfile,'(a)') 
     >   '" Following command assumes VSi4/DBBC input'
        write(lu_outfile,'(a)') 
     >   '" Please check and change if necessary'
      endif

       if(kfila10g_rack) then
        write(cbuf,'(a,"=0x",Z8.8,",,",f9.3)')
     >    'fila10g_mode', imask,samprate(istn,icode)                   
          call drudg_write(lu_outfile,cbuf)
        write(lu_outfile,'("fila10g_mode")') 
       endif 

        write(cbuf,'(a,"=",a,",0x",Z8.8,",,",f9.3)')
     >    lmode_cmd,lext_vdif, imask,samprate(istn,icode)
  
      call drudg_write(lu_outfile,cbuf)
      call drudg_write(lu_outfile,lmode_cmd)     
      if(kcomment_only) return

      if(kdbbc_rack) then
         cbuf="form="//lbit_mask_mode
         call drudg_write(lu_outfile,cbuf)
         write(lu_outfile,'("form")') 
      else if(kvlbac_rack .or. kcdas_rack) then 
         cbuf="vsi4="//lbit_mask_mode
         call drudg_write(lu_outfile,cbuf)
         write(lu_outfile,'("vsi4")') 
      else 
        if(lbit_mask_mode .eq. "geo" .or.
     &     lbit_mask_mode .eq. "vlba") then
          cbuf="vsi4="//lbit_mask_mode
          call drudg_write(lu_outfile,cbuf)
          write(lu_outfile,'("vsi4")')  
        else
          write(*,'("Warning! Mode ", a, " is not compatible with ",a)')
     &         lbit_mask_mode, cstrack(istn)
          goto 900 
        endif 
      endif 
      return

900   continue
      write(*,*) 
     > "*********************************************************"
      write(*,*)
     > "ERROR(proc_disk_tracks)! No valid Mark5B mask/mode found."
     >
      if(.not.kignore_mark5b_bad_mask) then
          write(*,*) "Please fix file and start again."
        stop
      endif
      write(*,*) "You will need to edit the PRC file."
      write(lu_outfile,'(a,/,a)')
     >  '"Please change the following command to reflect',
     >  '"the desired channel assignments and effective sample rate:'
         cbuf=lmode_cmd//'=ext,0xffffffff,,1.000'
      call drudg_write(lu_outfile,cbuf)
      call drudg_write(lu_outfile,lmode_cmd) 
      write(lu_outfile,'(a)') "form=UNKNOWN"

      end   
  

