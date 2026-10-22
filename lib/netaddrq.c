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

#અન્તર્ભાવ <net/if.h>
#અન્તર્ભાવ <netinet/in.h>
#અન્તર્ભાવ <linux/rtnetlink.h>
#અન્તર્ભાવ <linux/if_addr.h>
#અન્તર્ભાવ "netaddrq.h"
#અન્તર્ભાવ "list.h"
#અન્તર્ભાવ "debug.h"

/* Maximal number of interfaces. The algorithm has a quadratic complexity,
 * don't overflood it. */
અચલ પૂર્ણાંક max_ifaces = 12;

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETADDRQ_DEBUG_HELP	(1 << 0)
#define ULNETADDRQ_DEBUG_INIT	(1 << 1)
#define ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#define ULNETADDRQ_DEBUG_LIST	(1 << 3)
#define ULNETADDRQ_DEBUG_BEST	(1 << 4)

સ્થિર UL_DEBUG_DEFINE_MASK(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_BEST,	"searching best address" },

	{ ખાલી, 0 }
};

#define DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

સ્થિર શૂન્ય netaddrq_init_debug(શૂન્ય)
{
	જો (netaddrq_debug_mask)
		પાછા;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

સ્થિર અંતઃરેખ ગણના ul_netaddrq_ip_rating
evaluate_ip_quality(સંરચના ul_nl_addr *addr) {
	ગણના ul_netaddrq_ip_rating quality;

	પસંદગી (addr->ifa_scope) {
	સ્થિતિ RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		તોડો;
	સ્થિતિ RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		તોડો;
	સ્થિતિ RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		તોડો;
	મૂળભૂત:
		quality = ULNETLINK_RATING_BAD;
		તોડો;
	}
	જો (addr->ifa_flags & IFA_F_TEMPORARY) {
		જો (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	પાછા quality;
}

#define DBG_CASE(x) સ્થિતિ x: str = #x; તોડો
#define DBG_CASE_DEF8(x) મૂળભૂત: snprintf(strx+2, 3, "%02hhx", x); str = strx; તોડો
સ્થિર અક્ષર *ip_rating_as_string(ગણના ul_netaddrq_ip_rating q)
{
	અક્ષર *str;
	સ્થિર અક્ષર strx[5] = "0x";
	પસંદગી (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	પાછા str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
સ્થિર પૂર્ણાંક callback_addrq(સંરચના ul_nl_data *nl)
{
	સંરચના ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	સંરચના list_head *li, *ipq_list;
	સંરચના ul_netaddrq_iface *ifaceq = ખાલી;
	સંરચના ul_netaddrq_ip *ipq = ખાલી;
	પૂર્ણાંક rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_address(&(nl->addr)),
			      nl->addr.ifname));
	જો (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		જો ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		સંરચના ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_entry(li, સંરચના ul_netaddrq_iface, entry);
		જો (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			તોડો;
		}
		addrq->nifaces++;
	}

	જો (ifaceq == ખાલી) {
		જો (nl->rtm_event) {
			જો (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->overflow = true;
				પાછા UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			જો (!(ifaceq = malloc(કદ(સંરચના ul_netaddrq_iface))))
				પાછા -ENOMEM;
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_4));
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			જો (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				મુક્ત(ifaceq);
				પાછા -ENOMEM;
			}
			list_add_tail(&(ifaceq->entry), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} નહીંતર {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			પાછા UL_NL_SOFT_ERROR;
		}
	}
	જો (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} નહીંતર {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		સંરચના ul_netaddrq_ip *ipqq;

		ipqq = list_entry(li, સંરચના ul_netaddrq_ip, entry);
		જો (ipqq->addr->address_len == nl->addr.address_len)
			જો (!memcmp(ipqq->addr->address, nl->addr.address,
				   nl->addr.address_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				તોડો;
			}
	}
	જો (ipq == ખાલી) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	જો (UL_NL_IS_RTM_NEW(nl)) {
		સંરચના ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		જો (!addr) {
			rc = -ENOMEM;
			જાઓ error;
		}
		જો (ipq == ખાલી) {
			જો (!(ipq = malloc(કદ(સંરચના ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				જાઓ error;
			}
			ipq->addr = addr;
			list_add_tail(&(ipq->entry), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} નહીંતર {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_address(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} નહીંતર {
		/* UL_NL_RTM_DEL */
		જો (ipq == ખાલી)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			પાછા UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->entry));
		ul_nl_addr_free(ipq->addr);
		મુક્ત(ipq);
	error:
		જો (list_empty(&(ifaceq->ip_quality_list_4)) &&
		    list_empty(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->entry));
			addrq->nifaces--;
			મુક્ત(ifaceq->ifname);
			મુક્ત(ifaceq);
		}
	}
	જો (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		જો ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	પાછા rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
પૂર્ણાંક ul_netaddrq_init(સંરચના ul_nl_data *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, શૂન્ય *data)
{
	સંરચના ul_netaddrq_data *addrq;

	netaddrq_init_debug();
	જો (!(nl->data_addr = calloc(1, કદ(સંરચના ul_netaddrq_data))))
		પાછા -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_DATA(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_data = data;
	INIT_LIST_HEAD(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	પાછા 0;
}

ગણના ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(સંરચના list_head *ipq_list,
			   સંરચના ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX])
{
	સંરચના list_head *li;
	સંરચના ul_netaddrq_ip *ipq;
	ગણના ul_netaddrq_ip_rating threshold;

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_entry(li, સંરચના ul_netaddrq_ip, entry);

		જો (!(*best)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*best)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(BEST, (*best),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_address(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*best)[ipq->quality] = ipq;
		}

		જો (ipq->quality < threshold)
		{
			threshold = ipq->quality;
			DBG(BEST,
			    ul_debug("threshold %s", ip_rating_as_string(threshold)));

		}
	}
	પાછા threshold;
}

ગણના ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(સંરચના ul_nl_data *nl,
		     સંરચના ul_netaddrq_iface **best_ifaceq,
		     સંરચના ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	સંરચના ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	સંરચના list_head *li;
	સંરચના ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	ગણના ul_netaddrq_ip_rating threshold;

	જો (ifa_family == AF_INET) {
		ipqo = offsetof(સંરચના ul_netaddrq_iface, ip_quality_list_4);
	} નહીંતર {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(સંરચના ul_netaddrq_iface, ip_quality_list_6);
	}

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		સંરચના list_head *ipq_list;
		ગણના ul_netaddrq_ip_rating t;

		ifaceq = list_entry(li, સંરચના ul_netaddrq_iface, entry);

		ipq_list = (સંરચના list_head*)((અક્ષર*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, best);
		જો (t < threshold)
		{
			DBG_OBJ(BEST, *best,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*best_ifaceq = ifaceq;
			threshold = t;
		}
	}
	પાછા threshold;
}

અચલ અક્ષર *ul_netaddrq_get_best_ipp(સંરચના ul_nl_data *nl,
				     uint8_t ifa_family,
				     ગણના ul_netaddrq_ip_rating *threshold,
				     સંરચના ul_netaddrq_iface **best_ifaceq)
{
	સંરચના ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];

	memset(best, 0, કદ(best));
	*threshold = ul_netaddrq_bestaddr(nl, best_ifaceq, &best, ifa_family);
	જો (*threshold != __ULNETLINK_RATING_MAX)
		પાછા ul_nl_addr_ntop_address(best[*threshold]->addr);
	પાછા ખાલી;
}

સંરચના ul_netaddrq_iface *ul_netaddrq_iface_by_name(અચલ સંરચના ul_nl_data *nl,
						    અચલ અક્ષર *ifname)
{
	સંરચના list_head *li;
	સંરચના ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_entry(li, સંરચના ul_netaddrq_iface, entry);

		જો (!strcmp(ifaceq->ifname, ifname))
			પાછા ifaceq;
	}
	પાછા ખાલી;
}

#ifdef TEST_PROGRAM_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
ગણના addrq_print_mode {
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
સ્થિર શૂન્ય dump_iface_best(સંરચના ul_nl_data *nl,
			    સંરચના ul_netaddrq_iface *ifaceq)
{
	સંરચના ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];
	ગણના ul_netaddrq_ip_rating threshold;
	bool first = true;

	memset(best, 0, કદ(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best);
	જો (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	memset(best, 0, કદ(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &best);
	જો (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	જો (!first)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
સ્થિર શૂન્ય dump_iface_good(સંરચના ul_nl_data *nl,
			    સંરચના ul_netaddrq_iface *ifaceq)
{
	સંરચના ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	સંરચના ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	સંરચના list_head *li;
	ગણના ul_netaddrq_ip_rating threshold = __ULNETLINK_RATING_MAX - 1;
	ગણના ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool first = true;

	memset(best4, 0, કદ(best4));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, કદ(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	જો (fthreshold < threshold)
		threshold = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		સંરચના ul_netaddrq_ip *ipq;

		ipq = list_entry(li, સંરચના ul_netaddrq_ip, entry);
		જો (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[threshold])
		{
			જો (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			નહીંતર
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			જો (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				જાઓ temp_cont4;
			}
			નહીંતર
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		સંરચના ul_netaddrq_ip *ipq;

		ipq = list_entry(li, સંરચના ul_netaddrq_ip, entry);
		જો (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[threshold])
		{
			જો (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			નહીંતર
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			જો (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				જાઓ temp_cont6;
			}
			નહીંતર
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont6:;
	}
	જો (!first)
		fputs("\n", netout);
}

સ્થિર શૂન્ય dump_iface_all(સંરચના ul_nl_data *nl,
			   સંરચના ul_netaddrq_iface *ifaceq)
{
	સંરચના list_head *li;
	સંરચના ul_netaddrq_ip *ipq;
	bool first = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_entry(li, સંરચના ul_netaddrq_ip, entry);
		જો (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		નહીંતર
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_entry(li, સંરચના ul_netaddrq_ip, entry);
		જો (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		નહીંતર
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	જો (!first)
		fputs("\n", netout);
}

સ્થિર શૂન્ય dump_addrq(સંરચના ul_nl_data *nl, ગણના addrq_print_mode c)
{
	સંરચના list_head *li;
	સંરચના ul_netaddrq_iface *ifaceq;
	ગણના ul_netaddrq_ip_rating threshold;

	પસંદગી(c)
	{
	સ્થિતિ ADDRQ_MODE_BESTOFALL:
	{
		સંરચના ul_netaddrq_iface *best_ifaceq;
		અચલ અક્ષર *best_ipp;

		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET,
						    &threshold, &best_ifaceq);
		જો (best_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET6,
						    &threshold, &best_ifaceq);
		જો (best_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
	}
	તોડો;
	સ્થિતિ ADDRQ_MODE_BEST:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, સંરચના ul_netaddrq_iface, entry);

			dump_iface_best(nl, ifaceq);
		}
	}
	તોડો;
	સ્થિતિ ADDRQ_MODE_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, સંરચના ul_netaddrq_iface, entry);

			dump_iface_good(nl, ifaceq);
		}
	}
	તોડો;
	સ્થિતિ ADDRQ_MODE_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, સંરચના ul_netaddrq_iface, entry);

			dump_iface_all(nl,ifaceq);
		}
	તોડો;
	}
}

સ્થિર પૂર્ણાંક callback_post(સંરચના ul_nl_data *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	જો (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		જો (!(UL_NETADDRQ_DATA(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_DATA(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			પાછા 0;
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
	પાછા 0;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc __attribute__((__unused__)), અક્ષર *argv[] __attribute__((__unused__)))
{
	પૂર્ણાંક rc = 1;
	પૂર્ણાંક ulrc;
	સંરચના ul_nl_data nl;
	FILE *out = stdout;
	સંરચના ul_netaddrq_iface *ifaceq;
	અચલ અક્ષર *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	જો ((ul_netaddrq_init(&nl, ખાલી, callback_post, (શૂન્ય *)out)))
		પાછા -1;

	/* Dump addresses */
	જો (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		પાછા -1;
	જો (ul_nl_request_dump(&nl, RTM_GETADDR))
		જાઓ error;
	જો (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		જાઓ error;
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
	જો ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	નહીંતર
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
	જો (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	જો ((ul_nl_close(&nl)))
		rc = 1;
	પાછા rc;
}
#endif /* TEST_PROGRAM_NETADDRQ */
