#ifndef _LM_LOGIN_KEEP_H_
#define _LM_LOGIN_KEEP_H_

#define MAX_TRY_TIMES	(3)
#define INTERVAL_TIME_30 (30)
#define INTERVAL_TIME_600 (600)
#define ETHER_ADDR_LEN (6)

#define LM_LOGIN_URL	"sdk.wiair.com"
#define LM_LOGIN_PORT	(9210)
#define LM_RT_CPU	"BCM5357"
#define LM_RT_MEM	"8"

//#define LM_RT_VER	"V11.13.01.16_cn"
#define LM_RT_VER W311R_ECOS_SV"_"NORMAL_WEB_VERSION
#define LM_RT_FIRM	"Tenda"

#define LM_CMD_LOIN_KEY (1000)
#define LM_CMD_REG_REQ (1004)
#define LM_CMD_REG_ACK (1005)
#define LM_CMD_LOIN_REQ (1006)
#define LM_CMD_LOIN_ACK (1007)
#define LM_CMD_LOIN_RED (1008)

#define LM_CMD_KEEP_REQ (1101)
#define LM_CMD_KEEP_ACK (1102)

#define LM_CMD_KEEP_ONLINE_REQ (1104)
#define LM_CMD_KEEP_ONLINE_ACK (1105)
#define LM_CMD_KEEP_CONF_REQ (1106)
#define LM_CMD_KEEP_CONF_ACK (1107)
#define LM_CMD_KEEP_VERSION_REQ (1108)
#define LM_CMD_KEEP_VERSION_ACK (1109)

#define LM_CMD_KEEP_ONLINE_REQ_PERIOD (RC_MODULE_1S_TIME*60*30)

#define CONF_AES_TMP "/Dir_For_DD/config"
#define SERVER_PORT (80)

#define DIFF_RULE_DELIM ";"
#define SAME_RULE_DELIM "|"

#define HOST_STR "Host: "
#define HOST_LEN 6
#define REFERER_STR "Referer: "
#define REFERER_LEN 9
#define UA_STR "User-Agent: "
#define UA_LEN 12
#define COOKIE_STR "Cookie: "
#define COOKIE_LEN 8
#define CONTENT_T_STR "Content-Type: "
#define CONTENT_T_LEN 14
#define CONTENT_L_STR "Content-Length: "
#define CONTENT_L_LEN 16

enum LOGIN_KEEP_STATUS
{
	LM_INIT = 0,
	LM_CONN_LOGIN,
	LM_KEY_OK,
	LM_REG_ACK,
	LM_LOGIN_ACK,
	LM_LOGIN_RED,
	LM_CONN_KEEP,
	LM_KEEP_ACK,
	LM_END
};

enum get_file_status
{
	OK = 0,
	DOWNLOAD_FAIL,
	DOWNLOAD_TIMEOUT,
	FILE_NOT_FOUNT,
	DIR_NOT_EXIST,
	LENGTH_ERROR,
	NOT_FOR_THIS_PRO,
	CHECK_FILE_ERROR,
	WRITE_FILE_FAIL
};

enum keep_conf_flag
{
	KEEP_CONF_URL_CONF = 1,
	KEEP_CONF_URL_WHITE,
	KEEP_CONF_DATA_RULE = 1,
	KEEP_CONF_JS_RULE,
	KEEP_CONF_LOG_URL,
	KEEP_CONF_FLAG_MAX
};

enum keep_ver_action
{
	KEEP_VER_IMM = 1,
	KEEP_VER_DELAY,
	KEEP_VER_IDLE,
	KEEP_VER_ACT_MAX
};

struct lm_login_keep_hdr {
	uint16 length;
	uint16 cmd;
};

struct lm_login_key {
	int32 key;
};

struct lm_register_req {
	struct lm_login_keep_hdr hdr;
	int32 check;
	uint8 maclen;
	int8 *mac;
	uint8 cpulen;
	int8 *cpu;
	uint8 memlen;
	int8 *mem;
};

struct lm_register_ack {
	uint16 result;
	uint32 id;
	uint32 ip;
} __attribute__ ((aligned(1), packed));

struct lm_login_req {
	struct lm_login_keep_hdr hdr;
	int32 check;
	uint32 id;
};

struct lm_login_ack {
	uint16 result;
	uint32 ip;
} __attribute__ ((aligned(1), packed));

struct lm_login_redirect {
	uint32 ip;
	uint16 port;
	int32 checksum;
} __attribute__ ((aligned(1), packed));

struct lm_keep_req {
	struct lm_login_keep_hdr hdr;
	uint32 id;
	int32 check;
	uint8 verlen;
	int8 *version;
	uint8 typelen;
	int8 *type;
};

struct lm_keep_ack {
	uint16 result;
	int32 check;
} __attribute__ ((aligned(1), packed));

struct lm_conf_req {
	struct lm_login_keep_hdr hdr;
	int32 check;
	uint16 flag;
	uint8 verlen;
	int8 *version;
	uint8 typelen;
	int8 *type;
};

struct lm_conf_ack {
	uint16 result;
	uint16 flag;
	uint8 urllen;
	int8 *url;
	uint8 md5len;
	int8* md5;
} __attribute__ ((aligned(1), packed));

struct lm_router_online_req {
	struct lm_login_keep_hdr hdr;
	int32 check;
};

struct lm_router_online_ack {
	uint32 secs;
} __attribute__ ((aligned(1), packed));

struct lm_keep_version_req {
	struct lm_login_keep_hdr hdr;
	int32 check;
	uint8 verlen;
	int8 *version;
	uint8 typelen;
	int8 *type;
};

struct lm_keep_version {
	uint8 action;
	uint8 verlen;
	int8 *version;
	uint8 urllen;
	int8 *url;
	uint8 md5len;
	int8* md5;
} __attribute__ ((aligned(1), packed));

struct rule_str {
	int str_len;
	char *str;
	struct rule_str *next;
};

#endif

