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
#تعریف_کرو ULNETLINK_DEBUG_HELP	(1 << 0)
#تعریف_کرو ULNETLINK_DEBUG_INIT	(1 << 1)
#تعریف_کرو ULNETLINK_DEBUG_NLMSG	(1 << 2)
#تعریف_کرو ULNETLINK_DEBUG_ADDR	(1 << 3)

ساکن UL_DEBUG_DEFINE_نقاب(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ تہی, 0 }
};

#تعریف_کرو DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#تعریف_کرو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#تعریف_کرو ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

ساکن خالی netlink_init_debug(خالی)
{
	اگر (netlink_debug_نقاب)
		واپس;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

خالی ul_nl_init(ساخت ul_nl_ڈیٹا *nl)
{
	netlink_init_debug();
	memset(nl, 0, حجم(ساخت ul_nl_ڈیٹا));
}

صحیح ul_nl_request_dump(ساخت ul_nl_ڈیٹا *nl, uint16_t nlmsg_type)
{
	ساخت {
		ساخت nlmsghdr nh;
		ساخت rtgenmsg g;
	} req;

	memset(&req, 0, حجم(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(حجم(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	اگر (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		واپس -1;
	واپس 0;
}

#تعریف_کرو DBG_CASE(x) حالت x: str = #x; توڑو
#تعریف_کرو DBG_CASE_DEF8(x) طےشدہ: snprintf(strx+2, 3, "%02hhx", x); str = strx; توڑو
ساکن خالی dbg_addr(ساخت ul_nl_ڈیٹا *nl)
{
	حرف *str;
	حرف strx[5] = "0x";
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
ساکن صحیح process_addr(ساخت ul_nl_ڈیٹا *nl, ساخت nlmsghdr *nh)
{
	ساخت ifaddrmsg *ifaddr;
	ساخت rtattr *attr;
	صحیح len;
	ساکن حرف ifname[IF_NAMESIZE];
	bool has_local_پتہ = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, حجم(ساخت ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	اگر ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	ورنہ
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
	len = nh->nlmsg_len - NLMSG_LENGTH(حجم(*ifaddr));
	برائے (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		انتخاب (attr->rta_type) {
		حالت IFA_ADDRESS:
			nl->addr.ifa_پتہ = RTA_DATA(attr);
			nl->addr.ifa_پتہ_len = RTA_PAYLOAD(attr);
			اگر (!has_local_پتہ) {
				nl->addr.پتہ = RTA_DATA(attr);
				nl->addr.پتہ_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_پتہ ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_پتہ(&(nl->addr))));
		توڑو;
		حالت IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_پتہ = true;
			nl->addr.ifa_local = nl->addr.پتہ = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.پتہ_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		توڑو;
		حالت IFA_CACHEINFO:
		{
			ساخت ifa_cacheinfo *ci =
				(ساخت ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		توڑو;
		حالت IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		توڑو;
		طےشدہ:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		توڑو;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	واپس (*(nl->callback_addr))(nl);
}

ساکن صحیح process_پیغام(ساخت ul_nl_ڈیٹا *nl, ساخت nlmsghdr *nh)
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
		توڑو;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	طےشدہ:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		توڑو;

	}
	واپس rc;
}

صحیح ul_nl_process(ساخت ul_nl_ڈیٹا *nl, bool async, bool loop)
{
	حرف buf[BUFSIZ];
	ساخت sockaddr_nl snl;
	ساخت nlmsghdr *nh;
	صحیح rc;
	uint32_t len;

	ساخت iovec iov = {
		.iov_base = buf,
		.iov_len = حجم(buf)
	};
	ساخت msghdr پیغام = {
		.msg_name = &snl,
		.msg_namelen = حجم(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = تہی,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	جب (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &پیغام, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		اگر (rc < 0) {
			اگر (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				واپس UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			واپس rc;
		}
		ورنہ
			len = rc;

		برائے (nh = (ساخت nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			اگر (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				واپس -1;
			}
			اگر (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				واپس UL_NL_مکمل;
			}

			rc = process_پیغام(nl, nh);
			اگر (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				اگر (rc != UL_NL_SOFT_ERROR)
					واپس rc;
			}
		}
		اگر (!loop)
			واپس 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

صحیح ul_nl_open(ساخت ul_nl_ڈیٹا *nl, uint32_t nl_groups)
{
	ساخت sockaddr_nl addr = { 0, };
	صحیح sock;
	صحیح rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	اگر (sock < 0)
		واپس sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	اگر ((rc = bind(sock, (ساخت sockaddr *)&addr, حجم(addr))) >= 0)
	{
		nl->fd = sock;
		واپس 0;
	}
	ورنہ
	{
		close(sock);
		واپس rc;
	}
}

صحیح ul_nl_close(ساخت ul_nl_ڈیٹا *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	واپس close(nl->fd);
}

ساخت ul_nl_addr *ul_nl_addr_dup(ساخت ul_nl_addr *addr)
{
	ساخت ul_nl_addr *newaddr;
	newaddr = صفر_حافظہ_دو(1, حجم(ساخت ul_nl_addr));
	اگر (!newaddr)
		جاؤ error;
	حافظہ_نقل(newaddr, addr, حجم(ساخت ul_nl_addr));
	اگر (addr->ifa_پتہ_len) {
		newaddr->ifa_پتہ = حافظہ_دو(addr->ifa_پتہ_len);
		اگر (!newaddr->ifa_پتہ)
			جاؤ error;
		حافظہ_نقل(newaddr->ifa_پتہ, addr->ifa_پتہ,
		       addr->ifa_پتہ_len);
	}
	اگر (addr->ifa_local_len) {
		newaddr->ifa_local = حافظہ_دو(addr->ifa_local_len);
		اگر (!newaddr->ifa_local)
			جاؤ error;
		حافظہ_نقل(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	اگر (addr->پتہ == addr->ifa_local)
		newaddr->پتہ = newaddr->ifa_local;
	ورنہ
		newaddr->پتہ = newaddr->ifa_پتہ;
	اگر (!(newaddr->ifname = strdup(addr->ifname)))
		جاؤ error;
	واپس newaddr;
error:
	ul_nl_addr_free(newaddr);
	واپس تہی;
}

خالی ul_nl_addr_free(ساخت ul_nl_addr *addr)
{
	اگر (addr) {
		آزاد(addr->ifa_پتہ);
		آزاد(addr->ifa_local);
		آزاد(addr->ifname);
		آزاد(addr);
	}
}

ثابت حرف *ul_nl_addr_ntop(ثابت ساخت ul_nl_addr *addr, صحیح addrid) {
	ثابت خالی **ifa_addr = (ثابت خالی **)((ثابت حرف *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	ساکن حرف addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	اگر (addr->ifa_family == AF_INET)
		واپس inet_ntop(AF_INET,
				 *ifa_addr, addr_str, حجم(addr_str));
	ورنہ {
	/* if (addr->ifa_family == AF_INET6) */
		اگر (addr->ifa_scope == RT_SCOPE_LINK) {
			حرف *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, حجم(addr_str));
			p = addr_str;
			جب (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			واپس addr_str;
		} ورنہ
			واپس inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, حجم(addr_str));
	}
}

#اگر_تعریف_شدہ TEST_پروگرام_NETLINK
#شامل <stdio.h>

ساکن صحیح callback_addr(ساخت ul_nl_ڈیٹا *nl)
{
	حرف *str;

	لکھو("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	لکھو("  interface: %s\n", nl->addr.ifname);
	اگر (nl->addr.ifa_family == AF_INET)
		لکھو("  IPv4 %s\n",
		       ul_nl_addr_ntop_پتہ(&(nl->addr)));
	ورنہ
	/* if (nl->addr.ifa_family == AF_INET) */
		لکھو("  IPv6 %s\n",
		       ul_nl_addr_ntop_پتہ(&(nl->addr)));
	انتخاب (nl->addr.ifa_scope) {
	حالت RT_SCOPE_UNIVERSE:	str = "global"; توڑو;
	حالت RT_SCOPE_SITE:	str = "site"; توڑو;
	حالت RT_SCOPE_LINK:	str = "link"; توڑو;
	حالت RT_SCOPE_NOWHERE:	str = "nowhere"; توڑو;
	طےشدہ:		str = "other"; توڑو;
	}
	لکھو("  scope: %s\n", str);
	اگر (nl->addr.ifa_valid != UINT32_MAX)
		لکھو("  valid: %u\n", nl->addr.ifa_valid);
	ورنہ
		لکھو("  valid: forever\n");
	واپس 0;
}

صحیح مین(صحیح argc __attribute__((__unused__)),
	 حرف *argv[] __attribute__((__unused__)))
{
	صحیح rc;
	ساخت ul_nl_ڈیٹا nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	اگر ((rc = ul_nl_open(&nl, 0)))
		واپس 1;
	اگر (ul_nl_request_dump(&nl, RTM_GETADDR))
		جاؤ error;
	اگر (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_مکمل)
		جاؤ error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	اگر ((rc = ul_nl_close(&nl)) < 0)
		جاؤ error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	اگر ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		جاؤ error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	اگر (rc == UL_NL_RETURN)
		rc = 0;
error:
	اگر ((ul_nl_close(&nl)))
		rc = 1;
	واپس rc;
}
#اگر_ختم /* TEST_PROGRAM_NETLINK */
