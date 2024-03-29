c fscom_dum.i
c
c See fscom.i for information on the structure
c
      double precision ra50, dec50, radat, decdat, alat, wlong,
     . user_dev1_value, user_dev2_value

      REAL AZOFF, DECOFF, ELOFF, ISTPTP(2), ITACTP(2), RAOFF,
     . XOFF, YOFF, ep1950, tempwx, preswx, humiwx, speedwx,
     . cablev, systmp(MAX_DET), epoch, height, diaman, slew1, slew2,
     . lolim1, lolim2, uplim1, uplim2,
     . HORAZ(MAX_HOR), HOREL(MAX_HOR), motorv(2), inscint(2),inscsl(2),
     . outscint(2), outscsl(2), wrvolt(2), rateti_fs, POSNHD(2,2),
     . FREQVC(15), motorv2(2), wrvolt2(2), wrvolt4(2), wrvolt42(2),
     $     extbwvc(15),cablevl

      integer*4 iclbox, iclopr, spanti_fs, epochti_fs, offsetti_fs,
     . secsoffti_fs,ichfm_cn_tm, cips(2), freqif3, fm4enable(2),
     . TPIVC(15), mifd_tpi(3), bbc_tpi(2,14), vifd_tpi(4),
     . ifp_tpi(2*MAX_DAS), logchg

      logical khalt, kecho, kenastk(2,2), klvdt_fs(2),knewtape(2)

      INTEGER ICAPTP(2), ICHECK(23), ILOKVC(15), 
     . IRDYTP(2), IRENVC, ITRAKA(2), ITRAKB(2),  ibmat,
     . ibmcb, ionsor, ipashd(2,2), ispeed(2), ienatp(2),
     . idirtp(2), imodfm, iratfm, inp1if, inp2if, ndevlog, rack,
     . drive(2), imaxtpsd(2), ichvlba(19), iskdtpsd(2), itpthick(2),
     . vform_rate, capstan(2), stchk(4), sterp, 
     . erchk, drive_type(2), wx_met,
     . rack_type, wrhd_fs(2), vfm_xpnt, ichs2,
     . IBWVC(15), IFP2VC(16), vacsw(2), itpthick2(2), thin(2), vac4(2),
     . imixif3, vfmenablehi, vfmenablelo, ihdmndel(2),
     . iat1if, iat2if, iat3if,
     $ reccpu(2), select, rdhd_fs(2), ifd_set, if3_set, imk4fmv,
     $ ITPIVC(15),iapdflg, iswif3_fs(4),ipcalif3, directionwx,
     $ ibds, ndas, idasfilt, idasbits, ichlba(2*MAX_DAS), mk4sync_dflt,
     $ icomputer, satellite, dbbcddcv, dbbcpfbv, dbbc_cond_mods,
     $ dbbc_if_factors(MAX_DBBC_IF), dbbc_cont_cal_mode, m5b_crate,
     $ dbbcddcvc, dbbcddcsubv, dbbccontcalpol

      INTEGER*2 ILEXPER(4), ILLOG(4), ILNEWPR(4), ILNEWSK(4),
     . ILPRC(4), ILSKD(4), ILSTP(4), INEXT(3), LFEET_FS(3,2), lgen(2,2),
     . lnaant(4), lsorna(5), lfreqv(3,15), idevant(32), idevgpib(32),
     . idevlog(5,32), idevmcb(32), hwid, modelti_fs, stcnm(1,4),
     . cwrap(4), user_dev1_name(2), user_dev2_name(2),
     . idevds(32), hostpc_fs(40)

      common/fscom_dum/
     . ra50, dec50, radat, decdat, alat, wlong,
     . user_dev1_value, user_dev2_value,
     . AZOFF, DECOFF, ELOFF, ISTPTP, ITACTP, RAOFF,
     . XOFF, YOFF, ep1950, tempwx, preswx, humiwx, speedwx,
     . cablev, systmp, epoch, height, diaman, slew1, slew2,
     . lolim1, lolim2, uplim1, uplim2,
     . HORAZ, HOREL, motorv, inscint, inscsl, outscint,
     . outscsl, wrvolt, rateti_fs, POSNHD,  FREQVC, motorv2, wrvolt2,
     . wrvolt4,wrvolt42,extbwvc,cablevl,
c
     . iclbox, iclopr, spanti_fs, epochti_fs, offsetti_fs,
     . secsoffti_fs,ichfm_cn_tm, cips, freqif3, fm4enable,
     . TPIVC, mifd_tpi, bbc_tpi, vifd_tpi, ifp_tpi, logchg,
c
     . khalt, kecho, kenastk, klvdt_fs,knewtape,
c
     . ICAPTP, ICHECK, ILOKVC, IRDYTP, 
     . IRENVC, ITRAKA, ITRAKB, ibmat, ibmcb,
     . ionsor, ipashd, ispeed, ienatp,
     . idirtp, imodfm, iratfm, inp1if, inp2if, ndevlog,
     . rack, drive, imaxtpsd, ichvlba, iskdtpsd, wx_met,
     . itpthick, 
     . vform_rate, capstan, stchk, sterp, 
     . erchk, drive_type,
     . rack_type, wrhd_fs, vfm_xpnt, ichs2,
     . IBWVC, IFP2VC, vacsw, itpthick2, thin, vac4,
     . imixif3, vfmenablehi, vfmenablelo, ihdmndel,
     . iat1if, iat2if, iat3if,
     $ reccpu, select, rdhd_fs, ifd_set, if3_set, imk4fmv,
     $ ITPIVC,iapdflg,iswif3_fs, ipcalif3, directionwx,
     $ ibds, ndas, idasfilt, idasbits, ichlba, mk4sync_dflt,
     $ icomputer, satellite, dbbcddcv, dbbcpfbv, dbbc_cond_mods,
     $ dbbc_if_factors, dbbc_cont_cal_mode, m5b_crate, dbbcddcvc,
     $ dbbcddcsubv, dbbccontcalpol,
c
     . ILEXPER, ILLOG, ILNEWPR, ILNEWSK,
     . ILPRC, ILSKD, ILSTP, INEXT, LFEET_FS, lgen, lnaant, lsorna, 
     . lfreqv, idevant, idevgpib, idevlog, idevmcb, hwid, 
     . modelti_fs, stcnm, cwrap, user_dev1_name, user_dev2_name,
     . idevds, hostpc_fs
c
      CHARACTER*8 LEXPER, LLOG, LNEWPR, LNEWSK, LPRC, LSKD, LSTP
      character*16 dbbcddcvs
      character*1 dbbcddcvl
      character*16 fila10gvsi_in
c
      common/fscom_dum2/ LEXPER, LLOG, LNEWPR, LNEWSK, LPRC, LSKD, LSTP,
     $                   dbbcddcvs, fila10gvsi_in, dbbcddcvl

