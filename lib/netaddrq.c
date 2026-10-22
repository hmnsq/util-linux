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

#ଅନ୍ତର୍ଭାଵ <net/if.h>
#ଅନ୍ତର୍ଭାଵ <netinet/in.h>
#ଅନ୍ତର୍ଭାଵ <linux/rtnetlink.h>
#ଅନ୍ତର୍ଭାଵ <linux/if_addr.h>
#ଅନ୍ତର୍ଭାଵ "netaddrq.h"
#ଅନ୍ତର୍ଭାଵ "list.h"
#ଅନ୍ତର୍ଭାଵ "debug.h"

/* Maximal number of interfaces. The algorithm has a quadratic complexity,
 * don't overflood it. */
ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_ifaces = 12;

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETADDRQ_DEBUG_HELP	(1 << 0)
#define ULNETADDRQ_DEBUG_INIT	(1 << 1)
#define ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#define ULNETADDRQ_DEBUG_LIST	(1 << 3)
#define ULNETADDRQ_DEBUG_BEST	(1 << 4)

ସ୍ଥିର UL_DEBUG_DEFINE_MASK(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_BEST,	"searching best address" },

	{ ଖାଲି, 0 }
};

#define DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

ସ୍ଥିର ଶୂନ୍ୟ netaddrq_init_debug(ଶୂନ୍ୟ)
{
	ଯଦି (netaddrq_debug_mask)
		ଫେରନ୍ତୁ;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଗଣନା ul_netaddrq_ip_rating
evaluate_ip_quality(ସଂରଚନା ul_nl_addr *addr) {
	ଗଣନା ul_netaddrq_ip_rating quality;

	ବାଛ (addr->ifa_scope) {
	ସ୍ଥିତି RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		quality = ULNETLINK_RATING_BAD;
		ଭାଙ୍ଗ;
	}
	ଯଦି (addr->ifa_flags & IFA_F_TEMPORARY) {
		ଯଦି (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	ଫେରନ୍ତୁ quality;
}

#define DBG_CASE(x) ସ୍ଥିତି x: str = #x; ଭାଙ୍ଗ
#define DBG_CASE_DEF8(x) ପୂର୍ବନିର୍ଦ୍ଧାରିତ: snprintf(strx+2, 3, "%02hhx", x); str = strx; ଭାଙ୍ଗ
ସ୍ଥିର ଅକ୍ଷର *ip_rating_as_string(ଗଣନା ul_netaddrq_ip_rating q)
{
	ଅକ୍ଷର *str;
	ସ୍ଥିର ଅକ୍ଷର strx[5] = "0x";
	ବାଛ (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	ଫେରନ୍ତୁ str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା callback_addrq(ସଂରଚନା ul_nl_data *nl)
{
	ସଂରଚନା ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	ସଂରଚନା list_head *li, *ipq_list;
	ସଂରଚନା ul_netaddrq_iface *ifaceq = ଖାଲି;
	ସଂରଚନା ul_netaddrq_ip *ipq = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_address(&(nl->addr)),
			      nl->addr.ifname));
	ଯଦି (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		ଯଦି ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		ସଂରଚନା ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_entry(li, ସଂରଚନା ul_netaddrq_iface, entry);
		ଯଦି (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			ଭାଙ୍ଗ;
		}
		addrq->nifaces++;
	}

	ଯଦି (ifaceq == ଖାଲି) {
		ଯଦି (nl->rtm_event) {
			ଯଦି (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->overflow = true;
				ଫେରନ୍ତୁ UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			ଯଦି (!(ifaceq = malloc(ଆକାର(ସଂରଚନା ul_netaddrq_iface))))
				ଫେରନ୍ତୁ -ENOMEM;
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_4));
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			ଯଦି (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				ମୁକ୍ତ(ifaceq);
				ଫେରନ୍ତୁ -ENOMEM;
			}
			list_add_tail(&(ifaceq->entry), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} ଅନ୍ୟଥା {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			ଫେରନ୍ତୁ UL_NL_SOFT_ERROR;
		}
	}
	ଯଦି (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} ଅନ୍ୟଥା {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		ସଂରଚନା ul_netaddrq_ip *ipqq;

		ipqq = list_entry(li, ସଂରଚନା ul_netaddrq_ip, entry);
		ଯଦି (ipqq->addr->address_len == nl->addr.address_len)
			ଯଦି (!memcmp(ipqq->addr->address, nl->addr.address,
				   nl->addr.address_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				ଭାଙ୍ଗ;
			}
	}
	ଯଦି (ipq == ଖାଲି) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	ଯଦି (UL_NL_IS_RTM_NEW(nl)) {
		ସଂରଚନା ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		ଯଦି (!addr) {
			rc = -ENOMEM;
			ଯାଅ error;
		}
		ଯଦି (ipq == ଖାଲି) {
			ଯଦି (!(ipq = malloc(ଆକାର(ସଂରଚନା ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				ଯାଅ error;
			}
			ipq->addr = addr;
			list_add_tail(&(ipq->entry), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} ଅନ୍ୟଥା {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_address(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} ଅନ୍ୟଥା {
		/* UL_NL_RTM_DEL */
		ଯଦି (ipq == ଖାଲି)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			ଫେରନ୍ତୁ UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->entry));
		ul_nl_addr_free(ipq->addr);
		ମୁକ୍ତ(ipq);
	error:
		ଯଦି (list_empty(&(ifaceq->ip_quality_list_4)) &&
		    list_empty(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->entry));
			addrq->nifaces--;
			ମୁକ୍ତ(ifaceq->ifname);
			ମୁକ୍ତ(ifaceq);
		}
	}
	ଯଦି (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		ଯଦି ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	ଫେରନ୍ତୁ rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_netaddrq_init(ସଂରଚନା ul_nl_data *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, ଶୂନ୍ୟ *data)
{
	ସଂରଚନା ul_netaddrq_data *addrq;

	netaddrq_init_debug();
	ଯଦି (!(nl->data_addr = calloc(1, ଆକାର(ସଂରଚନା ul_netaddrq_data))))
		ଫେରନ୍ତୁ -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_DATA(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_data = data;
	INIT_LIST_HEAD(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	ଫେରନ୍ତୁ 0;
}

ଗଣନା ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(ସଂରଚନା list_head *ipq_list,
			   ସଂରଚନା ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX])
{
	ସଂରଚନା list_head *li;
	ସଂରଚନା ul_netaddrq_ip *ipq;
	ଗଣନା ul_netaddrq_ip_rating threshold;

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_entry(li, ସଂରଚନା ul_netaddrq_ip, entry);

		ଯଦି (!(*best)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*best)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(BEST, (*best),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_address(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*best)[ipq->quality] = ipq;
		}

		ଯଦି (ipq->quality < threshold)
		{
			threshold = ipq->quality;
			DBG(BEST,
			    ul_debug("threshold %s", ip_rating_as_string(threshold)));

		}
	}
	ଫେରନ୍ତୁ threshold;
}

ଗଣନା ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(ସଂରଚନା ul_nl_data *nl,
		     ସଂରଚନା ul_netaddrq_iface **best_ifaceq,
		     ସଂରଚନା ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	ସଂରଚନା ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	ସଂରଚନା list_head *li;
	ସଂରଚନା ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	ଗଣନା ul_netaddrq_ip_rating threshold;

	ଯଦି (ifa_family == AF_INET) {
		ipqo = offsetof(ସଂରଚନା ul_netaddrq_iface, ip_quality_list_4);
	} ଅନ୍ୟଥା {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(ସଂରଚନା ul_netaddrq_iface, ip_quality_list_6);
	}

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		ସଂରଚନା list_head *ipq_list;
		ଗଣନା ul_netaddrq_ip_rating t;

		ifaceq = list_entry(li, ସଂରଚନା ul_netaddrq_iface, entry);

		ipq_list = (ସଂରଚନା list_head*)((ଅକ୍ଷର*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, best);
		ଯଦି (t < threshold)
		{
			DBG_OBJ(BEST, *best,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*best_ifaceq = ifaceq;
			threshold = t;
		}
	}
	ଫେରନ୍ତୁ threshold;
}

ଅଚଳ ଅକ୍ଷର *ul_netaddrq_get_best_ipp(ସଂରଚନା ul_nl_data *nl,
				     uint8_t ifa_family,
				     ଗଣନା ul_netaddrq_ip_rating *threshold,
				     ସଂରଚନା ul_netaddrq_iface **best_ifaceq)
{
	ସଂରଚନା ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];

	memset(best, 0, ଆକାର(best));
	*threshold = ul_netaddrq_bestaddr(nl, best_ifaceq, &best, ifa_family);
	ଯଦି (*threshold != __ULNETLINK_RATING_MAX)
		ଫେରନ୍ତୁ ul_nl_addr_ntop_address(best[*threshold]->addr);
	ଫେରନ୍ତୁ ଖାଲି;
}

ସଂରଚନା ul_netaddrq_iface *ul_netaddrq_iface_by_name(ଅଚଳ ସଂରଚନା ul_nl_data *nl,
						    ଅଚଳ ଅକ୍ଷର *ifname)
{
	ସଂରଚନା list_head *li;
	ସଂରଚନା ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_entry(li, ସଂରଚନା ul_netaddrq_iface, entry);

		ଯଦି (!strcmp(ifaceq->ifname, ifname))
			ଫେରନ୍ତୁ ifaceq;
	}
	ଫେରନ୍ତୁ ଖାଲି;
}

#ifdef TEST_PROGRAM_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
ଗଣନା addrq_print_mode {
	ADDRQ_MODE_BESTOFALL,	/* Best address of all interfaces */
	ADDRQ_MODE_BEST,	/* Best address per interface */
	ADDRQ_MODE_GOOD,	/* All global or site addresses, if none, the
				 * longest living temporary, if none, link */
	ADDRQ_MODE_ALL		/* All available addresses */
};

/* In our example addrq->callback_data is a simple FILE *. In more complex
 * programs it could be a pointer to an arbitrary struct */
#define netout (FILE *)(UL_NETADDRQ_DATA(nl)->callback_data)

/* This example uses separate threshold for IPv4 and IPv6, so the best IPv4 and
 * best IPv6 addresses are printed. */
ସ୍ଥିର ଶୂନ୍ୟ dump_iface_best(ସଂରଚନା ul_nl_data *nl,
			    ସଂରଚନା ul_netaddrq_iface *ifaceq)
{
	ସଂରଚନା ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];
	ଗଣନା ul_netaddrq_ip_rating threshold;
	bool first = true;

	memset(best, 0, ଆକାର(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best);
	ଯଦି (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	memset(best, 0, ଆକାର(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &best);
	ଯଦି (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	ଯଦି (!first)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
ସ୍ଥିର ଶୂନ୍ୟ dump_iface_good(ସଂରଚନା ul_nl_data *nl,
			    ସଂରଚନା ul_netaddrq_iface *ifaceq)
{
	ସଂରଚନା ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	ସଂରଚନା ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	ସଂରଚନା list_head *li;
	ଗଣନା ul_netaddrq_ip_rating threshold = __ULNETLINK_RATING_MAX - 1;
	ଗଣନା ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool first = true;

	memset(best4, 0, ଆକାର(best4));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, ଆକାର(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	ଯଦି (fthreshold < threshold)
		threshold = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ସଂରଚନା ul_netaddrq_ip *ipq;

		ipq = list_entry(li, ସଂରଚନା ul_netaddrq_ip, entry);
		ଯଦି (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[threshold])
		{
			ଯଦି (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			ଅନ୍ୟଥା
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			ଯଦି (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				ଯାଅ temp_cont4;
			}
			ଅନ୍ୟଥା
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ସଂରଚନା ul_netaddrq_ip *ipq;

		ipq = list_entry(li, ସଂରଚନା ul_netaddrq_ip, entry);
		ଯଦି (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[threshold])
		{
			ଯଦି (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			ଅନ୍ୟଥା
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			ଯଦି (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				ଯାଅ temp_cont6;
			}
			ଅନ୍ୟଥା
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont6:;
	}
	ଯଦି (!first)
		fputs("\n", netout);
}

ସ୍ଥିର ଶୂନ୍ୟ dump_iface_all(ସଂରଚନା ul_nl_data *nl,
			   ସଂରଚନା ul_netaddrq_iface *ifaceq)
{
	ସଂରଚନା list_head *li;
	ସଂରଚନା ul_netaddrq_ip *ipq;
	bool first = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_entry(li, ସଂରଚନା ul_netaddrq_ip, entry);
		ଯଦି (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		ଅନ୍ୟଥା
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_entry(li, ସଂରଚନା ul_netaddrq_ip, entry);
		ଯଦି (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		ଅନ୍ୟଥା
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	ଯଦି (!first)
		fputs("\n", netout);
}

ସ୍ଥିର ଶୂନ୍ୟ dump_addrq(ସଂରଚନା ul_nl_data *nl, ଗଣନା addrq_print_mode c)
{
	ସଂରଚନା list_head *li;
	ସଂରଚନା ul_netaddrq_iface *ifaceq;
	ଗଣନା ul_netaddrq_ip_rating threshold;

	ବାଛ(c)
	{
	ସ୍ଥିତି ADDRQ_MODE_BESTOFALL:
	{
		ସଂରଚନା ul_netaddrq_iface *best_ifaceq;
		ଅଚଳ ଅକ୍ଷର *best_ipp;

		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET,
						    &threshold, &best_ifaceq);
		ଯଦି (best_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET6,
						    &threshold, &best_ifaceq);
		ଯଦି (best_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
	}
	ଭାଙ୍ଗ;
	ସ୍ଥିତି ADDRQ_MODE_BEST:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, ସଂରଚନା ul_netaddrq_iface, entry);

			dump_iface_best(nl, ifaceq);
		}
	}
	ଭାଙ୍ଗ;
	ସ୍ଥିତି ADDRQ_MODE_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, ସଂରଚନା ul_netaddrq_iface, entry);

			dump_iface_good(nl, ifaceq);
		}
	}
	ଭାଙ୍ଗ;
	ସ୍ଥିତି ADDRQ_MODE_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, ସଂରଚନା ul_netaddrq_iface, entry);

			dump_iface_all(nl,ifaceq);
		}
	ଭାଙ୍ଗ;
	}
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା callback_post(ସଂରଚନା ul_nl_data *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	ଯଦି (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		ଯଦି (!(UL_NETADDRQ_DATA(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_DATA(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			ଫେରନ୍ତୁ 0;
		}
		UL_NETADDRQ_DATA(nl)->ifaces_change_4 = false;
		UL_NETADDRQ_DATA(nl)->ifaces_change_6 = false;
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
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc __attribute__((__unused__)), ଅକ୍ଷର *argv[] __attribute__((__unused__)))
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 1;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ulrc;
	ସଂରଚନା ul_nl_data nl;
	FILE *out = stdout;
	ସଂରଚନା ul_netaddrq_iface *ifaceq;
	ଅଚଳ ଅକ୍ଷର *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	ଯଦି ((ul_netaddrq_init(&nl, ଖାଲି, callback_post, (ଶୂନ୍ୟ *)out)))
		ଫେରନ୍ତୁ -1;

	/* Dump addresses */
	ଯଦି (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		ଫେରନ୍ତୁ -1;
	ଯଦି (ul_nl_request_dump(&nl, RTM_GETADDR))
		ଯାଅ error;
	ଯଦି (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		ଯାଅ error;
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
	ଯଦି ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	ଅନ୍ୟଥା
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
	ଯଦି (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	ଯଦି ((ul_nl_close(&nl)))
		rc = 1;
	ଫେରନ୍ତୁ rc;
}
#endif /* TEST_PROGRAM_NETADDRQ */
