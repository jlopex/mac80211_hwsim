/*
 ============================================================================
 Name        : server.c
 Author      : Javier Lopez
 Version     :
 Copyright   : 
 Description :
 ============================================================================
 */

#include <pthread.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <asm/types.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>
#include <stdio.h>

int sock_fd = 0; 					//Socketdescriptor
unsigned char* buff = NULL;				//Buffer
char *listener_ifname;					//src if_name

long total_answ_packets = 0;
long total_recv_packets = 0;

void sigint(int signum) {

	struct ifreq ifr;

	if (sock_fd == -1)
		return;

	strncpy(ifr.ifr_name, listener_ifname, IFNAMSIZ);
	ioctl(sock_fd, SIOCGIFFLAGS, &ifr);
	ifr.ifr_flags &= ~IFF_PROMISC;
	ioctl(sock_fd, SIOCSIFFLAGS, &ifr);
	close(sock_fd);

	free(buff);

	printf("\nClient terminating....\n");
	printf("Totally answ: %ld packets\n", total_answ_packets);
	printf("Totally recv: %ld packets\n", total_recv_packets);

	exit(0);
}



int main(int argc, char *argv[]) {

	unsigned char listener_mac[6];
	
	struct ifreq ifr_listener;

	buff = (void*)malloc(ETH_FRAME_LEN+ETH_FCS_LEN); 	//Ethernet frame
	struct ethhdr *eth_hdr = (struct ethhdr *)buff; 	//Ethernet header
	unsigned char *data_ptr = buff + ETH_HLEN;
	struct sockaddr_ll s_addr;				//struct socketaddr
	int ifindex = 0;					//Ethernet Interface index
	int sent, recv;						//length of sent packet
	int i;							//counter

	if (argc !=2)
	{
		printf("Missing arguments.\n"
				"%s [listener_ifname] \n",argv[0]);
		exit(1);
	}

	listener_ifname = argv[1];

	//open socket
	sock_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); 
	if (sock_fd == -1) {
		perror("socket():");
		exit(1);
	}
	printf("Successfully opened socket: %i\n", sock_fd);

	// ethernet interface index
	strncpy(ifr_listener.ifr_name, listener_ifname, IFNAMSIZ);
	if (ioctl(sock_fd, SIOCGIFINDEX, &ifr_listener) == -1) {
		perror("SIOCGIFINDEX");
		exit(1);
	}
	ifindex = ifr_listener.ifr_ifindex;
	printf("Successfully got interface index: %i\n", ifindex);

	// retrieve MAC
	if (ioctl(sock_fd, SIOCGIFHWADDR, &ifr_listener) == -1) {
		perror("SIOCGIFINDEX");
		exit(1);
	}
	for (i = 0; i < 6; i++) {
		listener_mac[i] = ifr_listener.ifr_hwaddr.sa_data[i];
	}
	printf("LISTENER MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",	listener_mac[0],listener_mac[1],listener_mac[2],listener_mac[3],listener_mac[4],listener_mac[5]);


	// prepare sockaddr_ll
	s_addr.sll_family   = AF_PACKET;
	s_addr.sll_protocol = htons(ETH_P_ALL);
	s_addr.sll_ifindex  = ifindex;
	//s_addr.sll_hatype   = ARPHRD_ETHER;
	//s_addr.sll_pkttype  = PACKET_OTHERHOST;
	//s_addr.sll_halen    = ETH_ALEN;
	//s_addr.sll_addr[6]  = 0x00;
	//s_addr.sll_addr[7]  = 0x00;

 	if (bind(sock_fd, (struct sockaddr *) &s_addr, sizeof(s_addr)) == -1) {
                // error
		perror("binding:");
		exit(1);
        }

	// Set promiscious mode
	strncpy(ifr_listener.ifr_name,listener_ifname,IFNAMSIZ);
	ioctl(sock_fd,SIOCGIFFLAGS,&ifr_listener);
	ifr_listener.ifr_flags |= IFF_PROMISC;
	ioctl(sock_fd,SIOCGIFFLAGS,&ifr_listener);

	signal(SIGINT, sigint);
	printf("Waiting for packets....\n");

	while (1) {

		recv = recvfrom(sock_fd, buff, ETH_FRAME_LEN+ETH_FCS_LEN, 0, NULL, NULL);
		if (recv == -1) {
			perror("recvfrom():");
			exit(1);
		}
		total_recv_packets++;
		//See if we should answer (destination address == our MAC)
		if (memcmp( (const void*)eth_hdr->h_dest, (const void*)listener_mac, ETH_ALEN) == 0 ) {

/*			printf("DST address: %02X:%02X:%02X:%02X:%02X:%02X\n",	eth_hdr->h_dest[0],eth_hdr->h_dest[1],eth_hdr->h_dest[2],eth_hdr->h_dest[3],eth_hdr->h_dest[4],eth_hdr->h_dest[5]);
			printf("SRC address: %02X:%02X:%02X:%02X:%02X:%02X\n",	eth_hdr->h_source[0],eth_hdr->h_source[1],eth_hdr->h_source[2],eth_hdr->h_source[3],eth_hdr->h_source[4],eth_hdr->h_source[5]);
			printf("len:%d\n",recv);
			printf("Packet type ID field  :%#x\n", ntohs(eth_hdr->h_proto));
			printf("data_ptr[250]=%d\n",*(data_ptr));
*/		

			//exchange addresses in buffer
			memcpy(eth_hdr->h_dest, (void*)eth_hdr->h_source, ETH_ALEN);
			memcpy(eth_hdr->h_source, (void*)listener_mac, ETH_ALEN);

			//prepare sockaddr_ll

			s_addr.sll_family   = AF_PACKET;
			s_addr.sll_ifindex  = ifindex;
			s_addr.sll_halen    = ETH_ALEN;
			s_addr.sll_addr[0]  = eth_hdr->h_dest[0];
			s_addr.sll_addr[1]  = eth_hdr->h_dest[1];
			s_addr.sll_addr[2]  = eth_hdr->h_dest[2];
			s_addr.sll_addr[3]  = eth_hdr->h_dest[3];
			s_addr.sll_addr[4]  = eth_hdr->h_dest[4];
			s_addr.sll_addr[5]  = eth_hdr->h_dest[5];
			s_addr.sll_addr[6]  = 0x00;
			s_addr.sll_addr[7]  = 0x00; //MULTICAST

			//send answer
			sent = sendto(sock_fd, buff, recv, 0, (struct sockaddr*)&s_addr, sizeof(s_addr));
			if (sent == -1) {
				perror("sendto():");
				exit(1);
			}
			total_answ_packets++;
			if(!(total_answ_packets%10))
				printf("\rAnswered %ld packets...",total_answ_packets);
		}
	}
	return -1;
}
