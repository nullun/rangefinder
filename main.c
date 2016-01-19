#include <stdio.h>            // Standard Input/Output
#include <stdlib.h>           // For malloc
#include <string.h>           // For memset
#include <unistd.h>           // For close
#include <signal.h>           // Capture ctrl+c and exit gracefully
#include <sys/ioctl.h>        // Manipulate device parameters
#include <net/if.h>           // Interface structure (IFF_PROMISC)
#include <netinet/if_ether.h> // Ethernet Header declarations
#include <netinet/ip.h>       // IP Header declarations
#include <netinet/tcp.h>      // TCP Header declarations
#include <sys/socket.h>       // The meat
#include <arpa/inet.h>        // Easier to display IPs

void process_packet(unsigned char *, int);
void print_ethernet_header(unsigned char*, int);
void print_ip_header(unsigned char *, int);
void print_tcp_packet(unsigned char *, int);

char *interface = "wlan0";
int socket_raw;
FILE *logfile;
int total=0;
struct sockaddr_in source, dest;

// Disgusting hack. Rewrite.
int running = 1;
void int_handler(int dummy) {
	running = 0;
	printf("\nStopping...\n");
}

int main(void) {
	int data_size;
	socklen_t saddr_size;
	struct sockaddr saddr;

	unsigned char *buffer = (unsigned char *)malloc(65536);

	logfile = fopen("packets.log", "w");
	if (logfile == NULL) {
		printf("Unable to open file.\n");
		return 1;
	}

	// Create RAW Ethernet Socket
	socket_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	// Interface Request
	struct ifreq if_req;
	memset(&if_req, 0, sizeof(if_req));
	strncpy((char *)if_req.ifr_name, interface, strlen(interface));
	if_req.ifr_flags = IFF_UP|IFF_PROMISC;    // Interface UP and PROMISC.
	ioctl(socket_raw, SIOCSIFFLAGS, &if_req); // Set Interface

	// Apply the configuration to our socket.
	setsockopt(socket_raw, SOL_SOCKET, SO_BINDTODEVICE, (void *)&if_req, sizeof(if_req));

	if (socket_raw < 0) {
		printf("Unable to open socket.\n");
		return 1;
	}

	signal(SIGINT, int_handler);

	printf("Starting...\n");

	while(running) {
		saddr_size = sizeof(saddr);
		data_size = recvfrom(socket_raw, buffer, 65536, 0, &saddr, &saddr_size);
		if (data_size < 0) {
			printf("Unable to receive messages from socket.\n");
			return 1;
		}
		process_packet(buffer, data_size);
	}

	printf("Finished!\n");

	/* Remove Promiscuous Mode */
	if_req.ifr_flags &= ~IFF_PROMISC;         // Interface not PROMISC.
	ioctl(socket_raw, SIOCSIFFLAGS, &if_req); // Set Interface

	// Close RAW Ethernet Socket
	close(socket_raw);
	return 0;
}

void process_packet(unsigned char* buffer, int size) {
	struct iphdr *ip_header = (struct iphdr*)(buffer + sizeof(struct ethhdr));

	if (ip_header->protocol == 6) {
		++total;
		print_tcp_packet(buffer, size);
	}

	printf("Total Packets: %d\r", total);
}

void print_ethernet_header(unsigned char* buffer, int size) {
	struct ethhdr *eth_header = (struct ethhdr *)buffer;

	fprintf(logfile, "\n");
	fprintf(logfile, "Ethernet Header\n");
	fprintf(logfile, "\t|-Destination Address\t: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n", eth_header->h_dest[0], eth_header->h_dest[1], eth_header->h_dest[2], eth_header->h_dest[3], eth_header->h_dest[4], eth_header->h_dest[5]);
	fprintf(logfile, "\t|-Source Address\t: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n", eth_header->h_source[0], eth_header->h_source[1], eth_header->h_source[2], eth_header->h_source[3], eth_header->h_source[4], eth_header->h_source[5]);
	fprintf(logfile, "\t|-Protocol\t\t: %u\n", (unsigned short)eth_header->h_proto);
}

void print_ip_header(unsigned char* buffer, int size) {
	print_ethernet_header(buffer, size);

	unsigned short iphdrlen;

	struct iphdr *ip_header = (struct iphdr*)(buffer + sizeof(struct ethhdr));
	iphdrlen = ip_header->ihl*4;

	memset(&source, 0, sizeof(source));
	source.sin_addr.s_addr = ip_header->saddr;

	memset(&dest, 0, sizeof(dest));
	dest.sin_addr.s_addr = ip_header->daddr;

	fprintf(logfile, "\n");
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

	struct iphdr *ip_header = (struct iphdr*)(buffer + sizeof(struct ethhdr));
	iphdrlen = ip_header->ihl*4;

	struct tcphdr *tcp_header = (struct tcphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));

	fprintf(logfile, "\n ***** TCP PACKET *****\n");

	print_ip_header(buffer, size);

	fprintf(logfile, "\n");
	fprintf(logfile, "TCP Header\n");
	fprintf(logfile, "\t|-Source Port\t\t: %u\n", ntohs(tcp_header->source));
	fprintf(logfile, "\t|-Destination Port\t: %u\n", ntohs(tcp_header->dest));
	fprintf(logfile, "\t|-Sequence Number\t: %u\n", ntohs(tcp_header->seq));
	fprintf(logfile, "\t|-Header Length\t\t: %d DWORDS or %d BYTES\n", (unsigned int)tcp_header->doff, (unsigned int)tcp_header->doff*4);

	fprintf(logfile, "\n########################\n\n");
}

