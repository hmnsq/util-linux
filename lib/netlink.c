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

#অন্তর্ভাব <sys/socket.h>
#অন্তর্ভাব <arpa/inet.h>
#অন্তর্ভাব <netinet/in.h>
#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "netlink.h"
#অন্তর্ভাব "debug.h"
#অন্তর্ভাব "nls.h"

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETLINK_DEBUG_HELP	(1 << 0)
#define ULNETLINK_DEBUG_INIT	(1 << 1)
#define ULNETLINK_DEBUG_NLMSG	(1 << 2)
#define ULNETLINK_DEBUG_ADDR	(1 << 3)

স্থির UL_DEBUG_DEFINE_MASK(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ ফাঁকা, 0 }
};

#define DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

স্থির শূন্য netlink_init_debug(শূন্য)
{
	যদি (netlink_debug_mask)
		ফেরত;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

শূন্য ul_nl_init(কাঠামো ul_nl_তথ্য *nl)
{
	netlink_init_debug();
	memset(nl, 0, আকার(কাঠামো ul_nl_তথ্য));
}

পূর্ণসংখ্যা ul_nl_request_dump(কাঠামো ul_nl_তথ্য *nl, uint16_t nlmsg_type)
{
	কাঠামো {
		কাঠামো nlmsghdr nh;
		কাঠামো rtgenmsg g;
	} req;

	memset(&req, 0, আকার(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(আকার(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	যদি (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		ফেরত -1;
	ফেরত 0;
}

#define DBG_CASE(x) ক্ষেত্র x: str = #x; বিরতি
#define DBG_CASE_DEF8(x) পূর্বনির্ধারিত: snprintf(strx+2, 3, "%02hhx", x); str = strx; বিরতি
স্থির শূন্য dbg_addr(কাঠামো ul_nl_তথ্য *nl)
{
	অক্ষর *str;
	অক্ষর strx[5] = "0x";
	নির্বাচন (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	নির্বাচন (nl->addr.ifa_scope) {
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
স্থির পূর্ণসংখ্যা process_addr(কাঠামো ul_nl_তথ্য *nl, কাঠামো nlmsghdr *nh)
{
	কাঠামো ifaddrmsg *ifaddr;
	কাঠামো rtattr *attr;
	পূর্ণসংখ্যা len;
	স্থির অক্ষর ifname[IF_NAMESIZE];
	bool has_local_address = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, আকার(কাঠামো ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	যদি ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	অন্যথা
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
	len = nh->nlmsg_len - NLMSG_LENGTH(আকার(*ifaddr));
	জন্য (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		নির্বাচন (attr->rta_type) {
		ক্ষেত্র IFA_ADDRESS:
			nl->addr.ifa_address = RTA_DATA(attr);
			nl->addr.ifa_address_len = RTA_PAYLOAD(attr);
			যদি (!has_local_address) {
				nl->addr.address = RTA_DATA(attr);
				nl->addr.address_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_address ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_address(&(nl->addr))));
		বিরতি;
		ক্ষেত্র IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_address = true;
			nl->addr.ifa_local = nl->addr.address = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.address_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		বিরতি;
		ক্ষেত্র IFA_CACHEINFO:
		{
			কাঠামো ifa_cacheinfo *ci =
				(কাঠামো ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		বিরতি;
		ক্ষেত্র IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		বিরতি;
		পূর্বনির্ধারিত:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		বিরতি;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	ফেরত (*(nl->callback_addr))(nl);
}

স্থির পূর্ণসংখ্যা process_বার্তা(কাঠামো ul_nl_তথ্য *nl, কাঠামো nlmsghdr *nh)
{
	পূর্ণসংখ্যা rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	নির্বাচন (nh->nlmsg_type) {
	ক্ষেত্র RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	ক্ষেত্র RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		যদি (nl->callback_addr)
			rc = process_addr(nl, nh);
		বিরতি;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	পূর্বনির্ধারিত:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		বিরতি;

	}
	ফেরত rc;
}

পূর্ণসংখ্যা ul_nl_process(কাঠামো ul_nl_তথ্য *nl, bool async, bool loop)
{
	অক্ষর buf[BUFSIZ];
	কাঠামো sockaddr_nl snl;
	কাঠামো nlmsghdr *nh;
	পূর্ণসংখ্যা rc;
	uint32_t len;

	কাঠামো iovec iov = {
		.iov_base = buf,
		.iov_len = আকার(buf)
	};
	কাঠামো msghdr বার্তা = {
		.msg_name = &snl,
		.msg_namelen = আকার(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = ফাঁকা,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	যখন (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &বার্তা, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		যদি (rc < 0) {
			যদি (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				ফেরত UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			ফেরত rc;
		}
		অন্যথা
			len = rc;

		জন্য (nh = (কাঠামো nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			যদি (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				ফেরত -1;
			}
			যদি (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				ফেরত UL_NL_DONE;
			}

			rc = process_বার্তা(nl, nh);
			যদি (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				যদি (rc != UL_NL_SOFT_ERROR)
					ফেরত rc;
			}
		}
		যদি (!loop)
			ফেরত 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

পূর্ণসংখ্যা ul_nl_open(কাঠামো ul_nl_তথ্য *nl, uint32_t nl_groups)
{
	কাঠামো sockaddr_nl addr = { 0, };
	পূর্ণসংখ্যা sock;
	পূর্ণসংখ্যা rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	যদি (sock < 0)
		ফেরত sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	যদি ((rc = bind(sock, (কাঠামো sockaddr *)&addr, আকার(addr))) >= 0)
	{
		nl->fd = sock;
		ফেরত 0;
	}
	অন্যথা
	{
		close(sock);
		ফেরত rc;
	}
}

পূর্ণসংখ্যা ul_nl_close(কাঠামো ul_nl_তথ্য *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	ফেরত close(nl->fd);
}

কাঠামো ul_nl_addr *ul_nl_addr_dup(কাঠামো ul_nl_addr *addr)
{
	কাঠামো ul_nl_addr *newaddr;
	newaddr = calloc(1, আকার(কাঠামো ul_nl_addr));
	যদি (!newaddr)
		যাও error;
	memcpy(newaddr, addr, আকার(কাঠামো ul_nl_addr));
	যদি (addr->ifa_address_len) {
		newaddr->ifa_address = malloc(addr->ifa_address_len);
		যদি (!newaddr->ifa_address)
			যাও error;
		memcpy(newaddr->ifa_address, addr->ifa_address,
		       addr->ifa_address_len);
	}
	যদি (addr->ifa_local_len) {
		newaddr->ifa_local = malloc(addr->ifa_local_len);
		যদি (!newaddr->ifa_local)
			যাও error;
		memcpy(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	যদি (addr->address == addr->ifa_local)
		newaddr->address = newaddr->ifa_local;
	অন্যথা
		newaddr->address = newaddr->ifa_address;
	যদি (!(newaddr->ifname = strdup(addr->ifname)))
		যাও error;
	ফেরত newaddr;
error:
	ul_nl_addr_free(newaddr);
	ফেরত ফাঁকা;
}

শূন্য ul_nl_addr_free(কাঠামো ul_nl_addr *addr)
{
	যদি (addr) {
		মুক্ত(addr->ifa_address);
		মুক্ত(addr->ifa_local);
		মুক্ত(addr->ifname);
		মুক্ত(addr);
	}
}

ধ্রুবক অক্ষর *ul_nl_addr_ntop(ধ্রুবক কাঠামো ul_nl_addr *addr, পূর্ণসংখ্যা addrid) {
	ধ্রুবক শূন্য **ifa_addr = (ধ্রুবক শূন্য **)((ধ্রুবক অক্ষর *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	স্থির অক্ষর addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	যদি (addr->ifa_family == AF_INET)
		ফেরত inet_ntop(AF_INET,
				 *ifa_addr, addr_str, আকার(addr_str));
	অন্যথা {
	/* if (addr->ifa_family == AF_INET6) */
		যদি (addr->ifa_scope == RT_SCOPE_LINK) {
			অক্ষর *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, আকার(addr_str));
			p = addr_str;
			যখন (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			ফেরত addr_str;
		} অন্যথা
			ফেরত inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, আকার(addr_str));
	}
}

#ifdef TEST_PROGRAM_NETLINK
#অন্তর্ভাব <stdio.h>

স্থির পূর্ণসংখ্যা callback_addr(কাঠামো ul_nl_তথ্য *nl)
{
	অক্ষর *str;

	printf("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	printf("  interface: %s\n", nl->addr.ifname);
	যদি (nl->addr.ifa_family == AF_INET)
		printf("  IPv4 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	অন্যথা
	/* if (nl->addr.ifa_family == AF_INET) */
		printf("  IPv6 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	নির্বাচন (nl->addr.ifa_scope) {
	ক্ষেত্র RT_SCOPE_UNIVERSE:	str = "global"; বিরতি;
	ক্ষেত্র RT_SCOPE_SITE:	str = "site"; বিরতি;
	ক্ষেত্র RT_SCOPE_LINK:	str = "link"; বিরতি;
	ক্ষেত্র RT_SCOPE_NOWHERE:	str = "nowhere"; বিরতি;
	পূর্বনির্ধারিত:		str = "other"; বিরতি;
	}
	printf("  scope: %s\n", str);
	যদি (nl->addr.ifa_valid != UINT32_MAX)
		printf("  valid: %u\n", nl->addr.ifa_valid);
	অন্যথা
		printf("  valid: forever\n");
	ফেরত 0;
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc __attribute__((__unused__)),
	 অক্ষর *argv[] __attribute__((__unused__)))
{
	পূর্ণসংখ্যা rc;
	কাঠামো ul_nl_তথ্য nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	যদি ((rc = ul_nl_open(&nl, 0)))
		ফেরত 1;
	যদি (ul_nl_request_dump(&nl, RTM_GETADDR))
		যাও error;
	যদি (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		যাও error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	যদি ((rc = ul_nl_close(&nl)) < 0)
		যাও error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	যদি ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		যাও error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	যদি (rc == UL_NL_RETURN)
		rc = 0;
error:
	যদি ((ul_nl_close(&nl)))
		rc = 1;
	ফেরত rc;
}
#endif /* TEST_PROGRAM_NETLINK */
