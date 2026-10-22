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

#ಅನ್ತರ್ಭಾವ <sys/socket.h>
#ಅನ್ತರ್ಭಾವ <arpa/inet.h>
#ಅನ್ತರ್ಭಾವ <netinet/in.h>
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "netlink.h"
#ಅನ್ತರ್ಭಾವ "debug.h"
#ಅನ್ತರ್ಭಾವ "nls.h"

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETLINK_DEBUG_HELP	(1 << 0)
#define ULNETLINK_DEBUG_INIT	(1 << 1)
#define ULNETLINK_DEBUG_NLMSG	(1 << 2)
#define ULNETLINK_DEBUG_ADDR	(1 << 3)

ಸ್ಥಿರ UL_DEBUG_DEFINE_MASK(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ ರಿಕ್ತ, 0 }
};

#define DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

ಸ್ಥಿರ ಶೂನ್ಯ netlink_init_debug(ಶೂನ್ಯ)
{
	ಯದಿ (netlink_debug_mask)
		ಪ್ರತ್ಯಾಗಮ;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

ಶೂನ್ಯ ul_nl_init(ಸಂರಚನಾ ul_nl_data *nl)
{
	netlink_init_debug();
	memset(nl, 0, ಪರಿಮಾಣ(ಸಂರಚನಾ ul_nl_data));
}

ಪೂರ್ಣಾಙ್ಕ ul_nl_request_dump(ಸಂರಚನಾ ul_nl_data *nl, uint16_t nlmsg_type)
{
	ಸಂರಚನಾ {
		ಸಂರಚನಾ nlmsghdr nh;
		ಸಂರಚನಾ rtgenmsg g;
	} req;

	memset(&req, 0, ಪರಿಮಾಣ(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(ಪರಿಮಾಣ(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	ಯದಿ (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		ಪ್ರತ್ಯಾಗಮ -1;
	ಪ್ರತ್ಯಾಗಮ 0;
}

#define DBG_CASE(x) ಪಕ್ಷ x: str = #x; ಭಙ್ಗ
#define DBG_CASE_DEF8(x) ಸ್ವಭಾವ: snprintf(strx+2, 3, "%02hhx", x); str = strx; ಭಙ್ಗ
ಸ್ಥಿರ ಶೂನ್ಯ dbg_addr(ಸಂರಚನಾ ul_nl_data *nl)
{
	ವರ್ಣ *str;
	ವರ್ಣ strx[5] = "0x";
	ವಿಕಲ್ಪ (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	ವಿಕಲ್ಪ (nl->addr.ifa_scope) {
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ process_addr(ಸಂರಚನಾ ul_nl_data *nl, ಸಂರಚನಾ nlmsghdr *nh)
{
	ಸಂರಚನಾ ifaddrmsg *ifaddr;
	ಸಂರಚನಾ rtattr *attr;
	ಪೂರ್ಣಾಙ್ಕ len;
	ಸ್ಥಿರ ವರ್ಣ ifname[IF_NAMESIZE];
	bool has_local_address = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, ಪರಿಮಾಣ(ಸಂರಚನಾ ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	ಯದಿ ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	ಅನ್ಯಥಾ
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
	len = nh->nlmsg_len - NLMSG_LENGTH(ಪರಿಮಾಣ(*ifaddr));
	ಪ್ರತಿ (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		ವಿಕಲ್ಪ (attr->rta_type) {
		ಪಕ್ಷ IFA_ADDRESS:
			nl->addr.ifa_address = RTA_DATA(attr);
			nl->addr.ifa_address_len = RTA_PAYLOAD(attr);
			ಯದಿ (!has_local_address) {
				nl->addr.address = RTA_DATA(attr);
				nl->addr.address_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_address ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_address(&(nl->addr))));
		ಭಙ್ಗ;
		ಪಕ್ಷ IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_address = true;
			nl->addr.ifa_local = nl->addr.address = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.address_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		ಭಙ್ಗ;
		ಪಕ್ಷ IFA_CACHEINFO:
		{
			ಸಂರಚನಾ ifa_cacheinfo *ci =
				(ಸಂರಚನಾ ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		ಭಙ್ಗ;
		ಪಕ್ಷ IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		ಭಙ್ಗ;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	ಪ್ರತ್ಯಾಗಮ (*(nl->callback_addr))(nl);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ process_ಸಂದೇಶ(ಸಂರಚನಾ ul_nl_data *nl, ಸಂರಚನಾ nlmsghdr *nh)
{
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	ವಿಕಲ್ಪ (nh->nlmsg_type) {
	ಪಕ್ಷ RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	ಪಕ್ಷ RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		ಯದಿ (nl->callback_addr)
			rc = process_addr(nl, nh);
		ಭಙ್ಗ;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	ಸ್ವಭಾವ:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		ಭಙ್ಗ;

	}
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_nl_process(ಸಂರಚನಾ ul_nl_data *nl, bool async, bool loop)
{
	ವರ್ಣ buf[BUFSIZ];
	ಸಂರಚನಾ sockaddr_nl snl;
	ಸಂರಚನಾ nlmsghdr *nh;
	ಪೂರ್ಣಾಙ್ಕ rc;
	uint32_t len;

	ಸಂರಚನಾ iovec iov = {
		.iov_base = buf,
		.iov_len = ಪರಿಮಾಣ(buf)
	};
	ಸಂರಚನಾ msghdr ಸಂದೇಶ = {
		.msg_name = &snl,
		.msg_namelen = ಪರಿಮಾಣ(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = ರಿಕ್ತ,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	ಯಾವತ್ (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &ಸಂದೇಶ, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		ಯದಿ (rc < 0) {
			ಯದಿ (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				ಪ್ರತ್ಯಾಗಮ UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			ಪ್ರತ್ಯಾಗಮ rc;
		}
		ಅನ್ಯಥಾ
			len = rc;

		ಪ್ರತಿ (nh = (ಸಂರಚನಾ nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			ಯದಿ (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				ಪ್ರತ್ಯಾಗಮ -1;
			}
			ಯದಿ (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				ಪ್ರತ್ಯಾಗಮ UL_NL_DONE;
			}

			rc = process_ಸಂದೇಶ(nl, nh);
			ಯದಿ (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				ಯದಿ (rc != UL_NL_SOFT_ERROR)
					ಪ್ರತ್ಯಾಗಮ rc;
			}
		}
		ಯದಿ (!loop)
			ಪ್ರತ್ಯಾಗಮ 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

ಪೂರ್ಣಾಙ್ಕ ul_nl_open(ಸಂರಚನಾ ul_nl_data *nl, uint32_t nl_groups)
{
	ಸಂರಚನಾ sockaddr_nl addr = { 0, };
	ಪೂರ್ಣಾಙ್ಕ sock;
	ಪೂರ್ಣಾಙ್ಕ rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	ಯದಿ (sock < 0)
		ಪ್ರತ್ಯಾಗಮ sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	ಯದಿ ((rc = bind(sock, (ಸಂರಚನಾ sockaddr *)&addr, ಪರಿಮಾಣ(addr))) >= 0)
	{
		nl->fd = sock;
		ಪ್ರತ್ಯಾಗಮ 0;
	}
	ಅನ್ಯಥಾ
	{
		close(sock);
		ಪ್ರತ್ಯಾಗಮ rc;
	}
}

ಪೂರ್ಣಾಙ್ಕ ul_nl_close(ಸಂರಚನಾ ul_nl_data *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	ಪ್ರತ್ಯಾಗಮ close(nl->fd);
}

ಸಂರಚನಾ ul_nl_addr *ul_nl_addr_dup(ಸಂರಚನಾ ul_nl_addr *addr)
{
	ಸಂರಚನಾ ul_nl_addr *newaddr;
	newaddr = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ ul_nl_addr));
	ಯದಿ (!newaddr)
		ಗಚ್ಛ error;
	memcpy(newaddr, addr, ಪರಿಮಾಣ(ಸಂರಚನಾ ul_nl_addr));
	ಯದಿ (addr->ifa_address_len) {
		newaddr->ifa_address = malloc(addr->ifa_address_len);
		ಯದಿ (!newaddr->ifa_address)
			ಗಚ್ಛ error;
		memcpy(newaddr->ifa_address, addr->ifa_address,
		       addr->ifa_address_len);
	}
	ಯದಿ (addr->ifa_local_len) {
		newaddr->ifa_local = malloc(addr->ifa_local_len);
		ಯದಿ (!newaddr->ifa_local)
			ಗಚ್ಛ error;
		memcpy(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	ಯದಿ (addr->address == addr->ifa_local)
		newaddr->address = newaddr->ifa_local;
	ಅನ್ಯಥಾ
		newaddr->address = newaddr->ifa_address;
	ಯದಿ (!(newaddr->ifname = strdup(addr->ifname)))
		ಗಚ್ಛ error;
	ಪ್ರತ್ಯಾಗಮ newaddr;
error:
	ul_nl_addr_free(newaddr);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಶೂನ್ಯ ul_nl_addr_free(ಸಂರಚನಾ ul_nl_addr *addr)
{
	ಯದಿ (addr) {
		ಮುಕ್ತ(addr->ifa_address);
		ಮುಕ್ತ(addr->ifa_local);
		ಮುಕ್ತ(addr->ifname);
		ಮುಕ್ತ(addr);
	}
}

ಧ್ರುವ ವರ್ಣ *ul_nl_addr_ntop(ಧ್ರುವ ಸಂರಚನಾ ul_nl_addr *addr, ಪೂರ್ಣಾಙ್ಕ addrid) {
	ಧ್ರುವ ಶೂನ್ಯ **ifa_addr = (ಧ್ರುವ ಶೂನ್ಯ **)((ಧ್ರುವ ವರ್ಣ *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	ಸ್ಥಿರ ವರ್ಣ addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	ಯದಿ (addr->ifa_family == AF_INET)
		ಪ್ರತ್ಯಾಗಮ inet_ntop(AF_INET,
				 *ifa_addr, addr_str, ಪರಿಮಾಣ(addr_str));
	ಅನ್ಯಥಾ {
	/* if (addr->ifa_family == AF_INET6) */
		ಯದಿ (addr->ifa_scope == RT_SCOPE_LINK) {
			ವರ್ಣ *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, ಪರಿಮಾಣ(addr_str));
			p = addr_str;
			ಯಾವತ್ (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			ಪ್ರತ್ಯಾಗಮ addr_str;
		} ಅನ್ಯಥಾ
			ಪ್ರತ್ಯಾಗಮ inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, ಪರಿಮಾಣ(addr_str));
	}
}

#ifdef TEST_PROGRAM_NETLINK
#ಅನ್ತರ್ಭಾವ <stdio.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ callback_addr(ಸಂರಚನಾ ul_nl_data *nl)
{
	ವರ್ಣ *str;

	printf("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	printf("  interface: %s\n", nl->addr.ifname);
	ಯದಿ (nl->addr.ifa_family == AF_INET)
		printf("  IPv4 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	ಅನ್ಯಥಾ
	/* if (nl->addr.ifa_family == AF_INET) */
		printf("  IPv6 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	ವಿಕಲ್ಪ (nl->addr.ifa_scope) {
	ಪಕ್ಷ RT_SCOPE_UNIVERSE:	str = "global"; ಭಙ್ಗ;
	ಪಕ್ಷ RT_SCOPE_SITE:	str = "site"; ಭಙ್ಗ;
	ಪಕ್ಷ RT_SCOPE_LINK:	str = "link"; ಭಙ್ಗ;
	ಪಕ್ಷ RT_SCOPE_NOWHERE:	str = "nowhere"; ಭಙ್ಗ;
	ಸ್ವಭಾವ:		str = "other"; ಭಙ್ಗ;
	}
	printf("  scope: %s\n", str);
	ಯದಿ (nl->addr.ifa_valid != UINT32_MAX)
		printf("  valid: %u\n", nl->addr.ifa_valid);
	ಅನ್ಯಥಾ
		printf("  valid: forever\n");
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc __attribute__((__unused__)),
	 ವರ್ಣ *argv[] __attribute__((__unused__)))
{
	ಪೂರ್ಣಾಙ್ಕ rc;
	ಸಂರಚನಾ ul_nl_data nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	ಯದಿ ((rc = ul_nl_open(&nl, 0)))
		ಪ್ರತ್ಯಾಗಮ 1;
	ಯದಿ (ul_nl_request_dump(&nl, RTM_GETADDR))
		ಗಚ್ಛ error;
	ಯದಿ (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		ಗಚ್ಛ error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	ಯದಿ ((rc = ul_nl_close(&nl)) < 0)
		ಗಚ್ಛ error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	ಯದಿ ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		ಗಚ್ಛ error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	ಯದಿ (rc == UL_NL_RETURN)
		rc = 0;
error:
	ಯದಿ ((ul_nl_close(&nl)))
		rc = 1;
	ಪ್ರತ್ಯಾಗಮ rc;
}
#endif /* TEST_PROGRAM_NETLINK */
