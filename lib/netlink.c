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

#അന്തര്ഭാവ <sys/socket.h>
#അന്തര്ഭാവ <arpa/inet.h>
#അന്തര്ഭാവ <netinet/in.h>
#അന്തര്ഭാവ "strutils.h"
#അന്തര്ഭാവ "netlink.h"
#അന്തര്ഭാവ "debug.h"
#അന്തര്ഭാവ "nls.h"

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETLINK_DEBUG_HELP	(1 << 0)
#define ULNETLINK_DEBUG_INIT	(1 << 1)
#define ULNETLINK_DEBUG_NLMSG	(1 << 2)
#define ULNETLINK_DEBUG_ADDR	(1 << 3)

സ്ഥിതം UL_DEBUG_DEFINE_MASK(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ ശൂന്യ, 0 }
};

#define DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

സ്ഥിതം ശൂന്യം netlink_init_debug(ശൂന്യം)
{
	എങ്കിൽ (netlink_debug_mask)
		തിരികെ;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

ശൂന്യം ul_nl_init(ഘടന ul_nl_data *nl)
{
	netlink_init_debug();
	memset(nl, 0, വലുപ്പം(ഘടന ul_nl_data));
}

പൂർണ്ണസംഖ്യ ul_nl_request_dump(ഘടന ul_nl_data *nl, uint16_t nlmsg_type)
{
	ഘടന {
		ഘടന nlmsghdr nh;
		ഘടന rtgenmsg g;
	} req;

	memset(&req, 0, വലുപ്പം(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(വലുപ്പം(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	എങ്കിൽ (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		തിരികെ -1;
	തിരികെ 0;
}

#define DBG_CASE(x) അവസ്ഥ x: str = #x; നിർത്തുക
#define DBG_CASE_DEF8(x) സ്ഥിരസ്ഥിതി: snprintf(strx+2, 3, "%02hhx", x); str = strx; നിർത്തുക
സ്ഥിതം ശൂന്യം dbg_addr(ഘടന ul_nl_data *nl)
{
	അക്ഷരം *str;
	അക്ഷരം strx[5] = "0x";
	തിരഞ്ഞെടുപ്പ് (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	തിരഞ്ഞെടുപ്പ് (nl->addr.ifa_scope) {
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
സ്ഥിതം പൂർണ്ണസംഖ്യ process_addr(ഘടന ul_nl_data *nl, ഘടന nlmsghdr *nh)
{
	ഘടന ifaddrmsg *ifaddr;
	ഘടന rtattr *attr;
	പൂർണ്ണസംഖ്യ len;
	സ്ഥിതം അക്ഷരം ifname[IF_NAMESIZE];
	bool has_local_address = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, വലുപ്പം(ഘടന ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	എങ്കിൽ ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	അല്ലെങ്കിൽ
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
	len = nh->nlmsg_len - NLMSG_LENGTH(വലുപ്പം(*ifaddr));
	വേണ്ടി (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		തിരഞ്ഞെടുപ്പ് (attr->rta_type) {
		അവസ്ഥ IFA_ADDRESS:
			nl->addr.ifa_address = RTA_DATA(attr);
			nl->addr.ifa_address_len = RTA_PAYLOAD(attr);
			എങ്കിൽ (!has_local_address) {
				nl->addr.address = RTA_DATA(attr);
				nl->addr.address_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_address ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_address(&(nl->addr))));
		നിർത്തുക;
		അവസ്ഥ IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_address = true;
			nl->addr.ifa_local = nl->addr.address = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.address_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		നിർത്തുക;
		അവസ്ഥ IFA_CACHEINFO:
		{
			ഘടന ifa_cacheinfo *ci =
				(ഘടന ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		നിർത്തുക;
		അവസ്ഥ IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		നിർത്തുക;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	തിരികെ (*(nl->callback_addr))(nl);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ process_സന്ദേശം(ഘടന ul_nl_data *nl, ഘടന nlmsghdr *nh)
{
	പൂർണ്ണസംഖ്യ rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	തിരഞ്ഞെടുപ്പ് (nh->nlmsg_type) {
	അവസ്ഥ RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	അവസ്ഥ RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		എങ്കിൽ (nl->callback_addr)
			rc = process_addr(nl, nh);
		നിർത്തുക;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	സ്ഥിരസ്ഥിതി:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		നിർത്തുക;

	}
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ ul_nl_process(ഘടന ul_nl_data *nl, bool async, bool loop)
{
	അക്ഷരം buf[BUFSIZ];
	ഘടന sockaddr_nl snl;
	ഘടന nlmsghdr *nh;
	പൂർണ്ണസംഖ്യ rc;
	uint32_t len;

	ഘടന iovec iov = {
		.iov_base = buf,
		.iov_len = വലുപ്പം(buf)
	};
	ഘടന msghdr സന്ദേശം = {
		.msg_name = &snl,
		.msg_namelen = വലുപ്പം(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = ശൂന്യ,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	ആകുമ്പോൾ (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &സന്ദേശം, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		എങ്കിൽ (rc < 0) {
			എങ്കിൽ (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				തിരികെ UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			തിരികെ rc;
		}
		അല്ലെങ്കിൽ
			len = rc;

		വേണ്ടി (nh = (ഘടന nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			എങ്കിൽ (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				തിരികെ -1;
			}
			എങ്കിൽ (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				തിരികെ UL_NL_DONE;
			}

			rc = process_സന്ദേശം(nl, nh);
			എങ്കിൽ (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				എങ്കിൽ (rc != UL_NL_SOFT_ERROR)
					തിരികെ rc;
			}
		}
		എങ്കിൽ (!loop)
			തിരികെ 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

പൂർണ്ണസംഖ്യ ul_nl_open(ഘടന ul_nl_data *nl, uint32_t nl_groups)
{
	ഘടന sockaddr_nl addr = { 0, };
	പൂർണ്ണസംഖ്യ sock;
	പൂർണ്ണസംഖ്യ rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	എങ്കിൽ (sock < 0)
		തിരികെ sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	എങ്കിൽ ((rc = bind(sock, (ഘടന sockaddr *)&addr, വലുപ്പം(addr))) >= 0)
	{
		nl->fd = sock;
		തിരികെ 0;
	}
	അല്ലെങ്കിൽ
	{
		close(sock);
		തിരികെ rc;
	}
}

പൂർണ്ണസംഖ്യ ul_nl_close(ഘടന ul_nl_data *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	തിരികെ close(nl->fd);
}

ഘടന ul_nl_addr *ul_nl_addr_dup(ഘടന ul_nl_addr *addr)
{
	ഘടന ul_nl_addr *newaddr;
	newaddr = calloc(1, വലുപ്പം(ഘടന ul_nl_addr));
	എങ്കിൽ (!newaddr)
		പോകുക error;
	memcpy(newaddr, addr, വലുപ്പം(ഘടന ul_nl_addr));
	എങ്കിൽ (addr->ifa_address_len) {
		newaddr->ifa_address = malloc(addr->ifa_address_len);
		എങ്കിൽ (!newaddr->ifa_address)
			പോകുക error;
		memcpy(newaddr->ifa_address, addr->ifa_address,
		       addr->ifa_address_len);
	}
	എങ്കിൽ (addr->ifa_local_len) {
		newaddr->ifa_local = malloc(addr->ifa_local_len);
		എങ്കിൽ (!newaddr->ifa_local)
			പോകുക error;
		memcpy(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	എങ്കിൽ (addr->address == addr->ifa_local)
		newaddr->address = newaddr->ifa_local;
	അല്ലെങ്കിൽ
		newaddr->address = newaddr->ifa_address;
	എങ്കിൽ (!(newaddr->ifname = strdup(addr->ifname)))
		പോകുക error;
	തിരികെ newaddr;
error:
	ul_nl_addr_free(newaddr);
	തിരികെ ശൂന്യ;
}

ശൂന്യം ul_nl_addr_free(ഘടന ul_nl_addr *addr)
{
	എങ്കിൽ (addr) {
		സ്വതന്ത്രം(addr->ifa_address);
		സ്വതന്ത്രം(addr->ifa_local);
		സ്വതന്ത്രം(addr->ifname);
		സ്വതന്ത്രം(addr);
	}
}

സ്ഥിരം അക്ഷരം *ul_nl_addr_ntop(സ്ഥിരം ഘടന ul_nl_addr *addr, പൂർണ്ണസംഖ്യ addrid) {
	സ്ഥിരം ശൂന്യം **ifa_addr = (സ്ഥിരം ശൂന്യം **)((സ്ഥിരം അക്ഷരം *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	സ്ഥിതം അക്ഷരം addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	എങ്കിൽ (addr->ifa_family == AF_INET)
		തിരികെ inet_ntop(AF_INET,
				 *ifa_addr, addr_str, വലുപ്പം(addr_str));
	അല്ലെങ്കിൽ {
	/* if (addr->ifa_family == AF_INET6) */
		എങ്കിൽ (addr->ifa_scope == RT_SCOPE_LINK) {
			അക്ഷരം *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, വലുപ്പം(addr_str));
			p = addr_str;
			ആകുമ്പോൾ (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			തിരികെ addr_str;
		} അല്ലെങ്കിൽ
			തിരികെ inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, വലുപ്പം(addr_str));
	}
}

#ifdef TEST_PROGRAM_NETLINK
#അന്തര്ഭാവ <stdio.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ callback_addr(ഘടന ul_nl_data *nl)
{
	അക്ഷരം *str;

	printf("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	printf("  interface: %s\n", nl->addr.ifname);
	എങ്കിൽ (nl->addr.ifa_family == AF_INET)
		printf("  IPv4 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	അല്ലെങ്കിൽ
	/* if (nl->addr.ifa_family == AF_INET) */
		printf("  IPv6 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	തിരഞ്ഞെടുപ്പ് (nl->addr.ifa_scope) {
	അവസ്ഥ RT_SCOPE_UNIVERSE:	str = "global"; നിർത്തുക;
	അവസ്ഥ RT_SCOPE_SITE:	str = "site"; നിർത്തുക;
	അവസ്ഥ RT_SCOPE_LINK:	str = "link"; നിർത്തുക;
	അവസ്ഥ RT_SCOPE_NOWHERE:	str = "nowhere"; നിർത്തുക;
	സ്ഥിരസ്ഥിതി:		str = "other"; നിർത്തുക;
	}
	printf("  scope: %s\n", str);
	എങ്കിൽ (nl->addr.ifa_valid != UINT32_MAX)
		printf("  valid: %u\n", nl->addr.ifa_valid);
	അല്ലെങ്കിൽ
		printf("  valid: forever\n");
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc __attribute__((__unused__)),
	 അക്ഷരം *argv[] __attribute__((__unused__)))
{
	പൂർണ്ണസംഖ്യ rc;
	ഘടന ul_nl_data nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	എങ്കിൽ ((rc = ul_nl_open(&nl, 0)))
		തിരികെ 1;
	എങ്കിൽ (ul_nl_request_dump(&nl, RTM_GETADDR))
		പോകുക error;
	എങ്കിൽ (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		പോകുക error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	എങ്കിൽ ((rc = ul_nl_close(&nl)) < 0)
		പോകുക error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	എങ്കിൽ ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		പോകുക error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	എങ്കിൽ (rc == UL_NL_RETURN)
		rc = 0;
error:
	എങ്കിൽ ((ul_nl_close(&nl)))
		rc = 1;
	തിരികെ rc;
}
#endif /* TEST_PROGRAM_NETLINK */
