enum {
	HWSIM_A_UNSPEC,
	HWSIM_A_ADDR_RECEIVER,
	HWSIM_A_ADDR_TRANSMITTER,
	HWSIM_A_MSG_LEN,
	HWSIM_A_MSG,
	__HWSIM_A_MAX,
};
#define HWSIM_A_MAX (__HWSIM_A_MAX - 1)

static struct nla_policy hwsim_genl_policy[HWSIM_A_MAX + 1] = {
	[HWSIM_A_ADDR_RECEIVER] = { .type = NLA_UNSPEC, .len = 6*sizeof(u8) },
	[HWSIM_A_ADDR_TRANSMITTER] = { .type = NLA_UNSPEC, .len = 6*sizeof(u8) },
	[HWSIM_A_MSG_LEN] = { .type = NLA_U32 },
	[HWSIM_A_MSG] = { .type = NLA_STRING },
};

#define VERSION_NR 1

static struct genl_family hwsim_genl_family = {
	.id = GENL_ID_GENERATE,
	.hdrsize = 0,
	.name = "HWSIM",
	.version = VERSION_NR, 
	.maxattr = HWSIM_A_MAX,
};

enum {
	HWSIM_C_UNSPEC,
	HWSIM_C_REGISTER,
	HWSIM_C_FRAME,
	__HWSIM_C_MAX,
};
#define HWSIM_C_MAX (_HWSIM_C_MAX - 1)
