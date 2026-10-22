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
#define ULNETADDRQ_DEBUG_HELP	(1 << 0)
#define ULNETADDRQ_DEBUG_INIT	(1 << 1)
#define ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#define ULNETADDRQ_DEBUG_LIST	(1 << 3)
#define ULNETADDRQ_DEBUG_BEST	(1 << 4)

ایستا UL_DEBUG_DEFINE_MASK(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_BEST,	"searching best address" },

	{ پوچ, 0 }
};

#define DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

ایستا تهی netaddrq_init_debug(تهی)
{
	اگر (netaddrq_debug_mask)
		بازگشت;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

ایستا درونخطی شمارش ul_netaddrq_ip_rating
evaluate_ip_quality(ساختار ul_nl_addr *addr) {
	شمارش ul_netaddrq_ip_rating quality;

	انتخاب (addr->ifa_scope) {
	حالت RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		بشکن;
	حالت RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		بشکن;
	حالت RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		بشکن;
	پیشفرض:
		quality = ULNETLINK_RATING_BAD;
		بشکن;
	}
	اگر (addr->ifa_flags & IFA_F_TEMPORARY) {
		اگر (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	بازگشت quality;
}

#define DBG_CASE(x) حالت x: str = #x; بشکن
#define DBG_CASE_DEF8(x) پیشفرض: snprintf(strx+2, 3, "%02hhx", x); str = strx; بشکن
ایستا نویسه *ip_rating_as_string(شمارش ul_netaddrq_ip_rating q)
{
	نویسه *str;
	ایستا نویسه strx[5] = "0x";
	انتخاب (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	بازگشت str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
ایستا صحیح callback_addrq(ساختار ul_nl_data *nl)
{
	ساختار ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	ساختار list_head *li, *ipq_list;
	ساختار ul_netaddrq_iface *ifaceq = پوچ;
	ساختار ul_netaddrq_ip *ipq = پوچ;
	صحیح rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_address(&(nl->addr)),
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
		ساختار ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_entry(li, ساختار ul_netaddrq_iface, entry);
		اگر (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			بشکن;
		}
		addrq->nifaces++;
	}

	اگر (ifaceq == پوچ) {
		اگر (nl->rtm_event) {
			اگر (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->overflow = true;
				بازگشت UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			اگر (!(ifaceq = malloc(اندازه(ساختار ul_netaddrq_iface))))
				بازگشت -ENOMEM;
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_4));
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			اگر (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				آزاد(ifaceq);
				بازگشت -ENOMEM;
			}
			list_add_tail(&(ifaceq->entry), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} وگرنه {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			بازگشت UL_NL_SOFT_ERROR;
		}
	}
	اگر (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} وگرنه {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		ساختار ul_netaddrq_ip *ipqq;

		ipqq = list_entry(li, ساختار ul_netaddrq_ip, entry);
		اگر (ipqq->addr->address_len == nl->addr.address_len)
			اگر (!memcmp(ipqq->addr->address, nl->addr.address,
				   nl->addr.address_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				بشکن;
			}
	}
	اگر (ipq == پوچ) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	اگر (UL_NL_IS_RTM_NEW(nl)) {
		ساختار ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		اگر (!addr) {
			rc = -ENOMEM;
			برو error;
		}
		اگر (ipq == پوچ) {
			اگر (!(ipq = malloc(اندازه(ساختار ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				برو error;
			}
			ipq->addr = addr;
			list_add_tail(&(ipq->entry), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} وگرنه {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_address(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} وگرنه {
		/* UL_NL_RTM_DEL */
		اگر (ipq == پوچ)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			بازگشت UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->entry));
		ul_nl_addr_free(ipq->addr);
		آزاد(ipq);
	error:
		اگر (list_empty(&(ifaceq->ip_quality_list_4)) &&
		    list_empty(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->entry));
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
	بازگشت rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
صحیح ul_netaddrq_init(ساختار ul_nl_data *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, تهی *data)
{
	ساختار ul_netaddrq_data *addrq;

	netaddrq_init_debug();
	اگر (!(nl->data_addr = calloc(1, اندازه(ساختار ul_netaddrq_data))))
		بازگشت -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_DATA(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_data = data;
	INIT_LIST_HEAD(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	بازگشت 0;
}

شمارش ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(ساختار list_head *ipq_list,
			   ساختار ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX])
{
	ساختار list_head *li;
	ساختار ul_netaddrq_ip *ipq;
	شمارش ul_netaddrq_ip_rating threshold;

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_entry(li, ساختار ul_netaddrq_ip, entry);

		اگر (!(*best)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*best)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(BEST, (*best),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_address(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*best)[ipq->quality] = ipq;
		}

		اگر (ipq->quality < threshold)
		{
			threshold = ipq->quality;
			DBG(BEST,
			    ul_debug("threshold %s", ip_rating_as_string(threshold)));

		}
	}
	بازگشت threshold;
}

شمارش ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(ساختار ul_nl_data *nl,
		     ساختار ul_netaddrq_iface **best_ifaceq,
		     ساختار ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	ساختار ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	ساختار list_head *li;
	ساختار ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	شمارش ul_netaddrq_ip_rating threshold;

	اگر (ifa_family == AF_INET) {
		ipqo = offsetof(ساختار ul_netaddrq_iface, ip_quality_list_4);
	} وگرنه {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(ساختار ul_netaddrq_iface, ip_quality_list_6);
	}

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		ساختار list_head *ipq_list;
		شمارش ul_netaddrq_ip_rating t;

		ifaceq = list_entry(li, ساختار ul_netaddrq_iface, entry);

		ipq_list = (ساختار list_head*)((نویسه*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, best);
		اگر (t < threshold)
		{
			DBG_OBJ(BEST, *best,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*best_ifaceq = ifaceq;
			threshold = t;
		}
	}
	بازگشت threshold;
}

ثابت نویسه *ul_netaddrq_get_best_ipp(ساختار ul_nl_data *nl,
				     uint8_t ifa_family,
				     شمارش ul_netaddrq_ip_rating *threshold,
				     ساختار ul_netaddrq_iface **best_ifaceq)
{
	ساختار ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];

	memset(best, 0, اندازه(best));
	*threshold = ul_netaddrq_bestaddr(nl, best_ifaceq, &best, ifa_family);
	اگر (*threshold != __ULNETLINK_RATING_MAX)
		بازگشت ul_nl_addr_ntop_address(best[*threshold]->addr);
	بازگشت پوچ;
}

ساختار ul_netaddrq_iface *ul_netaddrq_iface_by_name(ثابت ساختار ul_nl_data *nl,
						    ثابت نویسه *ifname)
{
	ساختار list_head *li;
	ساختار ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_entry(li, ساختار ul_netaddrq_iface, entry);

		اگر (!strcmp(ifaceq->ifname, ifname))
			بازگشت ifaceq;
	}
	بازگشت پوچ;
}

#ifdef TEST_PROGRAM_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
شمارش addrq_print_mode {
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
ایستا تهی dump_iface_best(ساختار ul_nl_data *nl,
			    ساختار ul_netaddrq_iface *ifaceq)
{
	ساختار ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];
	شمارش ul_netaddrq_ip_rating threshold;
	bool first = true;

	memset(best, 0, اندازه(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best);
	اگر (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	memset(best, 0, اندازه(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &best);
	اگر (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	اگر (!first)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
ایستا تهی dump_iface_good(ساختار ul_nl_data *nl,
			    ساختار ul_netaddrq_iface *ifaceq)
{
	ساختار ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	ساختار ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	ساختار list_head *li;
	شمارش ul_netaddrq_ip_rating threshold = __ULNETLINK_RATING_MAX - 1;
	شمارش ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool first = true;

	memset(best4, 0, اندازه(best4));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, اندازه(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	اگر (fthreshold < threshold)
		threshold = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ساختار ul_netaddrq_ip *ipq;

		ipq = list_entry(li, ساختار ul_netaddrq_ip, entry);
		اگر (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[threshold])
		{
			اگر (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			وگرنه
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			اگر (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				برو temp_cont4;
			}
			وگرنه
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ساختار ul_netaddrq_ip *ipq;

		ipq = list_entry(li, ساختار ul_netaddrq_ip, entry);
		اگر (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[threshold])
		{
			اگر (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			وگرنه
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			اگر (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				برو temp_cont6;
			}
			وگرنه
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont6:;
	}
	اگر (!first)
		fputs("\n", netout);
}

ایستا تهی dump_iface_all(ساختار ul_nl_data *nl,
			   ساختار ul_netaddrq_iface *ifaceq)
{
	ساختار list_head *li;
	ساختار ul_netaddrq_ip *ipq;
	bool first = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_entry(li, ساختار ul_netaddrq_ip, entry);
		اگر (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		وگرنه
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_entry(li, ساختار ul_netaddrq_ip, entry);
		اگر (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		وگرنه
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	اگر (!first)
		fputs("\n", netout);
}

ایستا تهی dump_addrq(ساختار ul_nl_data *nl, شمارش addrq_print_mode c)
{
	ساختار list_head *li;
	ساختار ul_netaddrq_iface *ifaceq;
	شمارش ul_netaddrq_ip_rating threshold;

	انتخاب(c)
	{
	حالت ADDRQ_MODE_BESTOFALL:
	{
		ساختار ul_netaddrq_iface *best_ifaceq;
		ثابت نویسه *best_ipp;

		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET,
						    &threshold, &best_ifaceq);
		اگر (best_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET6,
						    &threshold, &best_ifaceq);
		اگر (best_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
	}
	بشکن;
	حالت ADDRQ_MODE_BEST:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, ساختار ul_netaddrq_iface, entry);

			dump_iface_best(nl, ifaceq);
		}
	}
	بشکن;
	حالت ADDRQ_MODE_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, ساختار ul_netaddrq_iface, entry);

			dump_iface_good(nl, ifaceq);
		}
	}
	بشکن;
	حالت ADDRQ_MODE_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, ساختار ul_netaddrq_iface, entry);

			dump_iface_all(nl,ifaceq);
		}
	بشکن;
	}
}

ایستا صحیح callback_post(ساختار ul_nl_data *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	اگر (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		اگر (!(UL_NETADDRQ_DATA(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_DATA(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			بازگشت 0;
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
	بازگشت 0;
}

صحیح اصلی(صحیح argc __attribute__((__unused__)), نویسه *argv[] __attribute__((__unused__)))
{
	صحیح rc = 1;
	صحیح ulrc;
	ساختار ul_nl_data nl;
	FILE *out = stdout;
	ساختار ul_netaddrq_iface *ifaceq;
	ثابت نویسه *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	اگر ((ul_netaddrq_init(&nl, پوچ, callback_post, (تهی *)out)))
		بازگشت -1;

	/* Dump addresses */
	اگر (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		بازگشت -1;
	اگر (ul_nl_request_dump(&nl, RTM_GETADDR))
		برو error;
	اگر (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		برو error;
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
	اگر ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	وگرنه
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
	اگر (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	اگر ((ul_nl_close(&nl)))
		rc = 1;
	بازگشت rc;
}
#endif /* TEST_PROGRAM_NETADDRQ */
