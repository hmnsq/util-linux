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

#అన్తర్భావ <sys/socket.h>
#అన్తర్భావ <arpa/inet.h>
#అన్తర్భావ <netinet/in.h>
#అన్తర్భావ "strutils.h"
#అన్తర్భావ "netlink.h"
#అన్తర్భావ "debug.h"
#అన్తర్భావ "nls.h"

/*
 * Debug stuff (based on include/debug.h)
 */
#నిర్వచించు ULNETLINK_DEBUG_HELP	(1 << 0)
#నిర్వచించు ULNETLINK_DEBUG_INIT	(1 << 1)
#నిర్వచించు ULNETLINK_DEBUG_NLMSG	(1 << 2)
#నిర్వచించు ULNETLINK_DEBUG_ADDR	(1 << 3)

స్థిర UL_DEBUG_DEFINE_ముసుగు(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ ఖాళీ, 0 }
};

#నిర్వచించు DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#నిర్వచించు DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#నిర్వచించు ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

స్థిర శూన్య netlink_init_debug(శూన్య)
{
	ఒకవేళ (netlink_debug_ముసుగు)
		తిరిగి;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

శూన్య ul_nl_init(నిర్మాణం ul_nl_దత్తాంశం *nl)
{
	netlink_init_debug();
	memset(nl, 0, పరిమాణం(నిర్మాణం ul_nl_దత్తాంశం));
}

పూర్ణాంకం ul_nl_request_dump(నిర్మాణం ul_nl_దత్తాంశం *nl, uint16_t nlmsg_type)
{
	నిర్మాణం {
		నిర్మాణం nlmsghdr nh;
		నిర్మాణం rtgenmsg g;
	} req;

	memset(&req, 0, పరిమాణం(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(పరిమాణం(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	ఒకవేళ (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		తిరిగి -1;
	తిరిగి 0;
}

#నిర్వచించు DBG_CASE(x) స్థితి x: str = #x; ఆపు
#నిర్వచించు DBG_CASE_DEF8(x) అప్రమేయ: snprintf(strx+2, 3, "%02hhx", x); str = strx; ఆపు
స్థిర శూన్య dbg_addr(నిర్మాణం ul_nl_దత్తాంశం *nl)
{
	అక్షరం *str;
	అక్షరం strx[5] = "0x";
	ఎంపిక (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	ఎంపిక (nl->addr.ifa_scope) {
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
స్థిర పూర్ణాంకం process_addr(నిర్మాణం ul_nl_దత్తాంశం *nl, నిర్మాణం nlmsghdr *nh)
{
	నిర్మాణం ifaddrmsg *ifaddr;
	నిర్మాణం rtattr *attr;
	పూర్ణాంకం len;
	స్థిర అక్షరం ifname[IF_NAMESIZE];
	bool has_local_చిరునామా = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, పరిమాణం(నిర్మాణం ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	ఒకవేళ ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	లేకపోతే
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
	len = nh->nlmsg_len - NLMSG_LENGTH(పరిమాణం(*ifaddr));
	కొరకు (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		ఎంపిక (attr->rta_type) {
		స్థితి IFA_ADDRESS:
			nl->addr.ifa_చిరునామా = RTA_DATA(attr);
			nl->addr.ifa_చిరునామా_len = RTA_PAYLOAD(attr);
			ఒకవేళ (!has_local_చిరునామా) {
				nl->addr.చిరునామా = RTA_DATA(attr);
				nl->addr.చిరునామా_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_చిరునామా ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_చిరునామా(&(nl->addr))));
		ఆపు;
		స్థితి IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_చిరునామా = true;
			nl->addr.ifa_local = nl->addr.చిరునామా = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.చిరునామా_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		ఆపు;
		స్థితి IFA_CACHEINFO:
		{
			నిర్మాణం ifa_cacheinfo *ci =
				(నిర్మాణం ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		ఆపు;
		స్థితి IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		ఆపు;
		అప్రమేయ:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		ఆపు;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	తిరిగి (*(nl->callback_addr))(nl);
}

స్థిర పూర్ణాంకం process_సందేశం(నిర్మాణం ul_nl_దత్తాంశం *nl, నిర్మాణం nlmsghdr *nh)
{
	పూర్ణాంకం rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	ఎంపిక (nh->nlmsg_type) {
	స్థితి RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	స్థితి RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		ఒకవేళ (nl->callback_addr)
			rc = process_addr(nl, nh);
		ఆపు;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	అప్రమేయ:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		ఆపు;

	}
	తిరిగి rc;
}

పూర్ణాంకం ul_nl_process(నిర్మాణం ul_nl_దత్తాంశం *nl, bool async, bool loop)
{
	అక్షరం buf[BUFSIZ];
	నిర్మాణం sockaddr_nl snl;
	నిర్మాణం nlmsghdr *nh;
	పూర్ణాంకం rc;
	uint32_t len;

	నిర్మాణం iovec iov = {
		.iov_base = buf,
		.iov_len = పరిమాణం(buf)
	};
	నిర్మాణం msghdr సందేశం = {
		.msg_name = &snl,
		.msg_namelen = పరిమాణం(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = ఖాళీ,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	అయితే (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &సందేశం, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		ఒకవేళ (rc < 0) {
			ఒకవేళ (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				తిరిగి UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			తిరిగి rc;
		}
		లేకపోతే
			len = rc;

		కొరకు (nh = (నిర్మాణం nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			ఒకవేళ (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				తిరిగి -1;
			}
			ఒకవేళ (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				తిరిగి UL_NL_పూర్తి;
			}

			rc = process_సందేశం(nl, nh);
			ఒకవేళ (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				ఒకవేళ (rc != UL_NL_SOFT_ERROR)
					తిరిగి rc;
			}
		}
		ఒకవేళ (!loop)
			తిరిగి 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

పూర్ణాంకం ul_nl_open(నిర్మాణం ul_nl_దత్తాంశం *nl, uint32_t nl_groups)
{
	నిర్మాణం sockaddr_nl addr = { 0, };
	పూర్ణాంకం sock;
	పూర్ణాంకం rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	ఒకవేళ (sock < 0)
		తిరిగి sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	ఒకవేళ ((rc = bind(sock, (నిర్మాణం sockaddr *)&addr, పరిమాణం(addr))) >= 0)
	{
		nl->fd = sock;
		తిరిగి 0;
	}
	లేకపోతే
	{
		close(sock);
		తిరిగి rc;
	}
}

పూర్ణాంకం ul_nl_close(నిర్మాణం ul_nl_దత్తాంశం *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	తిరిగి close(nl->fd);
}

నిర్మాణం ul_nl_addr *ul_nl_addr_dup(నిర్మాణం ul_nl_addr *addr)
{
	నిర్మాణం ul_nl_addr *newaddr;
	newaddr = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం ul_nl_addr));
	ఒకవేళ (!newaddr)
		వెళ్ళు error;
	జ్ఞాపకం_నకలు(newaddr, addr, పరిమాణం(నిర్మాణం ul_nl_addr));
	ఒకవేళ (addr->ifa_చిరునామా_len) {
		newaddr->ifa_చిరునామా = జ్ఞాపకం_ఇవ్వు(addr->ifa_చిరునామా_len);
		ఒకవేళ (!newaddr->ifa_చిరునామా)
			వెళ్ళు error;
		జ్ఞాపకం_నకలు(newaddr->ifa_చిరునామా, addr->ifa_చిరునామా,
		       addr->ifa_చిరునామా_len);
	}
	ఒకవేళ (addr->ifa_local_len) {
		newaddr->ifa_local = జ్ఞాపకం_ఇవ్వు(addr->ifa_local_len);
		ఒకవేళ (!newaddr->ifa_local)
			వెళ్ళు error;
		జ్ఞాపకం_నకలు(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	ఒకవేళ (addr->చిరునామా == addr->ifa_local)
		newaddr->చిరునామా = newaddr->ifa_local;
	లేకపోతే
		newaddr->చిరునామా = newaddr->ifa_చిరునామా;
	ఒకవేళ (!(newaddr->ifname = strdup(addr->ifname)))
		వెళ్ళు error;
	తిరిగి newaddr;
error:
	ul_nl_addr_free(newaddr);
	తిరిగి ఖాళీ;
}

శూన్య ul_nl_addr_free(నిర్మాణం ul_nl_addr *addr)
{
	ఒకవేళ (addr) {
		విడుదల(addr->ifa_చిరునామా);
		విడుదల(addr->ifa_local);
		విడుదల(addr->ifname);
		విడుదల(addr);
	}
}

స్థిరరాశి అక్షరం *ul_nl_addr_ntop(స్థిరరాశి నిర్మాణం ul_nl_addr *addr, పూర్ణాంకం addrid) {
	స్థిరరాశి శూన్య **ifa_addr = (స్థిరరాశి శూన్య **)((స్థిరరాశి అక్షరం *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	స్థిర అక్షరం addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	ఒకవేళ (addr->ifa_family == AF_INET)
		తిరిగి inet_ntop(AF_INET,
				 *ifa_addr, addr_str, పరిమాణం(addr_str));
	లేకపోతే {
	/* if (addr->ifa_family == AF_INET6) */
		ఒకవేళ (addr->ifa_scope == RT_SCOPE_LINK) {
			అక్షరం *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, పరిమాణం(addr_str));
			p = addr_str;
			అయితే (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			తిరిగి addr_str;
		} లేకపోతే
			తిరిగి inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, పరిమాణం(addr_str));
	}
}

#నిర్వచితమైతే TEST_కార్యక్రమం_NETLINK
#అన్తర్భావ <stdio.h>

స్థిర పూర్ణాంకం callback_addr(నిర్మాణం ul_nl_దత్తాంశం *nl)
{
	అక్షరం *str;

	రాయి("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	రాయి("  interface: %s\n", nl->addr.ifname);
	ఒకవేళ (nl->addr.ifa_family == AF_INET)
		రాయి("  IPv4 %s\n",
		       ul_nl_addr_ntop_చిరునామా(&(nl->addr)));
	లేకపోతే
	/* if (nl->addr.ifa_family == AF_INET) */
		రాయి("  IPv6 %s\n",
		       ul_nl_addr_ntop_చిరునామా(&(nl->addr)));
	ఎంపిక (nl->addr.ifa_scope) {
	స్థితి RT_SCOPE_UNIVERSE:	str = "global"; ఆపు;
	స్థితి RT_SCOPE_SITE:	str = "site"; ఆపు;
	స్థితి RT_SCOPE_LINK:	str = "link"; ఆపు;
	స్థితి RT_SCOPE_NOWHERE:	str = "nowhere"; ఆపు;
	అప్రమేయ:		str = "other"; ఆపు;
	}
	రాయి("  scope: %s\n", str);
	ఒకవేళ (nl->addr.ifa_valid != UINT32_MAX)
		రాయి("  valid: %u\n", nl->addr.ifa_valid);
	లేకపోతే
		రాయి("  valid: forever\n");
	తిరిగి 0;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc __attribute__((__unused__)),
	 అక్షరం *argv[] __attribute__((__unused__)))
{
	పూర్ణాంకం rc;
	నిర్మాణం ul_nl_దత్తాంశం nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	ఒకవేళ ((rc = ul_nl_open(&nl, 0)))
		తిరిగి 1;
	ఒకవేళ (ul_nl_request_dump(&nl, RTM_GETADDR))
		వెళ్ళు error;
	ఒకవేళ (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_పూర్తి)
		వెళ్ళు error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	ఒకవేళ ((rc = ul_nl_close(&nl)) < 0)
		వెళ్ళు error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	ఒకవేళ ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		వెళ్ళు error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	ఒకవేళ (rc == UL_NL_RETURN)
		rc = 0;
error:
	ఒకవేళ ((ul_nl_close(&nl)))
		rc = 1;
	తిరిగి rc;
}
#అయితే_ముగింపు /* TEST_PROGRAM_NETLINK */
