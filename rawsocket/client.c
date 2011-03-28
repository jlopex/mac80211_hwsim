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

int sock_fd = 0; 				//Socketdescriptor
unsigned char* out_buff = NULL;			//Buffer out main
unsigned char* in_buff = NULL;			//Buffer in thread_rcv

unsigned char snd_mac[6];

char *sender_ifname;				//src if_name
char *receiver_ifname;				//dst if_name

long total_sent_packets = 0;
long total_recv_packets = 0;

void * count_packets(void *ptr) {

	int recv;
	in_buff = (void*)malloc(ETH_FRAME_LEN+ETH_FCS_LEN);
	struct ethhdr *in_hdr = (struct ethhdr *)in_buff;
	while (1) {

		recv = recvfrom(sock_fd, in_buff, 1518, 0, NULL, NULL);

		if (recv == -1) {
			perror("recvfrom():");
			exit(1);
		}

		//See if we should answer (destination address == our MAC)
		if (in_hdr->h_proto == 0x6400  && memcmp( (const void*)in_hdr->h_dest, (const void*)snd_mac, ETH_ALEN) == 0 ) {
			total_recv_packets++;
/*			if (!(total_recv_packets%10))
				printf("total_recv_packets: %ld\n",total_recv_packets);*/
		}
	}
}


void sigint(int signum) {

	struct ifreq ifr;

	if (sock_fd == -1)
		return;

	strncpy(ifr.ifr_name, sender_ifname, IFNAMSIZ);
	ioctl(sock_fd, SIOCGIFFLAGS, &ifr);
	ifr.ifr_flags &= ~IFF_PROMISC;
	ioctl(sock_fd, SIOCSIFFLAGS, &ifr);
	close(sock_fd);

	free(out_buff);
	free(in_buff);

	float loss = 1-((double)total_recv_packets/(double)total_sent_packets);
	printf("Client terminating....\n");
	printf("Totally sent: %ld packets\n", total_sent_packets);
	printf("Totally recv: %ld packets\n", total_recv_packets);
	printf("Ploss=%f\n", loss);


	exit(0);
}


int main(int argc, char *argv[]) {

	pthread_t thread1;

	unsigned char rcv_mac[6]; // = {0x42, 0x00, 0x00, 0x00, 0x00, 0x00};
	
	struct ifreq ifr;

	out_buff = (void*)malloc(ETH_FRAME_LEN+ETH_FCS_LEN); 	//Buffer for ethernet frame
	unsigned char* data_ptr = out_buff + ETH_HLEN;		//Userdata in ethernet frame

	struct ethhdr *out_hdr = (struct ethhdr *)out_buff; 	//to ethernet header

	struct sockaddr_ll s_addr;				//sock addr
	int ifindex = 0;					//ethernet interface index
	int i;							//Counter
	int sent;						//length of sent packet
	int time;

	if (argc !=4)
    	{
    		printf("Missing arguments.\n"
    			"%s [src_ifname] [dest_ifname] [time msec] \n",argv[0]);
    		exit(1);
	}

	sender_ifname = argv[1];
	receiver_ifname = argv[2];
	time = atoi(argv[3]);

	//open socket
	sock_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)); 
	if (sock_fd == -1) {
		perror("socket():");
		exit(1);
	}
	printf("Successfully opened socket: %i\n", sock_fd);

	//retrieve ethernet interface index
	strncpy(ifr.ifr_name, receiver_ifname, IFNAMSIZ);
	if (ioctl(sock_fd, SIOCGIFINDEX, &ifr) == -1) {
		perror("SIOCGIFINDEX");
		exit(1);
	}
	ifindex = ifr.ifr_ifindex;
	printf("Successfully got interface index: %i\n", ifindex);

	//retrieve corresponding MAC
	if (ioctl(sock_fd, SIOCGIFHWADDR, &ifr) == -1) {
		perror("SIOCGIFINDEX");
		exit(1);
	}

	for (i = 0; i < 6; i++) {
		rcv_mac[i] = ifr.ifr_hwaddr.sa_data[i];
	}


	//retrieve ethernet interface index
	strncpy(ifr.ifr_name, sender_ifname, IFNAMSIZ);
	if (ioctl(sock_fd, SIOCGIFINDEX, &ifr) == -1) {
		perror("SIOCGIFINDEX");
		exit(1);
	}
	ifindex = ifr.ifr_ifindex;
	printf("Successfully got interface index: %i\n", ifindex);

	//retrieve corresponding MAC
	if (ioctl(sock_fd, SIOCGIFHWADDR, &ifr) == -1) {
		perror("SIOCGIFINDEX");
		exit(1);
	}
	for (i = 0; i < 6; i++) {
		snd_mac[i] = ifr.ifr_hwaddr.sa_data[i];
	}
	printf("SENDER MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
			snd_mac[0],snd_mac[1],snd_mac[2],snd_mac[3],snd_mac[4],snd_mac[5]);

	printf("RECEIVER MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
		rcv_mac[0],rcv_mac[1],rcv_mac[2],rcv_mac[3],rcv_mac[4],rcv_mac[5]);

	//prepare sockaddr_ll
	s_addr.sll_family   = AF_PACKET;
	s_addr.sll_protocol = htons(ETH_P_ALL);
	s_addr.sll_ifindex  = ifindex;
	s_addr.sll_halen    = ETH_ALEN;
	s_addr.sll_addr[0]  = rcv_mac[0];
	s_addr.sll_addr[1]  = rcv_mac[1];
	s_addr.sll_addr[2]  = rcv_mac[2];
	s_addr.sll_addr[3]  = rcv_mac[3];
	s_addr.sll_addr[4]  = rcv_mac[4];
	s_addr.sll_addr[5]  = rcv_mac[5];
	s_addr.sll_addr[6]  = 0x00;
	s_addr.sll_addr[7]  = 0x00; 

	if (bind(sock_fd, (struct sockaddr *) &s_addr, sizeof(s_addr)) == -1) { 
		// error 
		perror("binding:"); 
		exit(1); 
	} 
 
	// Set promiscious mode 
	strncpy(ifr.ifr_name,sender_ifname,IFNAMSIZ); 
	ioctl(sock_fd,SIOCGIFFLAGS,&ifr); 
	ifr.ifr_flags |= IFF_PROMISC; 
	ioctl(sock_fd,SIOCGIFFLAGS,&ifr);

	signal(SIGINT, sigint);
	printf("Sending packets....\n");

	//prepare buffer
	memcpy(out_hdr->h_dest, (void*)rcv_mac, ETH_ALEN);
	memcpy(out_hdr->h_source, (void*)snd_mac, ETH_ALEN);

	//fill it with some data....
	for (i = 0; i < 100; i++) {
		data_ptr[i] = (unsigned char)(0);
	}
	pthread_create( &thread1, NULL, count_packets, (void*) NULL);

	while (1) {
		sent = sendto(sock_fd, out_buff, i+ETH_HLEN, 0, (struct sockaddr*)&s_addr, sizeof(s_addr));
		total_sent_packets++;
		if (!(total_sent_packets%10)){
			printf("\rtotal_sent_packets: %ld "
				"total_recv_packets: %ld",total_sent_packets,total_recv_packets);
			/*printf("DST address: %02X:%02X:%02X:%02X:%02X:%02X\n",	out_hdr->h_dest[0],out_hdr->h_dest[1],out_hdr->h_dest[2],out_hdr->h_dest[3],out_hdr->h_dest[4],out_hdr->h_dest[5]);
			printf("SRC address: %02X:%02X:%02X:%02X:%02X:%02X\n",	out_hdr->h_source[0],out_hdr->h_source[1],out_hdr->h_source[2],out_hdr->h_source[3],out_hdr->h_source[4],out_hdr->h_source[5]);
			printf("len:%d\n",sent);
			printf("Packet type ID field  :%#x\n", ntohs(out_hdr->h_proto));
			printf("data_ptr[250]=%d\n",*(data_ptr+250)); */
		}	
		usleep(time*1000);
	}

	return -1;
}
