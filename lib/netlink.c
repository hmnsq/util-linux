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

#ضمن <sys/socket.h>
#ضمن <arpa/inet.h>
#ضمن <netinet/in.h>
#ضمن "strutils.h"
#ضمن "netlink.h"
#ضمن "debug.h"
#ضمن "nls.h"

/*
 * Debug stuff (based on include/debug.h)
 */
#وصف_ڪريو ULNETLINK_DEBUG_HELP	(1 << 0)
#وصف_ڪريو ULNETLINK_DEBUG_INIT	(1 << 1)
#وصف_ڪريو ULNETLINK_DEBUG_NLMSG	(1 << 2)
#وصف_ڪريو ULNETLINK_DEBUG_ADDR	(1 << 3)

ساكن UL_DEBUG_DEFINE_نقاب(netlink);
UL_DEBUG_DEFINE_MASKNAMES(netlink) =
{
	{ "all",   UL_DEBUG_ALL,		"complete netlink debugging" },
	{ "help",  ULNETLINK_DEBUG_HELP,	"this help" },
	{ "nlmsg", ULNETLINK_DEBUG_NLMSG,	"netlink message debugging" },
	{ "addr",  ULNETLINK_DEBUG_ADDR,	"netlink address processing" },

	{ عدم, 0 }
};

#وصف_ڪريو DBG(m, x)		__UL_DBG(netlink, ULNETLINK_DEBUG_, m, x)
#وصف_ڪريو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netlink, ULNETLINK_DEBUG_, m, h, x)
#وصف_ڪريو ON_DBG(m, x)		__UL_DBG_CALL(netlink, ULNETLINK_DEBUG_, m, x)

ساكن فراغ netlink_init_debug(فراغ)
{
	إذا (netlink_debug_نقاب)
		أرجع;

	__UL_INIT_DEBUG_FROM_ENV(netlink, ULNETLINK_DEBUG_, 0, ULNETLINK_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETLINK_DEBUG",
				UL_DEBUG_MASKNAMES(netlink)));
}

فراغ ul_nl_init(بنية ul_nl_معلومات *nl)
{
	netlink_init_debug();
	memset(nl, 0, حجم(بنية ul_nl_معلومات));
}

صحيح ul_nl_request_dump(بنية ul_nl_معلومات *nl, uint16_t nlmsg_type)
{
	بنية {
		بنية nlmsghdr nh;
		بنية rtgenmsg g;
	} req;

	memset(&req, 0, حجم(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(حجم(req.g));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.nh.nlmsg_type = nlmsg_type;
	req.g.rtgen_family = AF_NETLINK;
	nl->dumping = true;
	DBG_OBJ(NLMSG, nl, ul_debug("sending dump request"));
	إذا (send(nl->fd, &req, req.nh.nlmsg_len, 0) < 0)
		أرجع -1;
	أرجع 0;
}

#وصف_ڪريو DBG_CASE(x) حالة x: str = #x; اكسر
#وصف_ڪريو DBG_CASE_DEF8(x) افتراضي: snprintf(strx+2, 3, "%02hhx", x); str = strx; اكسر
ساكن فراغ dbg_addr(بنية ul_nl_معلومات *nl)
{
	حرف *str;
	حرف strx[5] = "0x";
	بدل (nl->addr.ifa_family) {
		DBG_CASE(AF_INET);
		DBG_CASE(AF_INET6);
		DBG_CASE_DEF8(nl->addr.ifa_family);
	}
	DBG(ADDR, ul_debug(" ifa_family: %s", str));
	بدل (nl->addr.ifa_scope) {
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
ساكن صحيح process_addr(بنية ul_nl_معلومات *nl, بنية nlmsghdr *nh)
{
	بنية ifaddrmsg *ifaddr;
	بنية rtattr *attr;
	صحيح len;
	ساكن حرف ifname[IF_NAMESIZE];
	bool has_local_پتو = false;

	DBG_OBJ(ADDR, nh, ul_debug("processing nlmsghdr"));
	memset(&(nl->addr), 0, حجم(بنية ul_nl_addr));

	/* Process ifaddrmsg. */
	ifaddr = NLMSG_DATA(nh);

	nl->addr.ifa_family = ifaddr->ifa_family;
	nl->addr.ifa_scope = ifaddr->ifa_scope;
	nl->addr.ifa_index = ifaddr->ifa_index;
	إذا ((if_indextoname(ifaddr->ifa_index, ifname)))
		nl->addr.ifname = ifname;
	وإلا
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
	لكل (attr = IFA_RTA(ifaddr); RTA_OK(attr, len);
	     attr = RTA_NEXT(attr, len)) {
		/* Process most common rta attributes */
		DBG_OBJ(ADDR, attr, ul_debug("processing rtattr"));
		بدل (attr->rta_type) {
		حالة IFA_ADDRESS:
			nl->addr.ifa_پتو = RTA_DATA(attr);
			nl->addr.ifa_پتو_len = RTA_PAYLOAD(attr);
			إذا (!has_local_پتو) {
				nl->addr.پتو = RTA_DATA(attr);
				nl->addr.پتو_len = RTA_PAYLOAD(attr);
			}
			DBG(ADDR,
			    ul_debug(" IFA_ADDRESS%s: %s",
				     (has_local_پتو ? "" :
				      " (setting address)"),
				     ul_nl_addr_ntop_ifa_پتو(&(nl->addr))));
		اكسر;
		حالة IFA_LOCAL:
			/* Point to Point interface listens has local address
			 * and listens there */
			has_local_پتو = true;
			nl->addr.ifa_local = nl->addr.پتو = RTA_DATA(attr);
			nl->addr.ifa_local_len =
				nl->addr.پتو_len = RTA_PAYLOAD(attr);
			DBG(ADDR,
			    ul_debug(" IFA_LOCAL (setting address): %s",
				     ul_nl_addr_ntop_ifa_local(&(nl->addr))));
		اكسر;
		حالة IFA_CACHEINFO:
		{
			بنية ifa_cacheinfo *ci =
				(بنية ifa_cacheinfo *)RTA_DATA(attr);
			nl->addr.ifa_valid = ci->ifa_valid;
			DBG(ADDR,
			    ul_debug(" IFA_CACHEINFO: ifa_prefered = %u,"
				     "ifa_valid = %u",
				     nl->addr.ifa_prefered,
				     nl->addr.ifa_valid));
		}
		اكسر;
		حالة IFA_FLAGS:
			nl->addr.ifa_flags = *(uint32_t *)(RTA_DATA(attr));
			DBG(ADDR, ul_debug(" IFA_FLAGS: 0x%08x",
					  nl->addr.ifa_flags));
		اكسر;
		افتراضي:
			DBG(ADDR, ul_debug(" rta_type = 0x%04x",
					  attr->rta_type));
		اكسر;
		}
	}
	DBG_OBJ(NLMSG, nl, ul_debug("callback %p", nl->callback_addr));
	أرجع (*(nl->callback_addr))(nl);
}

ساكن صحيح process_نياپو(بنية ul_nl_معلومات *nl, بنية nlmsghdr *nh)
{
	صحيح rc = 0;

	nl->rtm_event = UL_NL_RTM_DEL;
	بدل (nh->nlmsg_type) {
	حالة RTM_NEWADDR:
		nl->rtm_event = UL_NL_RTM_NEW;
		/* fallthrough */
	حالة RTM_DELADDR:
	/* If callback_addr is not set, skip process_addr */
		DBG_OBJ(NLMSG, nl, ul_debug("%s",
				       (UL_NL_IS_RTM_DEL(nl) ?
					"RTM_DELADDR" : "RTM_NEWADDR")));
		إذا (nl->callback_addr)
			rc = process_addr(nl, nh);
		اكسر;
	/* More can be implemented in future (RTM_NEWLINK, RTM_DELLINK etc.). */
	افتراضي:
		DBG_OBJ(NLMSG, nl, ul_debug("nlmsg_type = %hu", nh->nlmsg_type));
		اكسر;

	}
	أرجع rc;
}

صحيح ul_nl_process(بنية ul_nl_معلومات *nl, bool async, bool loop)
{
	حرف buf[BUFSIZ];
	بنية sockaddr_nl snl;
	بنية nlmsghdr *nh;
	صحيح rc;
	uint32_t len;

	بنية iovec iov = {
		.iov_base = buf,
		.iov_len = حجم(buf)
	};
	بنية msghdr نياپو = {
		.msg_name = &snl,
		.msg_namelen = حجم(snl),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = عدم,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	طالما (1) {
		DBG_OBJ(NLMSG, nl, ul_debug("waiting for message"));
		rc = recvmsg(nl->fd, &نياپو, (loop ? 0 :
					    (async ? MSG_DONTWAIT : 0)));
		DBG_OBJ(NLMSG, nl, ul_debug("got message"));
		إذا (rc < 0) {
			إذا (errno == EWOULDBLOCK || errno == EAGAIN) {
				DBG_OBJ(NLMSG, nl, ul_debug("no data"));
				أرجع UL_NL_WOULDBLOCK;
			}
			/* Failure, just stop listening for changes */
			nl->dumping = false;
			DBG_OBJ(NLMSG, nl, ul_debug("error"));
			أرجع rc;
		}
		وإلا
			len = rc;

		لكل (nh = (بنية nlmsghdr *)buf;
		     NLMSG_OK(nh, len);
		     nh = NLMSG_NEXT(nh, len)) {

			إذا (nh->nlmsg_type == NLMSG_ERROR) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_ERROR"));
				nl->dumping = false;
				أرجع -1;
			}
			إذا (nh->nlmsg_type == NLMSG_DONE) {
				DBG_OBJ(NLMSG, nl, ul_debug("NLMSG_DONE"));
				nl->dumping = false;
				أرجع UL_NL_پورو;
			}

			rc = process_نياپو(nl, nh);
			إذا (rc)
			{
				DBG_OBJ(NLMSG, nl,
				    ul_debug("process_msg() returned %d", rc));
				إذا (rc != UL_NL_SOFT_ERROR)
					أرجع rc;
			}
		}
		إذا (!loop)
			أرجع 0;
		DBG_OBJ(NLMSG, nl, ul_debug("looping until NLMSG_DONE"));
	}
}

صحيح ul_nl_open(بنية ul_nl_معلومات *nl, uint32_t nl_groups)
{
	بنية sockaddr_nl addr = { 0, };
	صحيح sock;
	صحيح rc;

	DBG_OBJ(NLMSG, nl, ul_debug("opening socket"));
	sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	إذا (sock < 0)
		أرجع sock;
	addr.nl_family = AF_NETLINK;
	addr.nl_pid = getpid();
	addr.nl_groups = nl_groups;
	إذا ((rc = bind(sock, (بنية sockaddr *)&addr, حجم(addr))) >= 0)
	{
		nl->fd = sock;
		أرجع 0;
	}
	وإلا
	{
		close(sock);
		أرجع rc;
	}
}

صحيح ul_nl_close(بنية ul_nl_معلومات *nl)
{
	DBG_OBJ(NLMSG, nl, ul_debug("closing socket"));
	أرجع close(nl->fd);
}

بنية ul_nl_addr *ul_nl_addr_dup(بنية ul_nl_addr *addr)
{
	بنية ul_nl_addr *newaddr;
	newaddr = صفر_يادگيري_ڏيو(1, حجم(بنية ul_nl_addr));
	إذا (!newaddr)
		اذهب error;
	يادگيري_نقل(newaddr, addr, حجم(بنية ul_nl_addr));
	إذا (addr->ifa_پتو_len) {
		newaddr->ifa_پتو = يادگيري_ڏيو(addr->ifa_پتو_len);
		إذا (!newaddr->ifa_پتو)
			اذهب error;
		يادگيري_نقل(newaddr->ifa_پتو, addr->ifa_پتو,
		       addr->ifa_پتو_len);
	}
	إذا (addr->ifa_local_len) {
		newaddr->ifa_local = يادگيري_ڏيو(addr->ifa_local_len);
		إذا (!newaddr->ifa_local)
			اذهب error;
		يادگيري_نقل(newaddr->ifa_local, addr->ifa_local,
		       addr->ifa_local_len);
	}
	إذا (addr->پتو == addr->ifa_local)
		newaddr->پتو = newaddr->ifa_local;
	وإلا
		newaddr->پتو = newaddr->ifa_پتو;
	إذا (!(newaddr->ifname = strdup(addr->ifname)))
		اذهب error;
	أرجع newaddr;
error:
	ul_nl_addr_free(newaddr);
	أرجع عدم;
}

فراغ ul_nl_addr_free(بنية ul_nl_addr *addr)
{
	إذا (addr) {
		حرر(addr->ifa_پتو);
		حرر(addr->ifa_local);
		حرر(addr->ifname);
		حرر(addr);
	}
}

ثابت حرف *ul_nl_addr_ntop(ثابت بنية ul_nl_addr *addr, صحيح addrid) {
	ثابت فراغ **ifa_addr = (ثابت فراغ **)((ثابت حرف *)addr + addrid);
	/* (INET6_ADDRSTRLEN-1) + (IF_NAMESIZE-1) + strlen("%") + 1 */
	ساكن حرف addr_str[INET6_ADDRSTRLEN+IF_NAMESIZE];

	إذا (addr->ifa_family == AF_INET)
		أرجع inet_ntop(AF_INET,
				 *ifa_addr, addr_str, حجم(addr_str));
	وإلا {
	/* if (addr->ifa_family == AF_INET6) */
		إذا (addr->ifa_scope == RT_SCOPE_LINK) {
			حرف *p;

			inet_ntop(AF_INET6,
				  *ifa_addr, addr_str, حجم(addr_str));
			p = addr_str;
			طالما (*p) p++;
			*p++ = '%';
			xstrncpy(p, addr->ifname, IF_NAMESIZE);
			أرجع addr_str;
		} وإلا
			أرجع inet_ntop(AF_INET6,
					 *ifa_addr, addr_str, حجم(addr_str));
	}
}

#جيڪڏهن_وصف_ٿيل TEST_پروگرام_NETLINK
#ضمن <stdio.h>

ساكن صحيح callback_addr(بنية ul_nl_معلومات *nl)
{
	حرف *str;

	لکو("%s address:\n", ((nl->rtm_event ? "Add" : "Delete")));
	لکو("  interface: %s\n", nl->addr.ifname);
	إذا (nl->addr.ifa_family == AF_INET)
		لکو("  IPv4 %s\n",
		       ul_nl_addr_ntop_پتو(&(nl->addr)));
	وإلا
	/* if (nl->addr.ifa_family == AF_INET) */
		لکو("  IPv6 %s\n",
		       ul_nl_addr_ntop_پتو(&(nl->addr)));
	بدل (nl->addr.ifa_scope) {
	حالة RT_SCOPE_UNIVERSE:	str = "global"; اكسر;
	حالة RT_SCOPE_SITE:	str = "site"; اكسر;
	حالة RT_SCOPE_LINK:	str = "link"; اكسر;
	حالة RT_SCOPE_NOWHERE:	str = "nowhere"; اكسر;
	افتراضي:		str = "other"; اكسر;
	}
	لکو("  scope: %s\n", str);
	إذا (nl->addr.ifa_valid != UINT32_MAX)
		لکو("  valid: %u\n", nl->addr.ifa_valid);
	وإلا
		لکو("  valid: forever\n");
	أرجع 0;
}

صحيح مکيه(صحيح argc __attribute__((__unused__)),
	 حرف *argv[] __attribute__((__unused__)))
{
	صحيح rc;
	بنية ul_nl_معلومات nl;

	/* Prepare netlink. */
	ul_nl_init(&nl);
	nl.callback_addr = callback_addr;

	/* Dump addresses */
	إذا ((rc = ul_nl_open(&nl, 0)))
		أرجع 1;
	إذا (ul_nl_request_dump(&nl, RTM_GETADDR))
		اذهب error;
	إذا (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_پورو)
		اذهب error;
	puts("RTM_GETADDR dump finished.");

	/* Close and later open. See note in the ul_nl_open() docs. */
	إذا ((rc = ul_nl_close(&nl)) < 0)
		اذهب error;

	/* Monitor further changes */
	puts("Going to monitor mode.");
	إذا ((rc = ul_nl_open(&nl, RTMGRP_LINK |
			     RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR)))
		اذهب error;
	/* In this example UL_NL_ABORT never appears, as callback does
	 * not use it. */
	rc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	إذا (rc == UL_NL_RETURN)
		rc = 0;
error:
	إذا ((ul_nl_close(&nl)))
		rc = 1;
	أرجع rc;
}
#جيڪڏهن_پورو /* TEST_PROGRAM_NETLINK */
