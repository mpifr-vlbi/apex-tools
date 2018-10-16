
 /* aus Linux Programmierung, Kap. 14 , */
/* Kap 14,programm server4.c , Seite 516 */
/* This server allows more clients to get a connection */
#include  <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>

#define PORT 1500
int main()
{
  int server_sockfd, client_sockfd;
  int server_len, client_len, iLoop=0;
  struct sockaddr_in server_address;
  struct sockaddr_in client_address;
   printf("Testprogramm nach Buch 'Linux Programmierung', Stones-Matthew\n");
  server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
     
   server_address.sin_family = AF_INET;
   server_address.sin_addr.s_addr =htonl (INADDR_ANY);
   server_address.sin_port = htons(PORT); /*port 1500*/
   server_len = sizeof(server_address);
   bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

 /*Wait for connection; some clients*/

   listen(server_sockfd, 5);
   signal(SIGCHLD, SIG_IGN);

      while(1)   {
       char rcv_buf[252];
       char wr_buf[256];
       char buff[256];
       int  iBuflen;
       
       *(rcv_buf) = '\0';
       *(wr_buf)='\0';
       printf("server waiting, iLoop=%d\n", iLoop);
 /*connection accepted */
       iLoop++;
       client_len = sizeof(client_address);
       client_sockfd = accept(server_sockfd,
       (struct sockaddr *)&client_address, &client_len);

       /* Start child process */

         if (fork() == 0) {
       /* this is a son process */
           printf ("read from client: ");

           read(client_sockfd, &rcv_buf, 256);
            printf ("%s \n", rcv_buf);
            sleep(0);
            /* send received string back with leeding 'ACK ' */
            strcpy(wr_buf,"ACK ");
            strcat(wr_buf, rcv_buf);
            printf("MSG back to client: wr_buf=%s\n", wr_buf);
            iBuflen=strlen(wr_buf);
            write(client_sockfd,&wr_buf,iBuflen); 
                       
           close(client_sockfd);
           exit (0);
         }  
       else {
/* we are in father process, client work is done*/
       close(client_sockfd);
         }
    }
}
