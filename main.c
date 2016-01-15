#include <stdio.h>       // Standard Input/Output
#include <stdlib.h>      // For malloc
#include <string.h>      // For memset
#include <netinet/tcp.h> // TCP Header declarations
#include <netinet/ip.h>  // IP Header declarations
#include <sys/socket.h>  // The meat
#include <arpa/inet.h>   // Easier to display IPs

void ProcessPacket(unsigned char *, int);
void print_ip_header(unsigned char *, int);
void print_tcp_packet(unsigned char *, int);

int socket_raw;
FILE *logfile;
int total=0;
struct sockaddr_in source, dest;

int main(void) {
	int data_size;
	socklen_t saddr_size;
	struct sockaddr saddr;
//	struct in_addr in;

	unsigned char *buffer = (unsigned char *)malloc(65536);

	logfile = fopen("packets.log", "w");
	if (logfile == NULL) {
		printf("Unable to open file.\n");
		return 1;
	}

	socket_raw = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	if (socket_raw < 0) {
		printf("Unable to open socket.\n");
		return 1;
	}

	while(1) {
		saddr_size = sizeof(saddr);
		data_size = recvfrom(socket_raw, buffer, 65536, 0, &saddr, &saddr_size);
		if (data_size < 0) {
			printf("Unable to receive messages from socket.\n");
			return 1;
		}
		ProcessPacket(buffer, data_size);
	}

	close(socket_raw);
	return 0;
}

void ProcessPacket(unsigned char* buffer, int size) {
	struct iphdr *ip_header = (struct iphdr*)buffer;

	if (ip_header->protocol == 6) {
		++total;
		print_tcp_packet(buffer, size);
	}

	printf("Total Packets: %d\r", total);
}

void print_ip_header(unsigned char* buffer, int size) {
	unsigned short iphdrlen;

	struct iphdr *ip_header = (struct iphdr *)buffer;
	iphdrlen = ip_header->ihl*4;

	memset(&source, 0, sizeof(source));
	source.sin_addr.s_addr = ip_header->saddr;

	memset(&dest, 0, sizeof(dest));
	dest.sin_addr.s_addr = ip_header->daddr;

	fprintf(logfile, "IP Header\n");
	fprintf(logfile, "\t|-IP Version\t\t: %d\n", (unsigned int)ip_header->version);
	fprintf(logfile, "\t|-IP Header Length\t: %d DWORDS or %d Bytes\n", (unsigned int)ip_header->ihl, ((unsigned int)(ip_header->ihl))*4);
	fprintf(logfile, "\t|-Type of Service\t: %d\n", (unsigned int)ip_header->tos);
	fprintf(logfile, "\t|-IP Total Length\t: %d Bytes (Size of Packet)\n", ntohs(ip_header->tot_len));
	fprintf(logfile, "\t|-Source IP\t\t: %s\n", inet_ntoa(source.sin_addr));
	fprintf(logfile, "\t|-Destination IP\t: %s\n", inet_ntoa(dest.sin_addr));
}

void print_tcp_packet(unsigned char* buffer, int size) {
	unsigned short iphdrlen;

	struct iphdr *ip_header = (struct iphdr *)buffer;
	iphdrlen = ip_header->ihl*4;

	struct tcphdr *tcp_header=(struct tcphdr*)(buffer + iphdrlen);

	fprintf(logfile, "\n ***** TCP PACKET *****\n\n");

	print_ip_header(buffer, size);

	fprintf(logfile, "\n");
	fprintf(logfile, "TCP Header\n");
	fprintf(logfile, "\t|-Source Port\t\t: %u\n", ntohs(tcp_header->source));
	fprintf(logfile, "\t|-Destination Port\t: %u\n", ntohs(tcp_header->dest));
	fprintf(logfile, "\t|-Sequence Number\t: %u\n", ntohs(tcp_header->seq));
	fprintf(logfile, "\t|-Header Length\t\t: %d DWORDS or %d BYTES\n", (unsigned int)tcp_header->doff, (unsigned int)tcp_header->doff*4);

	fprintf(logfile, "\n########################\n\n");
}

