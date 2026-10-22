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

#شامل <net/if.h>
#شامل <netinet/in.h>
#شامل <linux/rtnetlink.h>
#شامل <linux/if_addr.h>
#شامل "netaddrq.h"
#شامل "list.h"
#شامل "debug.h"

/* Maximal number of interfaces. The algorithm has a quadratic complexity,
 * don't overflood it. */
ثابت صحیح max_ifaces = 12;

/*
 * Debug stuff (based on include/debug.h)
 */
#تعریف_کرو ULNETADDRQ_DEBUG_HELP	(1 << 0)
#تعریف_کرو ULNETADDRQ_DEBUG_INIT	(1 << 1)
#تعریف_کرو ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#تعریف_کرو ULNETADDRQ_DEBUG_LIST	(1 << 3)
#تعریف_کرو ULNETADDRQ_DEBUG_بہترین	(1 << 4)

ساکن UL_DEBUG_DEFINE_نقاب(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_بہترین,	"searching best address" },

	{ تہی, 0 }
};

#تعریف_کرو DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#تعریف_کرو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#تعریف_کرو ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

ساکن خالی netaddrq_init_debug(خالی)
{
	اگر (netaddrq_debug_نقاب)
		واپس;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

ساکن اندرونی شمار ul_netaddrq_ip_rating
evaluate_ip_quality(ساخت ul_nl_addr *addr) {
	شمار ul_netaddrq_ip_rating quality;

	انتخاب (addr->ifa_scope) {
	حالت RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		توڑو;
	حالت RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		توڑو;
	حالت RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		توڑو;
	طےشدہ:
		quality = ULNETLINK_RATING_BAD;
		توڑو;
	}
	اگر (addr->ifa_flags & IFA_F_TEMPORARY) {
		اگر (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	واپس quality;
}

#تعریف_کرو DBG_CASE(x) حالت x: str = #x; توڑو
#تعریف_کرو DBG_CASE_DEF8(x) طےشدہ: snprintf(strx+2, 3, "%02hhx", x); str = strx; توڑو
ساکن حرف *ip_rating_as_string(شمار ul_netaddrq_ip_rating q)
{
	حرف *str;
	ساکن حرف strx[5] = "0x";
	انتخاب (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	واپس str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
ساکن صحیح callback_addrq(ساخت ul_nl_ڈیٹا *nl)
{
	ساخت ul_netaddrq_ڈیٹا *addrq = UL_NETADDRQ_ڈیٹا(nl);
	ساخت list_سر *li, *ipq_list;
	ساخت ul_netaddrq_iface *ifaceq = تہی;
	ساخت ul_netaddrq_ip *ipq = تہی;
	صحیح rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_پتہ(&(nl->addr)),
			      nl->addr.ifname));
	اگر (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		اگر ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		ساخت ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_اندراج(li, ساخت ul_netaddrq_iface, اندراج_2);
		اگر (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			توڑو;
		}
		addrq->nifaces++;
	}

	اگر (ifaceq == تہی) {
		اگر (nl->rtm_event) {
			اگر (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->حد_سے_تجاوز = true;
				واپس UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			اگر (!(ifaceq = حافظہ_دو(حجم(ساخت ul_netaddrq_iface))))
				واپس -ENOMEM;
			INIT_LIST_سر(&(ifaceq->ip_quality_list_4));
			INIT_LIST_سر(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			اگر (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				آزاد(ifaceq);
				واپس -ENOMEM;
			}
			list_جوڑو_دم(&(ifaceq->اندراج_2), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} ورنہ {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			واپس UL_NL_SOFT_ERROR;
		}
	}
	اگر (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} ورنہ {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		ساخت ul_netaddrq_ip *ipqq;

		ipqq = list_اندراج(li, ساخت ul_netaddrq_ip, اندراج_2);
		اگر (ipqq->addr->پتہ_len == nl->addr.پتہ_len)
			اگر (!memcmp(ipqq->addr->پتہ, nl->addr.پتہ,
				   nl->addr.پتہ_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				توڑو;
			}
	}
	اگر (ipq == تہی) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	اگر (UL_NL_IS_RTM_NEW(nl)) {
		ساخت ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		اگر (!addr) {
			rc = -ENOMEM;
			جاؤ error;
		}
		اگر (ipq == تہی) {
			اگر (!(ipq = حافظہ_دو(حجم(ساخت ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				جاؤ error;
			}
			ipq->addr = addr;
			list_جوڑو_دم(&(ipq->اندراج_2), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} ورنہ {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_پتہ(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} ورنہ {
		/* UL_NL_RTM_DEL */
		اگر (ipq == تہی)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			واپس UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->اندراج_2));
		ul_nl_addr_free(ipq->addr);
		آزاد(ipq);
	error:
		اگر (list_خالی_خانے(&(ifaceq->ip_quality_list_4)) &&
		    list_خالی_خانے(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->اندراج_2));
			addrq->nifaces--;
			آزاد(ifaceq->ifname);
			آزاد(ifaceq);
		}
	}
	اگر (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		اگر ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	واپس rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
صحیح ul_netaddrq_init(ساخت ul_nl_ڈیٹا *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, خالی *ڈیٹا)
{
	ساخت ul_netaddrq_ڈیٹا *addrq;

	netaddrq_init_debug();
	اگر (!(nl->ڈیٹا_addr = صفر_حافظہ_دو(1, حجم(ساخت ul_netaddrq_ڈیٹا))))
		واپس -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_ڈیٹا(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_ڈیٹا = ڈیٹا;
	INIT_LIST_سر(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	واپس 0;
}

شمار ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(ساخت list_سر *ipq_list,
			   ساخت ul_netaddrq_ip *(*بہترین)[__ULNETLINK_RATING_MAX])
{
	ساخت list_سر *li;
	ساخت ul_netaddrq_ip *ipq;
	شمار ul_netaddrq_ip_rating حد;

	حد = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_اندراج(li, ساخت ul_netaddrq_ip, اندراج_2);

		اگر (!(*بہترین)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*بہترین)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(بہترین_2, (*بہترین),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_پتہ(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*بہترین)[ipq->quality] = ipq;
		}

		اگر (ipq->quality < حد)
		{
			حد = ipq->quality;
			DBG(بہترین_2,
			    ul_debug("threshold %s", ip_rating_as_string(حد)));

		}
	}
	واپس حد;
}

شمار ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(ساخت ul_nl_ڈیٹا *nl,
		     ساخت ul_netaddrq_iface **بہترین_ifaceq,
		     ساخت ul_netaddrq_ip *(*بہترین)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	ساخت ul_netaddrq_ڈیٹا *addrq = UL_NETADDRQ_ڈیٹا(nl);
	ساخت list_سر *li;
	ساخت ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	شمار ul_netaddrq_ip_rating حد;

	اگر (ifa_family == AF_INET) {
		ipqo = offsetof(ساخت ul_netaddrq_iface, ip_quality_list_4);
	} ورنہ {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(ساخت ul_netaddrq_iface, ip_quality_list_6);
	}

	حد = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		ساخت list_سر *ipq_list;
		شمار ul_netaddrq_ip_rating t;

		ifaceq = list_اندراج(li, ساخت ul_netaddrq_iface, اندراج_2);

		ipq_list = (ساخت list_سر*)((حرف*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, بہترین);
		اگر (t < حد)
		{
			DBG_OBJ(بہترین_2, *بہترین,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*بہترین_ifaceq = ifaceq;
			حد = t;
		}
	}
	واپس حد;
}

ثابت حرف *ul_netaddrq_get_بہترین_ipp(ساخت ul_nl_ڈیٹا *nl,
				     uint8_t ifa_family,
				     شمار ul_netaddrq_ip_rating *حد,
				     ساخت ul_netaddrq_iface **بہترین_ifaceq)
{
	ساخت ul_netaddrq_ip *بہترین[__ULNETLINK_RATING_MAX];

	memset(بہترین, 0, حجم(بہترین));
	*حد = ul_netaddrq_bestaddr(nl, بہترین_ifaceq, &بہترین, ifa_family);
	اگر (*حد != __ULNETLINK_RATING_MAX)
		واپس ul_nl_addr_ntop_پتہ(بہترین[*حد]->addr);
	واپس تہی;
}

ساخت ul_netaddrq_iface *ul_netaddrq_iface_by_name(ثابت ساخت ul_nl_ڈیٹا *nl,
						    ثابت حرف *ifname)
{
	ساخت list_سر *li;
	ساخت ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_اندراج(li, ساخت ul_netaddrq_iface, اندراج_2);

		اگر (!strcmp(ifaceq->ifname, ifname))
			واپس ifaceq;
	}
	واپس تہی;
}

#اگر_تعریف_شدہ TEST_پروگرام_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
شمار addrq_print_منوال {
	ADDRQ_منوال_BESTOFALL,	/* Best address of all interfaces */
	ADDRQ_منوال_بہترین,	/* Best address per interface */
	ADDRQ_منوال_GOOD,	/* All global or site addresses, if none, the
				 * longest living temporary, if none, link */
	ADDRQ_منوال_ALL		/* All available addresses */
};

/* In our example addrq->callback_data is a simple FILE *. In more complex
 * programs it could be a pointer to an arbitrary struct */
#تعریف_کرو netout (FILE *)(UL_NETADDRQ_ڈیٹا(nl)->callback_ڈیٹا)

/* This example uses separate threshold for IPv4 and IPv6, so the best IPv4 and
 * best IPv6 addresses are printed. */
ساکن خالی dump_iface_بہترین(ساخت ul_nl_ڈیٹا *nl,
			    ساخت ul_netaddrq_iface *ifaceq)
{
	ساخت ul_netaddrq_ip *بہترین[__ULNETLINK_RATING_MAX];
	شمار ul_netaddrq_ip_rating حد;
	bool پہلا = true;

	memset(بہترین, 0, حجم(بہترین));
	حد = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &بہترین);
	اگر (حد != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (پہلا ? "best address" : " "),
		       ul_nl_addr_ntop_پتہ(بہترین[حد]->addr));
		پہلا = false;
	}
	memset(بہترین, 0, حجم(بہترین));
	حد = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &بہترین);
	اگر (حد != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (پہلا ? "best address" : " "),
		       ul_nl_addr_ntop_پتہ(بہترین[حد]->addr));
		پہلا = false;
	}
	اگر (!پہلا)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
ساکن خالی dump_iface_good(ساخت ul_nl_ڈیٹا *nl,
			    ساخت ul_netaddrq_iface *ifaceq)
{
	ساخت ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	ساخت ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	ساخت list_سر *li;
	شمار ul_netaddrq_ip_rating حد = __ULNETLINK_RATING_MAX - 1;
	شمار ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool پہلا = true;

	memset(best4, 0, حجم(best4));
	حد = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, حجم(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	اگر (fthreshold < حد)
		حد = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ساخت ul_netaddrq_ip *ipq;

		ipq = list_اندراج(li, ساخت ul_netaddrq_ip, اندراج_2);
		اگر (حد <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= حد ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[حد])
		{
			اگر (پہلا)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				پہلا = false;
			}
			ورنہ
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			اگر (حد == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_پتہ(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				جاؤ عارضی_cont4;
			}
			ورنہ
				fputs(ul_nl_addr_ntop_پتہ(ipq->addr),
				      netout);
		}
	عارضی_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ساخت ul_netaddrq_ip *ipq;

		ipq = list_اندراج(li, ساخت ul_netaddrq_ip, اندراج_2);
		اگر (حد <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= حد ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[حد])
		{
			اگر (پہلا)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				پہلا = false;
			}
			ورنہ
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			اگر (حد == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_پتہ(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				جاؤ عارضی_cont6;
			}
			ورنہ
				fputs(ul_nl_addr_ntop_پتہ(ipq->addr),
				      netout);
		}
	عارضی_cont6:;
	}
	اگر (!پہلا)
		fputs("\n", netout);
}

ساکن خالی dump_iface_all(ساخت ul_nl_ڈیٹا *nl,
			   ساخت ul_netaddrq_iface *ifaceq)
{
	ساخت list_سر *li;
	ساخت ul_netaddrq_ip *ipq;
	bool پہلا = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_اندراج(li, ساخت ul_netaddrq_ip, اندراج_2);
		اگر (پہلا)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			پہلا = false;
		}
		ورنہ
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_پتہ(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_اندراج(li, ساخت ul_netaddrq_ip, اندراج_2);
		اگر (پہلا)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			پہلا = false;
		}
		ورنہ
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_پتہ(ipq->addr), netout);
	}
	اگر (!پہلا)
		fputs("\n", netout);
}

ساکن خالی dump_addrq(ساخت ul_nl_ڈیٹا *nl, شمار addrq_print_منوال c)
{
	ساخت list_سر *li;
	ساخت ul_netaddrq_iface *ifaceq;
	شمار ul_netaddrq_ip_rating حد;

	انتخاب(c)
	{
	حالت ADDRQ_منوال_BESTOFALL:
	{
		ساخت ul_netaddrq_iface *بہترین_ifaceq;
		ثابت حرف *بہترین_ipp;

		بہترین_ipp = ul_netaddrq_get_بہترین_ipp(nl, AF_INET,
						    &حد, &بہترین_ifaceq);
		اگر (بہترین_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				بہترین_ipp, بہترین_ifaceq->ifname);
		بہترین_ipp = ul_netaddrq_get_بہترین_ipp(nl, AF_INET6,
						    &حد, &بہترین_ifaceq);
		اگر (بہترین_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				بہترین_ipp, بہترین_ifaceq->ifname);
	}
	توڑو;
	حالت ADDRQ_منوال_بہترین:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_اندراج(li, ساخت ul_netaddrq_iface, اندراج_2);

			dump_iface_بہترین(nl, ifaceq);
		}
	}
	توڑو;
	حالت ADDRQ_منوال_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_اندراج(li, ساخت ul_netaddrq_iface, اندراج_2);

			dump_iface_good(nl, ifaceq);
		}
	}
	توڑو;
	حالت ADDRQ_منوال_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_اندراج(li, ساخت ul_netaddrq_iface, اندراج_2);

			dump_iface_all(nl,ifaceq);
		}
	توڑو;
	}
}

ساکن صحیح callback_post(ساخت ul_nl_ڈیٹا *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	اگر (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		اگر (!(UL_NETADDRQ_ڈیٹا(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_ڈیٹا(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			واپس 0;
		}
		UL_NETADDRQ_ڈیٹا(nl)->ifaces_change_4 = false;
		UL_NETADDRQ_ڈیٹا(nl)->ifaces_change_6 = false;
		fputs("\n\nNetwork change detected:\n", netout);
		fputs("\nbest address:\n", netout);
		dump_addrq(nl, ADDRQ_منوال_BESTOFALL);

		fputs("\nbest addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_منوال_بہترین);

		fputs("\ngood addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_منوال_GOOD);

		fputs("\nall addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_منوال_ALL);
	}
	واپس 0;
}

صحیح مین(صحیح argc __attribute__((__unused__)), حرف *argv[] __attribute__((__unused__)))
{
	صحیح rc = 1;
	صحیح ulrc;
	ساخت ul_nl_ڈیٹا nl;
	FILE *برآمد = stdout;
	ساخت ul_netaddrq_iface *ifaceq;
	ثابت حرف *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	اگر ((ul_netaddrq_init(&nl, تہی, callback_post, (خالی *)برآمد)))
		واپس -1;

	/* Dump addresses */
	اگر (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		واپس -1;
	اگر (ul_nl_request_dump(&nl, RTM_GETADDR))
		جاؤ error;
	اگر (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_مکمل)
		جاؤ error;
	fputs("RTM_GETADDR dump finished.", برآمد);

	/* Example of several types of dump */
	fputs("\nbest address:\n", برآمد);
	dump_addrq(&nl, ADDRQ_منوال_BESTOFALL);

	fputs("\nbest addresses dump:\n", برآمد);
	dump_addrq(&nl, ADDRQ_منوال_بہترین);

	fputs("\ngood addresses dump:\n", برآمد);
	dump_addrq(&nl, ADDRQ_منوال_GOOD);

	fputs("\nall addresses dump:\n", برآمد);
	dump_addrq(&nl, ADDRQ_منوال_ALL);

	fputs("\naddresses for interface ", برآمد);
	اگر ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	ورنہ
		fprintf(برآمد, "%s not found.", ifname);

	/* Monitor further changes */
	fputs("\nGoing to monitor mode.\n", برآمد);

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
	اگر (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	اگر ((ul_nl_close(&nl)))
		rc = 1;
	واپس rc;
}
#اگر_ختم /* TEST_PROGRAM_NETADDRQ */
