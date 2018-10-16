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
     int iLoop=0;
     /*! Wait for requests and react on them */
    /* printf ("Server waits on connection\n");*/
     while (1)
     {  
         iLoop++;
         printf("Server waits on %d connection\n",iLoop); 
         if (uiServerSelectWait (&SServerSocket, 0, 0, vServerProcessRequest) > 1)
         {
             printf ("Server [ERROR] Wait on requests\n");
         }
     }

    return 0;
}
