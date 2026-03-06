
#include "PrologixEthGPIB.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>

/** Create instance for interaction with a Prologix GPIB-Ethernet converter */
PrologixEthGPIB::PrologixEthGPIB(const char* host, int port)
{
   m_host_ipv4 = std::string(host);
   m_tcp_port = port;
   m_sockfd = 0;
   m_connected = false;
}

PrologixEthGPIB::~PrologixEthGPIB()
{
   if(m_connected) {
      shutdown(m_sockfd, SHUT_RDWR);
      close(m_sockfd);
   }
}

/** Open a TCP connection to the Prologix device */
int PrologixEthGPIB::connect()
{
   int rc;

   if (m_connected) {
      this->disconnect();
   }

   sockaddr_in remoteAddr;
   remoteAddr.sin_family = AF_INET;
   remoteAddr.sin_port = htons(m_tcp_port);
   remoteAddr.sin_addr.s_addr = INADDR_ANY;

   // Convert string like "134.104.28.45" into in-addr
   rc = inet_pton(AF_INET, m_host_ipv4.c_str(), &remoteAddr.sin_addr);
   if (rc <= 0) {
      rc = errno;
      std::cerr << "PrologixEthGPIB::connect() address " << m_host_ipv4 << " not a valid IPv4 ip address (" << strerror(rc) << ")" << std::endl;
      return -rc;
   }

   // Connect
   m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
   rc = ::connect(m_sockfd, (struct sockaddr*) &remoteAddr, sizeof(remoteAddr));
   if (rc < 0) {
      rc = errno;
      std::cerr << "PrologixEthGPIB::connect() failed to connect to " << m_host_ipv4 << " (" << strerror(rc) << ")" << std::endl;
      return -rc;
   }

   struct timeval tv;
   tv.tv_sec = 5;
   tv.tv_usec = 0;
   setsockopt(m_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

   m_connected = true;

   return 0;
}

/** Close the TCP connection to the Prologix device */
int PrologixEthGPIB::disconnect()
{
   if(m_connected) {
      shutdown(m_sockfd, SHUT_RDWR);
      close(m_sockfd);
      m_connected = false;
   }
   return 0;
}

/** Initialize the Prologix device */
int PrologixEthGPIB::configure()
{
  std::vector<std::string> PrologixConfigCommands = {
      "++savecfg 0",
      "++auto 0",
      "++mode 1",
      "++read_tmo_ms 3000",
      "++ifc",
      "++eot_enable 0",
      "++eos 3"
   };

   if (!m_connected) {
      std::cerr << "PrologixEthGPIB::configure() error: not connected" << std::endl;
      return -1;
   }

   for (auto cmd : PrologixConfigCommands) {
      int nwr = this->write(cmd);
      if (nwr <= 0) {
         std::cerr << "PrologixEthGPIB::configure() error: failed to write command '" << cmd << "'" << std::endl;
         return nwr;
      }
   }

   return 0;
}

/** Choose the GPIB device to be addressed by later read(), write() commands. */
int PrologixEthGPIB::selectDevice(int deviceNr)
{
   std::ostringstream request;
   std::string reply;
   int nwr;

   // Request an address change
   m_devnr = deviceNr;
   request << "++addr " << deviceNr;
   nwr = this->write(request.str().c_str());
   if (nwr <= 0) {
      std::cerr << "PrologixEthGPIB::selectDevice(" << deviceNr << ") error, failed to send request '" << request.str() << "'" << std::endl;
      return nwr;
   }

   // Read back now active address
   request.str("");
   request << "++addr";
   nwr = this->write(request.str().c_str());
   if (nwr <= 0) {
      std::cerr << "PrologixEthGPIB::selectDevice(" << deviceNr << ") error, failed to send query '" << request.str() << "'" << std::endl;
      return nwr;
   }

   reply = this->read(false);
   std::cerr << "PrologixEthGPIB::selectDevice(" << deviceNr << ") ok, sent request '" << request.str() << "'" << ", got response '" << reply << "'" << std::endl;

   // todo: inspect reply?

   return 0;
}


/** Write a null-terminated command string to the current GPIB device. */
int PrologixEthGPIB::write(std::string msg)
{
   int nwr;

   msg = msg + m_eol;
   nwr = ::write(m_sockfd, msg.c_str(), msg.length());

   usleep(50000); // sleep 50 millisec

   return nwr;
}


/** Read one line of text from the current GPIB device.
 * @param prologixReadmode When true sends a '++read eoi' to the Prologix controller first
 */ 
std::string PrologixEthGPIB::read(bool prologixReadmode)
{
   char rxbuf[500] = { 0 };
   const char eol_marker =  m_eol.c_str()[0];
   unsigned int nrx = 0, n;

   if (prologixReadmode) {
      this->write("++read eoi");
   }

   while (nrx < (sizeof(rxbuf)-1)) {
      n = recvfrom(m_sockfd, rxbuf + nrx, 1, 0, NULL, NULL);
      if (n <= 0) {
         break;
      }
      if (rxbuf[nrx] == eol_marker) {
         rxbuf[nrx] = '\0';
         break;
      }
      nrx++;
   }
   rxbuf[nrx + 1] = '\0';

   return std::string(rxbuf);
}
