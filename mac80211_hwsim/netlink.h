#include <net/mac80211.h>

enum {
	HWSIM_ATTR_UNSPEC,
	HWSIM_ATTR_ADDR_RECEIVER,
	HWSIM_ATTR_ADDR_TRANSMITTER,
	HWSIM_ATTR_MSG_LEN,
	HWSIM_ATTR_MSG,
	HWSIM_ATTR_FLAGS,
	HWSIM_ATTR_RX_RATE,
	HWSIM_ATTR_SIGNAL,
	HWSIM_ATTR_TX_INFO,
	HWSIM_ATTR_CB_SKB,
	__HWSIM_ATTR_MAX,
};
#define HWSIM_ATTR_MAX (__HWSIM_ATTR_MAX - 1)

static struct nla_policy hwsim_genl_policy[HWSIM_ATTR_MAX + 1] = {
	[HWSIM_ATTR_ADDR_RECEIVER] = { .type = NLA_UNSPEC, .len = 6*sizeof(u8) },
	[HWSIM_ATTR_ADDR_TRANSMITTER] = { .type = NLA_UNSPEC, .len = 6*sizeof(u8) },
	[HWSIM_ATTR_MSG_LEN] = { .type = NLA_U32 },
	[HWSIM_ATTR_MSG] = { .type = NLA_STRING },
	[HWSIM_ATTR_FLAGS] = { .type = NLA_U32 },
	[HWSIM_ATTR_RX_RATE] = { .type = NLA_U32},
	[HWSIM_ATTR_SIGNAL] = { .type = NLA_U32 },
	[HWSIM_ATTR_TX_INFO] = { .type = NLA_UNSPEC, .len = IEEE80211_TX_MAX_RATES*sizeof(struct ieee80211_tx_rate)},
	[HWSIM_ATTR_CB_SKB] = { .type = NLA_UNSPEC, .len = 48*sizeof(char) },
};

#define VERSION_NR 1

static struct genl_family hwsim_genl_family = {
	.id = GENL_ID_GENERATE,
	.hdrsize = 0,
	.name = "HWSIM",
	.version = VERSION_NR,
	.maxattr = HWSIM_ATTR_MAX,
};

enum {
	HWSIM_CMD_UNSPEC,
	HWSIM_CMD_REGISTER,
	HWSIM_CMD_FRAME,
	HWSIM_CMD_TX_INFO_FRAME,
	__HWSIM_CMD_MAX,
};
#define HWSIM_CMD_MAX (_HWSIM_CMD_MAX - 1)
