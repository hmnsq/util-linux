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

#अन्तर्भाव <sys/socket.h>
#अन्तर्भाव <arpa/inet.h>
#अन्तर्भाव <netinet/in.h>
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "netlink.h"
#अन्तर्भाव "debug.h"
#अन्तर्भाव "nls.h"

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETLINK_DEBUG_HELP	(1 << 0)
#define ULNETLINK_DEBUG_INIT	(1 << 1)
#define ULNETLINK_DEBUG_NLMSG	(1 << 2)
#define ULNETLINK_DEBUG_ADDR	(1 << 3)

स्थिर UL_DEBUG_DEFINE_MASK(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ रिक्त, 0 }
};

#define DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

स्थिर शून्य netlink_init_debug(शून्य)
{
	यदि (netlink_debug_mask)
		प्रत्यागम;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

शून्य ul_nl_init(संरचना ul_nl_data *nl)
{
	netlink_init_debug();
	memset(nl, 0, परिमाण(संरचना ul_nl_data));
}

पूर्णाङ्क ul_nl_request_dump(संरचना ul_nl_data *nl, uint16_t nlmsg_type)
{
	संरचना {
		संरचना nlmsghdr nh;
		संरचना rtgenmsg g;
	} req;

	memset(&req, 0, परिमाण(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(परिमाण(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	यदि (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		प्रत्यागम -1;
	प्रत्यागम 0;
}

#define DBG_CASE(x) पक्ष x: str = #x; भङ्ग
#define DBG_CASE_DEF8(x) स्वभाव: snprintf(strx+2, 3, "%02hhx", x); str = strx; भङ्ग
स्थिर शून्य dbg_addr(संरचना ul_nl_data *nl)
{
	वर्ण *str;
	वर्ण strx[5] = "0x";
	विकल्प (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	विकल्प (nl->addr.ifa_scope) {
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
स्थिर पूर्णाङ्क process_addr(संरचना ul_nl_data *nl, संरचना nlmsghdr *nh)
{
	संरचना ifaddrmsg *ifaddr;
	संरचना rtattr *attr;
	पूर्णाङ्क len;
	स्थिर वर्ण ifname[IF_NAMESIZE];
	bool has_local_address = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, परिमाण(संरचना ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	यदि ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	अन्यथा
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
	len = nh->nlmsg_len - NLMSG_LENGTH(परिमाण(*ifaddr));
	प्रति (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		विकल्प (attr->rta_type) {
		पक्ष IFA_ADDRESS:
			nl->addr.ifa_address = RTA_DATA(attr);
			nl->addr.ifa_address_len = RTA_PAYLOAD(attr);
			यदि (!has_local_address) {
				nl->addr.address = RTA_DATA(attr);
				nl->addr.address_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_address ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_address(&(nl->addr))));
		भङ्ग;
		पक्ष IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_address = true;
			nl->addr.ifa_local = nl->addr.address = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.address_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		भङ्ग;
		पक्ष IFA_CACHEINFO:
		{
			संरचना ifa_cacheinfo *ci =
				(संरचना ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		भङ्ग;
		पक्ष IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		भङ्ग;
		स्वभाव:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		भङ्ग;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	प्रत्यागम (*(nl->callback_addr))(nl);
}

स्थिर पूर्णाङ्क process_संदेश(संरचना ul_nl_data *nl, संरचना nlmsghdr *nh)
{
	पूर्णाङ्क rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	विकल्प (nh->nlmsg_type) {
	पक्ष RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	पक्ष RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		यदि (nl->callback_addr)
			rc = process_addr(nl, nh);
		भङ्ग;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	स्वभाव:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		भङ्ग;

	}
	प्रत्यागम rc;
}

पूर्णाङ्क ul_nl_process(संरचना ul_nl_data *nl, bool async, bool loop)
{
	वर्ण buf[BUFSIZ];
	संरचना sockaddr_nl snl;
	संरचना nlmsghdr *nh;
	पूर्णाङ्क rc;
	uint32_t len;

	संरचना iovec iov = {
		.iov_base = buf,
		.iov_len = परिमाण(buf)
	};
	संरचना msghdr संदेश = {
		.msg_name = &snl,
		.msg_namelen = परिमाण(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = रिक्त,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	यावत् (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &संदेश, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		यदि (rc < 0) {
			यदि (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				प्रत्यागम UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			प्रत्यागम rc;
		}
		अन्यथा
			len = rc;

		प्रति (nh = (संरचना nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			यदि (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				प्रत्यागम -1;
			}
			यदि (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				प्रत्यागम UL_NL_DONE;
			}

			rc = process_संदेश(nl, nh);
			यदि (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				यदि (rc != UL_NL_SOFT_ERROR)
					प्रत्यागम rc;
			}
		}
		यदि (!loop)
			प्रत्यागम 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

पूर्णाङ्क ul_nl_open(संरचना ul_nl_data *nl, uint32_t nl_groups)
{
	संरचना sockaddr_nl addr = { 0, };
	पूर्णाङ्क sock;
	पूर्णाङ्क rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	यदि (sock < 0)
		प्रत्यागम sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	यदि ((rc = bind(sock, (संरचना sockaddr *)&addr, परिमाण(addr))) >= 0)
	{
		nl->fd = sock;
		प्रत्यागम 0;
	}
	अन्यथा
	{
		close(sock);
		प्रत्यागम rc;
	}
}

पूर्णाङ्क ul_nl_close(संरचना ul_nl_data *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	प्रत्यागम close(nl->fd);
}

संरचना ul_nl_addr *ul_nl_addr_dup(संरचना ul_nl_addr *addr)
{
	संरचना ul_nl_addr *newaddr;
	newaddr = calloc(1, परिमाण(संरचना ul_nl_addr));
	यदि (!newaddr)
		गच्छ error;
	memcpy(newaddr, addr, परिमाण(संरचना ul_nl_addr));
	यदि (addr->ifa_address_len) {
		newaddr->ifa_address = malloc(addr->ifa_address_len);
		यदि (!newaddr->ifa_address)
			गच्छ error;
		memcpy(newaddr->ifa_address, addr->ifa_address,
		       addr->ifa_address_len);
	}
	यदि (addr->ifa_local_len) {
		newaddr->ifa_local = malloc(addr->ifa_local_len);
		यदि (!newaddr->ifa_local)
			गच्छ error;
		memcpy(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	यदि (addr->address == addr->ifa_local)
		newaddr->address = newaddr->ifa_local;
	अन्यथा
		newaddr->address = newaddr->ifa_address;
	यदि (!(newaddr->ifname = strdup(addr->ifname)))
		गच्छ error;
	प्रत्यागम newaddr;
error:
	ul_nl_addr_free(newaddr);
	प्रत्यागम रिक्त;
}

शून्य ul_nl_addr_free(संरचना ul_nl_addr *addr)
{
	यदि (addr) {
		मुक्त(addr->ifa_address);
		मुक्त(addr->ifa_local);
		मुक्त(addr->ifname);
		मुक्त(addr);
	}
}

ध्रुव वर्ण *ul_nl_addr_ntop(ध्रुव संरचना ul_nl_addr *addr, पूर्णाङ्क addrid) {
	ध्रुव शून्य **ifa_addr = (ध्रुव शून्य **)((ध्रुव वर्ण *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	स्थिर वर्ण addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	यदि (addr->ifa_family == AF_INET)
		प्रत्यागम inet_ntop(AF_INET,
				 *ifa_addr, addr_str, परिमाण(addr_str));
	अन्यथा {
	/* if (addr->ifa_family == AF_INET6) */
		यदि (addr->ifa_scope == RT_SCOPE_LINK) {
			वर्ण *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, परिमाण(addr_str));
			p = addr_str;
			यावत् (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			प्रत्यागम addr_str;
		} अन्यथा
			प्रत्यागम inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, परिमाण(addr_str));
	}
}

#ifdef TEST_PROGRAM_NETLINK
#अन्तर्भाव <stdio.h>

स्थिर पूर्णाङ्क callback_addr(संरचना ul_nl_data *nl)
{
	वर्ण *str;

	printf("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	printf("  interface: %s\n", nl->addr.ifname);
	यदि (nl->addr.ifa_family == AF_INET)
		printf("  IPv4 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	अन्यथा
	/* if (nl->addr.ifa_family == AF_INET) */
		printf("  IPv6 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	विकल्प (nl->addr.ifa_scope) {
	पक्ष RT_SCOPE_UNIVERSE:	str = "global"; भङ्ग;
	पक्ष RT_SCOPE_SITE:	str = "site"; भङ्ग;
	पक्ष RT_SCOPE_LINK:	str = "link"; भङ्ग;
	पक्ष RT_SCOPE_NOWHERE:	str = "nowhere"; भङ्ग;
	स्वभाव:		str = "other"; भङ्ग;
	}
	printf("  scope: %s\n", str);
	यदि (nl->addr.ifa_valid != UINT32_MAX)
		printf("  valid: %u\n", nl->addr.ifa_valid);
	अन्यथा
		printf("  valid: forever\n");
	प्रत्यागम 0;
}

पूर्णाङ्क main(पूर्णाङ्क argc __attribute__((__unused__)),
	 वर्ण *argv[] __attribute__((__unused__)))
{
	पूर्णाङ्क rc;
	संरचना ul_nl_data nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	यदि ((rc = ul_nl_open(&nl, 0)))
		प्रत्यागम 1;
	यदि (ul_nl_request_dump(&nl, RTM_GETADDR))
		गच्छ error;
	यदि (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		गच्छ error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	यदि ((rc = ul_nl_close(&nl)) < 0)
		गच्छ error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	यदि ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		गच्छ error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	यदि (rc == UL_NL_RETURN)
		rc = 0;
error:
	यदि ((ul_nl_close(&nl)))
		rc = 1;
	प्रत्यागम rc;
}
#endif /* TEST_PROGRAM_NETLINK */
