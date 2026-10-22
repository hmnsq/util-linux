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

#અન્તર્ભાવ <sys/socket.h>
#અન્તર્ભાવ <arpa/inet.h>
#અન્તર્ભાવ <netinet/in.h>
#અન્તર્ભાવ "strutils.h"
#અન્તર્ભાવ "netlink.h"
#અન્તર્ભાવ "debug.h"
#અન્તર્ભાવ "nls.h"

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETLINK_DEBUG_HELP	(1 << 0)
#define ULNETLINK_DEBUG_INIT	(1 << 1)
#define ULNETLINK_DEBUG_NLMSG	(1 << 2)
#define ULNETLINK_DEBUG_ADDR	(1 << 3)

સ્થિર UL_DEBUG_DEFINE_MASK(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ ખાલી, 0 }
};

#define DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

સ્થિર શૂન્ય netlink_init_debug(શૂન્ય)
{
	જો (netlink_debug_mask)
		પાછા;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

શૂન્ય ul_nl_init(સંરચના ul_nl_data *nl)
{
	netlink_init_debug();
	memset(nl, 0, કદ(સંરચના ul_nl_data));
}

પૂર્ણાંક ul_nl_request_dump(સંરચના ul_nl_data *nl, uint16_t nlmsg_type)
{
	સંરચના {
		સંરચના nlmsghdr nh;
		સંરચના rtgenmsg g;
	} req;

	memset(&req, 0, કદ(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(કદ(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	જો (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		પાછા -1;
	પાછા 0;
}

#define DBG_CASE(x) સ્થિતિ x: str = #x; તોડો
#define DBG_CASE_DEF8(x) મૂળભૂત: snprintf(strx+2, 3, "%02hhx", x); str = strx; તોડો
સ્થિર શૂન્ય dbg_addr(સંરચના ul_nl_data *nl)
{
	અક્ષર *str;
	અક્ષર strx[5] = "0x";
	પસંદગી (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	પસંદગી (nl->addr.ifa_scope) {
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
સ્થિર પૂર્ણાંક process_addr(સંરચના ul_nl_data *nl, સંરચના nlmsghdr *nh)
{
	સંરચના ifaddrmsg *ifaddr;
	સંરચના rtattr *attr;
	પૂર્ણાંક len;
	સ્થિર અક્ષર ifname[IF_NAMESIZE];
	bool has_local_address = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, કદ(સંરચના ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	જો ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	નહીંતર
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
	len = nh->nlmsg_len - NLMSG_LENGTH(કદ(*ifaddr));
	માટે (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		પસંદગી (attr->rta_type) {
		સ્થિતિ IFA_ADDRESS:
			nl->addr.ifa_address = RTA_DATA(attr);
			nl->addr.ifa_address_len = RTA_PAYLOAD(attr);
			જો (!has_local_address) {
				nl->addr.address = RTA_DATA(attr);
				nl->addr.address_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_address ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_address(&(nl->addr))));
		તોડો;
		સ્થિતિ IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_address = true;
			nl->addr.ifa_local = nl->addr.address = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.address_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		તોડો;
		સ્થિતિ IFA_CACHEINFO:
		{
			સંરચના ifa_cacheinfo *ci =
				(સંરચના ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		તોડો;
		સ્થિતિ IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		તોડો;
		મૂળભૂત:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		તોડો;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	પાછા (*(nl->callback_addr))(nl);
}

સ્થિર પૂર્ણાંક process_સંદેશ(સંરચના ul_nl_data *nl, સંરચના nlmsghdr *nh)
{
	પૂર્ણાંક rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	પસંદગી (nh->nlmsg_type) {
	સ્થિતિ RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	સ્થિતિ RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		જો (nl->callback_addr)
			rc = process_addr(nl, nh);
		તોડો;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	મૂળભૂત:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		તોડો;

	}
	પાછા rc;
}

પૂર્ણાંક ul_nl_process(સંરચના ul_nl_data *nl, bool async, bool loop)
{
	અક્ષર buf[BUFSIZ];
	સંરચના sockaddr_nl snl;
	સંરચના nlmsghdr *nh;
	પૂર્ણાંક rc;
	uint32_t len;

	સંરચના iovec iov = {
		.iov_base = buf,
		.iov_len = કદ(buf)
	};
	સંરચના msghdr સંદેશ = {
		.msg_name = &snl,
		.msg_namelen = કદ(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = ખાલી,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	જ્યારે (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &સંદેશ, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		જો (rc < 0) {
			જો (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				પાછા UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			પાછા rc;
		}
		નહીંતર
			len = rc;

		માટે (nh = (સંરચના nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			જો (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				પાછા -1;
			}
			જો (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				પાછા UL_NL_DONE;
			}

			rc = process_સંદેશ(nl, nh);
			જો (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				જો (rc != UL_NL_SOFT_ERROR)
					પાછા rc;
			}
		}
		જો (!loop)
			પાછા 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

પૂર્ણાંક ul_nl_open(સંરચના ul_nl_data *nl, uint32_t nl_groups)
{
	સંરચના sockaddr_nl addr = { 0, };
	પૂર્ણાંક sock;
	પૂર્ણાંક rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	જો (sock < 0)
		પાછા sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	જો ((rc = bind(sock, (સંરચના sockaddr *)&addr, કદ(addr))) >= 0)
	{
		nl->fd = sock;
		પાછા 0;
	}
	નહીંતર
	{
		close(sock);
		પાછા rc;
	}
}

પૂર્ણાંક ul_nl_close(સંરચના ul_nl_data *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	પાછા close(nl->fd);
}

સંરચના ul_nl_addr *ul_nl_addr_dup(સંરચના ul_nl_addr *addr)
{
	સંરચના ul_nl_addr *newaddr;
	newaddr = calloc(1, કદ(સંરચના ul_nl_addr));
	જો (!newaddr)
		જાઓ error;
	memcpy(newaddr, addr, કદ(સંરચના ul_nl_addr));
	જો (addr->ifa_address_len) {
		newaddr->ifa_address = malloc(addr->ifa_address_len);
		જો (!newaddr->ifa_address)
			જાઓ error;
		memcpy(newaddr->ifa_address, addr->ifa_address,
		       addr->ifa_address_len);
	}
	જો (addr->ifa_local_len) {
		newaddr->ifa_local = malloc(addr->ifa_local_len);
		જો (!newaddr->ifa_local)
			જાઓ error;
		memcpy(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	જો (addr->address == addr->ifa_local)
		newaddr->address = newaddr->ifa_local;
	નહીંતર
		newaddr->address = newaddr->ifa_address;
	જો (!(newaddr->ifname = strdup(addr->ifname)))
		જાઓ error;
	પાછા newaddr;
error:
	ul_nl_addr_free(newaddr);
	પાછા ખાલી;
}

શૂન્ય ul_nl_addr_free(સંરચના ul_nl_addr *addr)
{
	જો (addr) {
		મુક્ત(addr->ifa_address);
		મુક્ત(addr->ifa_local);
		મુક્ત(addr->ifname);
		મુક્ત(addr);
	}
}

અચલ અક્ષર *ul_nl_addr_ntop(અચલ સંરચના ul_nl_addr *addr, પૂર્ણાંક addrid) {
	અચલ શૂન્ય **ifa_addr = (અચલ શૂન્ય **)((અચલ અક્ષર *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	સ્થિર અક્ષર addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	જો (addr->ifa_family == AF_INET)
		પાછા inet_ntop(AF_INET,
				 *ifa_addr, addr_str, કદ(addr_str));
	નહીંતર {
	/* if (addr->ifa_family == AF_INET6) */
		જો (addr->ifa_scope == RT_SCOPE_LINK) {
			અક્ષર *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, કદ(addr_str));
			p = addr_str;
			જ્યારે (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			પાછા addr_str;
		} નહીંતર
			પાછા inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, કદ(addr_str));
	}
}

#ifdef TEST_PROGRAM_NETLINK
#અન્તર્ભાવ <stdio.h>

સ્થિર પૂર્ણાંક callback_addr(સંરચના ul_nl_data *nl)
{
	અક્ષર *str;

	printf("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	printf("  interface: %s\n", nl->addr.ifname);
	જો (nl->addr.ifa_family == AF_INET)
		printf("  IPv4 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	નહીંતર
	/* if (nl->addr.ifa_family == AF_INET) */
		printf("  IPv6 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	પસંદગી (nl->addr.ifa_scope) {
	સ્થિતિ RT_SCOPE_UNIVERSE:	str = "global"; તોડો;
	સ્થિતિ RT_SCOPE_SITE:	str = "site"; તોડો;
	સ્થિતિ RT_SCOPE_LINK:	str = "link"; તોડો;
	સ્થિતિ RT_SCOPE_NOWHERE:	str = "nowhere"; તોડો;
	મૂળભૂત:		str = "other"; તોડો;
	}
	printf("  scope: %s\n", str);
	જો (nl->addr.ifa_valid != UINT32_MAX)
		printf("  valid: %u\n", nl->addr.ifa_valid);
	નહીંતર
		printf("  valid: forever\n");
	પાછા 0;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc __attribute__((__unused__)),
	 અક્ષર *argv[] __attribute__((__unused__)))
{
	પૂર્ણાંક rc;
	સંરચના ul_nl_data nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	જો ((rc = ul_nl_open(&nl, 0)))
		પાછા 1;
	જો (ul_nl_request_dump(&nl, RTM_GETADDR))
		જાઓ error;
	જો (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		જાઓ error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	જો ((rc = ul_nl_close(&nl)) < 0)
		જાઓ error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	જો ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		જાઓ error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	જો (rc == UL_NL_RETURN)
		rc = 0;
error:
	જો ((ul_nl_close(&nl)))
		rc = 1;
	પાછા rc;
}
#endif /* TEST_PROGRAM_NETLINK */
