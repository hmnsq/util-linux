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

#അന്തര്ഭാവ <net/if.h>
#അന്തര്ഭാവ <netinet/in.h>
#അന്തര്ഭാവ <linux/rtnetlink.h>
#അന്തര്ഭാവ <linux/if_addr.h>
#അന്തര്ഭാവ "netaddrq.h"
#അന്തര്ഭാവ "list.h"
#അന്തര്ഭാവ "debug.h"

/* Maximal number of interfaces. The algorithm has a quadratic complexity,
 * don't overflood it. */
സ്ഥിരം പൂർണ്ണസംഖ്യ max_ifaces = 12;

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETADDRQ_DEBUG_HELP	(1 << 0)
#define ULNETADDRQ_DEBUG_INIT	(1 << 1)
#define ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#define ULNETADDRQ_DEBUG_LIST	(1 << 3)
#define ULNETADDRQ_DEBUG_BEST	(1 << 4)

സ്ഥിതം UL_DEBUG_DEFINE_MASK(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_BEST,	"searching best address" },

	{ ശൂന്യ, 0 }
};

#define DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

സ്ഥിതം ശൂന്യം netaddrq_init_debug(ശൂന്യം)
{
	എങ്കിൽ (netaddrq_debug_mask)
		തിരികെ;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

സ്ഥിതം വരിയിൽ എണ്ണൽ ul_netaddrq_ip_rating
evaluate_ip_quality(ഘടന ul_nl_addr *addr) {
	എണ്ണൽ ul_netaddrq_ip_rating quality;

	തിരഞ്ഞെടുപ്പ് (addr->ifa_scope) {
	അവസ്ഥ RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		നിർത്തുക;
	അവസ്ഥ RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		നിർത്തുക;
	അവസ്ഥ RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		quality = ULNETLINK_RATING_BAD;
		നിർത്തുക;
	}
	എങ്കിൽ (addr->ifa_flags & IFA_F_TEMPORARY) {
		എങ്കിൽ (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	തിരികെ quality;
}

#define DBG_CASE(x) അവസ്ഥ x: str = #x; നിർത്തുക
#define DBG_CASE_DEF8(x) സ്ഥിരസ്ഥിതി: snprintf(strx+2, 3, "%02hhx", x); str = strx; നിർത്തുക
സ്ഥിതം അക്ഷരം *ip_rating_as_string(എണ്ണൽ ul_netaddrq_ip_rating q)
{
	അക്ഷരം *str;
	സ്ഥിതം അക്ഷരം strx[5] = "0x";
	തിരഞ്ഞെടുപ്പ് (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	തിരികെ str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
സ്ഥിതം പൂർണ്ണസംഖ്യ callback_addrq(ഘടന ul_nl_data *nl)
{
	ഘടന ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	ഘടന list_head *li, *ipq_list;
	ഘടന ul_netaddrq_iface *ifaceq = ശൂന്യ;
	ഘടന ul_netaddrq_ip *ipq = ശൂന്യ;
	പൂർണ്ണസംഖ്യ rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_address(&(nl->addr)),
			      nl->addr.ifname));
	എങ്കിൽ (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		എങ്കിൽ ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		ഘടന ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_entry(li, ഘടന ul_netaddrq_iface, entry);
		എങ്കിൽ (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			നിർത്തുക;
		}
		addrq->nifaces++;
	}

	എങ്കിൽ (ifaceq == ശൂന്യ) {
		എങ്കിൽ (nl->rtm_event) {
			എങ്കിൽ (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->overflow = true;
				തിരികെ UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			എങ്കിൽ (!(ifaceq = malloc(വലുപ്പം(ഘടന ul_netaddrq_iface))))
				തിരികെ -ENOMEM;
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_4));
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			എങ്കിൽ (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				സ്വതന്ത്രം(ifaceq);
				തിരികെ -ENOMEM;
			}
			list_add_tail(&(ifaceq->entry), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} അല്ലെങ്കിൽ {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			തിരികെ UL_NL_SOFT_ERROR;
		}
	}
	എങ്കിൽ (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} അല്ലെങ്കിൽ {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		ഘടന ul_netaddrq_ip *ipqq;

		ipqq = list_entry(li, ഘടന ul_netaddrq_ip, entry);
		എങ്കിൽ (ipqq->addr->address_len == nl->addr.address_len)
			എങ്കിൽ (!memcmp(ipqq->addr->address, nl->addr.address,
				   nl->addr.address_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				നിർത്തുക;
			}
	}
	എങ്കിൽ (ipq == ശൂന്യ) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	എങ്കിൽ (UL_NL_IS_RTM_NEW(nl)) {
		ഘടന ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		എങ്കിൽ (!addr) {
			rc = -ENOMEM;
			പോകുക error;
		}
		എങ്കിൽ (ipq == ശൂന്യ) {
			എങ്കിൽ (!(ipq = malloc(വലുപ്പം(ഘടന ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				പോകുക error;
			}
			ipq->addr = addr;
			list_add_tail(&(ipq->entry), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} അല്ലെങ്കിൽ {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_address(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} അല്ലെങ്കിൽ {
		/* UL_NL_RTM_DEL */
		എങ്കിൽ (ipq == ശൂന്യ)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			തിരികെ UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->entry));
		ul_nl_addr_free(ipq->addr);
		സ്വതന്ത്രം(ipq);
	error:
		എങ്കിൽ (list_empty(&(ifaceq->ip_quality_list_4)) &&
		    list_empty(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->entry));
			addrq->nifaces--;
			സ്വതന്ത്രം(ifaceq->ifname);
			സ്വതന്ത്രം(ifaceq);
		}
	}
	എങ്കിൽ (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		എങ്കിൽ ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	തിരികെ rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
പൂർണ്ണസംഖ്യ ul_netaddrq_init(ഘടന ul_nl_data *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, ശൂന്യം *data)
{
	ഘടന ul_netaddrq_data *addrq;

	netaddrq_init_debug();
	എങ്കിൽ (!(nl->data_addr = calloc(1, വലുപ്പം(ഘടന ul_netaddrq_data))))
		തിരികെ -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_DATA(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_data = data;
	INIT_LIST_HEAD(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	തിരികെ 0;
}

എണ്ണൽ ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(ഘടന list_head *ipq_list,
			   ഘടന ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX])
{
	ഘടന list_head *li;
	ഘടന ul_netaddrq_ip *ipq;
	എണ്ണൽ ul_netaddrq_ip_rating threshold;

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_entry(li, ഘടന ul_netaddrq_ip, entry);

		എങ്കിൽ (!(*best)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*best)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(BEST, (*best),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_address(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*best)[ipq->quality] = ipq;
		}

		എങ്കിൽ (ipq->quality < threshold)
		{
			threshold = ipq->quality;
			DBG(BEST,
			    ul_debug("threshold %s", ip_rating_as_string(threshold)));

		}
	}
	തിരികെ threshold;
}

എണ്ണൽ ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(ഘടന ul_nl_data *nl,
		     ഘടന ul_netaddrq_iface **best_ifaceq,
		     ഘടന ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	ഘടന ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	ഘടന list_head *li;
	ഘടന ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	എണ്ണൽ ul_netaddrq_ip_rating threshold;

	എങ്കിൽ (ifa_family == AF_INET) {
		ipqo = offsetof(ഘടന ul_netaddrq_iface, ip_quality_list_4);
	} അല്ലെങ്കിൽ {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(ഘടന ul_netaddrq_iface, ip_quality_list_6);
	}

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		ഘടന list_head *ipq_list;
		എണ്ണൽ ul_netaddrq_ip_rating t;

		ifaceq = list_entry(li, ഘടന ul_netaddrq_iface, entry);

		ipq_list = (ഘടന list_head*)((അക്ഷരം*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, best);
		എങ്കിൽ (t < threshold)
		{
			DBG_OBJ(BEST, *best,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*best_ifaceq = ifaceq;
			threshold = t;
		}
	}
	തിരികെ threshold;
}

സ്ഥിരം അക്ഷരം *ul_netaddrq_get_best_ipp(ഘടന ul_nl_data *nl,
				     uint8_t ifa_family,
				     എണ്ണൽ ul_netaddrq_ip_rating *threshold,
				     ഘടന ul_netaddrq_iface **best_ifaceq)
{
	ഘടന ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];

	memset(best, 0, വലുപ്പം(best));
	*threshold = ul_netaddrq_bestaddr(nl, best_ifaceq, &best, ifa_family);
	എങ്കിൽ (*threshold != __ULNETLINK_RATING_MAX)
		തിരികെ ul_nl_addr_ntop_address(best[*threshold]->addr);
	തിരികെ ശൂന്യ;
}

ഘടന ul_netaddrq_iface *ul_netaddrq_iface_by_name(സ്ഥിരം ഘടന ul_nl_data *nl,
						    സ്ഥിരം അക്ഷരം *ifname)
{
	ഘടന list_head *li;
	ഘടന ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_entry(li, ഘടന ul_netaddrq_iface, entry);

		എങ്കിൽ (!strcmp(ifaceq->ifname, ifname))
			തിരികെ ifaceq;
	}
	തിരികെ ശൂന്യ;
}

#ifdef TEST_PROGRAM_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
എണ്ണൽ addrq_print_mode {
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
സ്ഥിതം ശൂന്യം dump_iface_best(ഘടന ul_nl_data *nl,
			    ഘടന ul_netaddrq_iface *ifaceq)
{
	ഘടന ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];
	എണ്ണൽ ul_netaddrq_ip_rating threshold;
	bool first = true;

	memset(best, 0, വലുപ്പം(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best);
	എങ്കിൽ (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	memset(best, 0, വലുപ്പം(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &best);
	എങ്കിൽ (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	എങ്കിൽ (!first)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
സ്ഥിതം ശൂന്യം dump_iface_good(ഘടന ul_nl_data *nl,
			    ഘടന ul_netaddrq_iface *ifaceq)
{
	ഘടന ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	ഘടന ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	ഘടന list_head *li;
	എണ്ണൽ ul_netaddrq_ip_rating threshold = __ULNETLINK_RATING_MAX - 1;
	എണ്ണൽ ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool first = true;

	memset(best4, 0, വലുപ്പം(best4));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, വലുപ്പം(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	എങ്കിൽ (fthreshold < threshold)
		threshold = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ഘടന ul_netaddrq_ip *ipq;

		ipq = list_entry(li, ഘടന ul_netaddrq_ip, entry);
		എങ്കിൽ (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[threshold])
		{
			എങ്കിൽ (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			അല്ലെങ്കിൽ
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			എങ്കിൽ (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				പോകുക temp_cont4;
			}
			അല്ലെങ്കിൽ
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ഘടന ul_netaddrq_ip *ipq;

		ipq = list_entry(li, ഘടന ul_netaddrq_ip, entry);
		എങ്കിൽ (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[threshold])
		{
			എങ്കിൽ (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			അല്ലെങ്കിൽ
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			എങ്കിൽ (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				പോകുക temp_cont6;
			}
			അല്ലെങ്കിൽ
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont6:;
	}
	എങ്കിൽ (!first)
		fputs("\n", netout);
}

സ്ഥിതം ശൂന്യം dump_iface_all(ഘടന ul_nl_data *nl,
			   ഘടന ul_netaddrq_iface *ifaceq)
{
	ഘടന list_head *li;
	ഘടന ul_netaddrq_ip *ipq;
	bool first = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_entry(li, ഘടന ul_netaddrq_ip, entry);
		എങ്കിൽ (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		അല്ലെങ്കിൽ
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_entry(li, ഘടന ul_netaddrq_ip, entry);
		എങ്കിൽ (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		അല്ലെങ്കിൽ
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	എങ്കിൽ (!first)
		fputs("\n", netout);
}

സ്ഥിതം ശൂന്യം dump_addrq(ഘടന ul_nl_data *nl, എണ്ണൽ addrq_print_mode c)
{
	ഘടന list_head *li;
	ഘടന ul_netaddrq_iface *ifaceq;
	എണ്ണൽ ul_netaddrq_ip_rating threshold;

	തിരഞ്ഞെടുപ്പ്(c)
	{
	അവസ്ഥ ADDRQ_MODE_BESTOFALL:
	{
		ഘടന ul_netaddrq_iface *best_ifaceq;
		സ്ഥിരം അക്ഷരം *best_ipp;

		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET,
						    &threshold, &best_ifaceq);
		എങ്കിൽ (best_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET6,
						    &threshold, &best_ifaceq);
		എങ്കിൽ (best_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
	}
	നിർത്തുക;
	അവസ്ഥ ADDRQ_MODE_BEST:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, ഘടന ul_netaddrq_iface, entry);

			dump_iface_best(nl, ifaceq);
		}
	}
	നിർത്തുക;
	അവസ്ഥ ADDRQ_MODE_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, ഘടന ul_netaddrq_iface, entry);

			dump_iface_good(nl, ifaceq);
		}
	}
	നിർത്തുക;
	അവസ്ഥ ADDRQ_MODE_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, ഘടന ul_netaddrq_iface, entry);

			dump_iface_all(nl,ifaceq);
		}
	നിർത്തുക;
	}
}

സ്ഥിതം പൂർണ്ണസംഖ്യ callback_post(ഘടന ul_nl_data *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	എങ്കിൽ (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		എങ്കിൽ (!(UL_NETADDRQ_DATA(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_DATA(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			തിരികെ 0;
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
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc __attribute__((__unused__)), അക്ഷരം *argv[] __attribute__((__unused__)))
{
	പൂർണ്ണസംഖ്യ rc = 1;
	പൂർണ്ണസംഖ്യ ulrc;
	ഘടന ul_nl_data nl;
	FILE *out = stdout;
	ഘടന ul_netaddrq_iface *ifaceq;
	സ്ഥിരം അക്ഷരം *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	എങ്കിൽ ((ul_netaddrq_init(&nl, ശൂന്യ, callback_post, (ശൂന്യം *)out)))
		തിരികെ -1;

	/* Dump addresses */
	എങ്കിൽ (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		തിരികെ -1;
	എങ്കിൽ (ul_nl_request_dump(&nl, RTM_GETADDR))
		പോകുക error;
	എങ്കിൽ (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		പോകുക error;
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
	എങ്കിൽ ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	അല്ലെങ്കിൽ
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
	എങ്കിൽ (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	എങ്കിൽ ((ul_nl_close(&nl)))
		rc = 1;
	തിരികെ rc;
}
#endif /* TEST_PROGRAM_NETADDRQ */
