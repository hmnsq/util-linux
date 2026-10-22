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

#அந்தர்பாவ <sys/socket.h>
#அந்தர்பாவ <arpa/inet.h>
#அந்தர்பாவ <netinet/in.h>
#அந்தர்பாவ "strutils.h"
#அந்தர்பாவ "netlink.h"
#அந்தர்பாவ "debug.h"
#அந்தர்பாவ "nls.h"

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETLINK_DEBUG_HELP	(1 << 0)
#define ULNETLINK_DEBUG_INIT	(1 << 1)
#define ULNETLINK_DEBUG_NLMSG	(1 << 2)
#define ULNETLINK_DEBUG_ADDR	(1 << 3)

நிலையான UL_DEBUG_DEFINE_MASK(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ வெற்று, 0 }
};

#define DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

நிலையான வெறுமை netlink_init_debug(வெறுமை)
{
	எனில் (netlink_debug_mask)
		திரும்பு;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

வெறுமை ul_nl_init(கட்டமைப்பு ul_nl_data *nl)
{
	netlink_init_debug();
	memset(nl, 0, அளவு(கட்டமைப்பு ul_nl_data));
}

முழு ul_nl_request_dump(கட்டமைப்பு ul_nl_data *nl, uint16_t nlmsg_type)
{
	கட்டமைப்பு {
		கட்டமைப்பு nlmsghdr nh;
		கட்டமைப்பு rtgenmsg g;
	} req;

	memset(&req, 0, அளவு(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(அளவு(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	எனில் (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		திரும்பு -1;
	திரும்பு 0;
}

#define DBG_CASE(x) நிலை x: str = #x; நிறுத்து
#define DBG_CASE_DEF8(x) இயல்பு: snprintf(strx+2, 3, "%02hhx", x); str = strx; நிறுத்து
நிலையான வெறுமை dbg_addr(கட்டமைப்பு ul_nl_data *nl)
{
	எழுத்து *str;
	எழுத்து strx[5] = "0x";
	தேர்வு (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	தேர்வு (nl->addr.ifa_scope) {
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
நிலையான முழு process_addr(கட்டமைப்பு ul_nl_data *nl, கட்டமைப்பு nlmsghdr *nh)
{
	கட்டமைப்பு ifaddrmsg *ifaddr;
	கட்டமைப்பு rtattr *attr;
	முழு len;
	நிலையான எழுத்து ifname[IF_NAMESIZE];
	bool has_local_address = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, அளவு(கட்டமைப்பு ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	எனில் ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	இல்லையெனில்
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
	len = nh->nlmsg_len - NLMSG_LENGTH(அளவு(*ifaddr));
	க்கு (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		தேர்வு (attr->rta_type) {
		நிலை IFA_ADDRESS:
			nl->addr.ifa_address = RTA_DATA(attr);
			nl->addr.ifa_address_len = RTA_PAYLOAD(attr);
			எனில் (!has_local_address) {
				nl->addr.address = RTA_DATA(attr);
				nl->addr.address_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_address ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_address(&(nl->addr))));
		நிறுத்து;
		நிலை IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_address = true;
			nl->addr.ifa_local = nl->addr.address = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.address_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		நிறுத்து;
		நிலை IFA_CACHEINFO:
		{
			கட்டமைப்பு ifa_cacheinfo *ci =
				(கட்டமைப்பு ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		நிறுத்து;
		நிலை IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		நிறுத்து;
		இயல்பு:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		நிறுத்து;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	திரும்பு (*(nl->callback_addr))(nl);
}

நிலையான முழு process_செய்தி(கட்டமைப்பு ul_nl_data *nl, கட்டமைப்பு nlmsghdr *nh)
{
	முழு rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	தேர்வு (nh->nlmsg_type) {
	நிலை RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	நிலை RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		எனில் (nl->callback_addr)
			rc = process_addr(nl, nh);
		நிறுத்து;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	இயல்பு:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		நிறுத்து;

	}
	திரும்பு rc;
}

முழு ul_nl_process(கட்டமைப்பு ul_nl_data *nl, bool async, bool loop)
{
	எழுத்து buf[BUFSIZ];
	கட்டமைப்பு sockaddr_nl snl;
	கட்டமைப்பு nlmsghdr *nh;
	முழு rc;
	uint32_t len;

	கட்டமைப்பு iovec iov = {
		.iov_base = buf,
		.iov_len = அளவு(buf)
	};
	கட்டமைப்பு msghdr செய்தி = {
		.msg_name = &snl,
		.msg_namelen = அளவு(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = வெற்று,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	போது (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &செய்தி, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		எனில் (rc < 0) {
			எனில் (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				திரும்பு UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			திரும்பு rc;
		}
		இல்லையெனில்
			len = rc;

		க்கு (nh = (கட்டமைப்பு nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			எனில் (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				திரும்பு -1;
			}
			எனில் (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				திரும்பு UL_NL_DONE;
			}

			rc = process_செய்தி(nl, nh);
			எனில் (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				எனில் (rc != UL_NL_SOFT_ERROR)
					திரும்பு rc;
			}
		}
		எனில் (!loop)
			திரும்பு 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

முழு ul_nl_open(கட்டமைப்பு ul_nl_data *nl, uint32_t nl_groups)
{
	கட்டமைப்பு sockaddr_nl addr = { 0, };
	முழு sock;
	முழு rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	எனில் (sock < 0)
		திரும்பு sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	எனில் ((rc = bind(sock, (கட்டமைப்பு sockaddr *)&addr, அளவு(addr))) >= 0)
	{
		nl->fd = sock;
		திரும்பு 0;
	}
	இல்லையெனில்
	{
		close(sock);
		திரும்பு rc;
	}
}

முழு ul_nl_close(கட்டமைப்பு ul_nl_data *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	திரும்பு close(nl->fd);
}

கட்டமைப்பு ul_nl_addr *ul_nl_addr_dup(கட்டமைப்பு ul_nl_addr *addr)
{
	கட்டமைப்பு ul_nl_addr *newaddr;
	newaddr = calloc(1, அளவு(கட்டமைப்பு ul_nl_addr));
	எனில் (!newaddr)
		செல் error;
	memcpy(newaddr, addr, அளவு(கட்டமைப்பு ul_nl_addr));
	எனில் (addr->ifa_address_len) {
		newaddr->ifa_address = malloc(addr->ifa_address_len);
		எனில் (!newaddr->ifa_address)
			செல் error;
		memcpy(newaddr->ifa_address, addr->ifa_address,
		       addr->ifa_address_len);
	}
	எனில் (addr->ifa_local_len) {
		newaddr->ifa_local = malloc(addr->ifa_local_len);
		எனில் (!newaddr->ifa_local)
			செல் error;
		memcpy(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	எனில் (addr->address == addr->ifa_local)
		newaddr->address = newaddr->ifa_local;
	இல்லையெனில்
		newaddr->address = newaddr->ifa_address;
	எனில் (!(newaddr->ifname = strdup(addr->ifname)))
		செல் error;
	திரும்பு newaddr;
error:
	ul_nl_addr_free(newaddr);
	திரும்பு வெற்று;
}

வெறுமை ul_nl_addr_free(கட்டமைப்பு ul_nl_addr *addr)
{
	எனில் (addr) {
		விடுவி(addr->ifa_address);
		விடுவி(addr->ifa_local);
		விடுவி(addr->ifname);
		விடுவி(addr);
	}
}

மாறிலி எழுத்து *ul_nl_addr_ntop(மாறிலி கட்டமைப்பு ul_nl_addr *addr, முழு addrid) {
	மாறிலி வெறுமை **ifa_addr = (மாறிலி வெறுமை **)((மாறிலி எழுத்து *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	நிலையான எழுத்து addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	எனில் (addr->ifa_family == AF_INET)
		திரும்பு inet_ntop(AF_INET,
				 *ifa_addr, addr_str, அளவு(addr_str));
	இல்லையெனில் {
	/* if (addr->ifa_family == AF_INET6) */
		எனில் (addr->ifa_scope == RT_SCOPE_LINK) {
			எழுத்து *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, அளவு(addr_str));
			p = addr_str;
			போது (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			திரும்பு addr_str;
		} இல்லையெனில்
			திரும்பு inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, அளவு(addr_str));
	}
}

#ifdef TEST_PROGRAM_NETLINK
#அந்தர்பாவ <stdio.h>

நிலையான முழு callback_addr(கட்டமைப்பு ul_nl_data *nl)
{
	எழுத்து *str;

	printf("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	printf("  interface: %s\n", nl->addr.ifname);
	எனில் (nl->addr.ifa_family == AF_INET)
		printf("  IPv4 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	இல்லையெனில்
	/* if (nl->addr.ifa_family == AF_INET) */
		printf("  IPv6 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	தேர்வு (nl->addr.ifa_scope) {
	நிலை RT_SCOPE_UNIVERSE:	str = "global"; நிறுத்து;
	நிலை RT_SCOPE_SITE:	str = "site"; நிறுத்து;
	நிலை RT_SCOPE_LINK:	str = "link"; நிறுத்து;
	நிலை RT_SCOPE_NOWHERE:	str = "nowhere"; நிறுத்து;
	இயல்பு:		str = "other"; நிறுத்து;
	}
	printf("  scope: %s\n", str);
	எனில் (nl->addr.ifa_valid != UINT32_MAX)
		printf("  valid: %u\n", nl->addr.ifa_valid);
	இல்லையெனில்
		printf("  valid: forever\n");
	திரும்பு 0;
}

முழு முதன்மை(முழு argc __attribute__((__unused__)),
	 எழுத்து *argv[] __attribute__((__unused__)))
{
	முழு rc;
	கட்டமைப்பு ul_nl_data nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	எனில் ((rc = ul_nl_open(&nl, 0)))
		திரும்பு 1;
	எனில் (ul_nl_request_dump(&nl, RTM_GETADDR))
		செல் error;
	எனில் (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		செல் error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	எனில் ((rc = ul_nl_close(&nl)) < 0)
		செல் error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	எனில் ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		செல் error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	எனில் (rc == UL_NL_RETURN)
		rc = 0;
error:
	எனில் ((ul_nl_close(&nl)))
		rc = 1;
	திரும்பு rc;
}
#endif /* TEST_PROGRAM_NETLINK */
