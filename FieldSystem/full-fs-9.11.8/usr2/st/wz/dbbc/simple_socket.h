/***********************************************************************
* CVS Concurrent Versions Control
* ---------------------------------------------------------------------
* $RCSfile: $
* $Revision:  $
* ---------------------------------------------------------------------
* $Author: $
* $Date: $
* $Locker: $
* ---------------------------------------------------------------------
* $Log: $
*
***********************************************************************/
/*! \file
*   \brief Simple module to simplify the socket usage <br>
*
*   External functionality:<br>
*   - uiClientOpenSocket => Open socket at client side<br>
*   - uiServerInitFDSet => Init administration for server <br>
*   - uiServerOpenSocket => Open socket at server side <br>
*   - uiCloseSocket => Close dedicated socket in general
*   - uiSend => Send message
*   - uiReceive => Receive message
*   - uiServerSelectWait => Wait on communications at server side (uses internal select)
*   - vServerCloseAllOpenFD => Close all server filedescriptors
*
*   Note:<br>
*   - Compile/link with "gcc"
*
***********************************************************************
* Defined precompiler definitions: - <br>
* Defined namespaces: - <br>
* Defined exeption-handles: - <br>
***********************************************************************/

#ifndef __SIMPLE_SOCKET__
#define __SIMPLE_SOCKET__

#define SIMPLE_SOCKET_UDP 0
#define SIMPLE_SOCKET_TCP 1

#include <netinet/in.h>

typedef struct SimpleSocketStruct
{
    unsigned int uiSocketProtocol;
    int iSockFD;
    struct sockaddr_in STServerAddress;
} SimpleSocketType;

unsigned int uiClientOpenSocket (char * pcIP, unsigned int uiPort, unsigned int uiSocketProtocol, SimpleSocketType * SSocket);
unsigned int uiServerInitFDSet ();
unsigned int uiServerOpenSocket (unsigned int uiPort, unsigned int uiSocketProtocol, SimpleSocketType * SSocket);
unsigned int uiCloseSocket (SimpleSocketType * SSocket);
unsigned int uiSend (SimpleSocketType * SSocket, char * pcBuffer, unsigned long ulBufferLength);
unsigned int uiReceive (SimpleSocketType * SSocket, char * pcBuffer, unsigned long * ulBufferLength);
unsigned int uiServerSelectWait (SimpleSocketType * SSocket, unsigned long ulTimeoutSec, unsigned long ulTimeoutUSec, void (*fp)(SimpleSocketType));
void vServerCloseAllOpenFD ();

#endif /* __SIMPLE_SOCKET__ */

/***********************************************************************
* END OF FILE (CVS Concurrent Versions Control)
* ---------------------------------------------------------------------
* $RCSfile:  $
* $Revision:  $
***********************************************************************/
