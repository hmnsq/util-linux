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

#ಅನ್ತರ್ಭಾವ <net/if.h>
#ಅನ್ತರ್ಭಾವ <netinet/in.h>
#ಅನ್ತರ್ಭಾವ <linux/rtnetlink.h>
#ಅನ್ತರ್ಭಾವ <linux/if_addr.h>
#ಅನ್ತರ್ಭಾವ "netaddrq.h"
#ಅನ್ತರ್ಭಾವ "list.h"
#ಅನ್ತರ್ಭಾವ "debug.h"

/* Maximal number of interfaces. The algorithm has a quadratic complexity,
 * don't overflood it. */
ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ max_ifaces = 12;

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETADDRQ_DEBUG_HELP	(1 << 0)
#define ULNETADDRQ_DEBUG_INIT	(1 << 1)
#define ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#define ULNETADDRQ_DEBUG_LIST	(1 << 3)
#define ULNETADDRQ_DEBUG_BEST	(1 << 4)

ಸ್ಥಿರ UL_DEBUG_DEFINE_MASK(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_BEST,	"searching best address" },

	{ ರಿಕ್ತ, 0 }
};

#define DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

ಸ್ಥಿರ ಶೂನ್ಯ netaddrq_init_debug(ಶೂನ್ಯ)
{
	ಯದಿ (netaddrq_debug_mask)
		ಪ್ರತ್ಯಾಗಮ;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಗಣನಾ ul_netaddrq_ip_rating
evaluate_ip_quality(ಸಂರಚನಾ ul_nl_addr *addr) {
	ಗಣನಾ ul_netaddrq_ip_rating quality;

	ವಿಕಲ್ಪ (addr->ifa_scope) {
	ಪಕ್ಷ RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		ಭಙ್ಗ;
	ಪಕ್ಷ RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		ಭಙ್ಗ;
	ಪಕ್ಷ RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		quality = ULNETLINK_RATING_BAD;
		ಭಙ್ಗ;
	}
	ಯದಿ (addr->ifa_flags & IFA_F_TEMPORARY) {
		ಯದಿ (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	ಪ್ರತ್ಯಾಗಮ quality;
}

#define DBG_CASE(x) ಪಕ್ಷ x: str = #x; ಭಙ್ಗ
#define DBG_CASE_DEF8(x) ಸ್ವಭಾವ: snprintf(strx+2, 3, "%02hhx", x); str = strx; ಭಙ್ಗ
ಸ್ಥಿರ ವರ್ಣ *ip_rating_as_string(ಗಣನಾ ul_netaddrq_ip_rating q)
{
	ವರ್ಣ *str;
	ಸ್ಥಿರ ವರ್ಣ strx[5] = "0x";
	ವಿಕಲ್ಪ (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	ಪ್ರತ್ಯಾಗಮ str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ callback_addrq(ಸಂರಚನಾ ul_nl_data *nl)
{
	ಸಂರಚನಾ ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	ಸಂರಚನಾ list_head *li, *ipq_list;
	ಸಂರಚನಾ ul_netaddrq_iface *ifaceq = ರಿಕ್ತ;
	ಸಂರಚನಾ ul_netaddrq_ip *ipq = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_address(&(nl->addr)),
			      nl->addr.ifname));
	ಯದಿ (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		ಯದಿ ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		ಸಂರಚನಾ ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_entry(li, ಸಂರಚನಾ ul_netaddrq_iface, entry);
		ಯದಿ (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			ಭಙ್ಗ;
		}
		addrq->nifaces++;
	}

	ಯದಿ (ifaceq == ರಿಕ್ತ) {
		ಯದಿ (nl->rtm_event) {
			ಯದಿ (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->overflow = true;
				ಪ್ರತ್ಯಾಗಮ UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			ಯದಿ (!(ifaceq = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ ul_netaddrq_iface))))
				ಪ್ರತ್ಯಾಗಮ -ENOMEM;
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_4));
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			ಯದಿ (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				ಮುಕ್ತ(ifaceq);
				ಪ್ರತ್ಯಾಗಮ -ENOMEM;
			}
			list_add_tail(&(ifaceq->entry), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} ಅನ್ಯಥಾ {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			ಪ್ರತ್ಯಾಗಮ UL_NL_SOFT_ERROR;
		}
	}
	ಯದಿ (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} ಅನ್ಯಥಾ {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		ಸಂರಚನಾ ul_netaddrq_ip *ipqq;

		ipqq = list_entry(li, ಸಂರಚನಾ ul_netaddrq_ip, entry);
		ಯದಿ (ipqq->addr->address_len == nl->addr.address_len)
			ಯದಿ (!memcmp(ipqq->addr->address, nl->addr.address,
				   nl->addr.address_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				ಭಙ್ಗ;
			}
	}
	ಯದಿ (ipq == ರಿಕ್ತ) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	ಯದಿ (UL_NL_IS_RTM_NEW(nl)) {
		ಸಂರಚನಾ ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		ಯದಿ (!addr) {
			rc = -ENOMEM;
			ಗಚ್ಛ error;
		}
		ಯದಿ (ipq == ರಿಕ್ತ) {
			ಯದಿ (!(ipq = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				ಗಚ್ಛ error;
			}
			ipq->addr = addr;
			list_add_tail(&(ipq->entry), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} ಅನ್ಯಥಾ {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_address(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} ಅನ್ಯಥಾ {
		/* UL_NL_RTM_DEL */
		ಯದಿ (ipq == ರಿಕ್ತ)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			ಪ್ರತ್ಯಾಗಮ UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->entry));
		ul_nl_addr_free(ipq->addr);
		ಮುಕ್ತ(ipq);
	error:
		ಯದಿ (list_empty(&(ifaceq->ip_quality_list_4)) &&
		    list_empty(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->entry));
			addrq->nifaces--;
			ಮುಕ್ತ(ifaceq->ifname);
			ಮುಕ್ತ(ifaceq);
		}
	}
	ಯದಿ (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		ಯದಿ ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	ಪ್ರತ್ಯಾಗಮ rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
ಪೂರ್ಣಾಙ್ಕ ul_netaddrq_init(ಸಂರಚನಾ ul_nl_data *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, ಶೂನ್ಯ *data)
{
	ಸಂರಚನಾ ul_netaddrq_data *addrq;

	netaddrq_init_debug();
	ಯದಿ (!(nl->data_addr = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ ul_netaddrq_data))))
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_DATA(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_data = data;
	INIT_LIST_HEAD(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಗಣನಾ ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(ಸಂರಚನಾ list_head *ipq_list,
			   ಸಂರಚನಾ ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX])
{
	ಸಂರಚನಾ list_head *li;
	ಸಂರಚನಾ ul_netaddrq_ip *ipq;
	ಗಣನಾ ul_netaddrq_ip_rating threshold;

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_entry(li, ಸಂರಚನಾ ul_netaddrq_ip, entry);

		ಯದಿ (!(*best)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*best)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(BEST, (*best),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_address(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*best)[ipq->quality] = ipq;
		}

		ಯದಿ (ipq->quality < threshold)
		{
			threshold = ipq->quality;
			DBG(BEST,
			    ul_debug("threshold %s", ip_rating_as_string(threshold)));

		}
	}
	ಪ್ರತ್ಯಾಗಮ threshold;
}

ಗಣನಾ ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(ಸಂರಚನಾ ul_nl_data *nl,
		     ಸಂರಚನಾ ul_netaddrq_iface **best_ifaceq,
		     ಸಂರಚನಾ ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	ಸಂರಚನಾ ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	ಸಂರಚನಾ list_head *li;
	ಸಂರಚನಾ ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	ಗಣನಾ ul_netaddrq_ip_rating threshold;

	ಯದಿ (ifa_family == AF_INET) {
		ipqo = offsetof(ಸಂರಚನಾ ul_netaddrq_iface, ip_quality_list_4);
	} ಅನ್ಯಥಾ {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(ಸಂರಚನಾ ul_netaddrq_iface, ip_quality_list_6);
	}

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		ಸಂರಚನಾ list_head *ipq_list;
		ಗಣನಾ ul_netaddrq_ip_rating t;

		ifaceq = list_entry(li, ಸಂರಚನಾ ul_netaddrq_iface, entry);

		ipq_list = (ಸಂರಚನಾ list_head*)((ವರ್ಣ*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, best);
		ಯದಿ (t < threshold)
		{
			DBG_OBJ(BEST, *best,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*best_ifaceq = ifaceq;
			threshold = t;
		}
	}
	ಪ್ರತ್ಯಾಗಮ threshold;
}

ಧ್ರುವ ವರ್ಣ *ul_netaddrq_get_best_ipp(ಸಂರಚನಾ ul_nl_data *nl,
				     uint8_t ifa_family,
				     ಗಣನಾ ul_netaddrq_ip_rating *threshold,
				     ಸಂರಚನಾ ul_netaddrq_iface **best_ifaceq)
{
	ಸಂರಚನಾ ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];

	memset(best, 0, ಪರಿಮಾಣ(best));
	*threshold = ul_netaddrq_bestaddr(nl, best_ifaceq, &best, ifa_family);
	ಯದಿ (*threshold != __ULNETLINK_RATING_MAX)
		ಪ್ರತ್ಯಾಗಮ ul_nl_addr_ntop_address(best[*threshold]->addr);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸಂರಚನಾ ul_netaddrq_iface *ul_netaddrq_iface_by_name(ಧ್ರುವ ಸಂರಚನಾ ul_nl_data *nl,
						    ಧ್ರುವ ವರ್ಣ *ifname)
{
	ಸಂರಚನಾ list_head *li;
	ಸಂರಚನಾ ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_entry(li, ಸಂರಚನಾ ul_netaddrq_iface, entry);

		ಯದಿ (!strcmp(ifaceq->ifname, ifname))
			ಪ್ರತ್ಯಾಗಮ ifaceq;
	}
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

#ifdef TEST_PROGRAM_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
ಗಣನಾ addrq_print_mode {
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
ಸ್ಥಿರ ಶೂನ್ಯ dump_iface_best(ಸಂರಚನಾ ul_nl_data *nl,
			    ಸಂರಚನಾ ul_netaddrq_iface *ifaceq)
{
	ಸಂರಚನಾ ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];
	ಗಣನಾ ul_netaddrq_ip_rating threshold;
	bool first = true;

	memset(best, 0, ಪರಿಮಾಣ(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best);
	ಯದಿ (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	memset(best, 0, ಪರಿಮಾಣ(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &best);
	ಯದಿ (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	ಯದಿ (!first)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
ಸ್ಥಿರ ಶೂನ್ಯ dump_iface_good(ಸಂರಚನಾ ul_nl_data *nl,
			    ಸಂರಚನಾ ul_netaddrq_iface *ifaceq)
{
	ಸಂರಚನಾ ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	ಸಂರಚನಾ ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	ಸಂರಚನಾ list_head *li;
	ಗಣನಾ ul_netaddrq_ip_rating threshold = __ULNETLINK_RATING_MAX - 1;
	ಗಣನಾ ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool first = true;

	memset(best4, 0, ಪರಿಮಾಣ(best4));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, ಪರಿಮಾಣ(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	ಯದಿ (fthreshold < threshold)
		threshold = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ಸಂರಚನಾ ul_netaddrq_ip *ipq;

		ipq = list_entry(li, ಸಂರಚನಾ ul_netaddrq_ip, entry);
		ಯದಿ (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[threshold])
		{
			ಯದಿ (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			ಅನ್ಯಥಾ
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			ಯದಿ (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				ಗಚ್ಛ temp_cont4;
			}
			ಅನ್ಯಥಾ
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ಸಂರಚನಾ ul_netaddrq_ip *ipq;

		ipq = list_entry(li, ಸಂರಚನಾ ul_netaddrq_ip, entry);
		ಯದಿ (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[threshold])
		{
			ಯದಿ (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			ಅನ್ಯಥಾ
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			ಯದಿ (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				ಗಚ್ಛ temp_cont6;
			}
			ಅನ್ಯಥಾ
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont6:;
	}
	ಯದಿ (!first)
		fputs("\n", netout);
}

ಸ್ಥಿರ ಶೂನ್ಯ dump_iface_all(ಸಂರಚನಾ ul_nl_data *nl,
			   ಸಂರಚನಾ ul_netaddrq_iface *ifaceq)
{
	ಸಂರಚನಾ list_head *li;
	ಸಂರಚನಾ ul_netaddrq_ip *ipq;
	bool first = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_entry(li, ಸಂರಚನಾ ul_netaddrq_ip, entry);
		ಯದಿ (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		ಅನ್ಯಥಾ
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_entry(li, ಸಂರಚನಾ ul_netaddrq_ip, entry);
		ಯದಿ (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		ಅನ್ಯಥಾ
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	ಯದಿ (!first)
		fputs("\n", netout);
}

ಸ್ಥಿರ ಶೂನ್ಯ dump_addrq(ಸಂರಚನಾ ul_nl_data *nl, ಗಣನಾ addrq_print_mode c)
{
	ಸಂರಚನಾ list_head *li;
	ಸಂರಚನಾ ul_netaddrq_iface *ifaceq;
	ಗಣನಾ ul_netaddrq_ip_rating threshold;

	ವಿಕಲ್ಪ(c)
	{
	ಪಕ್ಷ ADDRQ_MODE_BESTOFALL:
	{
		ಸಂರಚನಾ ul_netaddrq_iface *best_ifaceq;
		ಧ್ರುವ ವರ್ಣ *best_ipp;

		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET,
						    &threshold, &best_ifaceq);
		ಯದಿ (best_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET6,
						    &threshold, &best_ifaceq);
		ಯದಿ (best_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
	}
	ಭಙ್ಗ;
	ಪಕ್ಷ ADDRQ_MODE_BEST:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, ಸಂರಚನಾ ul_netaddrq_iface, entry);

			dump_iface_best(nl, ifaceq);
		}
	}
	ಭಙ್ಗ;
	ಪಕ್ಷ ADDRQ_MODE_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, ಸಂರಚನಾ ul_netaddrq_iface, entry);

			dump_iface_good(nl, ifaceq);
		}
	}
	ಭಙ್ಗ;
	ಪಕ್ಷ ADDRQ_MODE_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, ಸಂರಚನಾ ul_netaddrq_iface, entry);

			dump_iface_all(nl,ifaceq);
		}
	ಭಙ್ಗ;
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ callback_post(ಸಂರಚನಾ ul_nl_data *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	ಯದಿ (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		ಯದಿ (!(UL_NETADDRQ_DATA(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_DATA(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			ಪ್ರತ್ಯಾಗಮ 0;
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
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc __attribute__((__unused__)), ವರ್ಣ *argv[] __attribute__((__unused__)))
{
	ಪೂರ್ಣಾಙ್ಕ rc = 1;
	ಪೂರ್ಣಾಙ್ಕ ulrc;
	ಸಂರಚನಾ ul_nl_data nl;
	FILE *out = stdout;
	ಸಂರಚನಾ ul_netaddrq_iface *ifaceq;
	ಧ್ರುವ ವರ್ಣ *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	ಯದಿ ((ul_netaddrq_init(&nl, ರಿಕ್ತ, callback_post, (ಶೂನ್ಯ *)out)))
		ಪ್ರತ್ಯಾಗಮ -1;

	/* Dump addresses */
	ಯದಿ (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		ಪ್ರತ್ಯಾಗಮ -1;
	ಯದಿ (ul_nl_request_dump(&nl, RTM_GETADDR))
		ಗಚ್ಛ error;
	ಯದಿ (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		ಗಚ್ಛ error;
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
	ಯದಿ ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	ಅನ್ಯಥಾ
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
	ಯದಿ (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	ಯದಿ ((ul_nl_close(&nl)))
		rc = 1;
	ಪ್ರತ್ಯಾಗಮ rc;
}
#endif /* TEST_PROGRAM_NETADDRQ */
