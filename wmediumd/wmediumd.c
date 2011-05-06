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
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *	02110-1301, USA.
 */

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <stdint.h>
#include <libconfig.h>
#include <getopt.h>

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
 * 	Generates a random double value
 */

double generate_random_double()
{

	return rand()/((double)RAND_MAX+1);
}

/*
 *	Send a tx_info frame to the kernel space.
 */

int send_tx_info_frame_nl(struct mac_address *dst, char *data, int data_len,
			  unsigned int flags, int signal,
			  struct ieee80211_tx_rate *tx_attempts, void *cb)
{

	msg = nlmsg_alloc();
	if (!msg) {
		printf("Error allocating new message MSG!\n");
		goto out;
	}

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, genl_family_get_id(family),
		    0, NLM_F_REQUEST, HWSIM_CMD_TX_INFO_FRAME, VERSION_NR);

	int rc;
	rc = nla_put(msg, HWSIM_ATTR_ADDR_TRANSMITTER,
		     sizeof(struct mac_address), dst);
	rc = nla_put(msg, HWSIM_ATTR_FRAME, data_len, data);
	rc = nla_put_u32(msg, HWSIM_ATTR_FLAGS, flags);
	rc = nla_put_u32(msg, HWSIM_ATTR_SIGNAL, signal);
	rc = nla_put(msg, HWSIM_ATTR_TX_INFO,
		     IEEE80211_MAX_RATES_PER_TX *
		     sizeof(struct ieee80211_tx_rate), tx_attempts);

	rc = nla_put(msg, HWSIM_ATTR_CB_SKB,
		     IEEE80211_CB_SIZE * sizeof(char), cb);

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

int send_cloned_frame_msg(struct mac_address *dst, char *data,
			  int data_len, int rate_idx, int signal)
{

	msg = nlmsg_alloc();
	if (!msg) {
		printf("Error allocating new message MSG!\n");
		goto out;
	}

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, genl_family_get_id(family),
		    0, NLM_F_REQUEST, HWSIM_CMD_FRAME, VERSION_NR);

	int rc;

	rc = nla_put(msg, HWSIM_ATTR_ADDR_RECEIVER,
		     sizeof(struct mac_address), dst);
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

int get_signal_by_rate(int rate_idx)
{
	const int rate2signal [] =
		{ -80,-77,-74,-71,-69,-66,-64,-62,-59,-56,-53,-50 };
	if (rate_idx >= 0 || rate_idx < IEEE80211_AVAILABLE_RATES)
		return rate2signal[rate_idx];
	return 0;
}

/*
 * 	Send a frame applying the loss probability of the link
 */

int send_frame_msg_apply_prob_and_rate(struct mac_address *src,
				       struct mac_address *dst,
				       char *data, int data_len, int rate_idx)
{

	/* At higher rates higher loss probability*/
	double prob_per_link = find_prob_by_addrs_and_rate(prob_matrix,
							   src,dst, rate_idx);
	double random_double = generate_random_double();

	if (random_double < prob_per_link) {
		printf("dropped\n");
		dropped++;
		return 0;
	} else {

		printf("sent\n");
		/*received signal level*/
		int signal = get_signal_by_rate(rate_idx);

		send_cloned_frame_msg(dst,data,data_len,rate_idx,signal);
		sent++;
		return 1;
	}
}

/*
 * 	Set a tx_rate struct to not valid values
 */

void set_all_rates_invalid(struct ieee80211_tx_rate* tx_rate)
{
	int i;
	/* set up all unused rates to be -1 */
	for (i=0; i < IEEE80211_MAX_RATES_PER_TX; i++) {
        	tx_rate[i].idx = -1;
		tx_rate[i].count = 0;
	}
}


/*
 * 	Iterate all the radios and send a copy of the frame to each interface.
 */

void send_frames_to_radios_with_retries(struct mac_address *src, char*data,
					int data_len, unsigned int flags,
					struct ieee80211_tx_rate *tx_rates,
					void *cb)
{

	struct mac_address *dst;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)data;
	struct ieee80211_tx_rate tx_attempts[IEEE80211_MAX_RATES_PER_TX];

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

				/*
				 * If origin and destination are the
				 * same just skip this iteration
				*/
				if(memcmp(src,dst,sizeof(struct mac_address))
					  == 0 ){
					continue;
				}

				/* Try to send it to a radio and if
				 * the frame is destined to this radio tx_ok
				*/
				if(send_frame_msg_apply_prob_and_rate(
					src, dst, data, data_len,
					tx_attempts[round].idx) &&
					memcmp(dst, hdr->addr1,
					sizeof(struct mac_address))==0) {
						tx_ok = 1;
				}
			}
			tx_attempts[round].count = counter;
			counter++;
		}
		round++;
	}

	if (tx_ok) {
		/* if tx is done and acked a frame with the tx_info is
		 * sent to original radio iface
		*/
		acked++;
		int signal = get_signal_by_rate(tx_attempts[round-1].idx);
		/* Let's flag this frame as ACK'ed */
		flags |= IEEE80211_TX_STAT_ACK;
		send_tx_info_frame_nl(src, data, data_len, flags,
				      signal,tx_attempts,cb);
	} else {
		send_tx_info_frame_nl(src, data, data_len, flags,
				      0, tx_attempts, cb);
	}
}

/*
 * 	Callback function to process messages received from kernel
 */

static int process_messages_cb(struct nl_msg *msg, void *arg)
{

	struct nlattr *attrs[HWSIM_ATTR_MAX+1];
	/* netlink header */
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	/* generic netlink header*/
	struct genlmsghdr *gnlh = nlmsg_data(nlh);

	if(gnlh->cmd == HWSIM_CMD_FRAME) {
		/* we get the attributes*/
		genlmsg_parse(nlh, 0, attrs, HWSIM_ATTR_MAX, NULL);
		if (attrs[HWSIM_ATTR_ADDR_TRANSMITTER]) {
			struct mac_address *src = (struct mac_address*)
				nla_data(attrs[HWSIM_ATTR_ADDR_TRANSMITTER]);

			unsigned int data_len =
				nla_len(attrs[HWSIM_ATTR_FRAME]);
			char* data = (char*)nla_data(attrs[HWSIM_ATTR_FRAME]);
			unsigned int flags =
				nla_get_u32(attrs[HWSIM_ATTR_FLAGS]);
			struct ieee80211_tx_rate *tx_rates =
				(struct ieee80211_tx_rate*)
				nla_data(attrs[HWSIM_ATTR_TX_INFO]);
			void *cb = nla_data(attrs[HWSIM_ATTR_CB_SKB]);

			received++;

			printf("frame [%d] length:%d\n",received,data_len);
			send_frames_to_radios_with_retries(src, data,
					data_len, flags, tx_rates, cb);
			//printf("\rreceived: %d tried: %d sent: %d acked: %d",
			//		received, dropped+sent, sent, acked);
		}
	}
	return 0;
}

/*
 * 	Send a register message to kernel
 */

int send_register_msg()
{

	msg = nlmsg_alloc();
	if (!msg) {
		printf("Error allocating new message MSG!\n");
		return -1;
	}

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, genl_family_get_id(family),
		    0, NLM_F_REQUEST, HWSIM_CMD_REGISTER, VERSION_NR);
	nl_send_auto_complete(sock,msg);
	nlmsg_free(msg);

	return 0;

}

/*
 * 	Init netlink
 */

void init_netlink()
{

	cb = nl_cb_alloc(NL_CB_VERBOSE);
	if (!cb) {
		printf("Error allocating netlink callbacks\n");
		exit(EXIT_FAILURE);
	}

	sock = nl_socket_alloc_cb(cb);
	if (!sock) {
		printf("Error allocationg netlink socket\n");
		exit(EXIT_FAILURE);
	}

	genl_connect(sock);
	genl_ctrl_alloc_cache(sock, &cache);

	family = genl_ctrl_search_by_name(cache, "MAC80211_HWSIM");

	if (!family) {
		printf("Family MAC80211_HWSIM not registered\n");
		exit(EXIT_FAILURE);
	}

	nl_cb_set(cb, NL_CB_MSG_IN, NL_CB_CUSTOM, process_messages_cb, NULL);

}

char *str_replace(const char *str, const char *old, const char *new)
{
	char *ret, *r;
	const char *p, *q;
	size_t len_str = strlen(str);
	size_t len_old = strlen(old);
	size_t len_new = strlen(new);
	size_t count;

	for(count = 0, p = str; (p = strstr(p, old)); p += len_old)
		count++;

	ret = malloc(count * (len_new - len_old) + len_str + 1);
	if(!ret)
		return NULL;

	for(r = ret, p = str; (q = strstr(p, old)); p = q + len_old) {
		count = q - p;
		memcpy(r, p, count);
		r += count;
		strcpy(r, new);
		r += len_new;
	}
	strcpy(r, p);
	return ret;
}

int write_buffer_to_file(char *file, char *buffer)
{
	FILE *p = NULL;

	p = fopen(file, "w");
	if (p== NULL) {
		return 1;
	}

	fwrite(buffer, strlen(buffer), 1, p);
	fclose(p);

	return 0;
}

/*
 *	Writes a sample configuration with matrix filled with a value to a file
 */

int write_config(char *file, int ifaces, float value)
{
	FILE *out;
	char *ptr, *ptr2;
	size_t size;
	config_t cfg;
	config_setting_t *root, *setting, *group, *array, *list;
	int i, j, rates = 12;

	/*Init tmp file stream*/
	out = open_memstream(&ptr, &size);
	if (out == NULL) {
		printf("Error generating stream\n");
		exit(EXIT_FAILURE);
	}
	/*Init config*/
	config_init(&cfg);

	/*Create a sample config schema*/
	root = config_root_setting(&cfg);
	/* Add some settings to the ifaces group. */
	group = config_setting_add(root, "ifaces", CONFIG_TYPE_GROUP);
	setting = config_setting_add(group, "count", CONFIG_TYPE_INT);
	config_setting_set_int(setting, ifaces);
	array = config_setting_add(group, "ids", CONFIG_TYPE_ARRAY);

	for(i = 0; i < ifaces; ++i) {
		setting = config_setting_add(array, NULL, CONFIG_TYPE_STRING);
		char buffer[25];
		sprintf (buffer, "42:00:00:00:%02d:00", i);
		config_setting_set_string(setting, buffer);
	}

	/* Add some settings to the prob group. */
	group = config_setting_add(root, "prob", CONFIG_TYPE_GROUP);
	setting = config_setting_add(group, "rates", CONFIG_TYPE_INT);
	config_setting_set_int(setting, rates);
	list = config_setting_add(group, "matrix_list", CONFIG_TYPE_LIST);
	for (j = 0; j < rates ; j++) {
		array = config_setting_add(list, NULL, CONFIG_TYPE_ARRAY);
		int diag_count = 0;
		for(i = 0; i < ifaces*ifaces; ++i) {
			setting = config_setting_add(array, NULL,
						     CONFIG_TYPE_FLOAT);
			if (diag_count == 0)
				config_setting_set_float(setting, -1.0);
			else
				config_setting_set_float(setting, value);
			diag_count++;
			if (diag_count > ifaces)
				diag_count = 0;
		}
	}
	/* Write in memory out file */
	config_write(&cfg, out);
	config_destroy(&cfg);
	fclose(out);

	/* Let's do some post processing */
	ptr2 = str_replace(ptr, "], ", "],\n\t");
	free(ptr);
	ptr = str_replace(ptr2, "( ", "(\n\t");
	free(ptr2);
	/* Let's add comments to the config file */
	ptr2 = str_replace(ptr, "ifaces :", "#\n# wmediumd sample config file\n#\n\nifaces :");
	free(ptr);
	ptr = str_replace(ptr2, "prob :", "\n#\n# probability matrices are defined in a rowcentric way \n# probability matrices are ordered from slower to fastest, check wmediumd documentation for more info\n#\n\nprob :");
	printf("%s",ptr);

	/*write the string to a file*/
	if(write_buffer_to_file(file, ptr)) {
		printf("Error while writing file.\n");
		free(ptr);
		exit(EXIT_FAILURE);
	}
	printf("New configuration successfully written to: %s\n", file);

	/*free ptr*/
	free(ptr);
	exit(EXIT_SUCCESS);
}

int load_config(const char *file)
{

	config_t cfg, *cf;
	const config_setting_t *ids, *prob_list, *mat_array;
	int count_ids, rates_prob, i, j;
	long int count_value, rates_value;

	/*initialize the config file*/
	cf = &cfg;
	config_init(cf);

	/*read the file*/
	if (!config_read_file(cf, file)) {
		printf("Error loading file %s at line:%d, reason: %s\n",
		file,
		config_error_line(cf),
		config_error_text(cf));
		config_destroy(cf);
		exit(EXIT_FAILURE);
    	}

	/*let's parse the values*/
	config_lookup_int(cf, "ifaces.count", &count_value);
	ids = config_lookup(cf, "ifaces.ids");
	count_ids = config_setting_length(ids);

	/*cross check*/
	if (count_value != count_ids) {
		printf("Error on ifaces.count");
		exit(EXIT_FAILURE);
	}

	printf("#_if = %d\n",count_ids);
	size = count_ids;
	/*Initialize the probability*/
	prob_matrix = init_probability(size);

	/*Fill the mac_addr*/
	for (i = 0; i < count_ids; i++) {
    		const char *str =  config_setting_get_string_elem(ids, i);
    		put_mac_address(string_to_mac_address(str),i);
    	}
	/*Print the mac_addr array*/
	print_mac_address_array();

	config_lookup_int(cf, "prob.rates", &rates_value);
	prob_list = config_lookup(cf,"prob.matrix_list");

	/*Get rates*/
	rates_prob = config_setting_length(prob_list);

	/*Some checks*/
	if(!config_setting_is_list(prob_list)
	   && rates_prob != rates_value) {
		printf("Error on prob_list");
		exit(EXIT_FAILURE);
	}

	/*Iterate all matrix arrays*/
	for (i=0; i < rates_prob ; i++) {
		int x = 0, y = 0;
		mat_array = config_setting_get_elem(prob_list,i);
		/*If any error break execution*/
		if (config_setting_length(mat_array) != count_ids*count_ids) {
    			exit(EXIT_FAILURE);
		}
		/*Iterate all values on matrix array*/
		for (j=0; j < config_setting_length(mat_array); ) {
			MATRIX_PROB(prob_matrix,size,x,y,i) =
			config_setting_get_float_elem(mat_array,j);
			//printf("%f, ", config_setting_get_float_elem(mat_array,j));
			x++;
			j++;
			/* if we finalized this row */
			if (j%count_ids==0) {
				y++;
				x=0;
				//printf("*******j:%d,count_ids:%d \n",j,count_ids);
			}
		}
	}
	config_destroy(cf);
	return (EXIT_SUCCESS);
}

void print_help(int exval)
{
	printf("wmediumd v%s - a wireless medium simulator\n", VERSION_STR);
	printf("wmediumd [-h] [-V] [-c FILE] [-o FILE]\n\n");

	printf("  -h              print this help and exit\n");
	printf("  -V              print version and exit\n\n");

	printf("  -c FILE         set intput config file\n");
	printf("  -o FILE         set output config file\n\n");

	exit(exval);
}


int main(int argc, char* argv[]) {

	int opt, ifaces;

	/* no arguments given */
	if(argc == 1) {
		fprintf(stderr, "This program needs arguments....\n\n");
		print_help(1);
	}

	while((opt = getopt(argc, argv, "hVc:o:")) != -1) {
		switch(opt) {
		case 'h':
			print_help(EXIT_SUCCESS);
    			break;
		case 'V':
			printf("wmediumd v%s - a wireless medium simulator "
			       "for mac80211_hwsim\n", VERSION_STR);
			exit(EXIT_SUCCESS);
			break;
		case 'c':
			printf("Input configuration file: %s\n", optarg);
			load_config(optarg);
			break;
		case 'o':
			printf("Output configuration file: %s\n", optarg);
			printf("How many interfaces are active?\n");
			scanf("%d",&ifaces);
			if (ifaces < 2) {
				printf("active interfaces must be at least 2\n");
				exit(EXIT_FAILURE);
			}
				write_config(optarg, ifaces, 0.0);
			break;
		case ':':
			printf("wmediumd: Error - Option `%c' "
			       "needs a value\n\n", optopt);
			print_help(EXIT_FAILURE);
			break;
		case '?':
			printf("wmediumd: Error - No such option:"
			       " `%c'\n\n", optopt);
			print_help(EXIT_FAILURE);
			break;
		}

	}

	if (optind < argc)
		print_help(EXIT_FAILURE);

	print_prob_matrix(prob_matrix);

	/*init netlink*/
	init_netlink();

	/*Send a register msg to the kernel*/
	if (send_register_msg()==0)
		printf("REGISTER SENT!\n");

	/*We wait for incoming msg*/
	while(1) {
		nl_recvmsgs_default(sock);
	}
	return EXIT_SUCCESS;
}
