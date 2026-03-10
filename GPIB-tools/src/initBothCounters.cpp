#include "HPCounterGPIB.h"
#include "PrologixEthGPIB.h"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
   // TODO: command line args?

   const char* prologix_ipv4addr = "10.0.2.93";
   const int prologix_tcp_port = 1234;
   int rc;

   std::vector<HPCounterGPIB*> counters;
   PrologixEthGPIB prologix(prologix_ipv4addr, prologix_tcp_port);
   counters.push_back(new HPCounterGPIB(prologix, 3));
   counters.push_back(new HPCounterGPIB(prologix, 4));

   rc = prologix.connect();
   if (rc != 0) {
      std::cout << "Failed to connect to " << prologix_ipv4addr << " TCP port " << prologix_tcp_port << std::endl;
      return -1;
   }

   rc = prologix.configure();
   if (rc != 0) {
      std::cout << "Failed to configure Prologix GPIB-LAN converter" << std::endl;
      return -1;
   }

   for(auto counter : counters) {
      std::cout << "Counter with GPIB address " << counter->getAddr() << " has ID " << counter->identify() << std::endl;
      rc = counter->configure();
      if (rc != 0) {
         std::cout << "Failed to configure counter with GPIB address " << counter->getAddr() << std::endl;
         return -1;
      }
   }

   return 0;
}

