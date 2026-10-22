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
ধ্রুব পূর্ণাঙ্ক max_ifaces = 12;

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

	{ রিক্ত, 0 }
};

#define DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

স্থির শূন্য netaddrq_init_debug(শূন্য)
{
	যদি (netaddrq_debug_mask)
		প্রত্যাগম;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

স্থির অন্তঃস্থ গণনা ul_netaddrq_ip_rating
evaluate_ip_quality(সংরচনা ul_nl_addr *addr) {
	গণনা ul_netaddrq_ip_rating quality;

	বিকল্প (addr->ifa_scope) {
	পক্ষ RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		ভঙ্গ;
	পক্ষ RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		ভঙ্গ;
	পক্ষ RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		ভঙ্গ;
	স্বভাব:
		quality = ULNETLINK_RATING_BAD;
		ভঙ্গ;
	}
	যদি (addr->ifa_flags & IFA_F_TEMPORARY) {
		যদি (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	প্রত্যাগম quality;
}

#define DBG_CASE(x) পক্ষ x: str = #x; ভঙ্গ
#define DBG_CASE_DEF8(x) স্বভাব: snprintf(strx+2, 3, "%02hhx", x); str = strx; ভঙ্গ
স্থির বর্ণ *ip_rating_as_string(গণনা ul_netaddrq_ip_rating q)
{
	বর্ণ *str;
	স্থির বর্ণ strx[5] = "0x";
	বিকল্প (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	প্রত্যাগম str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
স্থির পূর্ণাঙ্ক callback_addrq(সংরচনা ul_nl_data *nl)
{
	সংরচনা ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	সংরচনা list_head *li, *ipq_list;
	সংরচনা ul_netaddrq_iface *ifaceq = রিক্ত;
	সংরচনা ul_netaddrq_ip *ipq = রিক্ত;
	পূর্ণাঙ্ক rc;
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
		সংরচনা ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_entry(li, সংরচনা ul_netaddrq_iface, entry);
		যদি (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			ভঙ্গ;
		}
		addrq->nifaces++;
	}

	যদি (ifaceq == রিক্ত) {
		যদি (nl->rtm_event) {
			যদি (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->overflow = true;
				প্রত্যাগম UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			যদি (!(ifaceq = malloc(পরিমাণ(সংরচনা ul_netaddrq_iface))))
				প্রত্যাগম -ENOMEM;
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_4));
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			যদি (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				মুক্ত(ifaceq);
				প্রত্যাগম -ENOMEM;
			}
			list_add_tail(&(ifaceq->entry), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} অন্যথা {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			প্রত্যাগম UL_NL_SOFT_ERROR;
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
		সংরচনা ul_netaddrq_ip *ipqq;

		ipqq = list_entry(li, সংরচনা ul_netaddrq_ip, entry);
		যদি (ipqq->addr->address_len == nl->addr.address_len)
			যদি (!memcmp(ipqq->addr->address, nl->addr.address,
				   nl->addr.address_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				ভঙ্গ;
			}
	}
	যদি (ipq == রিক্ত) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	যদি (UL_NL_IS_RTM_NEW(nl)) {
		সংরচনা ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		যদি (!addr) {
			rc = -ENOMEM;
			গচ্ছ error;
		}
		যদি (ipq == রিক্ত) {
			যদি (!(ipq = malloc(পরিমাণ(সংরচনা ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				গচ্ছ error;
			}
			ipq->addr = addr;
			list_add_tail(&(ipq->entry), ipq_list);
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
		যদি (ipq == রিক্ত)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			প্রত্যাগম UL_NL_SOFT_ERROR;
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
	প্রত্যাগম rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
পূর্ণাঙ্ক ul_netaddrq_init(সংরচনা ul_nl_data *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, শূন্য *data)
{
	সংরচনা ul_netaddrq_data *addrq;

	netaddrq_init_debug();
	যদি (!(nl->data_addr = calloc(1, পরিমাণ(সংরচনা ul_netaddrq_data))))
		প্রত্যাগম -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_DATA(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_data = data;
	INIT_LIST_HEAD(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	প্রত্যাগম 0;
}

গণনা ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(সংরচনা list_head *ipq_list,
			   সংরচনা ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX])
{
	সংরচনা list_head *li;
	সংরচনা ul_netaddrq_ip *ipq;
	গণনা ul_netaddrq_ip_rating threshold;

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_entry(li, সংরচনা ul_netaddrq_ip, entry);

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
	প্রত্যাগম threshold;
}

গণনা ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(সংরচনা ul_nl_data *nl,
		     সংরচনা ul_netaddrq_iface **best_ifaceq,
		     সংরচনা ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	সংরচনা ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	সংরচনা list_head *li;
	সংরচনা ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	গণনা ul_netaddrq_ip_rating threshold;

	যদি (ifa_family == AF_INET) {
		ipqo = offsetof(সংরচনা ul_netaddrq_iface, ip_quality_list_4);
	} অন্যথা {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(সংরচনা ul_netaddrq_iface, ip_quality_list_6);
	}

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		সংরচনা list_head *ipq_list;
		গণনা ul_netaddrq_ip_rating t;

		ifaceq = list_entry(li, সংরচনা ul_netaddrq_iface, entry);

		ipq_list = (সংরচনা list_head*)((বর্ণ*)ifaceq + ipqo);

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
	প্রত্যাগম threshold;
}

ধ্রুব বর্ণ *ul_netaddrq_get_best_ipp(সংরচনা ul_nl_data *nl,
				     uint8_t ifa_family,
				     গণনা ul_netaddrq_ip_rating *threshold,
				     সংরচনা ul_netaddrq_iface **best_ifaceq)
{
	সংরচনা ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];

	memset(best, 0, পরিমাণ(best));
	*threshold = ul_netaddrq_bestaddr(nl, best_ifaceq, &best, ifa_family);
	যদি (*threshold != __ULNETLINK_RATING_MAX)
		প্রত্যাগম ul_nl_addr_ntop_address(best[*threshold]->addr);
	প্রত্যাগম রিক্ত;
}

সংরচনা ul_netaddrq_iface *ul_netaddrq_iface_by_name(ধ্রুব সংরচনা ul_nl_data *nl,
						    ধ্রুব বর্ণ *ifname)
{
	সংরচনা list_head *li;
	সংরচনা ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_entry(li, সংরচনা ul_netaddrq_iface, entry);

		যদি (!strcmp(ifaceq->ifname, ifname))
			প্রত্যাগম ifaceq;
	}
	প্রত্যাগম রিক্ত;
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
#define netout (FILE *)(UL_NETADDRQ_DATA(nl)->callback_data)

/* This example uses separate threshold for IPv4 and IPv6, so the best IPv4 and
 * best IPv6 addresses are printed. */
স্থির শূন্য dump_iface_best(সংরচনা ul_nl_data *nl,
			    সংরচনা ul_netaddrq_iface *ifaceq)
{
	সংরচনা ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];
	গণনা ul_netaddrq_ip_rating threshold;
	bool first = true;

	memset(best, 0, পরিমাণ(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best);
	যদি (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	memset(best, 0, পরিমাণ(best));
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
স্থির শূন্য dump_iface_good(সংরচনা ul_nl_data *nl,
			    সংরচনা ul_netaddrq_iface *ifaceq)
{
	সংরচনা ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	সংরচনা ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	সংরচনা list_head *li;
	গণনা ul_netaddrq_ip_rating threshold = __ULNETLINK_RATING_MAX - 1;
	গণনা ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool first = true;

	memset(best4, 0, পরিমাণ(best4));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, পরিমাণ(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	যদি (fthreshold < threshold)
		threshold = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		সংরচনা ul_netaddrq_ip *ipq;

		ipq = list_entry(li, সংরচনা ul_netaddrq_ip, entry);
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
				গচ্ছ temp_cont4;
			}
			অন্যথা
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		সংরচনা ul_netaddrq_ip *ipq;

		ipq = list_entry(li, সংরচনা ul_netaddrq_ip, entry);
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
				গচ্ছ temp_cont6;
			}
			অন্যথা
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont6:;
	}
	যদি (!first)
		fputs("\n", netout);
}

স্থির শূন্য dump_iface_all(সংরচনা ul_nl_data *nl,
			   সংরচনা ul_netaddrq_iface *ifaceq)
{
	সংরচনা list_head *li;
	সংরচনা ul_netaddrq_ip *ipq;
	bool first = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_entry(li, সংরচনা ul_netaddrq_ip, entry);
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
		ipq = list_entry(li, সংরচনা ul_netaddrq_ip, entry);
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

স্থির শূন্য dump_addrq(সংরচনা ul_nl_data *nl, গণনা addrq_print_mode c)
{
	সংরচনা list_head *li;
	সংরচনা ul_netaddrq_iface *ifaceq;
	গণনা ul_netaddrq_ip_rating threshold;

	বিকল্প(c)
	{
	পক্ষ ADDRQ_MODE_BESTOFALL:
	{
		সংরচনা ul_netaddrq_iface *best_ifaceq;
		ধ্রুব বর্ণ *best_ipp;

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
	ভঙ্গ;
	পক্ষ ADDRQ_MODE_BEST:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, সংরচনা ul_netaddrq_iface, entry);

			dump_iface_best(nl, ifaceq);
		}
	}
	ভঙ্গ;
	পক্ষ ADDRQ_MODE_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, সংরচনা ul_netaddrq_iface, entry);

			dump_iface_good(nl, ifaceq);
		}
	}
	ভঙ্গ;
	পক্ষ ADDRQ_MODE_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, সংরচনা ul_netaddrq_iface, entry);

			dump_iface_all(nl,ifaceq);
		}
	ভঙ্গ;
	}
}

স্থির পূর্ণাঙ্ক callback_post(সংরচনা ul_nl_data *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	যদি (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		যদি (!(UL_NETADDRQ_DATA(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_DATA(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			প্রত্যাগম 0;
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
	প্রত্যাগম 0;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc __attribute__((__unused__)), বর্ণ *argv[] __attribute__((__unused__)))
{
	পূর্ণাঙ্ক rc = 1;
	পূর্ণাঙ্ক ulrc;
	সংরচনা ul_nl_data nl;
	FILE *out = stdout;
	সংরচনা ul_netaddrq_iface *ifaceq;
	ধ্রুব বর্ণ *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	যদি ((ul_netaddrq_init(&nl, রিক্ত, callback_post, (শূন্য *)out)))
		প্রত্যাগম -1;

	/* Dump addresses */
	যদি (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		প্রত্যাগম -1;
	যদি (ul_nl_request_dump(&nl, RTM_GETADDR))
		গচ্ছ error;
	যদি (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		গচ্ছ error;
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
	প্রত্যাগম rc;
}
#endif /* TEST_PROGRAM_NETADDRQ */
