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
#define ULNETADDRQ_DEBUG_HELP	(1 << 0)
#define ULNETADDRQ_DEBUG_INIT	(1 << 1)
#define ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#define ULNETADDRQ_DEBUG_LIST	(1 << 3)
#define ULNETADDRQ_DEBUG_BEST	(1 << 4)

ساكن UL_DEBUG_DEFINE_MASK(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_BEST,	"searching best address" },

	{ عدم, 0 }
};

#define DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

ساكن فراغ netaddrq_init_debug(فراغ)
{
	אם (netaddrq_debug_mask)
		החזר;

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
	אם (addr->ifa_flags & IFA_F_TEMPORARY) {
		אם (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	החזר quality;
}

#define DBG_CASE(x) حالة x: str = #x; اكسر
#define DBG_CASE_DEF8(x) افتراضي: snprintf(strx+2, 3, "%02hhx", x); str = strx; اكسر
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
	החזר str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
ساكن صحيح callback_addrq(بنية ul_nl_data *nl)
{
	بنية ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	بنية list_head *li, *ipq_list;
	بنية ul_netaddrq_iface *ifaceq = عدم;
	بنية ul_netaddrq_ip *ipq = عدم;
	صحيح rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_address(&(nl->addr)),
			      nl->addr.ifname));
	אם (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		אם ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		بنية ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_entry(li, بنية ul_netaddrq_iface, entry);
		אם (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			اكسر;
		}
		addrq->nifaces++;
	}

	אם (ifaceq == عدم) {
		אם (nl->rtm_event) {
			אם (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->overflow = true;
				החזר UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			אם (!(ifaceq = malloc(حجم(بنية ul_netaddrq_iface))))
				החזר -ENOMEM;
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_4));
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			אם (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				حرر(ifaceq);
				החזר -ENOMEM;
			}
			list_add_tail(&(ifaceq->entry), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} אחרת {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			החזר UL_NL_SOFT_ERROR;
		}
	}
	אם (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} אחרת {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		بنية ul_netaddrq_ip *ipqq;

		ipqq = list_entry(li, بنية ul_netaddrq_ip, entry);
		אם (ipqq->addr->address_len == nl->addr.address_len)
			אם (!memcmp(ipqq->addr->address, nl->addr.address,
				   nl->addr.address_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				اكسر;
			}
	}
	אם (ipq == عدم) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	אם (UL_NL_IS_RTM_NEW(nl)) {
		بنية ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		אם (!addr) {
			rc = -ENOMEM;
			اذهب error;
		}
		אם (ipq == عدم) {
			אם (!(ipq = malloc(حجم(بنية ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				اذهب error;
			}
			ipq->addr = addr;
			list_add_tail(&(ipq->entry), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} אחרת {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_address(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} אחרת {
		/* UL_NL_RTM_DEL */
		אם (ipq == عدم)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			החזר UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->entry));
		ul_nl_addr_free(ipq->addr);
		حرر(ipq);
	error:
		אם (list_empty(&(ifaceq->ip_quality_list_4)) &&
		    list_empty(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->entry));
			addrq->nifaces--;
			حرر(ifaceq->ifname);
			حرر(ifaceq);
		}
	}
	אם (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		אם ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	החזר rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
صحيح ul_netaddrq_init(بنية ul_nl_data *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, فراغ *data)
{
	بنية ul_netaddrq_data *addrq;

	netaddrq_init_debug();
	אם (!(nl->data_addr = calloc(1, حجم(بنية ul_netaddrq_data))))
		החזר -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_DATA(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_data = data;
	INIT_LIST_HEAD(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	החזר 0;
}

تعداد ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(بنية list_head *ipq_list,
			   بنية ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX])
{
	بنية list_head *li;
	بنية ul_netaddrq_ip *ipq;
	تعداد ul_netaddrq_ip_rating threshold;

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_entry(li, بنية ul_netaddrq_ip, entry);

		אם (!(*best)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*best)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(BEST, (*best),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_address(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*best)[ipq->quality] = ipq;
		}

		אם (ipq->quality < threshold)
		{
			threshold = ipq->quality;
			DBG(BEST,
			    ul_debug("threshold %s", ip_rating_as_string(threshold)));

		}
	}
	החזר threshold;
}

تعداد ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(بنية ul_nl_data *nl,
		     بنية ul_netaddrq_iface **best_ifaceq,
		     بنية ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	بنية ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	بنية list_head *li;
	بنية ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	تعداد ul_netaddrq_ip_rating threshold;

	אם (ifa_family == AF_INET) {
		ipqo = offsetof(بنية ul_netaddrq_iface, ip_quality_list_4);
	} אחרת {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(بنية ul_netaddrq_iface, ip_quality_list_6);
	}

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		بنية list_head *ipq_list;
		تعداد ul_netaddrq_ip_rating t;

		ifaceq = list_entry(li, بنية ul_netaddrq_iface, entry);

		ipq_list = (بنية list_head*)((حرف*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, best);
		אם (t < threshold)
		{
			DBG_OBJ(BEST, *best,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*best_ifaceq = ifaceq;
			threshold = t;
		}
	}
	החזר threshold;
}

ثابت حرف *ul_netaddrq_get_best_ipp(بنية ul_nl_data *nl,
				     uint8_t ifa_family,
				     تعداد ul_netaddrq_ip_rating *threshold,
				     بنية ul_netaddrq_iface **best_ifaceq)
{
	بنية ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];

	memset(best, 0, حجم(best));
	*threshold = ul_netaddrq_bestaddr(nl, best_ifaceq, &best, ifa_family);
	אם (*threshold != __ULNETLINK_RATING_MAX)
		החזר ul_nl_addr_ntop_address(best[*threshold]->addr);
	החזר عدم;
}

بنية ul_netaddrq_iface *ul_netaddrq_iface_by_name(ثابت بنية ul_nl_data *nl,
						    ثابت حرف *ifname)
{
	بنية list_head *li;
	بنية ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_entry(li, بنية ul_netaddrq_iface, entry);

		אם (!strcmp(ifaceq->ifname, ifname))
			החזר ifaceq;
	}
	החזר عدم;
}

#ifdef TEST_PROGRAM_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
تعداد addrq_print_mode {
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
ساكن فراغ dump_iface_best(بنية ul_nl_data *nl,
			    بنية ul_netaddrq_iface *ifaceq)
{
	بنية ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];
	تعداد ul_netaddrq_ip_rating threshold;
	bool first = true;

	memset(best, 0, حجم(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best);
	אם (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	memset(best, 0, حجم(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &best);
	אם (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	אם (!first)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
ساكن فراغ dump_iface_good(بنية ul_nl_data *nl,
			    بنية ul_netaddrq_iface *ifaceq)
{
	بنية ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	بنية ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	بنية list_head *li;
	تعداد ul_netaddrq_ip_rating threshold = __ULNETLINK_RATING_MAX - 1;
	تعداد ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool first = true;

	memset(best4, 0, حجم(best4));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, حجم(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	אם (fthreshold < threshold)
		threshold = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		بنية ul_netaddrq_ip *ipq;

		ipq = list_entry(li, بنية ul_netaddrq_ip, entry);
		אם (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[threshold])
		{
			אם (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			אחרת
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			אם (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				اذهب temp_cont4;
			}
			אחרת
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		بنية ul_netaddrq_ip *ipq;

		ipq = list_entry(li, بنية ul_netaddrq_ip, entry);
		אם (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[threshold])
		{
			אם (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			אחרת
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			אם (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				اذهب temp_cont6;
			}
			אחרת
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont6:;
	}
	אם (!first)
		fputs("\n", netout);
}

ساكن فراغ dump_iface_all(بنية ul_nl_data *nl,
			   بنية ul_netaddrq_iface *ifaceq)
{
	بنية list_head *li;
	بنية ul_netaddrq_ip *ipq;
	bool first = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_entry(li, بنية ul_netaddrq_ip, entry);
		אם (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		אחרת
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_entry(li, بنية ul_netaddrq_ip, entry);
		אם (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		אחרת
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	אם (!first)
		fputs("\n", netout);
}

ساكن فراغ dump_addrq(بنية ul_nl_data *nl, تعداد addrq_print_mode c)
{
	بنية list_head *li;
	بنية ul_netaddrq_iface *ifaceq;
	تعداد ul_netaddrq_ip_rating threshold;

	بدل(c)
	{
	حالة ADDRQ_MODE_BESTOFALL:
	{
		بنية ul_netaddrq_iface *best_ifaceq;
		ثابت حرف *best_ipp;

		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET,
						    &threshold, &best_ifaceq);
		אם (best_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET6,
						    &threshold, &best_ifaceq);
		אם (best_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
	}
	اكسر;
	حالة ADDRQ_MODE_BEST:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, بنية ul_netaddrq_iface, entry);

			dump_iface_best(nl, ifaceq);
		}
	}
	اكسر;
	حالة ADDRQ_MODE_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, بنية ul_netaddrq_iface, entry);

			dump_iface_good(nl, ifaceq);
		}
	}
	اكسر;
	حالة ADDRQ_MODE_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, بنية ul_netaddrq_iface, entry);

			dump_iface_all(nl,ifaceq);
		}
	اكسر;
	}
}

ساكن صحيح callback_post(بنية ul_nl_data *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	אם (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		אם (!(UL_NETADDRQ_DATA(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_DATA(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			החזר 0;
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
	החזר 0;
}

صحيح main(صحيح argc __attribute__((__unused__)), حرف *argv[] __attribute__((__unused__)))
{
	صحيح rc = 1;
	صحيح ulrc;
	بنية ul_nl_data nl;
	FILE *out = stdout;
	بنية ul_netaddrq_iface *ifaceq;
	ثابت حرف *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	אם ((ul_netaddrq_init(&nl, عدم, callback_post, (فراغ *)out)))
		החזר -1;

	/* Dump addresses */
	אם (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		החזר -1;
	אם (ul_nl_request_dump(&nl, RTM_GETADDR))
		اذهب error;
	אם (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		اذهب error;
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
	אם ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	אחרת
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
	אם (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	אם ((ul_nl_close(&nl)))
		rc = 1;
	החזר rc;
}
#endif /* TEST_PROGRAM_NETADDRQ */
