/*
 * Netlink address quality rating list builder
 *
 * Copyright (C) 2025 Stanislav Brabec <sbrabec@suse.com>
 *
 * This program is freely distributable.
 *
 * This set of netlink callbacks kernel and creates
 * and/or maintains a linked list of requested type. Using callback functions
 * and custom data, it could be used for arbitrary purpose.
 *
 */

#ضمن <net/if.h>
#ضمن <netinet/in.h>
#ضمن <linux/rtnetlink.h>
#ضمن <linux/if_addr.h>
#ضمن "netaddrq.h"
#ضمن "list.h"
#ضمن "debug.h"

/* Maximal number of interfaces. The algorithm has a quadratic complexity,
 * don't overflood it. */
ثابت صحيح max_ifaces = 12;

/*
 * Debug stuff (based on include/debug.h)
 */
#تعریف_کن ULNETADDRQ_DEBUG_HELP	(1 << 0)
#تعریف_کن ULNETADDRQ_DEBUG_INIT	(1 << 1)
#تعریف_کن ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#تعریف_کن ULNETADDRQ_DEBUG_LIST	(1 << 3)
#تعریف_کن ULNETADDRQ_DEBUG_بهترین	(1 << 4)

ساكن UL_DEBUG_DEFINE_ماسک(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_بهترین,	"searching best address" },

	{ عدم, 0 }
};

#تعریف_کن DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#تعریف_کن DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#تعریف_کن ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

ساكن فراغ netaddrq_init_debug(فراغ)
{
	إذا (netaddrq_debug_ماسک)
		أرجع;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

ساكن مضمن تعداد ul_netaddrq_ip_rating
evaluate_ip_quality(بنية ul_nl_addr *addr) {
	تعداد ul_netaddrq_ip_rating quality;

	بدل (addr->ifa_scope) {
	حالة RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		اكسر;
	حالة RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		اكسر;
	حالة RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		اكسر;
	افتراضي:
		quality = ULNETLINK_RATING_BAD;
		اكسر;
	}
	إذا (addr->ifa_flags & IFA_F_TEMPORARY) {
		إذا (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	أرجع quality;
}

#تعریف_کن DBG_CASE(x) حالة x: str = #x; اكسر
#تعریف_کن DBG_CASE_DEF8(x) افتراضي: snprintf(strx+2, 3, "%02hhx", x); str = strx; اكسر
ساكن حرف *ip_rating_as_string(تعداد ul_netaddrq_ip_rating q)
{
	حرف *str;
	ساكن حرف strx[5] = "0x";
	بدل (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	أرجع str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
ساكن صحيح callback_addrq(بنية ul_nl_داده *nl)
{
	بنية ul_netaddrq_داده *addrq = UL_NETADDRQ_داده(nl);
	بنية list_سر *li, *ipq_list;
	بنية ul_netaddrq_iface *ifaceq = عدم;
	بنية ul_netaddrq_ip *ipq = عدم;
	صحيح rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_آدرس(&(nl->addr)),
			      nl->addr.ifname));
	إذا (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		إذا ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		بنية ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_مدخل(li, بنية ul_netaddrq_iface, مدخل);
		إذا (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			اكسر;
		}
		addrq->nifaces++;
	}

	إذا (ifaceq == عدم) {
		إذا (nl->rtm_event) {
			إذا (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->سرریز = true;
				أرجع UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			إذا (!(ifaceq = حافظه_بده(حجم(بنية ul_netaddrq_iface))))
				أرجع -ENOMEM;
			INIT_LIST_سر(&(ifaceq->ip_quality_list_4));
			INIT_LIST_سر(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			إذا (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				حرر(ifaceq);
				أرجع -ENOMEM;
			}
			list_جمع_کن_دم(&(ifaceq->مدخل), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} وإلا {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			أرجع UL_NL_SOFT_ERROR;
		}
	}
	إذا (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} وإلا {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		بنية ul_netaddrq_ip *ipqq;

		ipqq = list_مدخل(li, بنية ul_netaddrq_ip, مدخل);
		إذا (ipqq->addr->آدرس_len == nl->addr.آدرس_len)
			إذا (!memcmp(ipqq->addr->آدرس, nl->addr.آدرس,
				   nl->addr.آدرس_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				اكسر;
			}
	}
	إذا (ipq == عدم) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	إذا (UL_NL_IS_RTM_NEW(nl)) {
		بنية ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		إذا (!addr) {
			rc = -ENOMEM;
			اذهب error;
		}
		إذا (ipq == عدم) {
			إذا (!(ipq = حافظه_بده(حجم(بنية ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				اذهب error;
			}
			ipq->addr = addr;
			list_جمع_کن_دم(&(ipq->مدخل), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} وإلا {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_آدرس(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} وإلا {
		/* UL_NL_RTM_DEL */
		إذا (ipq == عدم)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			أرجع UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->مدخل));
		ul_nl_addr_free(ipq->addr);
		حرر(ipq);
	error:
		إذا (list_خالی(&(ifaceq->ip_quality_list_4)) &&
		    list_خالی(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->مدخل));
			addrq->nifaces--;
			حرر(ifaceq->ifname);
			حرر(ifaceq);
		}
	}
	إذا (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		إذا ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	أرجع rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
صحيح ul_netaddrq_init(بنية ul_nl_داده *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, فراغ *داده)
{
	بنية ul_netaddrq_داده *addrq;

	netaddrq_init_debug();
	إذا (!(nl->داده_addr = حافظه_صفر_بده(1, حجم(بنية ul_netaddrq_داده))))
		أرجع -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_داده(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_داده = داده;
	INIT_LIST_سر(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	أرجع 0;
}

تعداد ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(بنية list_سر *ipq_list,
			   بنية ul_netaddrq_ip *(*بهترین)[__ULNETLINK_RATING_MAX])
{
	بنية list_سر *li;
	بنية ul_netaddrq_ip *ipq;
	تعداد ul_netaddrq_ip_rating آستانه;

	آستانه = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_مدخل(li, بنية ul_netaddrq_ip, مدخل);

		إذا (!(*بهترین)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*بهترین)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(بهترین_2, (*بهترین),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_آدرس(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*بهترین)[ipq->quality] = ipq;
		}

		إذا (ipq->quality < آستانه)
		{
			آستانه = ipq->quality;
			DBG(بهترین_2,
			    ul_debug("threshold %s", ip_rating_as_string(آستانه)));

		}
	}
	أرجع آستانه;
}

تعداد ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(بنية ul_nl_داده *nl,
		     بنية ul_netaddrq_iface **بهترین_ifaceq,
		     بنية ul_netaddrq_ip *(*بهترین)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	بنية ul_netaddrq_داده *addrq = UL_NETADDRQ_داده(nl);
	بنية list_سر *li;
	بنية ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	تعداد ul_netaddrq_ip_rating آستانه;

	إذا (ifa_family == AF_INET) {
		ipqo = offsetof(بنية ul_netaddrq_iface, ip_quality_list_4);
	} وإلا {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(بنية ul_netaddrq_iface, ip_quality_list_6);
	}

	آستانه = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		بنية list_سر *ipq_list;
		تعداد ul_netaddrq_ip_rating t;

		ifaceq = list_مدخل(li, بنية ul_netaddrq_iface, مدخل);

		ipq_list = (بنية list_سر*)((حرف*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, بهترین);
		إذا (t < آستانه)
		{
			DBG_OBJ(بهترین_2, *بهترین,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*بهترین_ifaceq = ifaceq;
			آستانه = t;
		}
	}
	أرجع آستانه;
}

ثابت حرف *ul_netaddrq_get_بهترین_ipp(بنية ul_nl_داده *nl,
				     uint8_t ifa_family,
				     تعداد ul_netaddrq_ip_rating *آستانه,
				     بنية ul_netaddrq_iface **بهترین_ifaceq)
{
	بنية ul_netaddrq_ip *بهترین[__ULNETLINK_RATING_MAX];

	memset(بهترین, 0, حجم(بهترین));
	*آستانه = ul_netaddrq_bestaddr(nl, بهترین_ifaceq, &بهترین, ifa_family);
	إذا (*آستانه != __ULNETLINK_RATING_MAX)
		أرجع ul_nl_addr_ntop_آدرس(بهترین[*آستانه]->addr);
	أرجع عدم;
}

بنية ul_netaddrq_iface *ul_netaddrq_iface_by_name(ثابت بنية ul_nl_داده *nl,
						    ثابت حرف *ifname)
{
	بنية list_سر *li;
	بنية ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_مدخل(li, بنية ul_netaddrq_iface, مدخل);

		إذا (!strcmp(ifaceq->ifname, ifname))
			أرجع ifaceq;
	}
	أرجع عدم;
}

#اگر_تعریف_شده TEST_برنامه_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
تعداد addrq_print_نما {
	ADDRQ_نما_BESTOFALL,	/* Best address of all interfaces */
	ADDRQ_نما_بهترین,	/* Best address per interface */
	ADDRQ_نما_GOOD,	/* All global or site addresses, if none, the
				 * longest living temporary, if none, link */
	ADDRQ_نما_ALL		/* All available addresses */
};

/* In our example addrq->callback_data is a simple FILE *. In more complex
 * programs it could be a pointer to an arbitrary struct */
#تعریف_کن netout (FILE *)(UL_NETADDRQ_داده(nl)->callback_داده)

/* This example uses separate threshold for IPv4 and IPv6, so the best IPv4 and
 * best IPv6 addresses are printed. */
ساكن فراغ dump_iface_بهترین(بنية ul_nl_داده *nl,
			    بنية ul_netaddrq_iface *ifaceq)
{
	بنية ul_netaddrq_ip *بهترین[__ULNETLINK_RATING_MAX];
	تعداد ul_netaddrq_ip_rating آستانه;
	bool اولی = true;

	memset(بهترین, 0, حجم(بهترین));
	آستانه = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &بهترین);
	إذا (آستانه != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (اولی ? "best address" : " "),
		       ul_nl_addr_ntop_آدرس(بهترین[آستانه]->addr));
		اولی = false;
	}
	memset(بهترین, 0, حجم(بهترین));
	آستانه = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &بهترین);
	إذا (آستانه != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (اولی ? "best address" : " "),
		       ul_nl_addr_ntop_آدرس(بهترین[آستانه]->addr));
		اولی = false;
	}
	إذا (!اولی)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
ساكن فراغ dump_iface_good(بنية ul_nl_داده *nl,
			    بنية ul_netaddrq_iface *ifaceq)
{
	بنية ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	بنية ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	بنية list_سر *li;
	تعداد ul_netaddrq_ip_rating آستانه = __ULNETLINK_RATING_MAX - 1;
	تعداد ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool اولی = true;

	memset(best4, 0, حجم(best4));
	آستانه = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, حجم(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	إذا (fthreshold < آستانه)
		آستانه = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		بنية ul_netaddrq_ip *ipq;

		ipq = list_مدخل(li, بنية ul_netaddrq_ip, مدخل);
		إذا (آستانه <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= آستانه ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[آستانه])
		{
			إذا (اولی)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				اولی = false;
			}
			وإلا
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			إذا (آستانه == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_آدرس(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				اذهب موقت_cont4;
			}
			وإلا
				fputs(ul_nl_addr_ntop_آدرس(ipq->addr),
				      netout);
		}
	موقت_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		بنية ul_netaddrq_ip *ipq;

		ipq = list_مدخل(li, بنية ul_netaddrq_ip, مدخل);
		إذا (آستانه <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= آستانه ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[آستانه])
		{
			إذا (اولی)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				اولی = false;
			}
			وإلا
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			إذا (آستانه == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_آدرس(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				اذهب موقت_cont6;
			}
			وإلا
				fputs(ul_nl_addr_ntop_آدرس(ipq->addr),
				      netout);
		}
	موقت_cont6:;
	}
	إذا (!اولی)
		fputs("\n", netout);
}

ساكن فراغ dump_iface_all(بنية ul_nl_داده *nl,
			   بنية ul_netaddrq_iface *ifaceq)
{
	بنية list_سر *li;
	بنية ul_netaddrq_ip *ipq;
	bool اولی = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_مدخل(li, بنية ul_netaddrq_ip, مدخل);
		إذا (اولی)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			اولی = false;
		}
		وإلا
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_آدرس(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_مدخل(li, بنية ul_netaddrq_ip, مدخل);
		إذا (اولی)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			اولی = false;
		}
		وإلا
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_آدرس(ipq->addr), netout);
	}
	إذا (!اولی)
		fputs("\n", netout);
}

ساكن فراغ dump_addrq(بنية ul_nl_داده *nl, تعداد addrq_print_نما c)
{
	بنية list_سر *li;
	بنية ul_netaddrq_iface *ifaceq;
	تعداد ul_netaddrq_ip_rating آستانه;

	بدل(c)
	{
	حالة ADDRQ_نما_BESTOFALL:
	{
		بنية ul_netaddrq_iface *بهترین_ifaceq;
		ثابت حرف *بهترین_ipp;

		بهترین_ipp = ul_netaddrq_get_بهترین_ipp(nl, AF_INET,
						    &آستانه, &بهترین_ifaceq);
		إذا (بهترین_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				بهترین_ipp, بهترین_ifaceq->ifname);
		بهترین_ipp = ul_netaddrq_get_بهترین_ipp(nl, AF_INET6,
						    &آستانه, &بهترین_ifaceq);
		إذا (بهترین_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				بهترین_ipp, بهترین_ifaceq->ifname);
	}
	اكسر;
	حالة ADDRQ_نما_بهترین:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_مدخل(li, بنية ul_netaddrq_iface, مدخل);

			dump_iface_بهترین(nl, ifaceq);
		}
	}
	اكسر;
	حالة ADDRQ_نما_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_مدخل(li, بنية ul_netaddrq_iface, مدخل);

			dump_iface_good(nl, ifaceq);
		}
	}
	اكسر;
	حالة ADDRQ_نما_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_مدخل(li, بنية ul_netaddrq_iface, مدخل);

			dump_iface_all(nl,ifaceq);
		}
	اكسر;
	}
}

ساكن صحيح callback_post(بنية ul_nl_داده *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	إذا (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		إذا (!(UL_NETADDRQ_داده(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_داده(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			أرجع 0;
		}
		UL_NETADDRQ_داده(nl)->ifaces_change_4 = false;
		UL_NETADDRQ_داده(nl)->ifaces_change_6 = false;
		fputs("\n\nNetwork change detected:\n", netout);
		fputs("\nbest address:\n", netout);
		dump_addrq(nl, ADDRQ_نما_BESTOFALL);

		fputs("\nbest addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_نما_بهترین);

		fputs("\ngood addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_نما_GOOD);

		fputs("\nall addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_نما_ALL);
	}
	أرجع 0;
}

صحيح اصلی(صحيح argc __attribute__((__unused__)), حرف *argv[] __attribute__((__unused__)))
{
	صحيح rc = 1;
	صحيح ulrc;
	بنية ul_nl_داده nl;
	FILE *بیرون = stdout;
	بنية ul_netaddrq_iface *ifaceq;
	ثابت حرف *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	إذا ((ul_netaddrq_init(&nl, عدم, callback_post, (فراغ *)بیرون)))
		أرجع -1;

	/* Dump addresses */
	إذا (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		أرجع -1;
	إذا (ul_nl_request_dump(&nl, RTM_GETADDR))
		اذهب error;
	إذا (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_انجامشده)
		اذهب error;
	fputs("RTM_GETADDR dump finished.", بیرون);

	/* Example of several types of dump */
	fputs("\nbest address:\n", بیرون);
	dump_addrq(&nl, ADDRQ_نما_BESTOFALL);

	fputs("\nbest addresses dump:\n", بیرون);
	dump_addrq(&nl, ADDRQ_نما_بهترین);

	fputs("\ngood addresses dump:\n", بیرون);
	dump_addrq(&nl, ADDRQ_نما_GOOD);

	fputs("\nall addresses dump:\n", بیرون);
	dump_addrq(&nl, ADDRQ_نما_ALL);

	fputs("\naddresses for interface ", بیرون);
	إذا ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	وإلا
		fprintf(بیرون, "%s not found.", ifname);

	/* Monitor further changes */
	fputs("\nGoing to monitor mode.\n", بیرون);

	/* In this example UL_NL_RETURN never appears, as callback does
	 * not use it. */

	/* There are two different ways to create the loop:
	 *
	 * 1) Use UL_NL_LOOP and process the result in addrq->callback_post
	 *    (optionally excluding events with nl->dumping set. (We can
	 *    process dump output in the callback as well, but in many cases,
	 *    single run after finishing the dump is a better solution than
	 *    processing it after each message.
	 *
	 * 2) Make a loop, use UL_NL_ONESHOT, keep addrq->callback_post empty
	 *    and process the result in the loop.
	 */
	ulrc = ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP);
	إذا (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	إذا ((ul_nl_close(&nl)))
		rc = 1;
	أرجع rc;
}
#پایان_اگر /* TEST_PROGRAM_NETADDRQ */
