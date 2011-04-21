/*
 *	wmediumd, wireless medium simulator for mac80211_hwsim kernel module
 *	Copyright (C) 2011  Javier Lopez (jlopex@gmail.com)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version 2
 *	of the License, or (at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <stdint.h>

#include "wmediumd.h"
#include "probability.h"
#include "mac_address.h"
#include "ieee80211.h"

struct nl_sock *sock;
struct nl_msg *msg;
struct nl_cb *cb;
struct nl_cache *cache;
struct genl_family *family;

static int size;
static double *prob_matrix;
static int received = 0;
static int sent = 0;
static int dropped = 0;
static int acked = 0;

/*
 *	Send a tx_info frame to the kernel space.
 */

int send_tx_info_frame_nl(struct mac_address *dst, char *data, int data_len, unsigned int flags, int signal, struct ieee80211_tx_rate *tx_attempts, void *cb) {

	msg = nlmsg_alloc();
	if (!msg) {
		printf("Error allocating new message MSG!\n");
		goto out;
	}

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, genl_family_get_id(family), 0, NLM_F_REQUEST, HWSIM_CMD_TX_INFO_FRAME, VERSION_NR);

	int rc;
	rc = nla_put(msg, HWSIM_ATTR_ADDR_TRANSMITTER, sizeof(struct mac_address), dst);
	rc = nla_put(msg, HWSIM_ATTR_FRAME, data_len, data);
	rc = nla_put_u32(msg, HWSIM_ATTR_FLAGS, flags);
	rc = nla_put_u32(msg, HWSIM_ATTR_SIGNAL, signal);
	rc = nla_put(msg, HWSIM_ATTR_TX_INFO, IEEE80211_MAX_TX_RATES*sizeof(struct ieee80211_tx_rate),tx_attempts);
	rc = nla_put(msg, HWSIM_ATTR_CB_SKB, IEEE80211_CB_SIZE*sizeof(char), cb);

	if(rc!=0) {
		printf("Error filling payload\n");
		goto out;
	}

	nl_send_auto_complete(sock,msg);
	nlmsg_free(msg);

	return 0;

out:
	nlmsg_free(msg);
	return -1;
}

/*
 * 	Send a cloned frame to the kernel space.
 */

int send_cloned_frame_msg(struct mac_address *dst, char *data, int data_len, int rate_idx, int signal) {

	msg = nlmsg_alloc();
	if (!msg) {
		printf("Error allocating new message MSG!\n");
		goto out;
	}

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, genl_family_get_id(family), 0, NLM_F_REQUEST, HWSIM_CMD_FRAME, VERSION_NR);

	int rc;
	rc = nla_put(msg, HWSIM_ATTR_ADDR_RECEIVER, sizeof(struct mac_address), dst);
	rc = nla_put(msg, HWSIM_ATTR_FRAME, data_len, data);
	rc = nla_put_u32(msg, HWSIM_ATTR_RX_RATE, rate_idx);
	rc = nla_put_u32(msg, HWSIM_ATTR_SIGNAL, signal);

	if(rc!=0) {
		printf("Error filling payload\n");
		goto out;
	}

	nl_send_auto_complete(sock,msg);
	nlmsg_free(msg);

	return 0;
out:
	nlmsg_free(msg);
	return -1;
}

/*
 * 	Get a signal value by rate index
 */

int get_signal_by_rate(int rate_idx) {
	const int rate2signal [] = { -80,-77,-74,-71,-69,-66,-64,-62,-59,-56,-53,-50 };
	return rate2signal[rate_idx];
}

/*
 * 	Send a frame applying the loss probability of the link
 */

int send_frame_msg_apply_prob_and_rate(struct mac_address *src, struct mac_address *dst, char *data, int data_len, int rate_idx) {

	/* At higher rates higher loss probability*/
	double prob_per_link = get_prob_per_link_with_rate_idx(prob_matrix,src,dst,rate_idx);
	double random_double = generate_random_double();

	if (random_double < prob_per_link) {
		dropped++;
		return 0;
	} else {

		/*received signal level*/
		int signal = get_signal_by_rate(rate_idx);

		//send_frame_msg(src, dst, data, data_len, 0, rate_idx, signal, 0, 0);
		send_cloned_frame_msg(dst,data,data_len,rate_idx,signal);
		sent++;
		return 1;
	}

}

/*
 * 	Set a tx_rate struct to not valid values
 */

void set_all_rates_invalid(struct ieee80211_tx_rate* tx_rate) {
	int i;
	/* set up all unused rates to be -1 */
	for (i=0; i < IEEE80211_MAX_TX_RATES; i++) {
        	tx_rate[i].idx = -1;
		tx_rate[i].count = 0;
	}
}


/*
 * 	Iterate all the radios and send a copy of the packet to each interface.
 */

void send_frames_to_radios_with_retries(struct mac_address *src, char*data, int data_len, unsigned int flags, struct ieee80211_tx_rate *tx_rates, void *cb) {

	struct mac_address *dst;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)data;
	struct ieee80211_tx_rate tx_attempts[IEEE80211_MAX_TX_RATES];

	int round = 0, tx_ok = 0, counter, i;

	/* We prepare the tx_attempts struct */
	set_all_rates_invalid(tx_attempts);

	while (tx_rates[round].idx != -1 && tx_ok!=1) {

		counter = 1;

		/* Set rate index and flags used for this round */
		tx_attempts[round].idx = tx_rates[round].idx;
		tx_attempts[round].flags = tx_rates[round].flags;

		while(counter <= tx_rates[round].count && tx_ok !=1 ) {

			/* Broadcast the frame to all the radio ifaces*/
			for (i=0;i<size;i++) {

				dst =  get_mac_address(i);

				/* If origin and destination are the same just skip this iteration*/
				if(memcmp(src,dst,sizeof(struct mac_address))==0){
					continue;
				}

				/* Try to send it to a radio and if the frame is destined to this radio tx_ok*/
				if(send_frame_msg_apply_prob_and_rate(src, dst, data, data_len, tx_attempts[round].idx) &&
					memcmp(dst,hdr->addr1,sizeof(struct mac_address))==0) {
						tx_ok = 1;
				}
			}

			tx_attempts[round].count = counter;
			counter++;
		}

		round++;
	}

	if (tx_ok) {
		/* if tx is done and acked a frame with the tx_info is sent to original radio iface*/
		acked++;
		int signal = get_signal_by_rate(tx_attempts[counter-1].idx);
		/* Let's flag this frame as ACK'ed */
		flags |= IEEE80211_TX_STAT_ACK;
		send_tx_info_frame_nl(src,data,data_len,flags, signal,tx_attempts,cb);
	} else {
		send_tx_info_frame_nl(src,data,data_len,flags, 0, tx_attempts, cb);
	}
}

/*
 * 	Callback function to process messages received from kernel
 */

static int process_messages_cb(struct nl_msg *msg, void *arg) {

	struct nlattr *attrs[HWSIM_ATTR_MAX+1];
	/* netlink header */
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	/* generic netlink header*/
	struct genlmsghdr *gnlh = nlmsg_data(nlh);

	if(gnlh->cmd == HWSIM_CMD_FRAME) {
		/* we get the attributes*/
		genlmsg_parse(nlh, 0, attrs, HWSIM_ATTR_MAX, NULL);
		if (attrs[HWSIM_ATTR_ADDR_TRANSMITTER]) {
			struct mac_address *src = (struct mac_address*)nla_data(attrs[HWSIM_ATTR_ADDR_TRANSMITTER]);

			unsigned int data_len = nla_len(attrs[HWSIM_ATTR_FRAME]);
			char* data = (char*)nla_data(attrs[HWSIM_ATTR_FRAME]);
			unsigned int flags = nla_get_u32(attrs[HWSIM_ATTR_FLAGS]);
			struct ieee80211_tx_rate *tx_rates = (struct ieee80211_tx_rate*)nla_data(attrs[HWSIM_ATTR_TX_INFO]);
			void *cb = nla_data(attrs[HWSIM_ATTR_CB_SKB]);

			received++;

/*			if((flags & IEEE80211_TX_CTL_REQ_TX_STATUS) == IEEE80211_TX_CTL_REQ_TX_STATUS)
				printf("IEEE80211_TX_CTL_REQ_TX_STATUS is SET\n");
			if((flags & IEEE80211_TX_CTL_NO_ACK) == IEEE80211_TX_CTL_NO_ACK)
				printf("IEEE80211_TX_CTL_NO_ACK is SET\n");
			else
				printf("IEEE80211_TX_CTL_NO_ACK is !SET\n");
*/

			send_frames_to_radios_with_retries(src,data,data_len,flags,tx_rates,cb);
			printf("\rreceived: %d tried: %d sent: %d acked: %d", received, dropped+sent, sent, acked);
		}
	}

	return 0;
}

/*
 * 	Send a register message to kernel
 */

int send_register_msg() {

	msg = nlmsg_alloc();
	if (!msg) {
		printf("Error allocating new message MSG!\n");
		return -1;
	}

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, genl_family_get_id(family), 0, NLM_F_REQUEST, HWSIM_CMD_REGISTER, VERSION_NR);
	nl_send_auto_complete(sock,msg);
	nlmsg_free(msg);

	return 0;

}

/*
 * 	Init netlink
 */

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

	if (a_prob < 0 || a_prob > 1) {
		printf("Ploss applied must be a float value between 0 to 1 \n"
			"\t For example, to apply a loss prob of 50%% -> 0.5 \n");
		exit(1);
	}

	if (size < 2) {
		printf("The number of mesh ifaces must be at least 2\n");
		exit(1);
	}

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
