#ifndef PrologixEthGPIB_H
#define PrologixEthGPIB_H

#include <string>

class PrologixEthGPIB {

public:

   /** Create instance for interaction with a Prologix GPIB-Ethernet converter */
   PrologixEthGPIB(const char* host, int port = 1234);

   ~PrologixEthGPIB();

   /** Open a TCP connection to the Prologix device */
   int connect();

   /** Close the TCP connection to the Prologix device */
   int disconnect();

   /** Initialize the Prologix device */
   int configure();

   /** Choose the GPIB device to be addressed by later read(), write() commands. */
   int selectDevice(int deviceNr);

   /** Write a command string to the current GPIB device. */
   int write(std::string cmd);

   /** Read one line of text from the current GPIB device.
    * @param prologixReadmode When true sends a '++read eoi' to the Prologix controller first
    */
   std::string read(bool prologixReadmode = true);

   /** Return true if network connection to Prologix converter is open */
   bool isConnected() const { return m_connected; }

private:
   std::string m_host_ipv4;
   bool m_connected;
   int m_tcp_port;
   int m_sockfd;

private:
   const std::string m_eol = "\n";
   int m_devnr;

};

#endif
