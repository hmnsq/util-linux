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

#شامل <sys/socket.h>
#شامل <arpa/inet.h>
#شامل <netinet/in.h>
#شامل "strutils.h"
#شامل "netlink.h"
#شامل "debug.h"
#شامل "nls.h"

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETLINK_DEBUG_HELP	(1 << 0)
#define ULNETLINK_DEBUG_INIT	(1 << 1)
#define ULNETLINK_DEBUG_NLMSG	(1 << 2)
#define ULNETLINK_DEBUG_ADDR	(1 << 3)

ایستا UL_DEBUG_DEFINE_MASK(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ پوچ, 0 }
};

#define DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

ایستا تهی netlink_init_debug(تهی)
{
	اگر (netlink_debug_mask)
		بازگشت;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

تهی ul_nl_init(ساختار ul_nl_data *nl)
{
	netlink_init_debug();
	memset(nl, 0, اندازه(ساختار ul_nl_data));
}

صحیح ul_nl_request_dump(ساختار ul_nl_data *nl, uint16_t nlmsg_type)
{
	ساختار {
		ساختار nlmsghdr nh;
		ساختار rtgenmsg g;
	} req;

	memset(&req, 0, اندازه(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(اندازه(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	اگر (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		بازگشت -1;
	بازگشت 0;
}

#define DBG_CASE(x) حالت x: str = #x; بشکن
#define DBG_CASE_DEF8(x) پیشفرض: snprintf(strx+2, 3, "%02hhx", x); str = strx; بشکن
ایستا تهی dbg_addr(ساختار ul_nl_data *nl)
{
	نویسه *str;
	نویسه strx[5] = "0x";
	انتخاب (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	انتخاب (nl->addr.ifa_scope) {
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
ایستا صحیح process_addr(ساختار ul_nl_data *nl, ساختار nlmsghdr *nh)
{
	ساختار ifaddrmsg *ifaddr;
	ساختار rtattr *attr;
	صحیح len;
	ایستا نویسه ifname[IF_NAMESIZE];
	bool has_local_address = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, اندازه(ساختار ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	اگر ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	وگرنه
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
	len = nh->nlmsg_len - NLMSG_LENGTH(اندازه(*ifaddr));
	برای (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		انتخاب (attr->rta_type) {
		حالت IFA_ADDRESS:
			nl->addr.ifa_address = RTA_DATA(attr);
			nl->addr.ifa_address_len = RTA_PAYLOAD(attr);
			اگر (!has_local_address) {
				nl->addr.address = RTA_DATA(attr);
				nl->addr.address_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_address ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_address(&(nl->addr))));
		بشکن;
		حالت IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_address = true;
			nl->addr.ifa_local = nl->addr.address = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.address_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		بشکن;
		حالت IFA_CACHEINFO:
		{
			ساختار ifa_cacheinfo *ci =
				(ساختار ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		بشکن;
		حالت IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		بشکن;
		پیشفرض:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		بشکن;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	بازگشت (*(nl->callback_addr))(nl);
}

ایستا صحیح process_پیام(ساختار ul_nl_data *nl, ساختار nlmsghdr *nh)
{
	صحیح rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	انتخاب (nh->nlmsg_type) {
	حالت RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	حالت RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		اگر (nl->callback_addr)
			rc = process_addr(nl, nh);
		بشکن;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	پیشفرض:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		بشکن;

	}
	بازگشت rc;
}

صحیح ul_nl_process(ساختار ul_nl_data *nl, bool async, bool loop)
{
	نویسه buf[BUFSIZ];
	ساختار sockaddr_nl snl;
	ساختار nlmsghdr *nh;
	صحیح rc;
	uint32_t len;

	ساختار iovec iov = {
		.iov_base = buf,
		.iov_len = اندازه(buf)
	};
	ساختار msghdr پیام = {
		.msg_name = &snl,
		.msg_namelen = اندازه(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = پوچ,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	وقتی (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &پیام, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		اگر (rc < 0) {
			اگر (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				بازگشت UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			بازگشت rc;
		}
		وگرنه
			len = rc;

		برای (nh = (ساختار nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			اگر (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				بازگشت -1;
			}
			اگر (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				بازگشت UL_NL_DONE;
			}

			rc = process_پیام(nl, nh);
			اگر (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				اگر (rc != UL_NL_SOFT_ERROR)
					بازگشت rc;
			}
		}
		اگر (!loop)
			بازگشت 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

صحیح ul_nl_open(ساختار ul_nl_data *nl, uint32_t nl_groups)
{
	ساختار sockaddr_nl addr = { 0, };
	صحیح sock;
	صحیح rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	اگر (sock < 0)
		بازگشت sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	اگر ((rc = bind(sock, (ساختار sockaddr *)&addr, اندازه(addr))) >= 0)
	{
		nl->fd = sock;
		بازگشت 0;
	}
	وگرنه
	{
		close(sock);
		بازگشت rc;
	}
}

صحیح ul_nl_close(ساختار ul_nl_data *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	بازگشت close(nl->fd);
}

ساختار ul_nl_addr *ul_nl_addr_dup(ساختار ul_nl_addr *addr)
{
	ساختار ul_nl_addr *newaddr;
	newaddr = calloc(1, اندازه(ساختار ul_nl_addr));
	اگر (!newaddr)
		برو error;
	memcpy(newaddr, addr, اندازه(ساختار ul_nl_addr));
	اگر (addr->ifa_address_len) {
		newaddr->ifa_address = malloc(addr->ifa_address_len);
		اگر (!newaddr->ifa_address)
			برو error;
		memcpy(newaddr->ifa_address, addr->ifa_address,
		       addr->ifa_address_len);
	}
	اگر (addr->ifa_local_len) {
		newaddr->ifa_local = malloc(addr->ifa_local_len);
		اگر (!newaddr->ifa_local)
			برو error;
		memcpy(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	اگر (addr->address == addr->ifa_local)
		newaddr->address = newaddr->ifa_local;
	وگرنه
		newaddr->address = newaddr->ifa_address;
	اگر (!(newaddr->ifname = strdup(addr->ifname)))
		برو error;
	بازگشت newaddr;
error:
	ul_nl_addr_free(newaddr);
	بازگشت پوچ;
}

تهی ul_nl_addr_free(ساختار ul_nl_addr *addr)
{
	اگر (addr) {
		آزاد(addr->ifa_address);
		آزاد(addr->ifa_local);
		آزاد(addr->ifname);
		آزاد(addr);
	}
}

ثابت نویسه *ul_nl_addr_ntop(ثابت ساختار ul_nl_addr *addr, صحیح addrid) {
	ثابت تهی **ifa_addr = (ثابت تهی **)((ثابت نویسه *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	ایستا نویسه addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	اگر (addr->ifa_family == AF_INET)
		بازگشت inet_ntop(AF_INET,
				 *ifa_addr, addr_str, اندازه(addr_str));
	وگرنه {
	/* if (addr->ifa_family == AF_INET6) */
		اگر (addr->ifa_scope == RT_SCOPE_LINK) {
			نویسه *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, اندازه(addr_str));
			p = addr_str;
			وقتی (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			بازگشت addr_str;
		} وگرنه
			بازگشت inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, اندازه(addr_str));
	}
}

#ifdef TEST_PROGRAM_NETLINK
#شامل <stdio.h>

ایستا صحیح callback_addr(ساختار ul_nl_data *nl)
{
	نویسه *str;

	printf("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	printf("  interface: %s\n", nl->addr.ifname);
	اگر (nl->addr.ifa_family == AF_INET)
		printf("  IPv4 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	وگرنه
	/* if (nl->addr.ifa_family == AF_INET) */
		printf("  IPv6 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	انتخاب (nl->addr.ifa_scope) {
	حالت RT_SCOPE_UNIVERSE:	str = "global"; بشکن;
	حالت RT_SCOPE_SITE:	str = "site"; بشکن;
	حالت RT_SCOPE_LINK:	str = "link"; بشکن;
	حالت RT_SCOPE_NOWHERE:	str = "nowhere"; بشکن;
	پیشفرض:		str = "other"; بشکن;
	}
	printf("  scope: %s\n", str);
	اگر (nl->addr.ifa_valid != UINT32_MAX)
		printf("  valid: %u\n", nl->addr.ifa_valid);
	وگرنه
		printf("  valid: forever\n");
	بازگشت 0;
}

صحیح اصلی(صحیح argc __attribute__((__unused__)),
	 نویسه *argv[] __attribute__((__unused__)))
{
	صحیح rc;
	ساختار ul_nl_data nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	اگر ((rc = ul_nl_open(&nl, 0)))
		بازگشت 1;
	اگر (ul_nl_request_dump(&nl, RTM_GETADDR))
		برو error;
	اگر (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		برو error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	اگر ((rc = ul_nl_close(&nl)) < 0)
		برو error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	اگر ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		برو error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	اگر (rc == UL_NL_RETURN)
		rc = 0;
error:
	اگر ((ul_nl_close(&nl)))
		rc = 1;
	بازگشت rc;
}
#endif /* TEST_PROGRAM_NETLINK */
