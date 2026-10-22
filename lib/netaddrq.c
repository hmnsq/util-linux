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

#অন্তর্ভাব <net/if.h>
#অন্তর্ভাব <netinet/in.h>
#অন্তর্ভাব <linux/rtnetlink.h>
#অন্তর্ভাব <linux/if_addr.h>
#অন্তর্ভাব "netaddrq.h"
#অন্তর্ভাব "list.h"
#অন্তর্ভাব "debug.h"

/* Maximal number of interfaces. The algorithm has a quadratic complexity,
 * don't overflood it. */
ধ্রুবক পূর্ণসংখ্যা max_ifaces = 12;

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETADDRQ_DEBUG_HELP	(1 << 0)
#define ULNETADDRQ_DEBUG_INIT	(1 << 1)
#define ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#define ULNETADDRQ_DEBUG_LIST	(1 << 3)
#define ULNETADDRQ_DEBUG_BEST	(1 << 4)

স্থির UL_DEBUG_DEFINE_MASK(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_BEST,	"searching best address" },

	{ ফাঁকা, 0 }
};

#define DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

স্থির শূন্য netaddrq_init_debug(শূন্য)
{
	যদি (netaddrq_debug_mask)
		ফেরত;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

স্থির অন্তঃরেখ গণনা ul_netaddrq_ip_rating
evaluate_ip_quality(কাঠামো ul_nl_addr *addr) {
	গণনা ul_netaddrq_ip_rating quality;

	নির্বাচন (addr->ifa_scope) {
	ক্ষেত্র RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		বিরতি;
	ক্ষেত্র RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		বিরতি;
	ক্ষেত্র RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		বিরতি;
	পূর্বনির্ধারিত:
		quality = ULNETLINK_RATING_BAD;
		বিরতি;
	}
	যদি (addr->ifa_flags & IFA_F_TEMPORARY) {
		যদি (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	ফেরত quality;
}

#define DBG_CASE(x) ক্ষেত্র x: str = #x; বিরতি
#define DBG_CASE_DEF8(x) পূর্বনির্ধারিত: snprintf(strx+2, 3, "%02hhx", x); str = strx; বিরতি
স্থির অক্ষর *ip_rating_as_string(গণনা ul_netaddrq_ip_rating q)
{
	অক্ষর *str;
	স্থির অক্ষর strx[5] = "0x";
	নির্বাচন (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	ফেরত str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
স্থির পূর্ণসংখ্যা callback_addrq(কাঠামো ul_nl_তথ্য *nl)
{
	কাঠামো ul_netaddrq_তথ্য *addrq = UL_NETADDRQ_তথ্য(nl);
	কাঠামো list_শীর্ষ *li, *ipq_list;
	কাঠামো ul_netaddrq_iface *ifaceq = ফাঁকা;
	কাঠামো ul_netaddrq_ip *ipq = ফাঁকা;
	পূর্ণসংখ্যা rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_address(&(nl->addr)),
			      nl->addr.ifname));
	যদি (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		যদি ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		কাঠামো ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_entry(li, কাঠামো ul_netaddrq_iface, entry);
		যদি (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			বিরতি;
		}
		addrq->nifaces++;
	}

	যদি (ifaceq == ফাঁকা) {
		যদি (nl->rtm_event) {
			যদি (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->overflow = true;
				ফেরত UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			যদি (!(ifaceq = malloc(আকার(কাঠামো ul_netaddrq_iface))))
				ফেরত -ENOMEM;
			INIT_LIST_শীর্ষ(&(ifaceq->ip_quality_list_4));
			INIT_LIST_শীর্ষ(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			যদি (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				মুক্ত(ifaceq);
				ফেরত -ENOMEM;
			}
			list_add_লেজ(&(ifaceq->entry), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} অন্যথা {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			ফেরত UL_NL_SOFT_ERROR;
		}
	}
	যদি (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} অন্যথা {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		কাঠামো ul_netaddrq_ip *ipqq;

		ipqq = list_entry(li, কাঠামো ul_netaddrq_ip, entry);
		যদি (ipqq->addr->address_len == nl->addr.address_len)
			যদি (!memcmp(ipqq->addr->address, nl->addr.address,
				   nl->addr.address_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				বিরতি;
			}
	}
	যদি (ipq == ফাঁকা) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	যদি (UL_NL_IS_RTM_NEW(nl)) {
		কাঠামো ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		যদি (!addr) {
			rc = -ENOMEM;
			যাও error;
		}
		যদি (ipq == ফাঁকা) {
			যদি (!(ipq = malloc(আকার(কাঠামো ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				যাও error;
			}
			ipq->addr = addr;
			list_add_লেজ(&(ipq->entry), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} অন্যথা {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_address(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} অন্যথা {
		/* UL_NL_RTM_DEL */
		যদি (ipq == ফাঁকা)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			ফেরত UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->entry));
		ul_nl_addr_free(ipq->addr);
		মুক্ত(ipq);
	error:
		যদি (list_empty(&(ifaceq->ip_quality_list_4)) &&
		    list_empty(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->entry));
			addrq->nifaces--;
			মুক্ত(ifaceq->ifname);
			মুক্ত(ifaceq);
		}
	}
	যদি (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		যদি ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	ফেরত rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
পূর্ণসংখ্যা ul_netaddrq_init(কাঠামো ul_nl_তথ্য *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, শূন্য *তথ্য)
{
	কাঠামো ul_netaddrq_তথ্য *addrq;

	netaddrq_init_debug();
	যদি (!(nl->তথ্য_addr = calloc(1, আকার(কাঠামো ul_netaddrq_তথ্য))))
		ফেরত -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_তথ্য(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_তথ্য = তথ্য;
	INIT_LIST_শীর্ষ(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	ফেরত 0;
}

গণনা ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(কাঠামো list_শীর্ষ *ipq_list,
			   কাঠামো ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX])
{
	কাঠামো list_শীর্ষ *li;
	কাঠামো ul_netaddrq_ip *ipq;
	গণনা ul_netaddrq_ip_rating threshold;

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_entry(li, কাঠামো ul_netaddrq_ip, entry);

		যদি (!(*best)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*best)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(BEST, (*best),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_address(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*best)[ipq->quality] = ipq;
		}

		যদি (ipq->quality < threshold)
		{
			threshold = ipq->quality;
			DBG(BEST,
			    ul_debug("threshold %s", ip_rating_as_string(threshold)));

		}
	}
	ফেরত threshold;
}

গণনা ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(কাঠামো ul_nl_তথ্য *nl,
		     কাঠামো ul_netaddrq_iface **best_ifaceq,
		     কাঠামো ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	কাঠামো ul_netaddrq_তথ্য *addrq = UL_NETADDRQ_তথ্য(nl);
	কাঠামো list_শীর্ষ *li;
	কাঠামো ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	গণনা ul_netaddrq_ip_rating threshold;

	যদি (ifa_family == AF_INET) {
		ipqo = offsetof(কাঠামো ul_netaddrq_iface, ip_quality_list_4);
	} অন্যথা {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(কাঠামো ul_netaddrq_iface, ip_quality_list_6);
	}

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		কাঠামো list_শীর্ষ *ipq_list;
		গণনা ul_netaddrq_ip_rating t;

		ifaceq = list_entry(li, কাঠামো ul_netaddrq_iface, entry);

		ipq_list = (কাঠামো list_শীর্ষ*)((অক্ষর*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, best);
		যদি (t < threshold)
		{
			DBG_OBJ(BEST, *best,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*best_ifaceq = ifaceq;
			threshold = t;
		}
	}
	ফেরত threshold;
}

ধ্রুবক অক্ষর *ul_netaddrq_get_best_ipp(কাঠামো ul_nl_তথ্য *nl,
				     uint8_t ifa_family,
				     গণনা ul_netaddrq_ip_rating *threshold,
				     কাঠামো ul_netaddrq_iface **best_ifaceq)
{
	কাঠামো ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];

	memset(best, 0, আকার(best));
	*threshold = ul_netaddrq_bestaddr(nl, best_ifaceq, &best, ifa_family);
	যদি (*threshold != __ULNETLINK_RATING_MAX)
		ফেরত ul_nl_addr_ntop_address(best[*threshold]->addr);
	ফেরত ফাঁকা;
}

কাঠামো ul_netaddrq_iface *ul_netaddrq_iface_by_name(ধ্রুবক কাঠামো ul_nl_তথ্য *nl,
						    ধ্রুবক অক্ষর *ifname)
{
	কাঠামো list_শীর্ষ *li;
	কাঠামো ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_entry(li, কাঠামো ul_netaddrq_iface, entry);

		যদি (!strcmp(ifaceq->ifname, ifname))
			ফেরত ifaceq;
	}
	ফেরত ফাঁকা;
}

#ifdef TEST_PROGRAM_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
গণনা addrq_print_mode {
	ADDRQ_MODE_BESTOFALL,	/* Best address of all interfaces */
	ADDRQ_MODE_BEST,	/* Best address per interface */
	ADDRQ_MODE_GOOD,	/* All global or site addresses, if none, the
				 * longest living temporary, if none, link */
	ADDRQ_MODE_ALL		/* All available addresses */
};

/* In our example addrq->callback_data is a simple FILE *. In more complex
 * programs it could be a pointer to an arbitrary struct */
#define netout (FILE *)(UL_NETADDRQ_তথ্য(nl)->callback_তথ্য)

/* This example uses separate threshold for IPv4 and IPv6, so the best IPv4 and
 * best IPv6 addresses are printed. */
স্থির শূন্য dump_iface_best(কাঠামো ul_nl_তথ্য *nl,
			    কাঠামো ul_netaddrq_iface *ifaceq)
{
	কাঠামো ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];
	গণনা ul_netaddrq_ip_rating threshold;
	bool first = true;

	memset(best, 0, আকার(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best);
	যদি (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	memset(best, 0, আকার(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &best);
	যদি (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	যদি (!first)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
স্থির শূন্য dump_iface_good(কাঠামো ul_nl_তথ্য *nl,
			    কাঠামো ul_netaddrq_iface *ifaceq)
{
	কাঠামো ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	কাঠামো ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	কাঠামো list_শীর্ষ *li;
	গণনা ul_netaddrq_ip_rating threshold = __ULNETLINK_RATING_MAX - 1;
	গণনা ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool first = true;

	memset(best4, 0, আকার(best4));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, আকার(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	যদি (fthreshold < threshold)
		threshold = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		কাঠামো ul_netaddrq_ip *ipq;

		ipq = list_entry(li, কাঠামো ul_netaddrq_ip, entry);
		যদি (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[threshold])
		{
			যদি (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			অন্যথা
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			যদি (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				যাও অস্থায়ী_cont4;
			}
			অন্যথা
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	অস্থায়ী_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		কাঠামো ul_netaddrq_ip *ipq;

		ipq = list_entry(li, কাঠামো ul_netaddrq_ip, entry);
		যদি (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[threshold])
		{
			যদি (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			অন্যথা
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			যদি (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				যাও অস্থায়ী_cont6;
			}
			অন্যথা
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	অস্থায়ী_cont6:;
	}
	যদি (!first)
		fputs("\n", netout);
}

স্থির শূন্য dump_iface_all(কাঠামো ul_nl_তথ্য *nl,
			   কাঠামো ul_netaddrq_iface *ifaceq)
{
	কাঠামো list_শীর্ষ *li;
	কাঠামো ul_netaddrq_ip *ipq;
	bool first = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_entry(li, কাঠামো ul_netaddrq_ip, entry);
		যদি (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		অন্যথা
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_entry(li, কাঠামো ul_netaddrq_ip, entry);
		যদি (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		অন্যথা
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	যদি (!first)
		fputs("\n", netout);
}

স্থির শূন্য dump_addrq(কাঠামো ul_nl_তথ্য *nl, গণনা addrq_print_mode c)
{
	কাঠামো list_শীর্ষ *li;
	কাঠামো ul_netaddrq_iface *ifaceq;
	গণনা ul_netaddrq_ip_rating threshold;

	নির্বাচন(c)
	{
	ক্ষেত্র ADDRQ_MODE_BESTOFALL:
	{
		কাঠামো ul_netaddrq_iface *best_ifaceq;
		ধ্রুবক অক্ষর *best_ipp;

		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET,
						    &threshold, &best_ifaceq);
		যদি (best_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET6,
						    &threshold, &best_ifaceq);
		যদি (best_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
	}
	বিরতি;
	ক্ষেত্র ADDRQ_MODE_BEST:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, কাঠামো ul_netaddrq_iface, entry);

			dump_iface_best(nl, ifaceq);
		}
	}
	বিরতি;
	ক্ষেত্র ADDRQ_MODE_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, কাঠামো ul_netaddrq_iface, entry);

			dump_iface_good(nl, ifaceq);
		}
	}
	বিরতি;
	ক্ষেত্র ADDRQ_MODE_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, কাঠামো ul_netaddrq_iface, entry);

			dump_iface_all(nl,ifaceq);
		}
	বিরতি;
	}
}

স্থির পূর্ণসংখ্যা callback_post(কাঠামো ul_nl_তথ্য *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	যদি (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		যদি (!(UL_NETADDRQ_তথ্য(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_তথ্য(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			ফেরত 0;
		}
		UL_NETADDRQ_তথ্য(nl)->ifaces_change_4 = false;
		UL_NETADDRQ_তথ্য(nl)->ifaces_change_6 = false;
		fputs("\n\nNetwork change detected:\n", netout);
		fputs("\nbest address:\n", netout);
		dump_addrq(nl, ADDRQ_MODE_BESTOFALL);

		fputs("\nbest addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_MODE_BEST);

		fputs("\ngood addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_MODE_GOOD);

		fputs("\nall addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_MODE_ALL);
	}
	ফেরত 0;
}

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc __attribute__((__unused__)), অক্ষর *argv[] __attribute__((__unused__)))
{
	পূর্ণসংখ্যা rc = 1;
	পূর্ণসংখ্যা ulrc;
	কাঠামো ul_nl_তথ্য nl;
	FILE *out = stdout;
	কাঠামো ul_netaddrq_iface *ifaceq;
	ধ্রুবক অক্ষর *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	যদি ((ul_netaddrq_init(&nl, ফাঁকা, callback_post, (শূন্য *)out)))
		ফেরত -1;

	/* Dump addresses */
	যদি (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		ফেরত -1;
	যদি (ul_nl_request_dump(&nl, RTM_GETADDR))
		যাও error;
	যদি (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		যাও error;
	fputs("RTM_GETADDR dump finished.", out);

	/* Example of several types of dump */
	fputs("\nbest address:\n", out);
	dump_addrq(&nl, ADDRQ_MODE_BESTOFALL);

	fputs("\nbest addresses dump:\n", out);
	dump_addrq(&nl, ADDRQ_MODE_BEST);

	fputs("\ngood addresses dump:\n", out);
	dump_addrq(&nl, ADDRQ_MODE_GOOD);

	fputs("\nall addresses dump:\n", out);
	dump_addrq(&nl, ADDRQ_MODE_ALL);

	fputs("\naddresses for interface ", out);
	যদি ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	অন্যথা
		fprintf(out, "%s not found.", ifname);

	/* Monitor further changes */
	fputs("\nGoing to monitor mode.\n", out);

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
	যদি (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	যদি ((ul_nl_close(&nl)))
		rc = 1;
	ফেরত rc;
}
#endif /* TEST_PROGRAM_NETADDRQ */
