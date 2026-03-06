#include "HPCounterGPIB.h"
#include "PrologixEthGPIB.h"

#include <iostream>
#include <string>
#include <vector>


/** Read ID string from counter */
std::string HPCounterGPIB::identify()
{
   if (!m_host.isConnected()) {
      std::cerr << "HPCounterGPIB::identify() of device " << m_devnr << " : configure error, no TCP connection" << std::endl;
      return std::string("");
   }

   m_host.selectDevice(m_devnr);

   m_host.write("*IDN?");
   std::string id = m_host.read(true);

   return id;
}


/** Initialize the HP counter */
int HPCounterGPIB::configure()
{
  std::vector<std::string> CounterConfigCommands = {
     ":SENS:ROSC:SOURCE:AUTO ON",
     ":INP1:ATT 1",
     ":INP1:COUP DC",
     ":INP1:IMP 1E6 OHM",
     ":EVEN1:LEVEL 1.0",
     ":EVEN1:SLOP POS",
     ":INP2:ATT 1",
     ":INP2:COUP DC",
     ":INP2:IMP 1E6 OHM",
     ":EVEN2:LEVEL 1.0",
     ":EVEN2:SLOP POS",
     ":FUNC 'TINT'",
     ":TINT:ARM:STAR:SOUR IMM",
     ":TINT:ARM:STOP:SOUR IMM",
     ":INIT:CONT ON",
   };

   if (!m_host.isConnected()) {
      std::cerr << "HPCounterGPIB::configure() of device " << m_devnr << " : configure error, no TCP connection" << std::endl;
      return -1;
   }

   m_host.selectDevice(m_devnr);

   for (auto cmd : CounterConfigCommands) {
      int nwr = m_host.write(cmd);
      if (nwr <= 0) {
         std::cerr << "HPCounterGPIB::configure() of device " << m_devnr << " failed at command '" << cmd << "'" << std::endl;
         return nwr;
      }
   }

   return 0;
}

