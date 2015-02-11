
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <sys/time.h>
#include <signal.h>

#include <xlrtypes.h>
#include <xlrapi.h>

#define SS_REG_MACFLT_CTRL  0x02  // 1==mode0/ignore, 2==mode1/fillpattern, 4==mode2/invalid, |16==pktlen_filter_ena 
#define SS_REG_PAYLDOFFSET  0x03
#define SS_REG_DFOFFSET     0x04
#define SS_REG_PSNOFFSET    0x05
#define SS_REG_PACKETSIZE   0x06
#define SS_REG_FILLPATTERN  0x07
#define SS_REG_PACKETCOUNT  0x08
#define SS_REG_MACFILTER    0x0C  // |16==nofilter, ^16==filter
#define SS_REG_PACKETSIZE2  0x0D  // bits[15:0]
#define SS_REG_REJECTCOUNT  0x11
#define SS_REG_MACADDRBASE  0x12
#define SS_MAX_MAC_ADDRESSES  16

////////////////////////////////////////////////////////////////////////
// XLR call wrapper
////////////////////////////////////////////////////////////////////////

void printXLRError(const char* msg)
{
   int err = XLRGetLastError();
   char serr[XLR_ERROR_LENGTH+1];
   XLRGetErrorMessage(serr, err);
   fprintf(stderr, "%s: Error %s\n", msg, serr);
}

#define XLR_DEBUG 1
#define XLRTEST(statement)                                  \
  { if(XLR_DEBUG) printf("XLR: %s\n", #statement);          \
    XLR_RETURN_CODE rc = (statement);                       \
    if(XLR_DEBUG) printf("     returned %d\n", (int)rc); \
    if (rc != XLR_SUCCESS) {                                \
        int err = XLRGetLastError();                        \
        char serr[XLR_ERROR_LENGTH+1];                      \
        XLRGetErrorMessage(serr, err);                      \
        fprintf(stderr, "%s -- Error %s\n", #statement, serr); \
        return -1;                                           \
    } \
  }
#define XLRTEST2(statement)                                 \
  { if(XLR_DEBUG) printf("XLR: %s\n", #statement);          \
    XLR_RETURN_CODE rc = (statement);                       \
    if(XLR_DEBUG) printf("     returned %d\n", (int)rc); \
    if (rc != XLR_SUCCESS) {                                \
        int err = XLRGetLastError();                        \
        char serr[XLR_ERROR_LENGTH+1];                      \
        XLRGetErrorMessage(serr, err);                      \
        fprintf(stderr, "%s -- Error %s\n", #statement, serr); \
    } \
  }


////////////////////////////////////////////////////////////////////////
// Ctrl-C
////////////////////////////////////////////////////////////////////////

int die = 0;
typedef void (*sighandler_t)(int);
sighandler_t oldsiginthand;
void siginthand(int j)
{
	die = 1;
	signal(SIGINT, oldsiginthand);
}

////////////////////////////////////////////////////////////////////////
// HELP
////////////////////////////////////////////////////////////////////////

void usage()
{
   printf("\n");
   printf("mk5netdump <packetsize> <dataoffset> <filename> <bytes>\n\n");
   printf("   packetsize  : 5008 bytes to extract\n");
   printf("   dataoffset  : 40   bytes after start of the Ethernet frame\n");
   printf("   filename    : any file name and path to write to\n");
   printf("   bytes       : number of bytes to capture into file\n");
   printf("\n");
}

////////////////////////////////////////////////////////////////////////
// STREAMSTOR
////////////////////////////////////////////////////////////////////////

SSHANDLE streamstorInit(int packetsize, int dfoffset)
{
   SSHANDLE ssd;
   const int payldoffset = 0;

   /* configure Amazon card */
   XLRTEST( XLROpen(1, &ssd) );
#if 0
   XLRTEST( XLRReset(ssd) );
   XLRClose(ssd);
   XLRTEST( XLROpen(1, &ssd) );
#endif
   XLRTEST( XLRSetBankMode(ssd, SS_BANKMODE_NORMAL) );
   XLRTEST( XLRSetMode(ssd, SS_MODE_PASSTHRU) );
   XLRTEST( XLRClearChannels(ssd) );
   XLRTEST( XLRBindInputChannel(ssd, 28) ); // DB(28), FPDP(30)
   XLRTEST( XLRBindOutputChannel(ssd, 0) ); // FIFO=>PCI(0)
   XLRTEST( XLRSelectChannel(ssd, 28) );    // DB(28), FPDP(30)

   // enable data strobe clock on an Amazon daughterboard
#if 0
   XLRTEST2( XLRSetFPDPMode(ssd, SS_FPDPMODE_RECVM, SS_DBOPT_FPDPSTROB) );
   XLRTEST2( XLRSetDBMode(ssd,SS_FPDPMODE_RECVM, SS_DBOPT_FPDPNRASSERT) );
#endif

   /* configure 10G daughter board */
   printf(" packetsize=%u payloadoffset=%u dataframeoffset=%u\n", packetsize, payldoffset, dfoffset);
   XLRTEST( XLRWriteDBReg32(ssd, SS_REG_FILLPATTERN, 0xDEADBEEF) );
   XLRTEST( XLRWriteDBReg32(ssd, SS_REG_PAYLDOFFSET, payldoffset) );
   XLRTEST( XLRWriteDBReg32(ssd, SS_REG_DFOFFSET,    dfoffset) );
   XLRTEST( XLRWriteDBReg32(ssd, SS_REG_PACKETSIZE,  packetsize + 0x8000000) );
   XLRTEST( XLRWriteDBReg32(ssd, SS_REG_PACKETSIZE2, packetsize + 0x8000000) );
   XLRTEST( XLRWriteDBReg32(ssd, SS_REG_PSNOFFSET,   0) );
   XLRTEST( XLRWriteDBReg32(ssd, SS_REG_MACFILTER,   0x10) ); // promiscuous mode, no MAC filtering
   XLRTEST( XLRWriteDBReg32(ssd, SS_REG_MACFLT_CTRL, 0x01) ); // '000'==psnMode0, '010'==psnMode1, '100'==psnMode2
   for (int i=0; i<SS_MAX_MAC_ADDRESSES; i++)
   {
      XLRTEST( XLRWriteDBReg32(ssd, SS_REG_MACADDRBASE + 2*i,   0) );
      XLRTEST( XLRWriteDBReg32(ssd, SS_REG_MACADDRBASE + 2*i+1, 0) );
   }

   return ssd;
}

int streamstorCleanup(SSHANDLE ssd)
{
   XLRTEST( XLRStop(ssd) );
   XLRTEST( XLRSetMode(ssd, SS_MODE_SINGLE_CHANNEL) );
   XLRTEST( XLRClearChannels(ssd) );
   XLRTEST( XLRSelectChannel(ssd, 0) );
   XLRTEST( XLRBindOutputChannel(ssd, 0) );
   XLRClose(ssd);
   return 0;
}

int streamstorRecord(SSHANDLE ssd, FILE* fout, size_t bytes, size_t dataframesize, size_t dataframeoffset)
{
   UINT32 numrejected = 0;
   const size_t ipheader = 24;
   const size_t psnlength = (dataframeoffset > 32) ? (dataframeoffset-32) : 0;
   const size_t ethernetsize = dataframesize + ipheader + psnlength;
   const size_t bufsize = 2*ethernetsize * 128;
   const size_t bufpayload = 2*dataframesize * 128;

   // start capture
   XLRTEST2( XLRRecord(ssd, 1, 1) );

   // read fifo
   unsigned int *buf = (unsigned int*)memalign(65536, bufsize);
   while (bytes>0 && !die) {

      size_t rdsize = bufsize;
      rdsize -= (rdsize%8);
      if (rdsize <= 0) break;

      XLR_RETURN_CODE rc = XLRReadFifo(ssd, buf, rdsize, FALSE);
      if (rc != XLR_SUCCESS) {

         printXLRError("XLRReadFIFO");
         XLRReadDBReg32(ssd, SS_REG_REJECTCOUNT, &numrejected);
         printf("Number of rejected packets: %u\n", numrejected);
         die = 1;

      } else {

         // remove Ethernet frame "garbage" from start of each packet
         //fwrite(buf, rdsize, 1, fout);
         char* in = (char*)buf;
         char* last = in + bufsize;
         while (in < last) {
            fwrite(in + ipheader + psnlength, dataframesize, 1, fout);
            in += ethernetsize;
         }

         bytes = (bytes < bufpayload) ? 0 : (bytes-bufpayload);
      }
   }
   free(buf);

   // stop
   XLRTEST( XLRStop(ssd) );

   // stats
   XLRReadDBReg32(ssd, SS_REG_REJECTCOUNT, &numrejected);
   printf("Final number of rejected packets: %u\n", numrejected);

   // purge FIFO according to StreamStor documentation
   UINT64 remain = XLRGetFIFOLength(ssd);
   fprintf(stderr, "FIFO remaining length: %lu\n", remain);
   if (remain>0) {
      unsigned int *rbuf = (unsigned int*)malloc(sizeof(int)*remain);
      XLRTEST( XLRReadFifo(ssd, rbuf, remain, FALSE) );
   }
   XLRTEST( XLRStop(ssd) );

   return 0;
}

////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
   SSHANDLE ssd;
   FILE* fout;
   int packetsize = 5008;
   int dfoffset = 40;
   char* fname = 0;
   size_t len = 0;

   if (argc != (1+4)) {
      usage();
      return -1;
   }

   packetsize = atoi(argv[1]);
   dfoffset = atoi(argv[2]);
   fname = argv[3];
   len = atoll(argv[4]);

   oldsiginthand = signal(SIGINT, siginthand);

   ssd = streamstorInit(packetsize, dfoffset);
   fout = fopen(fname, "w");
   streamstorRecord(ssd, fout, len, packetsize, dfoffset);
   streamstorCleanup(ssd);

   return 0;
}
