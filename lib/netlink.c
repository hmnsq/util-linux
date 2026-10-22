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
	जर (netlink_debug_mask)
		परत;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

शून्य ul_nl_init(संरचना ul_nl_data *nl)
{
	netlink_init_debug();
	स_रखो(nl, 0, आकार(संरचना ul_nl_data));
}

पूर्णांक ul_nl_request_dump(संरचना ul_nl_data *nl, uint16_t nlmsg_type)
{
	संरचना {
		संरचना nlmsghdr nh;
		संरचना rtgenmsg g;
	} req;

	स_रखो(&req, 0, आकार(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(आकार(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	जर (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		परत -1;
	परत 0;
}

#define DBG_CASE(x) स्थिती x: str = #x; थांबा
#define DBG_CASE_DEF8(x) पूर्वनिर्धारित: snprintf(strx+2, 3, "%02hhx", x); str = strx; थांबा
स्थिर शून्य dbg_addr(संरचना ul_nl_data *nl)
{
	अक्षर *str;
	अक्षर strx[5] = "0x";
	निवड (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	निवड (nl->addr.ifa_scope) {
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
स्थिर पूर्णांक process_addr(संरचना ul_nl_data *nl, संरचना nlmsghdr *nh)
{
	संरचना ifaddrmsg *ifaddr;
	संरचना rtattr *attr;
	पूर्णांक len;
	स्थिर अक्षर ifname[IF_NAMESIZE];
	bool has_local_address = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	स_रखो(&(nl->addr), 0, आकार(संरचना ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	जर ((if_indextoname(ifaddr->ifa_index, ifname)))
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
	len = nh->nlmsg_len - NLMSG_LENGTH(आकार(*ifaddr));
	साठी (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		निवड (attr->rta_type) {
		स्थिती IFA_ADDRESS:
			nl->addr.ifa_address = RTA_DATA(attr);
			nl->addr.ifa_address_len = RTA_PAYLOAD(attr);
			जर (!has_local_address) {
				nl->addr.address = RTA_DATA(attr);
				nl->addr.address_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_address ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_address(&(nl->addr))));
		थांबा;
		स्थिती IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_address = true;
			nl->addr.ifa_local = nl->addr.address = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.address_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		थांबा;
		स्थिती IFA_CACHEINFO:
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
		थांबा;
		स्थिती IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		थांबा;
		पूर्वनिर्धारित:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		थांबा;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	परत (*(nl->callback_addr))(nl);
}

स्थिर पूर्णांक process_संदेश(संरचना ul_nl_data *nl, संरचना nlmsghdr *nh)
{
	पूर्णांक rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	निवड (nh->nlmsg_type) {
	स्थिती RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	स्थिती RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		जर (nl->callback_addr)
			rc = process_addr(nl, nh);
		थांबा;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	पूर्वनिर्धारित:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		थांबा;

	}
	परत rc;
}

पूर्णांक ul_nl_process(संरचना ul_nl_data *nl, bool async, bool loop)
{
	अक्षर buf[बफ_मान];
	संरचना sockaddr_nl snl;
	संरचना nlmsghdr *nh;
	पूर्णांक rc;
	uint32_t len;

	संरचना iovec iov = {
		.iov_base = buf,
		.iov_len = आकार(buf)
	};
	संरचना msghdr संदेश = {
		.msg_name = &snl,
		.msg_namelen = आकार(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = रिक्त,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	पर्यंत (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &संदेश, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		जर (rc < 0) {
			जर (त्रुटि_सं == EWOULDBLOCK || त्रुटि_सं == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				परत UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			परत rc;
		}
		अन्यथा
			len = rc;

		साठी (nh = (संरचना nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			जर (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				परत -1;
			}
			जर (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				परत UL_NL_DONE;
			}

			rc = process_संदेश(nl, nh);
			जर (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				जर (rc != UL_NL_SOFT_ERROR)
					परत rc;
			}
		}
		जर (!loop)
			परत 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

पूर्णांक ul_nl_open(संरचना ul_nl_data *nl, uint32_t nl_groups)
{
	संरचना sockaddr_nl addr = { 0, };
	पूर्णांक sock;
	पूर्णांक rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	जर (sock < 0)
		परत sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	जर ((rc = bind(sock, (संरचना sockaddr *)&addr, आकार(addr))) >= 0)
	{
		nl->fd = sock;
		परत 0;
	}
	अन्यथा
	{
		बंद(sock);
		परत rc;
	}
}

पूर्णांक ul_nl_close(संरचना ul_nl_data *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	परत बंद(nl->fd);
}

संरचना ul_nl_addr *ul_nl_addr_dup(संरचना ul_nl_addr *addr)
{
	संरचना ul_nl_addr *newaddr;
	newaddr = सुस्मृति(1, आकार(संरचना ul_nl_addr));
	जर (!newaddr)
		जा error;
	स_नकल(newaddr, addr, आकार(संरचना ul_nl_addr));
	जर (addr->ifa_address_len) {
		newaddr->ifa_address = दो_स्मृति(addr->ifa_address_len);
		जर (!newaddr->ifa_address)
			जा error;
		स_नकल(newaddr->ifa_address, addr->ifa_address,
		       addr->ifa_address_len);
	}
	जर (addr->ifa_local_len) {
		newaddr->ifa_local = दो_स्मृति(addr->ifa_local_len);
		जर (!newaddr->ifa_local)
			जा error;
		स_नकल(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	जर (addr->address == addr->ifa_local)
		newaddr->address = newaddr->ifa_local;
	अन्यथा
		newaddr->address = newaddr->ifa_address;
	जर (!(newaddr->ifname = strdup(addr->ifname)))
		जा error;
	परत newaddr;
error:
	ul_nl_addr_free(newaddr);
	परत रिक्त;
}

शून्य ul_nl_addr_free(संरचना ul_nl_addr *addr)
{
	जर (addr) {
		मुक्त(addr->ifa_address);
		मुक्त(addr->ifa_local);
		मुक्त(addr->ifname);
		मुक्त(addr);
	}
}

स्थिरांक अक्षर *ul_nl_addr_ntop(स्थिरांक संरचना ul_nl_addr *addr, पूर्णांक addrid) {
	स्थिरांक शून्य **ifa_addr = (स्थिरांक शून्य **)((स्थिरांक अक्षर *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	स्थिर अक्षर addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	जर (addr->ifa_family == AF_INET)
		परत inet_ntop(AF_INET,
				 *ifa_addr, addr_str, आकार(addr_str));
	अन्यथा {
	/* if (addr->ifa_family == AF_INET6) */
		जर (addr->ifa_scope == RT_SCOPE_LINK) {
			अक्षर *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, आकार(addr_str));
			p = addr_str;
			पर्यंत (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			परत addr_str;
		} अन्यथा
			परत inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, आकार(addr_str));
	}
}

#ifdef TEST_PROGRAM_NETLINK
#अन्तर्भाव <stdio.h>

स्थिर पूर्णांक callback_addr(संरचना ul_nl_data *nl)
{
	अक्षर *str;

	म_लिखो("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	म_लिखो("  interface: %s\n", nl->addr.ifname);
	जर (nl->addr.ifa_family == AF_INET)
		म_लिखो("  IPv4 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	अन्यथा
	/* if (nl->addr.ifa_family == AF_INET) */
		म_लिखो("  IPv6 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	निवड (nl->addr.ifa_scope) {
	स्थिती RT_SCOPE_UNIVERSE:	str = "global"; थांबा;
	स्थिती RT_SCOPE_SITE:	str = "site"; थांबा;
	स्थिती RT_SCOPE_LINK:	str = "link"; थांबा;
	स्थिती RT_SCOPE_NOWHERE:	str = "nowhere"; थांबा;
	पूर्वनिर्धारित:		str = "other"; थांबा;
	}
	म_लिखो("  scope: %s\n", str);
	जर (nl->addr.ifa_valid != UINT32_MAX)
		म_लिखो("  valid: %u\n", nl->addr.ifa_valid);
	अन्यथा
		म_लिखो("  valid: forever\n");
	परत 0;
}

पूर्णांक main(पूर्णांक argc __attribute__((__unused__)),
	 अक्षर *argv[] __attribute__((__unused__)))
{
	पूर्णांक rc;
	संरचना ul_nl_data nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	जर ((rc = ul_nl_open(&nl, 0)))
		परत 1;
	जर (ul_nl_request_dump(&nl, RTM_GETADDR))
		जा error;
	जर (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		जा error;
	माला_दो("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	जर ((rc = ul_nl_close(&nl)) < 0)
		जा error;

	/* Monitor further changes */
	माला_दो("Going to monitor mode.");
	जर ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		जा error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	जर (rc == UL_NL_RETURN)
		rc = 0;
error:
	जर ((ul_nl_close(&nl)))
		rc = 1;
	परत rc;
}
#endif /* TEST_PROGRAM_NETLINK */
