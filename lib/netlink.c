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

#समावेश <sys/socket.h>
#समावेश <arpa/inet.h>
#समावेश <netinet/in.h>
#समावेश "strutils.h"
#समावेश "netlink.h"
#समावेश "debug.h"
#समावेश "nls.h"

/*
 * Debug stuff (based on include/debug.h)
 */
#घोषणा ULNETLINK_DEBUG_HELP	(1 << 0)
#घोषणा ULNETLINK_DEBUG_INIT	(1 << 1)
#घोषणा ULNETLINK_DEBUG_NLMSG	(1 << 2)
#घोषणा ULNETLINK_DEBUG_ADDR	(1 << 3)

अटल UL_DEBUG_DEFINE_MASK(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ शून्य, 0 }
};

#घोषणा DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#घोषणा DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#घोषणा ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

अटल व्योम netlink_init_debug(व्योम)
{
	अगर (netlink_debug_mask)
		वापस;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

व्योम ul_nl_init(काष्ठा ul_nl_तथ्य *nl)
{
	netlink_init_debug();
	स_रखो(nl, 0, माप(काष्ठा ul_nl_तथ्य));
}

पूर्णांक ul_nl_request_dump(काष्ठा ul_nl_तथ्य *nl, uint16_t nlmsg_type)
{
	काष्ठा {
		काष्ठा nlmsghdr nh;
		काष्ठा rtgenmsg g;
	} req;

	स_रखो(&req, 0, माप(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(माप(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	अगर (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		वापस -1;
	वापस 0;
}

#घोषणा DBG_CASE(x) हाल x: str = #x; अवरोध
#घोषणा DBG_CASE_DEF8(x) शेष: snprintf(strx+2, 3, "%02hhx", x); str = strx; अवरोध
अटल व्योम dbg_addr(काष्ठा ul_nl_तथ्य *nl)
{
	अक्षर *str;
	अक्षर strx[5] = "0x";
	चयन (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	चयन (nl->addr.ifa_scope) {
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
अटल पूर्णांक process_addr(काष्ठा ul_nl_तथ्य *nl, काष्ठा nlmsghdr *nh)
{
	काष्ठा ifaddrmsg *ifaddr;
	काष्ठा rtattr *attr;
	पूर्णांक len;
	अटल अक्षर ifname[IF_NAMESIZE];
	bool has_local_address = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	स_रखो(&(nl->addr), 0, माप(काष्ठा ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	अगर ((if_indextoname(ifaddr->ifa_index, ifname)))
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
	len = nh->nlmsg_len - NLMSG_LENGTH(माप(*ifaddr));
	क्रम (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		चयन (attr->rta_type) {
		हाल IFA_ADDRESS:
			nl->addr.ifa_address = RTA_DATA(attr);
			nl->addr.ifa_address_len = RTA_PAYLOAD(attr);
			अगर (!has_local_address) {
				nl->addr.address = RTA_DATA(attr);
				nl->addr.address_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_address ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_address(&(nl->addr))));
		अवरोध;
		हाल IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_address = true;
			nl->addr.ifa_local = nl->addr.address = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.address_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		अवरोध;
		हाल IFA_CACHEINFO:
		{
			काष्ठा ifa_cacheinfo *ci =
				(काष्ठा ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		अवरोध;
		हाल IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		अवरोध;
		शेष:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		अवरोध;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	वापस (*(nl->callback_addr))(nl);
}

अटल पूर्णांक process_संदेश(काष्ठा ul_nl_तथ्य *nl, काष्ठा nlmsghdr *nh)
{
	पूर्णांक rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	चयन (nh->nlmsg_type) {
	हाल RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	हाल RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		अगर (nl->callback_addr)
			rc = process_addr(nl, nh);
		अवरोध;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	शेष:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		अवरोध;

	}
	वापस rc;
}

पूर्णांक ul_nl_process(काष्ठा ul_nl_तथ्य *nl, bool async, bool loop)
{
	अक्षर buf[बफ_मान];
	काष्ठा sockaddr_nl snl;
	काष्ठा nlmsghdr *nh;
	पूर्णांक rc;
	uint32_t len;

	काष्ठा iovec iov = {
		.iov_base = buf,
		.iov_len = माप(buf)
	};
	काष्ठा msghdr संदेश = {
		.msg_name = &snl,
		.msg_namelen = माप(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = शून्य,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	जबतक (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &संदेश, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		अगर (rc < 0) {
			अगर (त्रुटि_सं == EWOULDBLOCK || त्रुटि_सं == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				वापस UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			वापस rc;
		}
		अन्यथा
			len = rc;

		क्रम (nh = (काष्ठा nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			अगर (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				वापस -1;
			}
			अगर (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				वापस UL_NL_DONE;
			}

			rc = process_संदेश(nl, nh);
			अगर (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				अगर (rc != UL_NL_SOFT_ERROR)
					वापस rc;
			}
		}
		अगर (!loop)
			वापस 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

पूर्णांक ul_nl_open(काष्ठा ul_nl_तथ्य *nl, uint32_t nl_groups)
{
	काष्ठा sockaddr_nl addr = { 0, };
	पूर्णांक sock;
	पूर्णांक rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	अगर (sock < 0)
		वापस sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	अगर ((rc = bind(sock, (काष्ठा sockaddr *)&addr, माप(addr))) >= 0)
	{
		nl->fd = sock;
		वापस 0;
	}
	अन्यथा
	{
		बंद(sock);
		वापस rc;
	}
}

पूर्णांक ul_nl_close(काष्ठा ul_nl_तथ्य *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	वापस बंद(nl->fd);
}

काष्ठा ul_nl_addr *ul_nl_addr_dup(काष्ठा ul_nl_addr *addr)
{
	काष्ठा ul_nl_addr *newaddr;
	newaddr = सुस्मृति(1, माप(काष्ठा ul_nl_addr));
	अगर (!newaddr)
		जाओ error;
	स_नकल(newaddr, addr, माप(काष्ठा ul_nl_addr));
	अगर (addr->ifa_address_len) {
		newaddr->ifa_address = दो_स्मृति(addr->ifa_address_len);
		अगर (!newaddr->ifa_address)
			जाओ error;
		स_नकल(newaddr->ifa_address, addr->ifa_address,
		       addr->ifa_address_len);
	}
	अगर (addr->ifa_local_len) {
		newaddr->ifa_local = दो_स्मृति(addr->ifa_local_len);
		अगर (!newaddr->ifa_local)
			जाओ error;
		स_नकल(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	अगर (addr->address == addr->ifa_local)
		newaddr->address = newaddr->ifa_local;
	अन्यथा
		newaddr->address = newaddr->ifa_address;
	अगर (!(newaddr->ifname = strdup(addr->ifname)))
		जाओ error;
	वापस newaddr;
error:
	ul_nl_addr_free(newaddr);
	वापस शून्य;
}

व्योम ul_nl_addr_free(काष्ठा ul_nl_addr *addr)
{
	अगर (addr) {
		मुक्त(addr->ifa_address);
		मुक्त(addr->ifa_local);
		मुक्त(addr->ifname);
		मुक्त(addr);
	}
}

स्थिर अक्षर *ul_nl_addr_ntop(स्थिर काष्ठा ul_nl_addr *addr, पूर्णांक addrid) {
	स्थिर व्योम **ifa_addr = (स्थिर व्योम **)((स्थिर अक्षर *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	अटल अक्षर addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	अगर (addr->ifa_family == AF_INET)
		वापस inet_ntop(AF_INET,
				 *ifa_addr, addr_str, माप(addr_str));
	अन्यथा {
	/* if (addr->ifa_family == AF_INET6) */
		अगर (addr->ifa_scope == RT_SCOPE_LINK) {
			अक्षर *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, माप(addr_str));
			p = addr_str;
			जबतक (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			वापस addr_str;
		} अन्यथा
			वापस inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, माप(addr_str));
	}
}

#अगर_घोषित TEST_PROGRAM_NETLINK
#समावेश <stdio.h>

अटल पूर्णांक callback_addr(काष्ठा ul_nl_तथ्य *nl)
{
	अक्षर *str;

	म_लिखो("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	म_लिखो("  interface: %s\n", nl->addr.ifname);
	अगर (nl->addr.ifa_family == AF_INET)
		म_लिखो("  IPv4 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	अन्यथा
	/* if (nl->addr.ifa_family == AF_INET) */
		म_लिखो("  IPv6 %s\n",
		       ul_nl_addr_ntop_address(&(nl->addr)));
	चयन (nl->addr.ifa_scope) {
	हाल RT_SCOPE_UNIVERSE:	str = "global"; अवरोध;
	हाल RT_SCOPE_SITE:	str = "site"; अवरोध;
	हाल RT_SCOPE_LINK:	str = "link"; अवरोध;
	हाल RT_SCOPE_NOWHERE:	str = "nowhere"; अवरोध;
	शेष:		str = "other"; अवरोध;
	}
	म_लिखो("  scope: %s\n", str);
	अगर (nl->addr.ifa_valid != UINT32_MAX)
		म_लिखो("  valid: %u\n", nl->addr.ifa_valid);
	अन्यथा
		म_लिखो("  valid: forever\n");
	वापस 0;
}

पूर्णांक मुख्य(पूर्णांक argc __attribute__((__unused__)),
	 अक्षर *argv[] __attribute__((__unused__)))
{
	पूर्णांक rc;
	काष्ठा ul_nl_तथ्य nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	अगर ((rc = ul_nl_open(&nl, 0)))
		वापस 1;
	अगर (ul_nl_request_dump(&nl, RTM_GETADDR))
		जाओ error;
	अगर (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		जाओ error;
	माला_दो("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	अगर ((rc = ul_nl_close(&nl)) < 0)
		जाओ error;

	/* Monitor further changes */
	माला_दो("Going to monitor mode.");
	अगर ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		जाओ error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	अगर (rc == UL_NL_RETURN)
		rc = 0;
error:
	अगर ((ul_nl_close(&nl)))
		rc = 1;
	वापस rc;
}
#पूर्ण_अगर /* TEST_PROGRAM_NETLINK */
