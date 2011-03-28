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

struct nl_sock *sock;
struct nl_msg *msg;
struct nl_cb *cb;
struct nl_cache *cache;
struct genl_family *family;

static double *prob_matrix;
static int accepted = 0;
static int dropped = 0;

int send_frame_msg(struct mac_address *receiver, struct mac_address *transmitter, char *data, int data_len) {

	msg = nlmsg_alloc();
	if (!msg) {
		printf("Error allocating new message MSG!\n");
		return -1;
	} 

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, genl_family_get_id(family), 0, NLM_F_REQUEST, HWSIM_C_FRAME, VERSION_NR);

	int rc;
	rc = nla_put(msg, HWSIM_A_ADDR_RECEIVER, sizeof(struct mac_address), receiver);
	rc = nla_put(msg, HWSIM_A_ADDR_TRANSMITTER, sizeof(struct mac_address), transmitter);
	rc = nla_put_u32(msg, HWSIM_A_MSG_LEN, data_len);
	rc = nla_put(msg, HWSIM_A_MSG, data_len, data);
	if(rc!=0) {
		printf("Error filling payload\n");
	}
	
	nl_send_auto_complete(sock,msg);
	nlmsg_free(msg);

	return 0;
}

static int process_messages_cb(struct nl_msg *msg, void *arg) {
     
	struct nlattr *attrs[HWSIM_A_MAX+1];
	//Saco el header de netlink (el 1º)
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	//Saco el genlmsghdr
	struct genlmsghdr *gnlh = nlmsg_data(nlh);	

	if(gnlh->cmd == HWSIM_C_FRAME) {
		// Parseo el mensaje para conseguir los attributos
		genlmsg_parse(nlh, 0, attrs, HWSIM_A_MAX, NULL);
		if (attrs[HWSIM_A_ADDR_RECEIVER] && attrs[HWSIM_A_ADDR_TRANSMITTER]) {
			struct mac_address *r = (struct mac_address*)nla_data(attrs[HWSIM_A_ADDR_RECEIVER]);
			struct mac_address *t = (struct mac_address*)nla_data(attrs[HWSIM_A_ADDR_TRANSMITTER]);

			/*printf("R:%02X:%02X:%02X:%02X:%02X:%02X\n",r->addr[0],r->addr[1],r->addr[2],r->addr[3],r->addr[4],r->addr[5]);
			printf("T:%02X:%02X:%02X:%02X:%02X:%02X\n",t->addr[0],t->addr[1],t->addr[2],t->addr[3],t->addr[4],t->addr[5]);*/

			int data_len = nla_get_u32(attrs[HWSIM_A_MSG_LEN]);
			char* data = (char*)nla_data(attrs[HWSIM_A_MSG]);

			if (should_drop_frame(prob_matrix,t,r)) {
				//printf("DISCARDED\n");
				dropped++;
			} else {
				//printf("ACCEPTED \n");
				send_frame_msg(r, t, data, data_len);
				accepted++;
			}
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


	int size = atoi(argv[1]);
	double a_prob = atof(argv[2]);

	prob_matrix = malloc(sizeof(double)*(size*size));

	printf("%d MAC address registered\n",size);

	init_probability(size);

	fill_prob_matrix((double*)prob_matrix,a_prob);
	print_prob_matrix((double*)prob_matrix);

	init_netlink();

	if (send_register_msg()==0)
		printf("REGISTER SENT!\n");

	while(1) {
		nl_recvmsgs_default(sock);
	}
	return 1;
}
