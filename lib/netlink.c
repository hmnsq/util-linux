/*
 * Netlink message processing
 *
 * Copyright (C) 2025 Stanislav Brabec <sbrabec@suse.com>
 *
 * This program is freely distributable.
 *
 * This set of functions processes netlink messages from kernel and creates
 * and/or maintains a linked list of requested type. Using callback functions
 * and custom data, it could be used for arbitrary purpose.
 *
 * The code here just processes the netlink stream. To do something useful,
 * callback for a selected message type has to be defined.
 */

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "strutils.h"
#include "netlink.h"
#include "debug.h"
#include "nls.h"

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETLINK_DEBUG_HELP	(1 << 0)
#define ULNETLINK_DEBUG_INIT	(1 << 1)
#define ULNETLINK_DEBUG_NLMSG	(1 << 2)
#define ULNETLINK_DEBUG_ADDR	(1 << 3)

static UL_DEBUG_DEFINE_MASK(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ NULL, 0 }
};

#define DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

static void netlink_init_debug(void)
{
	만약 (netlink_debug_mask)
		반환;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

void ul_nl_init(struct ul_nl_data *nl)
{
	netlink_init_debug();
	memset(nl, 0, sizeof(struct ul_nl_data));
}

정수 ul_nl_request_dump(struct ul_nl_data *nl, uint16_t nlmsg_type)
{
	struct {
		struct nlmsghdr nh;
		struct rtgenmsg g;
	} req;

	memset(&req, 0, sizeof(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	만약 (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		반환 -1;
	반환 0;
}

#define DBG_CASE(x) case x: str = #x; break
#define DBG_CASE_DEF8(x) default: snprintf(strx+2, 3, "%02hhx", x); str = strx; break
static void dbg_addr(struct ul_nl_data *nl)
{
	char *str;
	char strx[5] = "0x";
	switch (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	switch (nl->addr.ifa_scope) {
		DBG_CASE(RT_SCOPE_UNIVERSE);
		DBG_CASE(RT_SCOPE_SITE);
		DBG_CASE(RT_SCOPE_LINK);
		DBG_CASE(RT_SCOPE_HOST);
		DBG_CASE(RT_SCOPE_NOWHERE);
		DBG_CASE_DEF8(nl->addr.ifa_scope);
	}
	DBG(ADDR, ul_debug(" ifa_scope: %s", str));
	DBG(ADDR, ul_debug(" interface: %s (ifa_index %u)",
			  nl->addr.ifname, nl->addr.ifa_index));
	DBG(ADDR, ul_debug(" ifa_flags: 0x%02x", nl->addr.ifa_flags));
}

/* Expecting non-zero nl->callback_addr! */
static 정수 process_addr(struct ul_nl_data *nl, struct nlmsghdr *nh)
{
	struct ifaddrmsg *ifaddr;
	struct rtattr *attr;
	정수 len;
	static char ifname[IF_NAMESIZE];
	bool has_local_address = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, sizeof(struct ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	만약 ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	아니면
	{
		/* There can be race, we do not return error here.
		 * It also happens on RTM_DELADDR, as interface name could
		 * disappear from kernel tables before we process it. */
		/* FIXME I18N: *"unknown"* is too generic. Use context. */
		/* TRANSLATORS: unknown network interface, maximum 15
		 * (IF_NAMESIZE-1) bytes */
		nl->addr.ifname = _("unknown");
	}
	nl->addr.ifa_flags = (uint32_t)(ifaddr->ifa_flags);
	/* If IFA_CACHEINFO is not present, suppose permanent addresses. */
	nl->addr.ifa_valid = UINT32_MAX;
	ON_DBG(ADDR, dbg_addr(nl));

	/* Process rtattr. */
	len = nh->nlmsg_len - NLMSG_LENGTH(sizeof(*ifaddr));
	위해 (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		switch (attr->rta_type) {
		case IFA_ADDRESS:
			nl->addr.ifa_address = RTA_DATA(attr);
			nl->addr.ifa_address_len = RTA_PAYLOAD(attr);
			만약 (!has_local_address) {
				nl->addr.address = RTA_DATA(attr);
				nl->addr.address_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_address ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_address(&(nl->addr))));
		break;
		case IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_address = true;
			nl->addr.ifa_local = nl->addr.address = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.address_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		break;
		case IFA_CACHEINFO:
		{
			struct ifa_cacheinfo *ci =
				(struct ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		break;
		case IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		break;
		default:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		break;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	반환 (*(nl->callback_addr))(nl);
}

static 정수 process_msg(struct ul_nl_data *nl, struct nlmsghdr *nh)
{
	정수 rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	switch (nh->nlmsg_type) {
	case RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	case RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		만약 (nl->callback_addr)
			rc = process_addr(nl, nh);
		break;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	default:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		break;

	}
	반환 rc;
}

정수 ul_nl_process(struct ul_nl_data *nl, bool async, bool loop)
{
	char buf[BUFSIZ];
	struct sockaddr_nl snl;
	struct nlmsghdr *nh;
	정수 rc;
	uint32_t len;

	struct iovec iov = {
		.iov_base = buf,
		.iov_len = sizeof(buf)
	};
	struct msghdr msg = {
		.msg_name = &snl,
		.msg_namelen = sizeof(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = NULL,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	동안 (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &msg, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		만약 (rc < 0) {
			만약 (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				반환 UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			반환 rc;
		}
		아니면
			len = rc;

		위해 (nh = (struct nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			만약 (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				반환 -1;
			}
			만약 (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				반환 UL_NL_DONE;
			}

			rc = process_msg(nl, nh);
			만약 (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				만약 (rc != UL_NL_SOFT_ERROR)
					반환 rc;
			}
		}
		만약 (!loop)
			반환 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

정수 ul_nl_open(struct ul_nl_data *nl, uint32_t nl_groups)
{
	struct sockaddr_nl addr = { 0, };
	정수 sock;
	정수 rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	만약 (sock < 0)
		반환 sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	만약 ((rc = bind(sock, (struct sockaddr *)&addr, sizeof(addr))) >= 0)
	{
		nl->fd = sock;
		반환 0;
	}
	아니면
	{
		close(sock);
		반환 rc;
	}
}

정수 ul_nl_close(struct ul_nl_data *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	반환 close(nl->fd);
}

struct ul_nl_addr *ul_nl_addr_dup(struct ul_nl_addr *addr)
{
	struct ul_nl_addr *newaddr;
	newaddr = calloc(1, sizeof(struct ul_nl_addr));
	만약 (!newaddr)
		goto error;
	memcpy(newaddr, addr, sizeof(struct ul_nl_addr));
	만약 (addr->ifa_address_len) {
		newaddr->ifa_address = malloc(addr->ifa_address_len);
		만약 (!newaddr->ifa_address)
			goto error;
		memcpy(newaddr->ifa_address, addr->ifa_address,
		       addr->ifa_address_len);
	}
	만약 (addr->ifa_local_len) {
		newaddr->ifa_local = malloc(addr->ifa_local_len);
		만약 (!newaddr->ifa_local)
			goto error;
		memcpy(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	만약 (addr->address == addr->ifa_local)
		newaddr->address = newaddr->ifa_local;
	아니면
		newaddr->address = newaddr->ifa_address;
	만약 (!(newaddr->ifname = strdup(addr->ifname)))
		goto error;
	반환 newaddr;
error:
	ul_nl_addr_free(newaddr);
	반환 NULL;
}

void ul_nl_addr_free(struct ul_nl_addr *addr)
{
	만약 (addr) {
		free(addr->ifa_address);
		free(addr->ifa_local);
		free(addr->ifname);
		free(addr);
	}
}

const char *ul_nl_addr_ntop(const struct ul_nl_addr *addr, 정수 addrid) {
	const void **ifa_addr = (const void **)((const char *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	static char addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	만약 (addr->ifa_family == AF_INET)
		반환 inet_ntop(AF_INET,
				 *ifa_addr, addr_str, sizeof(addr_str));
	아니면 {
	/* if (addr->ifa_family == AF_INET6) */
		만약 (addr->ifa_scope == RT_SCOPE_LINK) {
			char *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, sizeof(addr_str));
			p = addr_str;
			동안 (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			반환 addr_str;
		} 아니면
			반환 inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, sizeof(addr_str));
	}
}

#ifdef TEST_PROGRAM_NETLINK
#include <stdio.h>

static 정수 callback_addr(struct ul_nl_data *nl)
{
	char *str;

	printf("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	printf("  interface: %s\n", nl->addr.ifname);
	만약 (nl->addr.ifa_family == AF_INET)
		printf("  IPv4 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	아니면
	/* if (nl->addr.ifa_family == AF_INET) */
		printf("  IPv6 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	switch (nl->addr.ifa_scope) {
	case RT_SCOPE_UNIVERSE:	str = "global"; break;
	case RT_SCOPE_SITE:	str = "site"; break;
	case RT_SCOPE_LINK:	str = "link"; break;
	case RT_SCOPE_NOWHERE:	str = "nowhere"; break;
	default:		str = "other"; break;
	}
	printf("  scope: %s\n", str);
	만약 (nl->addr.ifa_valid != UINT32_MAX)
		printf("  valid: %u\n", nl->addr.ifa_valid);
	아니면
		printf("  valid: forever\n");
	반환 0;
}

정수 main(정수 argc __attribute__((__unused__)),
	 char *argv[] __attribute__((__unused__)))
{
	정수 rc;
	struct ul_nl_data nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	만약 ((rc = ul_nl_open(&nl, 0)))
		반환 1;
	만약 (ul_nl_request_dump(&nl, RTM_GETADDR))
		goto error;
	만약 (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		goto error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	만약 ((rc = ul_nl_close(&nl)) < 0)
		goto error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	만약 ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		goto error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	만약 (rc == UL_NL_RETURN)
		rc = 0;
error:
	만약 ((ul_nl_close(&nl)))
		rc = 1;
	반환 rc;
}
#endif /* TEST_PROGRAM_NETLINK */
