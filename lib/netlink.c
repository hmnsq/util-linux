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

#ଅନ୍ତର୍ଭାଵ <sys/socket.h>
#ଅନ୍ତର୍ଭାଵ <arpa/inet.h>
#ଅନ୍ତର୍ଭାଵ <netinet/in.h>
#ଅନ୍ତର୍ଭାଵ "strutils.h"
#ଅନ୍ତର୍ଭାଵ "netlink.h"
#ଅନ୍ତର୍ଭାଵ "debug.h"
#ଅନ୍ତର୍ଭାଵ "nls.h"

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETLINK_DEBUG_HELP	(1 << 0)
#define ULNETLINK_DEBUG_INIT	(1 << 1)
#define ULNETLINK_DEBUG_NLMSG	(1 << 2)
#define ULNETLINK_DEBUG_ADDR	(1 << 3)

ସ୍ଥିର UL_DEBUG_DEFINE_MASK(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ ଖାଲି, 0 }
};

#define DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

ସ୍ଥିର ଶୂନ୍ୟ netlink_init_debug(ଶୂନ୍ୟ)
{
	ଯଦି (netlink_debug_mask)
		ଫେରନ୍ତୁ;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

ଶୂନ୍ୟ ul_nl_init(ସଂରଚନା ul_nl_data *nl)
{
	netlink_init_debug();
	memset(nl, 0, ଆକାର(ସଂରଚନା ul_nl_data));
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_nl_request_dump(ସଂରଚନା ul_nl_data *nl, uint16_t nlmsg_type)
{
	ସଂରଚନା {
		ସଂରଚନା nlmsghdr nh;
		ସଂରଚନା rtgenmsg g;
	} req;

	memset(&req, 0, ଆକାର(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(ଆକାର(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	ଯଦି (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		ଫେରନ୍ତୁ -1;
	ଫେରନ୍ତୁ 0;
}

#define DBG_CASE(x) ସ୍ଥିତି x: str = #x; ଭାଙ୍ଗ
#define DBG_CASE_DEF8(x) ପୂର୍ବନିର୍ଦ୍ଧାରିତ: snprintf(strx+2, 3, "%02hhx", x); str = strx; ଭାଙ୍ଗ
ସ୍ଥିର ଶୂନ୍ୟ dbg_addr(ସଂରଚନା ul_nl_data *nl)
{
	ଅକ୍ଷର *str;
	ଅକ୍ଷର strx[5] = "0x";
	ବାଛ (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	ବାଛ (nl->addr.ifa_scope) {
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା process_addr(ସଂରଚନା ul_nl_data *nl, ସଂରଚନା nlmsghdr *nh)
{
	ସଂରଚନା ifaddrmsg *ifaddr;
	ସଂରଚନା rtattr *attr;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
	ସ୍ଥିର ଅକ୍ଷର ifname[IF_NAMESIZE];
	bool has_local_address = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, ଆକାର(ସଂରଚନା ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	ଯଦି ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	ଅନ୍ୟଥା
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
	len = nh->nlmsg_len - NLMSG_LENGTH(ଆକାର(*ifaddr));
	ପାଇଁ (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		ବାଛ (attr->rta_type) {
		ସ୍ଥିତି IFA_ADDRESS:
			nl->addr.ifa_address = RTA_DATA(attr);
			nl->addr.ifa_address_len = RTA_PAYLOAD(attr);
			ଯଦି (!has_local_address) {
				nl->addr.address = RTA_DATA(attr);
				nl->addr.address_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_address ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_address(&(nl->addr))));
		ଭାଙ୍ଗ;
		ସ୍ଥିତି IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_address = true;
			nl->addr.ifa_local = nl->addr.address = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.address_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		ଭାଙ୍ଗ;
		ସ୍ଥିତି IFA_CACHEINFO:
		{
			ସଂରଚନା ifa_cacheinfo *ci =
				(ସଂରଚନା ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		ଭାଙ୍ଗ;
		ସ୍ଥିତି IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		ଭାଙ୍ଗ;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	ଫେରନ୍ତୁ (*(nl->callback_addr))(nl);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା process_ସନ୍ଦେଶ(ସଂରଚନା ul_nl_data *nl, ସଂରଚନା nlmsghdr *nh)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	ବାଛ (nh->nlmsg_type) {
	ସ୍ଥିତି RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	ସ୍ଥିତି RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		ଯଦି (nl->callback_addr)
			rc = process_addr(nl, nh);
		ଭାଙ୍ଗ;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		ଭାଙ୍ଗ;

	}
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_nl_process(ସଂରଚନା ul_nl_data *nl, bool async, bool loop)
{
	ଅକ୍ଷର buf[BUFSIZ];
	ସଂରଚନା sockaddr_nl snl;
	ସଂରଚନା nlmsghdr *nh;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	uint32_t len;

	ସଂରଚନା iovec iov = {
		.iov_base = buf,
		.iov_len = ଆକାର(buf)
	};
	ସଂରଚନା msghdr ସନ୍ଦେଶ = {
		.msg_name = &snl,
		.msg_namelen = ଆକାର(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = ଖାଲି,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	ଯେତେବେଳେ (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &ସନ୍ଦେଶ, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		ଯଦି (rc < 0) {
			ଯଦି (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				ଫେରନ୍ତୁ UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			ଫେରନ୍ତୁ rc;
		}
		ଅନ୍ୟଥା
			len = rc;

		ପାଇଁ (nh = (ସଂରଚନା nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			ଯଦି (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				ଫେରନ୍ତୁ -1;
			}
			ଯଦି (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				ଫେରନ୍ତୁ UL_NL_DONE;
			}

			rc = process_ସନ୍ଦେଶ(nl, nh);
			ଯଦି (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				ଯଦି (rc != UL_NL_SOFT_ERROR)
					ଫେରନ୍ତୁ rc;
			}
		}
		ଯଦି (!loop)
			ଫେରନ୍ତୁ 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_nl_open(ସଂରଚନା ul_nl_data *nl, uint32_t nl_groups)
{
	ସଂରଚନା sockaddr_nl addr = { 0, };
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sock;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	ଯଦି (sock < 0)
		ଫେରନ୍ତୁ sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	ଯଦି ((rc = bind(sock, (ସଂରଚନା sockaddr *)&addr, ଆକାର(addr))) >= 0)
	{
		nl->fd = sock;
		ଫେରନ୍ତୁ 0;
	}
	ଅନ୍ୟଥା
	{
		close(sock);
		ଫେରନ୍ତୁ rc;
	}
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_nl_close(ସଂରଚନା ul_nl_data *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	ଫେରନ୍ତୁ close(nl->fd);
}

ସଂରଚନା ul_nl_addr *ul_nl_addr_dup(ସଂରଚନା ul_nl_addr *addr)
{
	ସଂରଚନା ul_nl_addr *newaddr;
	newaddr = calloc(1, ଆକାର(ସଂରଚନା ul_nl_addr));
	ଯଦି (!newaddr)
		ଯାଅ error;
	memcpy(newaddr, addr, ଆକାର(ସଂରଚନା ul_nl_addr));
	ଯଦି (addr->ifa_address_len) {
		newaddr->ifa_address = malloc(addr->ifa_address_len);
		ଯଦି (!newaddr->ifa_address)
			ଯାଅ error;
		memcpy(newaddr->ifa_address, addr->ifa_address,
		       addr->ifa_address_len);
	}
	ଯଦି (addr->ifa_local_len) {
		newaddr->ifa_local = malloc(addr->ifa_local_len);
		ଯଦି (!newaddr->ifa_local)
			ଯାଅ error;
		memcpy(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	ଯଦି (addr->address == addr->ifa_local)
		newaddr->address = newaddr->ifa_local;
	ଅନ୍ୟଥା
		newaddr->address = newaddr->ifa_address;
	ଯଦି (!(newaddr->ifname = strdup(addr->ifname)))
		ଯାଅ error;
	ଫେରନ୍ତୁ newaddr;
error:
	ul_nl_addr_free(newaddr);
	ଫେରନ୍ତୁ ଖାଲି;
}

ଶୂନ୍ୟ ul_nl_addr_free(ସଂରଚନା ul_nl_addr *addr)
{
	ଯଦି (addr) {
		ମୁକ୍ତ(addr->ifa_address);
		ମୁକ୍ତ(addr->ifa_local);
		ମୁକ୍ତ(addr->ifname);
		ମୁକ୍ତ(addr);
	}
}

ଅଚଳ ଅକ୍ଷର *ul_nl_addr_ntop(ଅଚଳ ସଂରଚନା ul_nl_addr *addr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା addrid) {
	ଅଚଳ ଶୂନ୍ୟ **ifa_addr = (ଅଚଳ ଶୂନ୍ୟ **)((ଅଚଳ ଅକ୍ଷର *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	ସ୍ଥିର ଅକ୍ଷର addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	ଯଦି (addr->ifa_family == AF_INET)
		ଫେରନ୍ତୁ inet_ntop(AF_INET,
				 *ifa_addr, addr_str, ଆକାର(addr_str));
	ଅନ୍ୟଥା {
	/* if (addr->ifa_family == AF_INET6) */
		ଯଦି (addr->ifa_scope == RT_SCOPE_LINK) {
			ଅକ୍ଷର *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, ଆକାର(addr_str));
			p = addr_str;
			ଯେତେବେଳେ (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			ଫେରନ୍ତୁ addr_str;
		} ଅନ୍ୟଥା
			ଫେରନ୍ତୁ inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, ଆକାର(addr_str));
	}
}

#ifdef TEST_PROGRAM_NETLINK
#ଅନ୍ତର୍ଭାଵ <stdio.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା callback_addr(ସଂରଚନା ul_nl_data *nl)
{
	ଅକ୍ଷର *str;

	printf("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	printf("  interface: %s\n", nl->addr.ifname);
	ଯଦି (nl->addr.ifa_family == AF_INET)
		printf("  IPv4 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	ଅନ୍ୟଥା
	/* if (nl->addr.ifa_family == AF_INET) */
		printf("  IPv6 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	ବାଛ (nl->addr.ifa_scope) {
	ସ୍ଥିତି RT_SCOPE_UNIVERSE:	str = "global"; ଭାଙ୍ଗ;
	ସ୍ଥିତି RT_SCOPE_SITE:	str = "site"; ଭାଙ୍ଗ;
	ସ୍ଥିତି RT_SCOPE_LINK:	str = "link"; ଭାଙ୍ଗ;
	ସ୍ଥିତି RT_SCOPE_NOWHERE:	str = "nowhere"; ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:		str = "other"; ଭାଙ୍ଗ;
	}
	printf("  scope: %s\n", str);
	ଯଦି (nl->addr.ifa_valid != UINT32_MAX)
		printf("  valid: %u\n", nl->addr.ifa_valid);
	ଅନ୍ୟଥା
		printf("  valid: forever\n");
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc __attribute__((__unused__)),
	 ଅକ୍ଷର *argv[] __attribute__((__unused__)))
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	ସଂରଚନା ul_nl_data nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	ଯଦି ((rc = ul_nl_open(&nl, 0)))
		ଫେରନ୍ତୁ 1;
	ଯଦି (ul_nl_request_dump(&nl, RTM_GETADDR))
		ଯାଅ error;
	ଯଦି (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		ଯାଅ error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	ଯଦି ((rc = ul_nl_close(&nl)) < 0)
		ଯାଅ error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	ଯଦି ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		ଯାଅ error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	ଯଦି (rc == UL_NL_RETURN)
		rc = 0;
error:
	ଯଦି ((ul_nl_close(&nl)))
		rc = 1;
	ଫେରନ୍ତୁ rc;
}
#endif /* TEST_PROGRAM_NETLINK */
