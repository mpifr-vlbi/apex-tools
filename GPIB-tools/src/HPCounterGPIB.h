#ifndef HPCounterGPIB_h
#define HPCounterGPIB_h

#include <string>

class PrologixEthGPIB;

class HPCounterGPIB {

public:

   /** Create instance of HP counter residing behind given Prologix GPIB-Ethernet converter */
   HPCounterGPIB(PrologixEthGPIB& host, int gpib_dev_nr) : m_host(host),m_devnr(gpib_dev_nr),m_verbose(true) { }
   ~HPCounterGPIB() { }

   /** Read ID string from HP counter */
   std::string identify();

   /** Initialize the HP counter */
   int configure();

   /** Send a query command and return its response */
   std::string query(std::string);

   /** Return GPIB device address */
   int getAddr() const { return m_devnr; }

private:
   PrologixEthGPIB& m_host;
   int m_devnr;
   bool m_verbose;

};

#endif
