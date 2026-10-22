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

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/socket.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <arpa/inet.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <netinet/in.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "netlink.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"

/*
 * Debug stuff (based on include/debug.h)
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULNETLINK_DEBUG_HELP	(1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULNETLINK_DEBUG_INIT	(1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULNETLINK_DEBUG_NLMSG	(1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULNETLINK_DEBUG_ADDR	(1 << 3)

ਸਥਿਰ UL_DEBUG_DEFINE_ਨਕਾਬ(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ ਨਿਰਾ, 0 }
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

ਸਥਿਰ ਖਾਲੀ netlink_init_debug(ਖਾਲੀ)
{
	ਜੇ (netlink_debug_ਨਕਾਬ)
		ਵਾਪਸ;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

ਖਾਲੀ ul_nl_init(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl)
{
	netlink_init_debug();
	memset(nl, 0, ਆਕਾਰ(ਬਣਤਰ ul_nl_ਅੰਕੜੇ));
}

ਪੂਰਨ ul_nl_request_dump(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl, uint16_t nlmsg_type)
{
	ਬਣਤਰ {
		ਬਣਤਰ nlmsghdr nh;
		ਬਣਤਰ rtgenmsg g;
	} req;

	memset(&req, 0, ਆਕਾਰ(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(ਆਕਾਰ(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	ਜੇ (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		ਵਾਪਸ -1;
	ਵਾਪਸ 0;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG_CASE(x) ਹਾਲਤ x: str = #x; ਰੋਕੋ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG_CASE_DEF8(x) ਮੂਲ: snprintf(strx+2, 3, "%02hhx", x); str = strx; ਰੋਕੋ
ਸਥਿਰ ਖਾਲੀ dbg_addr(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl)
{
	ਅੱਖਰ *str;
	ਅੱਖਰ strx[5] = "0x";
	ਚੋਣ (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	ਚੋਣ (nl->addr.ifa_scope) {
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
ਸਥਿਰ ਪੂਰਨ process_addr(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl, ਬਣਤਰ nlmsghdr *nh)
{
	ਬਣਤਰ ifaddrmsg *ifaddr;
	ਬਣਤਰ rtattr *attr;
	ਪੂਰਨ len;
	ਸਥਿਰ ਅੱਖਰ ifname[IF_NAMESIZE];
	bool has_local_ਪਤਾ = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, ਆਕਾਰ(ਬਣਤਰ ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	ਜੇ ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	ਨਹੀਂਤਾਂ
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
	len = nh->nlmsg_len - NLMSG_LENGTH(ਆਕਾਰ(*ifaddr));
	ਲਈ (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		ਚੋਣ (attr->rta_type) {
		ਹਾਲਤ IFA_ADDRESS:
			nl->addr.ifa_ਪਤਾ = RTA_DATA(attr);
			nl->addr.ifa_ਪਤਾ_len = RTA_PAYLOAD(attr);
			ਜੇ (!has_local_ਪਤਾ) {
				nl->addr.ਪਤਾ = RTA_DATA(attr);
				nl->addr.ਪਤਾ_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_ਪਤਾ ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_ਪਤਾ(&(nl->addr))));
		ਰੋਕੋ;
		ਹਾਲਤ IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_ਪਤਾ = true;
			nl->addr.ifa_local = nl->addr.ਪਤਾ = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.ਪਤਾ_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		ਰੋਕੋ;
		ਹਾਲਤ IFA_CACHEINFO:
		{
			ਬਣਤਰ ifa_cacheinfo *ci =
				(ਬਣਤਰ ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		ਰੋਕੋ;
		ਹਾਲਤ IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		ਰੋਕੋ;
		ਮੂਲ:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		ਰੋਕੋ;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	ਵਾਪਸ (*(nl->callback_addr))(nl);
}

ਸਥਿਰ ਪੂਰਨ process_ਸੁਨੇਹਾ(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl, ਬਣਤਰ nlmsghdr *nh)
{
	ਪੂਰਨ rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	ਚੋਣ (nh->nlmsg_type) {
	ਹਾਲਤ RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	ਹਾਲਤ RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		ਜੇ (nl->callback_addr)
			rc = process_addr(nl, nh);
		ਰੋਕੋ;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	ਮੂਲ:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		ਰੋਕੋ;

	}
	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_nl_process(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl, bool async, bool loop)
{
	ਅੱਖਰ buf[BUFSIZ];
	ਬਣਤਰ sockaddr_nl snl;
	ਬਣਤਰ nlmsghdr *nh;
	ਪੂਰਨ rc;
	uint32_t len;

	ਬਣਤਰ iovec iov = {
		.iov_base = buf,
		.iov_len = ਆਕਾਰ(buf)
	};
	ਬਣਤਰ msghdr ਸੁਨੇਹਾ = {
		.msg_name = &snl,
		.msg_namelen = ਆਕਾਰ(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = ਨਿਰਾ,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	ਜਦੋਂ (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &ਸੁਨੇਹਾ, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		ਜੇ (rc < 0) {
			ਜੇ (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				ਵਾਪਸ UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			ਵਾਪਸ rc;
		}
		ਨਹੀਂਤਾਂ
			len = rc;

		ਲਈ (nh = (ਬਣਤਰ nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			ਜੇ (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				ਵਾਪਸ -1;
			}
			ਜੇ (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				ਵਾਪਸ UL_NL_ਪੂਰਾ;
			}

			rc = process_ਸੁਨੇਹਾ(nl, nh);
			ਜੇ (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				ਜੇ (rc != UL_NL_SOFT_ERROR)
					ਵਾਪਸ rc;
			}
		}
		ਜੇ (!loop)
			ਵਾਪਸ 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

ਪੂਰਨ ul_nl_open(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl, uint32_t nl_groups)
{
	ਬਣਤਰ sockaddr_nl addr = { 0, };
	ਪੂਰਨ sock;
	ਪੂਰਨ rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	ਜੇ (sock < 0)
		ਵਾਪਸ sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	ਜੇ ((rc = bind(sock, (ਬਣਤਰ sockaddr *)&addr, ਆਕਾਰ(addr))) >= 0)
	{
		nl->fd = sock;
		ਵਾਪਸ 0;
	}
	ਨਹੀਂਤਾਂ
	{
		close(sock);
		ਵਾਪਸ rc;
	}
}

ਪੂਰਨ ul_nl_close(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	ਵਾਪਸ close(nl->fd);
}

ਬਣਤਰ ul_nl_addr *ul_nl_addr_dup(ਬਣਤਰ ul_nl_addr *addr)
{
	ਬਣਤਰ ul_nl_addr *newaddr;
	newaddr = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ ul_nl_addr));
	ਜੇ (!newaddr)
		ਜਾਓ error;
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(newaddr, addr, ਆਕਾਰ(ਬਣਤਰ ul_nl_addr));
	ਜੇ (addr->ifa_ਪਤਾ_len) {
		newaddr->ifa_ਪਤਾ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(addr->ifa_ਪਤਾ_len);
		ਜੇ (!newaddr->ifa_ਪਤਾ)
			ਜਾਓ error;
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(newaddr->ifa_ਪਤਾ, addr->ifa_ਪਤਾ,
		       addr->ifa_ਪਤਾ_len);
	}
	ਜੇ (addr->ifa_local_len) {
		newaddr->ifa_local = ਸਮ੍ਰਿਤੀ_ਦਿਓ(addr->ifa_local_len);
		ਜੇ (!newaddr->ifa_local)
			ਜਾਓ error;
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	ਜੇ (addr->ਪਤਾ == addr->ifa_local)
		newaddr->ਪਤਾ = newaddr->ifa_local;
	ਨਹੀਂਤਾਂ
		newaddr->ਪਤਾ = newaddr->ifa_ਪਤਾ;
	ਜੇ (!(newaddr->ifname = strdup(addr->ifname)))
		ਜਾਓ error;
	ਵਾਪਸ newaddr;
error:
	ul_nl_addr_free(newaddr);
	ਵਾਪਸ ਨਿਰਾ;
}

ਖਾਲੀ ul_nl_addr_free(ਬਣਤਰ ul_nl_addr *addr)
{
	ਜੇ (addr) {
		ਮੁਕਤ(addr->ifa_ਪਤਾ);
		ਮੁਕਤ(addr->ifa_local);
		ਮੁਕਤ(addr->ifname);
		ਮੁਕਤ(addr);
	}
}

ਅਚਲ ਅੱਖਰ *ul_nl_addr_ntop(ਅਚਲ ਬਣਤਰ ul_nl_addr *addr, ਪੂਰਨ addrid) {
	ਅਚਲ ਖਾਲੀ **ifa_addr = (ਅਚਲ ਖਾਲੀ **)((ਅਚਲ ਅੱਖਰ *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	ਸਥਿਰ ਅੱਖਰ addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	ਜੇ (addr->ifa_family == AF_INET)
		ਵਾਪਸ inet_ntop(AF_INET,
				 *ifa_addr, addr_str, ਆਕਾਰ(addr_str));
	ਨਹੀਂਤਾਂ {
	/* if (addr->ifa_family == AF_INET6) */
		ਜੇ (addr->ifa_scope == RT_SCOPE_LINK) {
			ਅੱਖਰ *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, ਆਕਾਰ(addr_str));
			p = addr_str;
			ਜਦੋਂ (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			ਵਾਪਸ addr_str;
		} ਨਹੀਂਤਾਂ
			ਵਾਪਸ inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, ਆਕਾਰ(addr_str));
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_NETLINK
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>

ਸਥਿਰ ਪੂਰਨ callback_addr(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl)
{
	ਅੱਖਰ *str;

	ਲਿਖੋ("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	ਲਿਖੋ("  interface: %s\n", nl->addr.ifname);
	ਜੇ (nl->addr.ifa_family == AF_INET)
		ਲਿਖੋ("  IPv4 %s\n",
		       ul_nl_addr_ntop_ਪਤਾ(&(nl->addr)));
	ਨਹੀਂਤਾਂ
	/* if (nl->addr.ifa_family == AF_INET) */
		ਲਿਖੋ("  IPv6 %s\n",
		       ul_nl_addr_ntop_ਪਤਾ(&(nl->addr)));
	ਚੋਣ (nl->addr.ifa_scope) {
	ਹਾਲਤ RT_SCOPE_UNIVERSE:	str = "global"; ਰੋਕੋ;
	ਹਾਲਤ RT_SCOPE_SITE:	str = "site"; ਰੋਕੋ;
	ਹਾਲਤ RT_SCOPE_LINK:	str = "link"; ਰੋਕੋ;
	ਹਾਲਤ RT_SCOPE_NOWHERE:	str = "nowhere"; ਰੋਕੋ;
	ਮੂਲ:		str = "other"; ਰੋਕੋ;
	}
	ਲਿਖੋ("  scope: %s\n", str);
	ਜੇ (nl->addr.ifa_valid != UINT32_MAX)
		ਲਿਖੋ("  valid: %u\n", nl->addr.ifa_valid);
	ਨਹੀਂਤਾਂ
		ਲਿਖੋ("  valid: forever\n");
	ਵਾਪਸ 0;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc __attribute__((__unused__)),
	 ਅੱਖਰ *argv[] __attribute__((__unused__)))
{
	ਪੂਰਨ rc;
	ਬਣਤਰ ul_nl_ਅੰਕੜੇ nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	ਜੇ ((rc = ul_nl_open(&nl, 0)))
		ਵਾਪਸ 1;
	ਜੇ (ul_nl_request_dump(&nl, RTM_GETADDR))
		ਜਾਓ error;
	ਜੇ (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_ਪੂਰਾ)
		ਜਾਓ error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	ਜੇ ((rc = ul_nl_close(&nl)) < 0)
		ਜਾਓ error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	ਜੇ ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		ਜਾਓ error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	ਜੇ (rc == UL_NL_RETURN)
		rc = 0;
error:
	ਜੇ ((ul_nl_close(&nl)))
		rc = 1;
	ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_NETLINK */
