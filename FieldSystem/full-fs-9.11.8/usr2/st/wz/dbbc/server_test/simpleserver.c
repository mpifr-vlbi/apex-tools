/* simpleserver.c */
/* http://www.welzl.at/teaching/cn-alt/unterlagen/csockets/ */

#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>
#define SERVER_PORT	1500 	/* Ein Port der auch von 'gewoehnlichen' Benutzern verwendet werden darf */
#define MSG_SIZE	256		/* Die maximale Anzahl an Zeichen, die 'msg' enhalten darf */


int main(int argc, char **argv){
	int fromsocket; /* Socket, der auf ankommende Anfragen wartet */
	int client; /* Socket fuer die Kommunikation mit einem Client */
	struct sockaddr_in fromaddr; /* Adresse die 'belauscht' werden soll */
	char msg[MSG_SIZE]; /* der Buffer fuer die Nachrichten */
	char help[MSG_SIZE] = "ACK,"; /* Hilfsvariable */
	int bytes; /* hier wird die Anzahl der gelesenen Bytes gespeichert */

	printf("%s: server is up and running ...\n", argv[0]);

	/* Socket erzeugen; verbindungsorientiert mit TCP */
	fromsocket = socket(PF_INET,SOCK_STREAM,0);
	if(fromsocket == -1){
		fprintf(stderr, "%s: cannot open socket\n", argv[0]);
		exit(1);
	}

	/* Adresse zum 'Lauschen' definieren */
	fromaddr.sin_family = PF_INET;
	fromaddr.sin_addr.s_addr =  htonl(INADDR_ANY);
 /* eine Anfrage kann von jeder IPAdresse kommen */
	fromaddr.sin_port = htons(SERVER_PORT);

	/* Socket an die Adresse und den Port binden */
	if(bind(fromsocket, (struct sockaddr *)&fromaddr, sizeof(fromaddr)) == -1){
		fprintf(stderr, "%s: cannot bind socket\n", argv[0]);
		exit(1);
	}

	/* Socket 'horchen' lassen; die maximale Laenge der Warteschlange wird auf 3 gesetzt */
	if(listen(fromsocket, 3) == -1){
		fprintf(stderr, "%s: unable to listen\n", argv[0]);
		exit(1);
	}

	/* Auf Anfragen warten */
	for(;;){ /* Endlosschleife; "forever" */
		/* Socket fuer ankommende Anfrage erzeugen */
		if((client = accept(fromsocket, NULL, NULL)) == -1){
			fprintf(stderr, "%s: error while receiving request\n", argv[0]);
			exit(1);
		}

		/* Nachricht vom Client lesen */
		if((bytes = recv(client, msg, sizeof(msg), 0)) == -1){
			fprintf(stderr, "%s: error while receiving reply\n", argv[0]);
			exit(1);
		}
		/* es handelt sich um einen String,
                  also darf man das abschliessende '\0'
		nicht vergessen */
		msg[bytes] = '\0';

		printf("%s: received request from '%s'\n", argv[0], msg);

		/* vor die empfangene Nachricht "ACK, " stellen */
		strcat(help, msg);
		strcpy(msg, help);
		strcpy(help, "ACK,");

		/* Antwort an Client senden */
		if(send(client, msg, strlen(msg), 0) == -1){
			fprintf(stderr, "%s: error while replying\n", argv[0]);
			exit(1);
		}

		/* den Client-Socket freigeben */
		close(client);
	}

	exit(0);
	return 0;
}
