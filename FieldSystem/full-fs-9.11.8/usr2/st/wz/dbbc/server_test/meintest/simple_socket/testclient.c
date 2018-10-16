#include "simple_socket.h"
#include "signal.h"

#define SIMPLE_SOCKET_PROTOCOL SIMPLE_SOCKET_TCP
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
    for (iClientMessageCounter = 0; iClientMessageCounter < 100; iClientMessageCounter++)
    {
        /*! Last message sends shutdown order */
        if (iClientMessageCounter == 4)
        {
            sprintf (acClientBuffer, "CLIENT: This is msg-no. %d", iClientMessageCounter+1);
           /* sprintf (acClientBuffer, "CLIENT: shutdown"); */
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
        sleep (1);
    }
    printf ("Client close socket\n");
    if (uiCloseSocket (&SClientSocket))
    {
        printf ("[ERROR] Client closes socket\n");
    }

    return 0;
}
