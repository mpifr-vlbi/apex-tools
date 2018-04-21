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
*   - Compile/link example with "gcc -o simple_socket -D__SIMPLE_SOCKET_MAIN__ simple_socket.c"
*
***********************************************************************
* Defined precompiler definitions: - <br>
* Defined namespaces: - <br>
* Defined exeption-handles: - <br>
***********************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#include "simple_socket.h"

fd_set g_STFDSet;  /*! Filedescriptor set to handle server filedescriptors */
static unsigned int g_uiOpenFD[FD_SETSIZE]; /*! An array to mark open filedescriptors */

/***********************************************************************
    function  uiClientOpenSocket                                            
 ***********************************************************************/
/*!           Open socket at client side
    \param    char * pcIP -> String with IP-address
    \param    unsigned int uiPort -> Port for connection (server must listen to that)
    \param    unsigned int uiSocketProtocol -> Define used protocol (0=UDP, 1=TCP)
    \param    SimpleSocketType * SSocket <- If open succeeded the resulting socket descriptor
    \return   unsigned int <- Errorcode (0 = ok, >0 = error)     
 ***********************************************************************/
/*  author    Alexander Neidhardt                                  
    date      11.09.2008                                          
    revision  -            
    info      -              
 ***********************************************************************/
unsigned int uiClientOpenSocket (char * pcIP, unsigned int uiPort, unsigned int uiSocketProtocol, SimpleSocketType * SSocket)
{
    struct sockaddr_in STBindAddress;
    struct hostent * hostent_ptr;
    
    /*! Set socket protocol */
    SSocket->uiSocketProtocol = uiSocketProtocol;
    
    /*! Create socket */
    if (uiSocketProtocol == SIMPLE_SOCKET_UDP)
    {
        /*! UDP */
        if ((SSocket->iSockFD = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
        {
            return 1;
        }
        
	    STBindAddress.sin_family = AF_INET;
	    STBindAddress.sin_addr.s_addr = htonl (INADDR_ANY);
	    STBindAddress.sin_port = 0;
        if (bind (SSocket->iSockFD, (struct sockaddr *) & STBindAddress, sizeof (STBindAddress)) == -1)
        {
            close (SSocket->iSockFD);
            return 2;
        }
        
        /*! Define address of server */
        SSocket->STServerAddress.sin_family = AF_INET;
        SSocket->STServerAddress.sin_port = htons(uiPort);
        if ((hostent_ptr = gethostbyname(pcIP)) == NULL)
        {
            close (SSocket->iSockFD);
            return 2;
        }
        bcopy ( hostent_ptr->h_addr, &(SSocket->STServerAddress.sin_addr.s_addr), hostent_ptr->h_length);
    }
    else
    {
        /*! TCP */
        if ((SSocket->iSockFD = socket (AF_INET, SOCK_STREAM, 0)) == -1)
        {
            return 1;
        }

        /*! Derive address information and port */ 
        SSocket->STServerAddress.sin_family = AF_INET;
        SSocket->STServerAddress.sin_port = htons(uiPort);
        if ((hostent_ptr = gethostbyname(pcIP)) == NULL)
        {
            close (SSocket->iSockFD);
            return 2;
        }
        bcopy ( hostent_ptr->h_addr, &(SSocket->STServerAddress.sin_addr.s_addr), hostent_ptr->h_length);

        /*! Connect address with socket */
        if ((connect (SSocket->iSockFD, (struct sockaddr *) & (SSocket->STServerAddress), sizeof (SSocket->STServerAddress))) == -1)
        {
            close (SSocket->iSockFD);
            return 3;
        }
    }
    
    return 0;
}

/***********************************************************************
    function  uiServerInitFDSet                                            
 ***********************************************************************/
/*!           Init administration for server
    \param    -
    \return   unsigned int <- Errorcode (0 = ok, >0 = error)     
 ***********************************************************************/
/*  author    Alexander Neidhardt                                  
    date      11.09.2008                                          
    revision  -            
    info      -              
 ***********************************************************************/
unsigned int uiServerInitFDSet ()
{
    unsigned int uiIndex;
     
    FD_ZERO (&g_STFDSet);
    for (uiIndex = 0; uiIndex < FD_SETSIZE; uiIndex++)
        g_uiOpenFD[uiIndex] = 0;
    return 0;
}

/***********************************************************************
    function  uiServerOpenSocket                                            
 ***********************************************************************/
/*!           Open socket at server side
    \param    unsigned int uiPort -> Port for connection the server should listen to
    \param    unsigned int uiSocketProtocol -> Define used protocol (0=UDP, 1=TCP)
    \param    SimpleSocketType * SSocket <- If open succeeded the resulting socket descriptor
    \return   unsigned int <- Errorcode (0 = ok, >0 = error)     
 ***********************************************************************/
/*  author    Alexander Neidhardt                                  
    date      11.09.2008                                          
    revision  -            
    info      -              
 ***********************************************************************/
unsigned int uiServerOpenSocket (unsigned int uiPort, unsigned int uiSocketProtocol, SimpleSocketType * SSocket)
{
    SSocket->uiSocketProtocol = uiSocketProtocol;

    /*! Create socket */ 
    if (uiSocketProtocol == SIMPLE_SOCKET_UDP)
    {
        /*! UDP */
        if ((SSocket->iSockFD = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
        {
            return 1;
        }
        
        /*! Bind socket to address */
        SSocket->STServerAddress.sin_family = AF_INET;
        SSocket->STServerAddress.sin_addr.s_addr = htonl (INADDR_ANY);
        SSocket->STServerAddress.sin_port = htons (uiPort);
        if (bind (SSocket->iSockFD, (struct sockaddr *) &(SSocket->STServerAddress), sizeof (SSocket->STServerAddress)) == -1)
        {
            close (SSocket->iSockFD);
            return 2;
        }
    }
    else
    {
        /*! TCP */
        if ((SSocket->iSockFD = socket (AF_INET, SOCK_STREAM, 0)) == -1)
        {
            return 1;
        }
        
        /*! Bind socket to address */
        SSocket->STServerAddress.sin_family = AF_INET;
        SSocket->STServerAddress.sin_addr.s_addr = htonl (INADDR_ANY);
        SSocket->STServerAddress.sin_port = htons (uiPort);
        if (bind (SSocket->iSockFD, (struct sockaddr *) & (SSocket->STServerAddress), sizeof (SSocket->STServerAddress)) == -1)
        {
            close (SSocket->iSockFD);
            return 2;
        }
        
        /*! Activate the listen on socket */
        if (listen (SSocket->iSockFD, 5) == -1)
        {
            close (SSocket->iSockFD);
            return 3;
        }
    }
    
    FD_SET (SSocket->iSockFD, &g_STFDSet);
    g_uiOpenFD[SSocket->iSockFD] = 1;
    
    return 0;
}

/***********************************************************************
    function  uiCloseSocket                                            
 ***********************************************************************/
/*!           Close dedicated socket in general
    \param    SimpleSocketType * SSocket -> Socket descriptor to close
    \return   unsigned int <- Errorcode (0 = ok, >0 = error)     
 ***********************************************************************/
/*  author    Alexander Neidhardt                                  
    date      11.09.2008                                          
    revision  -            
    info      -              
 ***********************************************************************/
unsigned int uiCloseSocket (SimpleSocketType * SSocket)
{
    /*! Close socket filedescriptor */
    if (close (SSocket->iSockFD) == -1)
        return 1;
    
    return 0;
}

/***********************************************************************
    function  uiSend                                            
 ***********************************************************************/
/*!           Send message
    \param    SimpleSocketType * SSocket -> Socket descriptor to write to
    \param    char * pcBuffer -> Binary buffer with message
    \param    unsigned long ulBufferLength -> Total length of message
    \return   unsigned int <- Errorcode (0 = ok, >0 = error)     
 ***********************************************************************/
/*  author    Alexander Neidhardt                                  
    date      11.09.2008                                          
    revision  -            
    info      -              
 ***********************************************************************/
unsigned int uiSend (SimpleSocketType * SSocket, char * pcBuffer, unsigned long ulBufferLength)
{
    long lRetVal = 0;
    
    /*! Write to socket */
    if (SSocket->uiSocketProtocol != SIMPLE_SOCKET_UDP)
    {
        if ((lRetVal = write (SSocket->iSockFD, pcBuffer, ulBufferLength)) == -1 ||
            (unsigned long) lRetVal != ulBufferLength)
        {
            return 1;
        }

    }
    else
    {
        if (((lRetVal = sendto(SSocket->iSockFD, pcBuffer, ulBufferLength, 0, (struct sockaddr *)&(SSocket->STServerAddress), sizeof(SSocket->STServerAddress))) == -1) || 
             (unsigned long) lRetVal != ulBufferLength)
        {
                return 1;
        }
    }
    
    return 0;
}

/***********************************************************************
    function  uiReceive                                            
 ***********************************************************************/
/*!           Receive message
    \param    SimpleSocketType * SSocket <-> Socket descriptor to read from
    \param    char * pcBuffer <- Binary buffer with message
    \param    unsigned long * ulBufferLength <-> Must first contain the buffer size and is then filled with the read length
    \return   unsigned int <- Errorcode (0 = ok, >0 = error)     
 ***********************************************************************/
/*  author    Alexander Neidhardt                                  
    date      11.09.2008                                          
    revision  -            
    info      -              
 ***********************************************************************/
unsigned int uiReceive (SimpleSocketType * SSocket, char * pcBuffer, unsigned long * ulBufferLength)
{
    long lRetVal = 0;
    unsigned long ulIndex;
    int iLength;
    
    /*! Init buffer */
    for (ulIndex = 0; ulIndex < *ulBufferLength; ulIndex++)
        pcBuffer[ulIndex] = '\0';
    
    /*! Read into buffer */
    if (SSocket->uiSocketProtocol != SIMPLE_SOCKET_UDP)
    {
        if ((lRetVal = read (SSocket->iSockFD, pcBuffer, *ulBufferLength)) == -1)
            return 1;
    }
    else
    {
        if ((lRetVal = recvfrom (SSocket->iSockFD, pcBuffer, *ulBufferLength, 0, (struct sockaddr *)&(SSocket->STServerAddress), &iLength)) == -1)
            return 1;
    }

    /*! Set real read length */
    *ulBufferLength = (unsigned long) lRetVal;
    return 0;
}

/***********************************************************************
    function  uiServerSelectWait                                            
 ***********************************************************************/
/*!           Receive message
    \param    SimpleSocketType * SSocket -> Socket to listen to
    \param    unsigned long ulTimeoutSec -> Possible timeout in sec in combination with ulTimeoutUSec (both equal 0 means no timeout)
    \param    unsigned long ulTimeoutUSec -> Possible timeout in usec in combination with ulTimeoutSec (both equal 0 means no timeout)
    \param    void (*fp)(int) -> Function pointer to a callback function which is called when a message is detected
    \return   unsigned int <- Errorcode (0 = ok, 1 = timeout, >1 = error)     
 ***********************************************************************/
/*  author    Alexander Neidhardt                                  
    date      11.09.2008                                          
    revision  -            
    info      -              
 ***********************************************************************/
unsigned int uiServerSelectWait (SimpleSocketType * SSocket, unsigned long ulTimeoutSec, unsigned long ulTimeoutUSec, void (*fp)(SimpleSocketType))
{
    int iSelectRetVal;                   /*! Return value of select */
    int iFD;                             /*! Variable filedescriptors */
    int iRead;                           /*! Flag to check if a client wants a connection close */
    int iClientAddrLength;               /*! Size of client address info length */
    int iFDNewClient;                    /*! Filedescriptor for a new client */
    struct timeval STTimeout;            /*! Timeout settings for select */
    static struct sockaddr_in STClientAddress;  /*! The client address info of a new client */
    SimpleSocketType SNewPortSocket;
    
    SNewPortSocket = *SSocket;

    if (ulTimeoutSec == 0 && ulTimeoutUSec == 0)
    {
        /*! No time out in select */
        iSelectRetVal = select (FD_SETSIZE, &g_STFDSet, (fd_set *) 0, (fd_set *) 0, NULL);

        /*! Error while select */
        if (iSelectRetVal < 0)
        {
            return 2;
        }

        /*! Timeout */
        if (iSelectRetVal == 0)
        {
            return 2;
        }
    }
    else
    {
        /*! Select with timeout */
        STTimeout.tv_sec = ulTimeoutSec; 
        STTimeout.tv_usec = ulTimeoutUSec;
        iSelectRetVal = select (FD_SETSIZE, &g_STFDSet, (fd_set *) 0, (fd_set *) 0, &STTimeout);

        /*! Error while select */
        if (iSelectRetVal < 0)
        {
            return 2;
        }

        /*! Timeout */
        if (iSelectRetVal == 0)
        {
            return 1;
        }
    }
    /*! Check filedesctriptors */
    for (iFD = 0; iFD < FD_SETSIZE; iFD++)
    {
        if (FD_ISSET (iFD, &g_STFDSet))
        {
            if (SSocket->uiSocketProtocol == SIMPLE_SOCKET_UDP)
            {
                /*! UDP directly calls callback function */
                (*fp)(*SSocket);
            }
            else
            {
                if (iFD == SSocket->iSockFD)
                {
                    /*! A new client wants to get in contact => accept and insert into filedescriptor set*/
                    iClientAddrLength = sizeof (STClientAddress);
                
                    iFDNewClient = accept (iFD, (struct sockaddr *) & STClientAddress, &iClientAddrLength);
                    FD_SET (iFDNewClient, &g_STFDSet);

                    /*! Set in the filedescriptor array */
                    if (iFDNewClient >= FD_SETSIZE)
                    {
                        return 2;
                    }
                    g_uiOpenFD[iFD] = 1;
                }
                else
                {
                    /*! Check if a client wants to stop communication */
                    ioctl (iFD, FIONREAD, &iRead);
                    if (iRead == 0)
                    {
                        /*! Close communication */
                        close (iFD);
                        FD_CLR (iFD, &g_STFDSet);
                        g_uiOpenFD[iFD] = 0;
                    }
                    else
                    {
                        /*! Client wants to communicate => call function */
                        SNewPortSocket.iSockFD = iFD;
                        (*fp)(SNewPortSocket);
                        return 0;
                    }
                }
            }
        }
    }
    
    return 0;
}

/***********************************************************************
    function  vServerCloseAllOpenFD                                            
 ***********************************************************************/
/*!           Close all server socket filedescriptors
    \param    -
    \return   -    
 ***********************************************************************/
/*  author    Alexander Neidhardt                                  
    date      11.09.2008                                          
    revision  -            
    info      -              
 ***********************************************************************/
void vServerCloseAllOpenFD ()
{
    unsigned int uiFD;
    for (uiFD = 0; uiFD < FD_SETSIZE; uiFD++)
    {
        g_uiOpenFD[uiFD] = 0;
        close (uiFD);
    }    
}

/**********************************************************************
 * TEST-main
 * Compile with gcc -D__SIMPLE_SOCKET_MAIN__ simple_socket.c
 **********************************************************************/
#ifdef __SIMPLE_SOCKET_MAIN__
#define SIMPLE_SOCKET_PROTOCOL SIMPLE_SOCKET_UDP
#define SIMPLE_SOCKET_PORT     1500

static int iClientSockFD; /*! Client filedescriptor to handle it within vClientTermHandler (...) */
    
/***********************************************************************
    function  vClientTermHandler                                            
 ***********************************************************************/
/*!           Own handler to handle client terminations with CTRL-C
              so that all opened descriptors would be closed properly  
    \param    int signal -> Signal identifier 
    \return   -    
 ***********************************************************************/
/*  author    Alexander Neidhardt                                  
    date      11.09.2008                                          
    revision  -            
    info      -              
 ***********************************************************************/
void vClientTermHandler(int signal)
{
    close (iClientSockFD);
    exit(0);
}

/***********************************************************************
    function  vServerTermHandler                                            
 ***********************************************************************/
/*!           Own handler to handle server terminations with CTRL-C
              so that all opened descriptors would be closed properly  
    \param    int signal -> Signal identifier 
    \return   -    
 ***********************************************************************/
/*  author    Alexander Neidhardt                                  
    date      11.09.2008                                          
    revision  -            
    info      -              
 ***********************************************************************/
void vServerTermHandler(int signal)
{
    vServerCloseAllOpenFD ();
    exit(0);
}

/***********************************************************************
    function  vServerProcessRequest                                            
 ***********************************************************************/
/*!           Callback function which is call from server function  
    \param    SimpleSocketType SSocket -> Socket descriptor which threw a communication event
    \return   -    
 ***********************************************************************/
/*  author    Alexander Neidhardt                                  
    date      11.09.2008                                          
    revision  -            
    info      -              
 ***********************************************************************/
void vServerProcessRequest (SimpleSocketType SSocket)
{
    char acServerBuffer[256];
    unsigned long ulServerReadBufferLength;
    int iShutdown = 0;

    /*! Receive message */
    printf ("Server receive msg: ");
    ulServerReadBufferLength = 256;
    if (uiReceive (&SSocket, acServerBuffer, &ulServerReadBufferLength))
    {
        printf ("\n[ERROR] Server receive msg\n");
    }
    printf ("%s\n", acServerBuffer);
    if (!strcmp (acServerBuffer, "CLIENT: shutdown"))
        iShutdown = 1;
    
    /*! Send acknowledge message */
    sprintf (acServerBuffer, "Server: ACK");
    printf ("Server send msg: %s\n", acServerBuffer);
    if (uiSend (&SSocket, acServerBuffer, strlen(acServerBuffer)+1))
    {
        printf ("[ERROR] Server send msg: %s\n", acServerBuffer);
    }
    
    /*! When shutdown detected => shutdown server */
    if (iShutdown)
    {
        printf ("Server shutdown\n");
        vServerTermHandler(SIGQUIT);
        exit(0);
    }
}

/***********************************************************************
    function  main                                            
 ***********************************************************************/
/*!           Main function 
    \param    -
    \return   int <- Errorcode (0=ok, 1=error)   
 ***********************************************************************/
/*  author    Alexander Neidhardt                                  
    date      11.09.2008                                          
    revision  -            
    info      -              
 ***********************************************************************/
int main ()
{
    pid_t iProcessID;
    int iClientMessageCounter = 0;
    char acClientBuffer[256];
    unsigned long ulClientReadBufferLength;
    SimpleSocketType SClientSocket;
    SimpleSocketType SServerSocket;
    
    printf ("Start of test ...\n");
    
    if ((iProcessID = fork ()) == 0)
    {
        /****************************************************************/
        /*! SON-process = client                                         */
        /****************************************************************/
        /*! Set termination signal handler */
        signal (SIGKILL, vClientTermHandler);
        signal (SIGQUIT, vClientTermHandler);
        signal (SIGTERM, vClientTermHandler);
        signal (SIGINT, vClientTermHandler);
        printf ("Son (Client) started\n");
        printf ("Client sleeps 1 sec\n");
        sleep (1);
        /*! Open socket */
        printf ("Client open socket\n");
        if (uiClientOpenSocket ("127.0.0.1", SIMPLE_SOCKET_PORT, SIMPLE_SOCKET_PROTOCOL, &SClientSocket))
        {
            printf ("[ERROR] Client opens socket\n");
            return 1;
        }
        iClientSockFD = SClientSocket.iSockFD;
        /*! Repead send five times */
        for (iClientMessageCounter = 0; iClientMessageCounter < 5; iClientMessageCounter++)
        {
            /*! Last message sends shutdown order */
            if (iClientMessageCounter == 4)
            {
                sprintf (acClientBuffer, "CLIENT: shutdown");
            }
            else
            {
                sprintf (acClientBuffer, "CLIENT: This is msg-no. %d", iClientMessageCounter+1);
            }

            /*! Send message */
            printf ("Client send msg: %s\n", acClientBuffer);
            if (uiSend (&SClientSocket, acClientBuffer, strlen(acClientBuffer)+1))
            {
                printf ("Client [ERROR] Client send msg: %s\n", acClientBuffer);
                break;
            }
            printf ("Client receive msg: ");
            /*! Receive acknowledge message */
            ulClientReadBufferLength = 256;
            if (uiReceive (&SClientSocket, acClientBuffer, &ulClientReadBufferLength))
            {
                printf ("\nClient [ERROR] Client receive msg\n");
                break;
            }
            printf ("%s\n", acClientBuffer);
            /*! Sleep until next send */
            printf ("Client sleeps 1 sec\n");
            if (iClientMessageCounter < 4)
            {
                sleep (1);
            }
        }
        printf ("Client close socket\n");
        if (uiCloseSocket (&SClientSocket))
        {
            printf ("[ERROR] Client closes socket\n");
        }
    }
    else
    {
        if (iProcessID != -1)
        {
            /****************************************************************/
            /*! FATHER-process = server                                      */
            /****************************************************************/
            /*! Set termination signal handler */
            signal (SIGKILL, vServerTermHandler);
            signal (SIGQUIT, vServerTermHandler);
            signal (SIGTERM, vServerTermHandler);
            signal (SIGINT, vServerTermHandler);
            printf ("Father (Server) started\n");
            /*! Open listen socket */
            printf ("Server open socket\n");
            if (uiServerOpenSocket (SIMPLE_SOCKET_PORT, SIMPLE_SOCKET_PROTOCOL, &SServerSocket))
            {
                printf ("Server [ERROR] Open socket\n");
                return 1;
            }
            
            /*! Wait for requests and react on them */
            printf ("Server waits on connection\n");
            while (1)
            {
                if (uiServerSelectWait (&SServerSocket, 0, 0, vServerProcessRequest) > 1)
                {
                    printf ("Server [ERROR] Wait on requests\n");
                }
            }
        }
        else
        {
            printf ("[ERROR] Fork failed\n");
            return 1;
        }
    }
    return 0;
}
#endif

/***********************************************************************
* END OF FILE (CVS Concurrent Versions Control)
* ---------------------------------------------------------------------
* $RCSfile:  $
* $Revision:  $
***********************************************************************/
