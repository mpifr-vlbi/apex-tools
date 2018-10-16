/* simpleudpserver.c */
/* http://www.welzl.at/teaching/cn-alt/unterlagen/csockets/ */

#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>

#define SERVER_PORT	1500 	/* Ein Port der auch von 'gewöhnlichen' Benutzern verwendet werden darf */
#define MSG_SIZE	128		/* Die maximale Anzahl an Zeichen, die 'msg' enhalten darf */


int main(int argc, char **argv){
	int fromsocket; /* Socket, der auf ankommende Anfragen wartet */
	struct sockaddr_in fromaddr, cliaddr;
	char msg[MSG_SIZE]; /* der Buffer für die Nachrichten */
	char help[MSG_SIZE]; /* Hilfsvariable */
	int bytes; /* hier wird die Anzahl der gelesenen Bytes gespeichert */
	int cli_size;

	printf("%s: server is up and running ...\n", argv[0]);

	/* Socket erzeugen; mit UDP */
	fromsocket = socket(PF_INET,SOCK_DGRAM,0);
	if(fromsocket == -1){
		fprintf(stderr, "%s: cannot open socket\n", argv[0]);
		exit(1);
	}

	/* Adresse zum Empfangen definieren */
	fromaddr.sin_family = PF_INET;
	fromaddr.sin_addr.s_addr =  INADDR_ANY; /* eine Anfrage kann von jeder IPAdresse kommen */
	fromaddr.sin_port = htons(SERVER_PORT); /* Port den der Server belegt */

	/* Socket an Port binden */
	if(bind(fromsocket, (struct sockaddr *)&fromaddr, sizeof(fromaddr)) == -1){
		fprintf(stderr, "%s: cannot bind socket\n", argv[0]);
		exit(1);
	}

	/* Auf Anfragen warten */
	for(;;){ /* Endlosschleife; "forever" */
		/* Nachricht vom Client lesen und Adresse des Client in cliaddr speichern */
		cli_size = sizeof(cliaddr);
		if((bytes = recvfrom(fromsocket, msg, sizeof(msg), 0, (struct sockaddr *)&cliaddr, &cli_size)) == -1){
			fprintf(stderr, "%s: error while receiving reply\n", argv[0]);
			exit(1);
		}
		/* es handelt sich um einen String, also darf man das abschließende '\0'
		nicht vergessen */
		msg[bytes] = '\0';

		printf("%s: received request from '%s'\n", argv[0], msg);

		/* vor die empfangene Nachricht "Hallo " stellen */
		strcpy(help, "Hallo ");
		strcat(help, msg);
		strcpy(msg, help);

		/* Antwort an Client senden */
		if(sendto(fromsocket, msg, strlen(msg), 0, (struct sockaddr *)&cliaddr, cli_size) == -1){
			fprintf(stderr, "%s: error while replying\n", argv[0]);
			exit(1);
		}
	}

	exit(0);
	return 0;
}
