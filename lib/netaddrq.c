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
#تعریف_کړه ULNETADDRQ_DEBUG_HELP	(1 << 0)
#تعریف_کړه ULNETADDRQ_DEBUG_INIT	(1 << 1)
#تعریف_کړه ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#تعریف_کړه ULNETADDRQ_DEBUG_LIST	(1 << 3)
#تعریف_کړه ULNETADDRQ_DEBUG_غوره	(1 << 4)

ساكن UL_DEBUG_DEFINE_ماسک(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_غوره,	"searching best address" },

	{ عدم, 0 }
};

#تعریف_کړه DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#تعریف_کړه DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#تعریف_کړه ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

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

#تعریف_کړه DBG_CASE(x) حالة x: str = #x; اكسر
#تعریف_کړه DBG_CASE_DEF8(x) افتراضي: snprintf(strx+2, 3, "%02hhx", x); str = strx; اكسر
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
ساكن صحيح callback_addrq(بنية ul_nl_معلومات *nl)
{
	بنية ul_netaddrq_معلومات *addrq = UL_NETADDRQ_معلومات(nl);
	بنية list_سر *li, *ipq_list;
	بنية ul_netaddrq_iface *ifaceq = عدم;
	بنية ul_netaddrq_ip *ipq = عدم;
	صحيح rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_پته(&(nl->addr)),
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

		ifaceqq = list_ثبت(li, بنية ul_netaddrq_iface, ثبت);
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
				addrq->ډکېدنه = true;
				أرجع UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			إذا (!(ifaceq = حافظه_ورکړه(حجم(بنية ul_netaddrq_iface))))
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
			list_جمع_کړه_لکۍ(&(ifaceq->ثبت), &(addrq->ifaces));
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

		ipqq = list_ثبت(li, بنية ul_netaddrq_ip, ثبت);
		إذا (ipqq->addr->پته_len == nl->addr.پته_len)
			إذا (!memcmp(ipqq->addr->پته, nl->addr.پته,
				   nl->addr.پته_len)) {
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
			إذا (!(ipq = حافظه_ورکړه(حجم(بنية ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				اذهب error;
			}
			ipq->addr = addr;
			list_جمع_کړه_لکۍ(&(ipq->ثبت), ipq_list);
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
				ul_nl_addr_ntop_پته(&(nl->addr)),
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
		list_del(&(ipq->ثبت));
		ul_nl_addr_free(ipq->addr);
		حرر(ipq);
	error:
		إذا (list_تشه(&(ifaceq->ip_quality_list_4)) &&
		    list_تشه(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->ثبت));
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
صحيح ul_netaddrq_init(بنية ul_nl_معلومات *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, فراغ *معلومات)
{
	بنية ul_netaddrq_معلومات *addrq;

	netaddrq_init_debug();
	إذا (!(nl->معلومات_addr = حافظه_صفر_ورکړه(1, حجم(بنية ul_netaddrq_معلومات))))
		أرجع -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_معلومات(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_معلومات = معلومات;
	INIT_LIST_سر(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	أرجع 0;
}

تعداد ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(بنية list_سر *ipq_list,
			   بنية ul_netaddrq_ip *(*غوره)[__ULNETLINK_RATING_MAX])
{
	بنية list_سر *li;
	بنية ul_netaddrq_ip *ipq;
	تعداد ul_netaddrq_ip_rating پوله;

	پوله = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_ثبت(li, بنية ul_netaddrq_ip, ثبت);

		إذا (!(*غوره)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*غوره)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(غوره_2, (*غوره),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_پته(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*غوره)[ipq->quality] = ipq;
		}

		إذا (ipq->quality < پوله)
		{
			پوله = ipq->quality;
			DBG(غوره_2,
			    ul_debug("threshold %s", ip_rating_as_string(پوله)));

		}
	}
	أرجع پوله;
}

تعداد ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(بنية ul_nl_معلومات *nl,
		     بنية ul_netaddrq_iface **غوره_ifaceq,
		     بنية ul_netaddrq_ip *(*غوره)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	بنية ul_netaddrq_معلومات *addrq = UL_NETADDRQ_معلومات(nl);
	بنية list_سر *li;
	بنية ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	تعداد ul_netaddrq_ip_rating پوله;

	إذا (ifa_family == AF_INET) {
		ipqo = offsetof(بنية ul_netaddrq_iface, ip_quality_list_4);
	} وإلا {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(بنية ul_netaddrq_iface, ip_quality_list_6);
	}

	پوله = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		بنية list_سر *ipq_list;
		تعداد ul_netaddrq_ip_rating t;

		ifaceq = list_ثبت(li, بنية ul_netaddrq_iface, ثبت);

		ipq_list = (بنية list_سر*)((حرف*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, غوره);
		إذا (t < پوله)
		{
			DBG_OBJ(غوره_2, *غوره,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*غوره_ifaceq = ifaceq;
			پوله = t;
		}
	}
	أرجع پوله;
}

ثابت حرف *ul_netaddrq_get_غوره_ipp(بنية ul_nl_معلومات *nl,
				     uint8_t ifa_family,
				     تعداد ul_netaddrq_ip_rating *پوله,
				     بنية ul_netaddrq_iface **غوره_ifaceq)
{
	بنية ul_netaddrq_ip *غوره[__ULNETLINK_RATING_MAX];

	memset(غوره, 0, حجم(غوره));
	*پوله = ul_netaddrq_bestaddr(nl, غوره_ifaceq, &غوره, ifa_family);
	إذا (*پوله != __ULNETLINK_RATING_MAX)
		أرجع ul_nl_addr_ntop_پته(غوره[*پوله]->addr);
	أرجع عدم;
}

بنية ul_netaddrq_iface *ul_netaddrq_iface_by_name(ثابت بنية ul_nl_معلومات *nl,
						    ثابت حرف *ifname)
{
	بنية list_سر *li;
	بنية ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_ثبت(li, بنية ul_netaddrq_iface, ثبت);

		إذا (!strcmp(ifaceq->ifname, ifname))
			أرجع ifaceq;
	}
	أرجع عدم;
}

#که_تعریف_شوی TEST_پروګرام_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
تعداد addrq_print_ډېرپېښ {
	ADDRQ_ډېرپېښ_BESTOFALL,	/* Best address of all interfaces */
	ADDRQ_ډېرپېښ_غوره,	/* Best address per interface */
	ADDRQ_ډېرپېښ_GOOD,	/* All global or site addresses, if none, the
				 * longest living temporary, if none, link */
	ADDRQ_ډېرپېښ_ALL		/* All available addresses */
};

/* In our example addrq->callback_data is a simple FILE *. In more complex
 * programs it could be a pointer to an arbitrary struct */
#تعریف_کړه netout (FILE *)(UL_NETADDRQ_معلومات(nl)->callback_معلومات)

/* This example uses separate threshold for IPv4 and IPv6, so the best IPv4 and
 * best IPv6 addresses are printed. */
ساكن فراغ dump_iface_غوره(بنية ul_nl_معلومات *nl,
			    بنية ul_netaddrq_iface *ifaceq)
{
	بنية ul_netaddrq_ip *غوره[__ULNETLINK_RATING_MAX];
	تعداد ul_netaddrq_ip_rating پوله;
	bool لومړی = true;

	memset(غوره, 0, حجم(غوره));
	پوله = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &غوره);
	إذا (پوله != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (لومړی ? "best address" : " "),
		       ul_nl_addr_ntop_پته(غوره[پوله]->addr));
		لومړی = false;
	}
	memset(غوره, 0, حجم(غوره));
	پوله = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &غوره);
	إذا (پوله != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (لومړی ? "best address" : " "),
		       ul_nl_addr_ntop_پته(غوره[پوله]->addr));
		لومړی = false;
	}
	إذا (!لومړی)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
ساكن فراغ dump_iface_good(بنية ul_nl_معلومات *nl,
			    بنية ul_netaddrq_iface *ifaceq)
{
	بنية ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	بنية ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	بنية list_سر *li;
	تعداد ul_netaddrq_ip_rating پوله = __ULNETLINK_RATING_MAX - 1;
	تعداد ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool لومړی = true;

	memset(best4, 0, حجم(best4));
	پوله = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, حجم(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	إذا (fthreshold < پوله)
		پوله = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		بنية ul_netaddrq_ip *ipq;

		ipq = list_ثبت(li, بنية ul_netaddrq_ip, ثبت);
		إذا (پوله <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= پوله ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[پوله])
		{
			إذا (لومړی)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				لومړی = false;
			}
			وإلا
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			إذا (پوله == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_پته(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				اذهب لنډمهاله_cont4;
			}
			وإلا
				fputs(ul_nl_addr_ntop_پته(ipq->addr),
				      netout);
		}
	لنډمهاله_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		بنية ul_netaddrq_ip *ipq;

		ipq = list_ثبت(li, بنية ul_netaddrq_ip, ثبت);
		إذا (پوله <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= پوله ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[پوله])
		{
			إذا (لومړی)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				لومړی = false;
			}
			وإلا
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			إذا (پوله == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_پته(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				اذهب لنډمهاله_cont6;
			}
			وإلا
				fputs(ul_nl_addr_ntop_پته(ipq->addr),
				      netout);
		}
	لنډمهاله_cont6:;
	}
	إذا (!لومړی)
		fputs("\n", netout);
}

ساكن فراغ dump_iface_all(بنية ul_nl_معلومات *nl,
			   بنية ul_netaddrq_iface *ifaceq)
{
	بنية list_سر *li;
	بنية ul_netaddrq_ip *ipq;
	bool لومړی = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_ثبت(li, بنية ul_netaddrq_ip, ثبت);
		إذا (لومړی)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			لومړی = false;
		}
		وإلا
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_پته(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_ثبت(li, بنية ul_netaddrq_ip, ثبت);
		إذا (لومړی)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			لومړی = false;
		}
		وإلا
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_پته(ipq->addr), netout);
	}
	إذا (!لومړی)
		fputs("\n", netout);
}

ساكن فراغ dump_addrq(بنية ul_nl_معلومات *nl, تعداد addrq_print_ډېرپېښ c)
{
	بنية list_سر *li;
	بنية ul_netaddrq_iface *ifaceq;
	تعداد ul_netaddrq_ip_rating پوله;

	بدل(c)
	{
	حالة ADDRQ_ډېرپېښ_BESTOFALL:
	{
		بنية ul_netaddrq_iface *غوره_ifaceq;
		ثابت حرف *غوره_ipp;

		غوره_ipp = ul_netaddrq_get_غوره_ipp(nl, AF_INET,
						    &پوله, &غوره_ifaceq);
		إذا (غوره_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				غوره_ipp, غوره_ifaceq->ifname);
		غوره_ipp = ul_netaddrq_get_غوره_ipp(nl, AF_INET6,
						    &پوله, &غوره_ifaceq);
		إذا (غوره_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				غوره_ipp, غوره_ifaceq->ifname);
	}
	اكسر;
	حالة ADDRQ_ډېرپېښ_غوره:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_ثبت(li, بنية ul_netaddrq_iface, ثبت);

			dump_iface_غوره(nl, ifaceq);
		}
	}
	اكسر;
	حالة ADDRQ_ډېرپېښ_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_ثبت(li, بنية ul_netaddrq_iface, ثبت);

			dump_iface_good(nl, ifaceq);
		}
	}
	اكسر;
	حالة ADDRQ_ډېرپېښ_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_ثبت(li, بنية ul_netaddrq_iface, ثبت);

			dump_iface_all(nl,ifaceq);
		}
	اكسر;
	}
}

ساكن صحيح callback_post(بنية ul_nl_معلومات *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	إذا (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		إذا (!(UL_NETADDRQ_معلومات(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_معلومات(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			أرجع 0;
		}
		UL_NETADDRQ_معلومات(nl)->ifaces_change_4 = false;
		UL_NETADDRQ_معلومات(nl)->ifaces_change_6 = false;
		fputs("\n\nNetwork change detected:\n", netout);
		fputs("\nbest address:\n", netout);
		dump_addrq(nl, ADDRQ_ډېرپېښ_BESTOFALL);

		fputs("\nbest addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_ډېرپېښ_غوره);

		fputs("\ngood addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_ډېرپېښ_GOOD);

		fputs("\nall addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_ډېرپېښ_ALL);
	}
	أرجع 0;
}

صحيح اصلي(صحيح argc __attribute__((__unused__)), حرف *argv[] __attribute__((__unused__)))
{
	صحيح rc = 1;
	صحيح ulrc;
	بنية ul_nl_معلومات nl;
	FILE *بهر = stdout;
	بنية ul_netaddrq_iface *ifaceq;
	ثابت حرف *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	إذا ((ul_netaddrq_init(&nl, عدم, callback_post, (فراغ *)بهر)))
		أرجع -1;

	/* Dump addresses */
	إذا (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		أرجع -1;
	إذا (ul_nl_request_dump(&nl, RTM_GETADDR))
		اذهب error;
	إذا (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_بشپړ)
		اذهب error;
	fputs("RTM_GETADDR dump finished.", بهر);

	/* Example of several types of dump */
	fputs("\nbest address:\n", بهر);
	dump_addrq(&nl, ADDRQ_ډېرپېښ_BESTOFALL);

	fputs("\nbest addresses dump:\n", بهر);
	dump_addrq(&nl, ADDRQ_ډېرپېښ_غوره);

	fputs("\ngood addresses dump:\n", بهر);
	dump_addrq(&nl, ADDRQ_ډېرپېښ_GOOD);

	fputs("\nall addresses dump:\n", بهر);
	dump_addrq(&nl, ADDRQ_ډېرپېښ_ALL);

	fputs("\naddresses for interface ", بهر);
	إذا ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	وإلا
		fprintf(بهر, "%s not found.", ifname);

	/* Monitor further changes */
	fputs("\nGoing to monitor mode.\n", بهر);

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
#د_که_پای /* TEST_PROGRAM_NETADDRQ */
