/*
 ============================================================================
 Name        : nlclient.c
 Author      : Javier Lopez
 Version     :
 Copyright   : 
 Description :
 ============================================================================
 */


#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <stdint.h>

#include "nlclient.h"
#include "probability.h"
#include "mac_address.h"
#include "ieee80211_hdr.h"

struct nl_sock *sock;
struct nl_msg *msg;
struct nl_cb *cb;
struct nl_cache *cache;
struct genl_family *family;

static int size;
static double *prob_matrix;
static int accepted = 0;
static int dropped = 0;

/*
 * Send a frame to the kernel space.
 */


int send_frame_msg(struct mac_address *src, struct mac_address *dst, char *data, int data_len, int ack) {

	msg = nlmsg_alloc();
	if (!msg) {
		printf("Error allocating new message MSG!\n");
		return -1;
	} 

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, genl_family_get_id(family), 0, NLM_F_REQUEST, HWSIM_C_FRAME, VERSION_NR);

	int rc;
	rc = nla_put(msg, HWSIM_A_ADDR_RECEIVER, sizeof(struct mac_address), dst);
	rc = nla_put(msg, HWSIM_A_ADDR_TRANSMITTER, sizeof(struct mac_address), src);
	rc = nla_put_u32(msg, HWSIM_A_MSG_LEN, data_len);
	rc = nla_put(msg, HWSIM_A_MSG, data_len, data);
	rc = nla_put_u8(msg, HWSIM_A_ACK, ack);
	if(rc!=0) {
		printf("Error filling payload\n");
	}
	
	nl_send_auto_complete(sock,msg);
	nlmsg_free(msg);

	return 0;
}

/*
 * Iterate all the radios and send a copy of the packet to each interface.
 */

int send_frame_to_radios(struct mac_address *src, char*data, int data_len) {

	int i;
	int ack = 0;
	struct mac_address *dst;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)data;

	for (i=0;i<size;i++) {
		
		dst =  get_mac_address(i);

		if(memcmp(src,dst,sizeof(struct mac_address))==0){
			continue;
		}
/*		
		printf("SRC:%02X:%02X:%02X:%02X:%02X:%02X\n",src->addr[0],src->addr[1],src->addr[2],src->addr[3],src->addr[4],src->addr[5]);
		printf("DST:%02X:%02X:%02X:%02X:%02X:%02X\n",dst->addr[0],dst->addr[1],dst->addr[2],dst->addr[3],dst->addr[4],dst->addr[5]);
		printf("*********************************\n");
*/
		if (should_drop_frame(prob_matrix,src,dst)) {
			dropped++;
		} else {
			send_frame_msg(src, dst, data, data_len,0);
			accepted++;
			/*if the mac_address in the hdr == to the mac_address we are sending*/
			if(memcmp(dst,hdr->addr1,sizeof(struct mac_address))==0){ 
				//printf("ACK\n");
				ack++;
			}
		}
			
	}
	/* By agreement a frame with same mac_addr in both attibutes will be considered a tx info packet*/
	send_frame_msg(src, src, data, data_len, ack);
	return 0;
}


static int process_messages_cb(struct nl_msg *msg, void *arg) {
     
	struct nlattr *attrs[HWSIM_A_MAX+1];
	//netlink header
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	//generic netlink header
	struct genlmsghdr *gnlh = nlmsg_data(nlh);	
	
	if(gnlh->cmd == HWSIM_C_FRAME) {
		// we get the attributes 
		genlmsg_parse(nlh, 0, attrs, HWSIM_A_MAX, NULL);
		if (attrs[HWSIM_A_ADDR_TRANSMITTER]) {
			struct mac_address *src = (struct mac_address*)nla_data(attrs[HWSIM_A_ADDR_TRANSMITTER]);

			int data_len = nla_get_u32(attrs[HWSIM_A_MSG_LEN]);
			char* data = (char*)nla_data(attrs[HWSIM_A_MSG]);

			send_frame_to_radios(src,data,data_len);

			printf("\raccepted: %d dropped: %d TOTAL: %d", accepted, dropped, accepted+dropped);
		}
	}
	
	return 0;
}

/* HANDLER DEFS END */

int send_register_msg() {
	
	msg = nlmsg_alloc();
	if (!msg) {
		printf("Error allocating new message MSG!\n");
		return -1;
	} 

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, genl_family_get_id(family), 0, NLM_F_REQUEST, HWSIM_C_REGISTER, VERSION_NR);
	nl_send_auto_complete(sock,msg);
	nlmsg_free(msg);

	return 0;

}

void init_netlink() {

	cb = nl_cb_alloc(NL_CB_CUSTOM);
	if (!cb) {
		printf("Error allocating netlink callbacks\n");
		exit (1);
	}

	sock = nl_socket_alloc_cb(cb);
	if (!sock) {
		printf("Error allocationg netlink socket\n");
		exit (1);
	}

	genl_connect(sock);
	genl_ctrl_alloc_cache(sock, &cache);

	family = genl_ctrl_search_by_name(cache, "HWSIM");

	if (!family) {
		printf("Family HWSIM not registered\n");
		exit (1);
	}

	nl_cb_set(cb, NL_CB_MSG_IN, NL_CB_CUSTOM, process_messages_cb, NULL);

}



int main(int argc, char* argv[]) {

	if(argc!=3) {
		printf("Missing arguments.\n"
			"%s [num of mesh ifaces] [Ploss applied]\n",argv[0]);
		exit(1);
	}
	
	size = atoi(argv[1]);
	double a_prob = atof(argv[2]);

	prob_matrix = malloc(sizeof(double)*(size*size));

	printf("%d MAC address registered\n",size);

	/*Init the probability*/
	init_probability(size);

	/*Fill the matrix with a fixed probability*/
	fill_prob_matrix((double*)prob_matrix,a_prob);
	print_prob_matrix((double*)prob_matrix);

	/*init netlink*/
	init_netlink();

	/*Send a register msg to the kernel*/
	if (send_register_msg()==0)
		printf("REGISTER SENT!\n");

	/*We wait for incoming msg*/
	while(1) {
		nl_recvmsgs_default(sock);
	}
	return 1;
}
