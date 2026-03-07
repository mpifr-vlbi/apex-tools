#include "HPCounterGPIB.h"
#include "PrologixEthGPIB.h"

#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include <unistd.h>

int main(int argc, char** argv)
{
   // TODO: command line args?

   const char* prologix_ipv4addr = "10.0.2.93";
   const int prologix_tcp_port = 1234;
   bool reconfigure = false;
   int rc;

   int samplePeriodSec = 10;
   char *outfilename = strdup("pollCounters_both.out");

   std::vector<HPCounterGPIB*> counters;
   PrologixEthGPIB prologix(prologix_ipv4addr, prologix_tcp_port);
   counters.push_back(new HPCounterGPIB(prologix, 3));
   counters.push_back(new HPCounterGPIB(prologix, 4));

   rc = prologix.connect();
   if (rc != 0) {
      std::cout << "Failed to connect to " << prologix_ipv4addr << " TCP port " << prologix_tcp_port << std::endl;
      exit(1);
   }

   //=============================================================================================
   //=== CONFIGURE THE Prologix & GPIB DEVICE(s)                                               ===
   //=============================================================================================

   if (reconfigure) {
      rc = prologix.configure();
      if (rc != 0) {
         std::cout << "Failed to configure Prologix GPIB-LAN converter" << std::endl;
         exit(2);
      }
      for(auto counter : counters) {
         std::cout << "Counter with GPIB address " << counter->getAddr() << " has ID " << counter->identify() << std::endl;
         rc = counter->configure();
         if (rc != 0) {
            std::cout << "Failed to configure counter with GPIB address " << counter->getAddr() << std::endl;
            exit(3);
         }
      }
   }

   prologix.disconnect();

   //=============================================================================================
   //=== MEASUREMENT PHASE                                                                     ===
   //=============================================================================================

   FILE *fp = fopen(outfilename, "w");
   if (fp == NULL) {
      printf("Trouble opening output file %s\n", outfilename);
      exit(4);
   }
   std::cout << "Logging to " << outfilename << std::endl;

   time_t timeStart;
   time(&timeStart);

   int iter = 0;

   while (true) {

      time_t timeNow;
      time(&timeNow);
      double elapsedTime = difftime(timeNow, timeStart);

      std::cout << "Measurement " << iter << " at " << (long int)elapsedTime << " sec, time " << timeNow << ", readings ";
      fprintf(fp, "%-13zu %-8.1f ", (size_t)timeNow, elapsedTime);

      prologix.connect();
      for(auto counter : counters) {
          std::string value_str = counter->query("FETCH:TINT?");

          double value_f64 = 0;
          sscanf(value_str.c_str(), "%lf", &value_f64);

          std::cout << value_str << " ";
          fprintf(fp, "%12.9le ", value_f64);
      }
      prologix.disconnect();

      std::cout << std::endl;
      fprintf(fp, "\n");
      fflush(fp);

      sleep(samplePeriodSec);
      iter++;
   }

   return 0;
}
