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

#அந்தர்பாவ <net/if.h>
#அந்தர்பாவ <netinet/in.h>
#அந்தர்பாவ <linux/rtnetlink.h>
#அந்தர்பாவ <linux/if_addr.h>
#அந்தர்பாவ "netaddrq.h"
#அந்தர்பாவ "list.h"
#அந்தர்பாவ "debug.h"

/* Maximal number of interfaces. The algorithm has a quadratic complexity,
 * don't overflood it. */
மாறிலி முழு max_ifaces = 12;

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETADDRQ_DEBUG_HELP	(1 << 0)
#define ULNETADDRQ_DEBUG_INIT	(1 << 1)
#define ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#define ULNETADDRQ_DEBUG_LIST	(1 << 3)
#define ULNETADDRQ_DEBUG_BEST	(1 << 4)

நிலையான UL_DEBUG_DEFINE_MASK(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_BEST,	"searching best address" },

	{ வெற்று, 0 }
};

#define DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

நிலையான வெறுமை netaddrq_init_debug(வெறுமை)
{
	எனில் (netaddrq_debug_mask)
		திரும்பு;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

நிலையான உள்வரி எண்ணிக்கை ul_netaddrq_ip_rating
evaluate_ip_quality(கட்டமைப்பு ul_nl_addr *addr) {
	எண்ணிக்கை ul_netaddrq_ip_rating quality;

	தேர்வு (addr->ifa_scope) {
	நிலை RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		நிறுத்து;
	நிலை RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		நிறுத்து;
	நிலை RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		நிறுத்து;
	இயல்பு:
		quality = ULNETLINK_RATING_BAD;
		நிறுத்து;
	}
	எனில் (addr->ifa_flags & IFA_F_TEMPORARY) {
		எனில் (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	திரும்பு quality;
}

#define DBG_CASE(x) நிலை x: str = #x; நிறுத்து
#define DBG_CASE_DEF8(x) இயல்பு: snprintf(strx+2, 3, "%02hhx", x); str = strx; நிறுத்து
நிலையான எழுத்து *ip_rating_as_string(எண்ணிக்கை ul_netaddrq_ip_rating q)
{
	எழுத்து *str;
	நிலையான எழுத்து strx[5] = "0x";
	தேர்வு (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	திரும்பு str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
நிலையான முழு callback_addrq(கட்டமைப்பு ul_nl_data *nl)
{
	கட்டமைப்பு ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	கட்டமைப்பு list_head *li, *ipq_list;
	கட்டமைப்பு ul_netaddrq_iface *ifaceq = வெற்று;
	கட்டமைப்பு ul_netaddrq_ip *ipq = வெற்று;
	முழு rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_address(&(nl->addr)),
			      nl->addr.ifname));
	எனில் (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		எனில் ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		கட்டமைப்பு ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_entry(li, கட்டமைப்பு ul_netaddrq_iface, entry);
		எனில் (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			நிறுத்து;
		}
		addrq->nifaces++;
	}

	எனில் (ifaceq == வெற்று) {
		எனில் (nl->rtm_event) {
			எனில் (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->overflow = true;
				திரும்பு UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			எனில் (!(ifaceq = malloc(அளவு(கட்டமைப்பு ul_netaddrq_iface))))
				திரும்பு -ENOMEM;
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_4));
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			எனில் (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				விடுவி(ifaceq);
				திரும்பு -ENOMEM;
			}
			list_add_tail(&(ifaceq->entry), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} இல்லையெனில் {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			திரும்பு UL_NL_SOFT_ERROR;
		}
	}
	எனில் (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} இல்லையெனில் {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		கட்டமைப்பு ul_netaddrq_ip *ipqq;

		ipqq = list_entry(li, கட்டமைப்பு ul_netaddrq_ip, entry);
		எனில் (ipqq->addr->address_len == nl->addr.address_len)
			எனில் (!memcmp(ipqq->addr->address, nl->addr.address,
				   nl->addr.address_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				நிறுத்து;
			}
	}
	எனில் (ipq == வெற்று) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	எனில் (UL_NL_IS_RTM_NEW(nl)) {
		கட்டமைப்பு ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		எனில் (!addr) {
			rc = -ENOMEM;
			செல் error;
		}
		எனில் (ipq == வெற்று) {
			எனில் (!(ipq = malloc(அளவு(கட்டமைப்பு ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				செல் error;
			}
			ipq->addr = addr;
			list_add_tail(&(ipq->entry), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} இல்லையெனில் {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_address(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} இல்லையெனில் {
		/* UL_NL_RTM_DEL */
		எனில் (ipq == வெற்று)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			திரும்பு UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->entry));
		ul_nl_addr_free(ipq->addr);
		விடுவி(ipq);
	error:
		எனில் (list_empty(&(ifaceq->ip_quality_list_4)) &&
		    list_empty(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->entry));
			addrq->nifaces--;
			விடுவி(ifaceq->ifname);
			விடுவி(ifaceq);
		}
	}
	எனில் (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		எனில் ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	திரும்பு rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
முழு ul_netaddrq_init(கட்டமைப்பு ul_nl_data *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, வெறுமை *data)
{
	கட்டமைப்பு ul_netaddrq_data *addrq;

	netaddrq_init_debug();
	எனில் (!(nl->data_addr = calloc(1, அளவு(கட்டமைப்பு ul_netaddrq_data))))
		திரும்பு -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_DATA(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_data = data;
	INIT_LIST_HEAD(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	திரும்பு 0;
}

எண்ணிக்கை ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(கட்டமைப்பு list_head *ipq_list,
			   கட்டமைப்பு ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX])
{
	கட்டமைப்பு list_head *li;
	கட்டமைப்பு ul_netaddrq_ip *ipq;
	எண்ணிக்கை ul_netaddrq_ip_rating threshold;

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_entry(li, கட்டமைப்பு ul_netaddrq_ip, entry);

		எனில் (!(*best)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*best)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(BEST, (*best),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_address(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*best)[ipq->quality] = ipq;
		}

		எனில் (ipq->quality < threshold)
		{
			threshold = ipq->quality;
			DBG(BEST,
			    ul_debug("threshold %s", ip_rating_as_string(threshold)));

		}
	}
	திரும்பு threshold;
}

எண்ணிக்கை ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(கட்டமைப்பு ul_nl_data *nl,
		     கட்டமைப்பு ul_netaddrq_iface **best_ifaceq,
		     கட்டமைப்பு ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	கட்டமைப்பு ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	கட்டமைப்பு list_head *li;
	கட்டமைப்பு ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	எண்ணிக்கை ul_netaddrq_ip_rating threshold;

	எனில் (ifa_family == AF_INET) {
		ipqo = offsetof(கட்டமைப்பு ul_netaddrq_iface, ip_quality_list_4);
	} இல்லையெனில் {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(கட்டமைப்பு ul_netaddrq_iface, ip_quality_list_6);
	}

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		கட்டமைப்பு list_head *ipq_list;
		எண்ணிக்கை ul_netaddrq_ip_rating t;

		ifaceq = list_entry(li, கட்டமைப்பு ul_netaddrq_iface, entry);

		ipq_list = (கட்டமைப்பு list_head*)((எழுத்து*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, best);
		எனில் (t < threshold)
		{
			DBG_OBJ(BEST, *best,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*best_ifaceq = ifaceq;
			threshold = t;
		}
	}
	திரும்பு threshold;
}

மாறிலி எழுத்து *ul_netaddrq_get_best_ipp(கட்டமைப்பு ul_nl_data *nl,
				     uint8_t ifa_family,
				     எண்ணிக்கை ul_netaddrq_ip_rating *threshold,
				     கட்டமைப்பு ul_netaddrq_iface **best_ifaceq)
{
	கட்டமைப்பு ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];

	memset(best, 0, அளவு(best));
	*threshold = ul_netaddrq_bestaddr(nl, best_ifaceq, &best, ifa_family);
	எனில் (*threshold != __ULNETLINK_RATING_MAX)
		திரும்பு ul_nl_addr_ntop_address(best[*threshold]->addr);
	திரும்பு வெற்று;
}

கட்டமைப்பு ul_netaddrq_iface *ul_netaddrq_iface_by_name(மாறிலி கட்டமைப்பு ul_nl_data *nl,
						    மாறிலி எழுத்து *ifname)
{
	கட்டமைப்பு list_head *li;
	கட்டமைப்பு ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_entry(li, கட்டமைப்பு ul_netaddrq_iface, entry);

		எனில் (!strcmp(ifaceq->ifname, ifname))
			திரும்பு ifaceq;
	}
	திரும்பு வெற்று;
}

#ifdef TEST_PROGRAM_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
எண்ணிக்கை addrq_print_mode {
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
நிலையான வெறுமை dump_iface_best(கட்டமைப்பு ul_nl_data *nl,
			    கட்டமைப்பு ul_netaddrq_iface *ifaceq)
{
	கட்டமைப்பு ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];
	எண்ணிக்கை ul_netaddrq_ip_rating threshold;
	bool first = true;

	memset(best, 0, அளவு(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best);
	எனில் (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	memset(best, 0, அளவு(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &best);
	எனில் (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	எனில் (!first)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
நிலையான வெறுமை dump_iface_good(கட்டமைப்பு ul_nl_data *nl,
			    கட்டமைப்பு ul_netaddrq_iface *ifaceq)
{
	கட்டமைப்பு ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	கட்டமைப்பு ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	கட்டமைப்பு list_head *li;
	எண்ணிக்கை ul_netaddrq_ip_rating threshold = __ULNETLINK_RATING_MAX - 1;
	எண்ணிக்கை ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool first = true;

	memset(best4, 0, அளவு(best4));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, அளவு(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	எனில் (fthreshold < threshold)
		threshold = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		கட்டமைப்பு ul_netaddrq_ip *ipq;

		ipq = list_entry(li, கட்டமைப்பு ul_netaddrq_ip, entry);
		எனில் (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[threshold])
		{
			எனில் (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			இல்லையெனில்
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			எனில் (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				செல் temp_cont4;
			}
			இல்லையெனில்
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		கட்டமைப்பு ul_netaddrq_ip *ipq;

		ipq = list_entry(li, கட்டமைப்பு ul_netaddrq_ip, entry);
		எனில் (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[threshold])
		{
			எனில் (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			இல்லையெனில்
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			எனில் (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				செல் temp_cont6;
			}
			இல்லையெனில்
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont6:;
	}
	எனில் (!first)
		fputs("\n", netout);
}

நிலையான வெறுமை dump_iface_all(கட்டமைப்பு ul_nl_data *nl,
			   கட்டமைப்பு ul_netaddrq_iface *ifaceq)
{
	கட்டமைப்பு list_head *li;
	கட்டமைப்பு ul_netaddrq_ip *ipq;
	bool first = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_entry(li, கட்டமைப்பு ul_netaddrq_ip, entry);
		எனில் (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		இல்லையெனில்
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_entry(li, கட்டமைப்பு ul_netaddrq_ip, entry);
		எனில் (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		இல்லையெனில்
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	எனில் (!first)
		fputs("\n", netout);
}

நிலையான வெறுமை dump_addrq(கட்டமைப்பு ul_nl_data *nl, எண்ணிக்கை addrq_print_mode c)
{
	கட்டமைப்பு list_head *li;
	கட்டமைப்பு ul_netaddrq_iface *ifaceq;
	எண்ணிக்கை ul_netaddrq_ip_rating threshold;

	தேர்வு(c)
	{
	நிலை ADDRQ_MODE_BESTOFALL:
	{
		கட்டமைப்பு ul_netaddrq_iface *best_ifaceq;
		மாறிலி எழுத்து *best_ipp;

		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET,
						    &threshold, &best_ifaceq);
		எனில் (best_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET6,
						    &threshold, &best_ifaceq);
		எனில் (best_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
	}
	நிறுத்து;
	நிலை ADDRQ_MODE_BEST:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, கட்டமைப்பு ul_netaddrq_iface, entry);

			dump_iface_best(nl, ifaceq);
		}
	}
	நிறுத்து;
	நிலை ADDRQ_MODE_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, கட்டமைப்பு ul_netaddrq_iface, entry);

			dump_iface_good(nl, ifaceq);
		}
	}
	நிறுத்து;
	நிலை ADDRQ_MODE_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, கட்டமைப்பு ul_netaddrq_iface, entry);

			dump_iface_all(nl,ifaceq);
		}
	நிறுத்து;
	}
}

நிலையான முழு callback_post(கட்டமைப்பு ul_nl_data *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	எனில் (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		எனில் (!(UL_NETADDRQ_DATA(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_DATA(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			திரும்பு 0;
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
	திரும்பு 0;
}

முழு முதன்மை(முழு argc __attribute__((__unused__)), எழுத்து *argv[] __attribute__((__unused__)))
{
	முழு rc = 1;
	முழு ulrc;
	கட்டமைப்பு ul_nl_data nl;
	FILE *out = stdout;
	கட்டமைப்பு ul_netaddrq_iface *ifaceq;
	மாறிலி எழுத்து *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	எனில் ((ul_netaddrq_init(&nl, வெற்று, callback_post, (வெறுமை *)out)))
		திரும்பு -1;

	/* Dump addresses */
	எனில் (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		திரும்பு -1;
	எனில் (ul_nl_request_dump(&nl, RTM_GETADDR))
		செல் error;
	எனில் (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		செல் error;
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
	எனில் ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	இல்லையெனில்
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
	எனில் (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	எனில் ((ul_nl_close(&nl)))
		rc = 1;
	திரும்பு rc;
}
#endif /* TEST_PROGRAM_NETADDRQ */
