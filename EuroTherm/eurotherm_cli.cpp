
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <netdb.h>
#include <math.h>
#include <unistd.h>

#define BIND_IFACE_IP "10.0.2.102"
#define EUROTHERM_IP "10.0.2.97"
#define EUROTHERM_PORT 10001

static struct sockaddr_in remote_addr;
static struct sockaddr_in local_addr;

static unsigned short int calculate_crc(unsigned char *z_p, unsigned short int z_message_length);


int udp_open()
{
    int s;
    struct timeval tv;

    remote_addr.sin_family = PF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(EUROTHERM_IP);
    remote_addr.sin_port = htons(EUROTHERM_PORT);

    s = socket(PF_INET, SOCK_DGRAM, 0);

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    local_addr.sin_family = PF_INET;
    local_addr.sin_addr.s_addr = inet_addr(BIND_IFACE_IP); //INADDR_ANY; // EUROTHERM_IP
    local_addr.sin_port = htons(EUROTHERM_PORT);

    if(bind(s, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1) {
        perror("cannot bind socket\n");
        exit(1);
    }

    return s;
}


int udp_send(int s, const void* msg, size_t msglen)
{
    int rc;
    rc = sendto(s, msg, msglen, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
    if (rc < 0) {
        perror("sendto() UDP to remote");
        exit(1);
    }
    return rc;
}


int udp_send_with_crc(int s, unsigned char* msg, size_t msglen)
{
    int rc;
    calculate_crc(msg, msglen); // appends 2-byte CRC16
    rc = sendto(s, msg, msglen+2, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
    if (rc < 0) {
        perror("sendto() UDP to remote");
        exit(1);
    }
    return rc;
}

int udp_recv(int s, void* msg, size_t maxlen)
{
    int rc;
    rc = recvfrom(s, msg, maxlen, 0, NULL, NULL);
    if (rc < 0) {
       fprintf(stderr, "error receiving reply\n");
    }
    return rc;
}




unsigned short int calculate_crc(unsigned char *z_p, unsigned short int z_message_length)
/* From Eurotherm "2000 series Communications Handbook" p37 */
/* CRC runs cyclic Redundancy Check Algorithm on input z_p */
/* Returns value of 16 bit CRC after completion and */
/* always adds 2 crc bytes to message */
/* returns 0 if incoming message has correct CRC */
{
	unsigned short int CRC= 0xffff;
	unsigned short int next;
	unsigned short int carry;
	unsigned short int n;
	unsigned char crch, crcl;
	unsigned short int messageByteIndex, messageByteCounter;
	messageByteCounter = z_message_length;
	messageByteIndex = 0;
	while (messageByteCounter--) {
		next = (unsigned short int)z_p[messageByteIndex];
		CRC ^= next;
		for (n = 0; n < 8; n++) {
			carry = CRC & 1;
			CRC >>= 1;
			if (carry) {
				CRC ^= 0xA001;
			}
		}
		messageByteIndex++;
	}
	crch = CRC / 256;
	crcl = CRC % 256;
	z_p[z_message_length++] = crcl;
	z_p[z_message_length] = crch;
	return CRC;
}


int main(void)
{
    unsigned char buf[512] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0 };

    int s = udp_open();

    buf[2] = 0x01; buf[3] = 0x26;
    udp_send_with_crc(s, buf, 6);

    udp_recv(s, buf, sizeof(buf));

    return 0;
}
