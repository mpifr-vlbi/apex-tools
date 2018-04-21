/* simpleudpclient.c */
/* http://www.welzl.at/teaching/cn-alt/unterlagen/csockets/ */

#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>

#define SERVER_PORT	1500	/* Ein Port der auch von 'gewöhnlichen' Benutzern verwendet werden darf */
#define MSG_SIZE	128		/* Die maximale Anzahl an Zeichen, die 'msg' enhalten darf */

int main(int argc, char **argv){
	int tosocket; /* der Socket, der zum Senden an den Server verwendet wird */
	/* struct sockaddr_in wird in netinet/in.h beschrieben */
	struct sockaddr_in toaddr; /* die Adresse des Servers wird hier gespeichert */
	char msg[MSG_SIZE]; /* der Buffer der für die Nachricht verwendet wird */
	int bytes; /* die Anzahl der gelesenen Bytes wird hier gespeichert */

	/* Kommandozeilenargumente überprüfen */
	if(argc != 3){
		fprintf(stderr, "usage : %s <serveraddr> <clientname> \n", argv[0]);
		exit(1);
	}

	printf("%s: sending data to '%s'\n", argv[0], argv[1]);

	/* zu sendende Nachricht erstellen */
	strcpy(msg, argv[2]);

	/* Socket erzeugen; mit UDP */
	tosocket = socket(PF_INET,SOCK_DGRAM,0);
	if(tosocket == -1){
		fprintf(stderr, "%s: cannot open socket\n", argv[0]);
		exit(1);
	}

	/* Socket binden */
	toaddr.sin_family = PF_INET;
	toaddr.sin_addr.s_addr = INADDR_ANY;
	toaddr.sin_port = 0;
	if(bind(tosocket, (struct sockaddr *)&toaddr, sizeof(toaddr)) == -1){
		fprintf(stderr, "%s: cannot bind socket\n", argv[0]);
		exit(1);
	}

	/* Adresse des Servers definieren */
	toaddr.sin_addr.s_addr = inet_addr(argv[1]);
	toaddr.sin_port = htons(SERVER_PORT);
	/* Nachricht senden */
	if(sendto(tosocket, msg, sizeof(msg), 0, (struct sockaddr *)&toaddr, sizeof(toaddr)) == -1){
		fprintf(stderr, "%s: error while sending request to '%s'\n", argv[0], argv[1]);
		exit(1);
	}

	/* Antwort vom Server empfangen */
	if((bytes = recvfrom(tosocket, msg, sizeof(msg), 0, NULL, NULL)) == -1){
		fprintf(stderr, "%s: error while receiving reply\n", argv[0]);
		exit(1);
	}
	msg[bytes] = '\0'; /* Man kann nicht davon ausgehen, dass der Sender
	den String korrekt abgeschlossen hat, deshalb muss man beim empfangen
	von Strings immer '\0' anhängen */

	printf("%s: received message '%s' from '%s'\n", argv[0], msg, argv[1]);

	close(tosocket); /* stattdessen könnte man auch mit exit() aussteigen;
	dabei werden alle Sockets automatisch geschlossen */
	/* exit(0); */
	return 0;
}
