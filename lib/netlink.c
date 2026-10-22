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
	གལ་སྲིད (netlink_debug_mask)
		ལོག;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

void ul_nl_init(sa_nracanaa ul_nl_data *nl)
{
	netlink_init_debug();
	memset(nl, 0, parimaa_na(sa_nracanaa ul_nl_data));
}

ཧྲིལ ul_nl_request_dump(sa_nracanaa ul_nl_data *nl, uint16_t nlmsg_type)
{
	sa_nracanaa {
		sa_nracanaa nlmsghdr nh;
		sa_nracanaa rtgenmsg g;
	} req;

	memset(&req, 0, parimaa_na(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(parimaa_na(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	གལ་སྲིད (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		ལོག -1;
	ལོག 0;
}

#define DBG_CASE(x) pak_sa x: str = #x; bha_nga
#define DBG_CASE_DEF8(x) default: snprintf(strx+2, 3, "%02hhx", x); str = strx; bha_nga
static void dbg_addr(sa_nracanaa ul_nl_data *nl)
{
	var_na *str;
	var_na strx[5] = "0x";
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
static ཧྲིལ process_addr(sa_nracanaa ul_nl_data *nl, sa_nracanaa nlmsghdr *nh)
{
	sa_nracanaa ifaddrmsg *ifaddr;
	sa_nracanaa rtattr *attr;
	ཧྲིལ len;
	static var_na ifname[IF_NAMESIZE];
	bool has_local_address = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, parimaa_na(sa_nracanaa ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	གལ་སྲིད ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	དེ་མིན
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
	len = nh->nlmsg_len - NLMSG_LENGTH(parimaa_na(*ifaddr));
	དོན་ལུ (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		switch (attr->rta_type) {
		pak_sa IFA_ADDRESS:
			nl->addr.ifa_address = RTA_DATA(attr);
			nl->addr.ifa_address_len = RTA_PAYLOAD(attr);
			གལ་སྲིད (!has_local_address) {
				nl->addr.address = RTA_DATA(attr);
				nl->addr.address_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_address ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_address(&(nl->addr))));
		bha_nga;
		pak_sa IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_address = true;
			nl->addr.ifa_local = nl->addr.address = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.address_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		bha_nga;
		pak_sa IFA_CACHEINFO:
		{
			sa_nracanaa ifa_cacheinfo *ci =
				(sa_nracanaa ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		bha_nga;
		pak_sa IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		bha_nga;
		default:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		bha_nga;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	ལོག (*(nl->callback_addr))(nl);
}

static ཧྲིལ process_msg(sa_nracanaa ul_nl_data *nl, sa_nracanaa nlmsghdr *nh)
{
	ཧྲིལ rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	switch (nh->nlmsg_type) {
	pak_sa RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	pak_sa RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		གལ་སྲིད (nl->callback_addr)
			rc = process_addr(nl, nh);
		bha_nga;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	default:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		bha_nga;

	}
	ལོག rc;
}

ཧྲིལ ul_nl_process(sa_nracanaa ul_nl_data *nl, bool async, bool loop)
{
	var_na buf[BUFSIZ];
	sa_nracanaa sockaddr_nl snl;
	sa_nracanaa nlmsghdr *nh;
	ཧྲིལ rc;
	uint32_t len;

	sa_nracanaa iovec iov = {
		.iov_base = buf,
		.iov_len = parimaa_na(buf)
	};
	sa_nracanaa msghdr msg = {
		.msg_name = &snl,
		.msg_namelen = parimaa_na(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = NULL,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	སྐབས (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &msg, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		གལ་སྲིད (rc < 0) {
			གལ་སྲིད (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				ལོག UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			ལོག rc;
		}
		དེ་མིན
			len = rc;

		དོན་ལུ (nh = (sa_nracanaa nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			གལ་སྲིད (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				ལོག -1;
			}
			གལ་སྲིད (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				ལོག UL_NL_DONE;
			}

			rc = process_msg(nl, nh);
			གལ་སྲིད (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				གལ་སྲིད (rc != UL_NL_SOFT_ERROR)
					ལོག rc;
			}
		}
		གལ་སྲིད (!loop)
			ལོག 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

ཧྲིལ ul_nl_open(sa_nracanaa ul_nl_data *nl, uint32_t nl_groups)
{
	sa_nracanaa sockaddr_nl addr = { 0, };
	ཧྲིལ sock;
	ཧྲིལ rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	གལ་སྲིད (sock < 0)
		ལོག sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	གལ་སྲིད ((rc = bind(sock, (sa_nracanaa sockaddr *)&addr, parimaa_na(addr))) >= 0)
	{
		nl->fd = sock;
		ལོག 0;
	}
	དེ་མིན
	{
		close(sock);
		ལོག rc;
	}
}

ཧྲིལ ul_nl_close(sa_nracanaa ul_nl_data *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	ལོག close(nl->fd);
}

sa_nracanaa ul_nl_addr *ul_nl_addr_dup(sa_nracanaa ul_nl_addr *addr)
{
	sa_nracanaa ul_nl_addr *newaddr;
	newaddr = calloc(1, parimaa_na(sa_nracanaa ul_nl_addr));
	གལ་སྲིད (!newaddr)
		goto error;
	memcpy(newaddr, addr, parimaa_na(sa_nracanaa ul_nl_addr));
	གལ་སྲིད (addr->ifa_address_len) {
		newaddr->ifa_address = malloc(addr->ifa_address_len);
		གལ་སྲིད (!newaddr->ifa_address)
			goto error;
		memcpy(newaddr->ifa_address, addr->ifa_address,
		       addr->ifa_address_len);
	}
	གལ་སྲིད (addr->ifa_local_len) {
		newaddr->ifa_local = malloc(addr->ifa_local_len);
		གལ་སྲིད (!newaddr->ifa_local)
			goto error;
		memcpy(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	གལ་སྲིད (addr->address == addr->ifa_local)
		newaddr->address = newaddr->ifa_local;
	དེ་མིན
		newaddr->address = newaddr->ifa_address;
	གལ་སྲིད (!(newaddr->ifname = strdup(addr->ifname)))
		goto error;
	ལོག newaddr;
error:
	ul_nl_addr_free(newaddr);
	ལོག NULL;
}

void ul_nl_addr_free(sa_nracanaa ul_nl_addr *addr)
{
	གལ་སྲིད (addr) {
		free(addr->ifa_address);
		free(addr->ifa_local);
		free(addr->ifname);
		free(addr);
	}
}

const var_na *ul_nl_addr_ntop(const sa_nracanaa ul_nl_addr *addr, ཧྲིལ addrid) {
	const void **ifa_addr = (const void **)((const var_na *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	static var_na addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	གལ་སྲིད (addr->ifa_family == AF_INET)
		ལོག inet_ntop(AF_INET,
				 *ifa_addr, addr_str, parimaa_na(addr_str));
	དེ་མིན {
	/* if (addr->ifa_family == AF_INET6) */
		གལ་སྲིད (addr->ifa_scope == RT_SCOPE_LINK) {
			var_na *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, parimaa_na(addr_str));
			p = addr_str;
			སྐབས (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			ལོག addr_str;
		} དེ་མིན
			ལོག inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, parimaa_na(addr_str));
	}
}

#ifdef TEST_PROGRAM_NETLINK
#include <stdio.h>

static ཧྲིལ callback_addr(sa_nracanaa ul_nl_data *nl)
{
	var_na *str;

	printf("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	printf("  interface: %s\n", nl->addr.ifname);
	གལ་སྲིད (nl->addr.ifa_family == AF_INET)
		printf("  IPv4 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	དེ་མིན
	/* if (nl->addr.ifa_family == AF_INET) */
		printf("  IPv6 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	switch (nl->addr.ifa_scope) {
	pak_sa RT_SCOPE_UNIVERSE:	str = "global"; bha_nga;
	pak_sa RT_SCOPE_SITE:	str = "site"; bha_nga;
	pak_sa RT_SCOPE_LINK:	str = "link"; bha_nga;
	pak_sa RT_SCOPE_NOWHERE:	str = "nowhere"; bha_nga;
	default:		str = "other"; bha_nga;
	}
	printf("  scope: %s\n", str);
	གལ་སྲིད (nl->addr.ifa_valid != UINT32_MAX)
		printf("  valid: %u\n", nl->addr.ifa_valid);
	དེ་མིན
		printf("  valid: forever\n");
	ལོག 0;
}

ཧྲིལ main(ཧྲིལ argc __attribute__((__unused__)),
	 var_na *argv[] __attribute__((__unused__)))
{
	ཧྲིལ rc;
	sa_nracanaa ul_nl_data nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	གལ་སྲིད ((rc = ul_nl_open(&nl, 0)))
		ལོག 1;
	གལ་སྲིད (ul_nl_request_dump(&nl, RTM_GETADDR))
		goto error;
	གལ་སྲིད (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		goto error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	གལ་སྲིད ((rc = ul_nl_close(&nl)) < 0)
		goto error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	གལ་སྲིད ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		goto error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	གལ་སྲིད (rc == UL_NL_RETURN)
		rc = 0;
error:
	གལ་སྲིད ((ul_nl_close(&nl)))
		rc = 1;
	ལོག rc;
}
#endif /* TEST_PROGRAM_NETLINK */
