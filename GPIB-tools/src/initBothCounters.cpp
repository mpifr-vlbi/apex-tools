
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

   PrologixEthGPIB prologix(prologix_ipv4addr, prologix_tcp_port);
   HPCounterGPIB counter1(prologix, /*gpib addr:*/ 3);
   HPCounterGPIB counter2(prologix, /*gpib addr:*/ 4);

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

   std::cout << "Counter with GPIB address " << counter1.getAddr() << " has ID " << counter1.identify() << std::endl;
   std::cout << "Counter with GPIB address " << counter2.getAddr() << " has ID " << counter2.identify() << std::endl;

   rc = counter1.configure();
   if (rc != 0) {
      std::cout << "Failed to configure counter with GPIB address " << counter1.getAddr() << std::endl;
      return -1;
   }

   rc = counter2.configure();
   if (rc != 0) {
      std::cout << "Failed to configure counter with GPIB address " << counter1.getAddr() << std::endl;
      return -1;
   }

   return 0;
}

