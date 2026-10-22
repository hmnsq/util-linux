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
#ವ್ಯಾಖ್ಯಾನಿಸು ULNETLINK_DEBUG_HELP	(1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು ULNETLINK_DEBUG_INIT	(1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು ULNETLINK_DEBUG_NLMSG	(1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು ULNETLINK_DEBUG_ADDR	(1 << 3)

ಸ್ಥಿರ UL_DEBUG_DEFINE_ಮುಖವಾಡ(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ ಖಾಲಿ, 0 }
};

#ವ್ಯಾಖ್ಯಾನಿಸು DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#ವ್ಯಾಖ್ಯಾನಿಸು DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#ವ್ಯಾಖ್ಯಾನಿಸು ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

ಸ್ಥಿರ ಶೂನ್ಯ netlink_init_debug(ಶೂನ್ಯ)
{
	ಆದರೆ (netlink_debug_ಮುಖವಾಡ)
		ಹಿಂತಿರುಗಿ;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

ಶೂನ್ಯ ul_nl_init(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl)
{
	netlink_init_debug();
	memset(nl, 0, ಗಾತ್ರ(ರಚನೆ ul_nl_ದತ್ತಾಂಶ));
}

ಪೂರ್ಣಾಂಕ ul_nl_request_dump(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl, uint16_t nlmsg_type)
{
	ರಚನೆ {
		ರಚನೆ nlmsghdr nh;
		ರಚನೆ rtgenmsg g;
	} req;

	memset(&req, 0, ಗಾತ್ರ(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(ಗಾತ್ರ(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	ಆದರೆ (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		ಹಿಂತಿರುಗಿ -1;
	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿಸು DBG_CASE(x) ಸ್ಥಿತಿ x: str = #x; ನಿಲ್ಲಿಸು
#ವ್ಯಾಖ್ಯಾನಿಸು DBG_CASE_DEF8(x) ಪೂರ್ವನಿಯೋಜಿತ: snprintf(strx+2, 3, "%02hhx", x); str = strx; ನಿಲ್ಲಿಸು
ಸ್ಥಿರ ಶೂನ್ಯ dbg_addr(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl)
{
	ಅಕ್ಷರ *str;
	ಅಕ್ಷರ strx[5] = "0x";
	ಆಯ್ಕೆ (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	ಆಯ್ಕೆ (nl->addr.ifa_scope) {
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ process_addr(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl, ರಚನೆ nlmsghdr *nh)
{
	ರಚನೆ ifaddrmsg *ifaddr;
	ರಚನೆ rtattr *attr;
	ಪೂರ್ಣಾಂಕ len;
	ಸ್ಥಿರ ಅಕ್ಷರ ifname[IF_NAMESIZE];
	bool has_local_ವಿಳಾಸ = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, ಗಾತ್ರ(ರಚನೆ ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	ಆದರೆ ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	ಇಲ್ಲದಿದ್ದರೆ
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
	len = nh->nlmsg_len - NLMSG_LENGTH(ಗಾತ್ರ(*ifaddr));
	ಗಾಗಿ (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		ಆಯ್ಕೆ (attr->rta_type) {
		ಸ್ಥಿತಿ IFA_ADDRESS:
			nl->addr.ifa_ವಿಳಾಸ = RTA_DATA(attr);
			nl->addr.ifa_ವಿಳಾಸ_len = RTA_PAYLOAD(attr);
			ಆದರೆ (!has_local_ವಿಳಾಸ) {
				nl->addr.ವಿಳಾಸ = RTA_DATA(attr);
				nl->addr.ವಿಳಾಸ_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_ವಿಳಾಸ ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_ವಿಳಾಸ(&(nl->addr))));
		ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_ವಿಳಾಸ = true;
			nl->addr.ifa_local = nl->addr.ವಿಳಾಸ = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.ವಿಳಾಸ_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ IFA_CACHEINFO:
		{
			ರಚನೆ ifa_cacheinfo *ci =
				(ರಚನೆ ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		ನಿಲ್ಲಿಸು;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	ಹಿಂತಿರುಗಿ (*(nl->callback_addr))(nl);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ process_ಸಂದೇಶ(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl, ರಚನೆ nlmsghdr *nh)
{
	ಪೂರ್ಣಾಂಕ rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	ಆಯ್ಕೆ (nh->nlmsg_type) {
	ಸ್ಥಿತಿ RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	ಸ್ಥಿತಿ RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		ಆದರೆ (nl->callback_addr)
			rc = process_addr(nl, nh);
		ನಿಲ್ಲಿಸು;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	ಪೂರ್ವನಿಯೋಜಿತ:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		ನಿಲ್ಲಿಸು;

	}
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ ul_nl_process(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl, bool async, bool loop)
{
	ಅಕ್ಷರ buf[BUFSIZ];
	ರಚನೆ sockaddr_nl snl;
	ರಚನೆ nlmsghdr *nh;
	ಪೂರ್ಣಾಂಕ rc;
	uint32_t len;

	ರಚನೆ iovec iov = {
		.iov_base = buf,
		.iov_len = ಗಾತ್ರ(buf)
	};
	ರಚನೆ msghdr ಸಂದೇಶ = {
		.msg_name = &snl,
		.msg_namelen = ಗಾತ್ರ(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = ಖಾಲಿ,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	ಆಗ (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &ಸಂದೇಶ, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		ಆದರೆ (rc < 0) {
			ಆದರೆ (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				ಹಿಂತಿರುಗಿ UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			ಹಿಂತಿರುಗಿ rc;
		}
		ಇಲ್ಲದಿದ್ದರೆ
			len = rc;

		ಗಾಗಿ (nh = (ರಚನೆ nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			ಆದರೆ (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				ಹಿಂತಿರುಗಿ -1;
			}
			ಆದರೆ (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				ಹಿಂತಿರುಗಿ UL_NL_ಮುಗಿದ;
			}

			rc = process_ಸಂದೇಶ(nl, nh);
			ಆದರೆ (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				ಆದರೆ (rc != UL_NL_SOFT_ERROR)
					ಹಿಂತಿರುಗಿ rc;
			}
		}
		ಆದರೆ (!loop)
			ಹಿಂತಿರುಗಿ 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

ಪೂರ್ಣಾಂಕ ul_nl_open(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl, uint32_t nl_groups)
{
	ರಚನೆ sockaddr_nl addr = { 0, };
	ಪೂರ್ಣಾಂಕ sock;
	ಪೂರ್ಣಾಂಕ rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	ಆದರೆ (sock < 0)
		ಹಿಂತಿರುಗಿ sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	ಆದರೆ ((rc = bind(sock, (ರಚನೆ sockaddr *)&addr, ಗಾತ್ರ(addr))) >= 0)
	{
		nl->fd = sock;
		ಹಿಂತಿರುಗಿ 0;
	}
	ಇಲ್ಲದಿದ್ದರೆ
	{
		close(sock);
		ಹಿಂತಿರುಗಿ rc;
	}
}

ಪೂರ್ಣಾಂಕ ul_nl_close(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	ಹಿಂತಿರುಗಿ close(nl->fd);
}

ರಚನೆ ul_nl_addr *ul_nl_addr_dup(ರಚನೆ ul_nl_addr *addr)
{
	ರಚನೆ ul_nl_addr *newaddr;
	newaddr = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ ul_nl_addr));
	ಆದರೆ (!newaddr)
		ಹೋಗು error;
	ಸ್ಮರಣೆ_ನಕಲು(newaddr, addr, ಗಾತ್ರ(ರಚನೆ ul_nl_addr));
	ಆದರೆ (addr->ifa_ವಿಳಾಸ_len) {
		newaddr->ifa_ವಿಳಾಸ = ಸ್ಮರಣೆ_ಕೊಡು(addr->ifa_ವಿಳಾಸ_len);
		ಆದರೆ (!newaddr->ifa_ವಿಳಾಸ)
			ಹೋಗು error;
		ಸ್ಮರಣೆ_ನಕಲು(newaddr->ifa_ವಿಳಾಸ, addr->ifa_ವಿಳಾಸ,
		       addr->ifa_ವಿಳಾಸ_len);
	}
	ಆದರೆ (addr->ifa_local_len) {
		newaddr->ifa_local = ಸ್ಮರಣೆ_ಕೊಡು(addr->ifa_local_len);
		ಆದರೆ (!newaddr->ifa_local)
			ಹೋಗು error;
		ಸ್ಮರಣೆ_ನಕಲು(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	ಆದರೆ (addr->ವಿಳಾಸ == addr->ifa_local)
		newaddr->ವಿಳಾಸ = newaddr->ifa_local;
	ಇಲ್ಲದಿದ್ದರೆ
		newaddr->ವಿಳಾಸ = newaddr->ifa_ವಿಳಾಸ;
	ಆದರೆ (!(newaddr->ifname = strdup(addr->ifname)))
		ಹೋಗು error;
	ಹಿಂತಿರುಗಿ newaddr;
error:
	ul_nl_addr_free(newaddr);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಶೂನ್ಯ ul_nl_addr_free(ರಚನೆ ul_nl_addr *addr)
{
	ಆದರೆ (addr) {
		ಬಿಡುಗಡೆ(addr->ifa_ವಿಳಾಸ);
		ಬಿಡುಗಡೆ(addr->ifa_local);
		ಬಿಡುಗಡೆ(addr->ifname);
		ಬಿಡುಗಡೆ(addr);
	}
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ul_nl_addr_ntop(ಸ್ಥಿರಾಂಕ ರಚನೆ ul_nl_addr *addr, ಪೂರ್ಣಾಂಕ addrid) {
	ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **ifa_addr = (ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ **)((ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	ಸ್ಥಿರ ಅಕ್ಷರ addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	ಆದರೆ (addr->ifa_family == AF_INET)
		ಹಿಂತಿರುಗಿ inet_ntop(AF_INET,
				 *ifa_addr, addr_str, ಗಾತ್ರ(addr_str));
	ಇಲ್ಲದಿದ್ದರೆ {
	/* if (addr->ifa_family == AF_INET6) */
		ಆದರೆ (addr->ifa_scope == RT_SCOPE_LINK) {
			ಅಕ್ಷರ *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, ಗಾತ್ರ(addr_str));
			p = addr_str;
			ಆಗ (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			ಹಿಂತಿರುಗಿ addr_str;
		} ಇಲ್ಲದಿದ್ದರೆ
			ಹಿಂತಿರುಗಿ inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, ಗಾತ್ರ(addr_str));
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_NETLINK
#ಅನ್ತರ್ಭಾವ <stdio.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ callback_addr(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl)
{
	ಅಕ್ಷರ *str;

	ಬರೆ("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	ಬರೆ("  interface: %s\n", nl->addr.ifname);
	ಆದರೆ (nl->addr.ifa_family == AF_INET)
		ಬರೆ("  IPv4 %s\n",
		       ul_nl_addr_ntop_ವಿಳಾಸ(&(nl->addr)));
	ಇಲ್ಲದಿದ್ದರೆ
	/* if (nl->addr.ifa_family == AF_INET) */
		ಬರೆ("  IPv6 %s\n",
		       ul_nl_addr_ntop_ವಿಳಾಸ(&(nl->addr)));
	ಆಯ್ಕೆ (nl->addr.ifa_scope) {
	ಸ್ಥಿತಿ RT_SCOPE_UNIVERSE:	str = "global"; ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ RT_SCOPE_SITE:	str = "site"; ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ RT_SCOPE_LINK:	str = "link"; ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ RT_SCOPE_NOWHERE:	str = "nowhere"; ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:		str = "other"; ನಿಲ್ಲಿಸು;
	}
	ಬರೆ("  scope: %s\n", str);
	ಆದರೆ (nl->addr.ifa_valid != UINT32_MAX)
		ಬರೆ("  valid: %u\n", nl->addr.ifa_valid);
	ಇಲ್ಲದಿದ್ದರೆ
		ಬರೆ("  valid: forever\n");
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc __attribute__((__unused__)),
	 ಅಕ್ಷರ *argv[] __attribute__((__unused__)))
{
	ಪೂರ್ಣಾಂಕ rc;
	ರಚನೆ ul_nl_ದತ್ತಾಂಶ nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	ಆದರೆ ((rc = ul_nl_open(&nl, 0)))
		ಹಿಂತಿರುಗಿ 1;
	ಆದರೆ (ul_nl_request_dump(&nl, RTM_GETADDR))
		ಹೋಗು error;
	ಆದರೆ (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_ಮುಗಿದ)
		ಹೋಗು error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	ಆದರೆ ((rc = ul_nl_close(&nl)) < 0)
		ಹೋಗು error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	ಆದರೆ ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		ಹೋಗು error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	ಆದರೆ (rc == UL_NL_RETURN)
		rc = 0;
error:
	ಆದರೆ ((ul_nl_close(&nl)))
		rc = 1;
	ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_NETLINK */
