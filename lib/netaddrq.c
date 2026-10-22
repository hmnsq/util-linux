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

#include <net/if.h>
#include <netinet/in.h>
#include <linux/rtnetlink.h>
#include <linux/if_addr.h>
#include "netaddrq.h"
#include "list.h"
#include "debug.h"

/* Maximal number of interfaces. The algorithm has a quadratic complexity,
 * don't overflood it. */
const ကိန်းပြည့် max_ifaces = 12;

/*
 * Debug stuff (based on include/debug.h)
 */
#define ULNETADDRQ_DEBUG_HELP	(1 << 0)
#define ULNETADDRQ_DEBUG_INIT	(1 << 1)
#define ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#define ULNETADDRQ_DEBUG_LIST	(1 << 3)
#define ULNETADDRQ_DEBUG_BEST	(1 << 4)

static UL_DEBUG_DEFINE_MASK(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_BEST,	"searching best address" },

	{ NULL, 0 }
};

#define DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

static void netaddrq_init_debug(void)
{
	အကယ်၍ (netaddrq_debug_mask)
		ပြန်ပေး;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

static anta_hstha ga_nanaa ul_netaddrq_ip_rating
evaluate_ip_quality(sa_nracanaa ul_nl_addr *addr) {
	ga_nanaa ul_netaddrq_ip_rating quality;

	switch (addr->ifa_scope) {
	pak_sa RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		bha_nga;
	pak_sa RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		bha_nga;
	pak_sa RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		bha_nga;
	default:
		quality = ULNETLINK_RATING_BAD;
		bha_nga;
	}
	အကယ်၍ (addr->ifa_flags & IFA_F_TEMPORARY) {
		အကယ်၍ (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	ပြန်ပေး quality;
}

#define DBG_CASE(x) pak_sa x: str = #x; bha_nga
#define DBG_CASE_DEF8(x) default: snprintf(strx+2, 3, "%02hhx", x); str = strx; bha_nga
static var_na *ip_rating_as_string(ga_nanaa ul_netaddrq_ip_rating q)
{
	var_na *str;
	static var_na strx[5] = "0x";
	switch (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	ပြန်ပေး str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
static ကိန်းပြည့် callback_addrq(sa_nracanaa ul_nl_data *nl)
{
	sa_nracanaa ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	sa_nracanaa list_head *li, *ipq_list;
	sa_nracanaa ul_netaddrq_iface *ifaceq = NULL;
	sa_nracanaa ul_netaddrq_ip *ipq = NULL;
	ကိန်းပြည့် rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_address(&(nl->addr)),
			      nl->addr.ifname));
	အကယ်၍ (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		အကယ်၍ ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		sa_nracanaa ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_entry(li, sa_nracanaa ul_netaddrq_iface, entry);
		အကယ်၍ (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			bha_nga;
		}
		addrq->nifaces++;
	}

	အကယ်၍ (ifaceq == NULL) {
		အကယ်၍ (nl->rtm_event) {
			အကယ်၍ (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->overflow = true;
				ပြန်ပေး UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			အကယ်၍ (!(ifaceq = malloc(parimaa_na(sa_nracanaa ul_netaddrq_iface))))
				ပြန်ပေး -ENOMEM;
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_4));
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			အကယ်၍ (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				free(ifaceq);
				ပြန်ပေး -ENOMEM;
			}
			list_add_tail(&(ifaceq->entry), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} မဟုတ်ပါက {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			ပြန်ပေး UL_NL_SOFT_ERROR;
		}
	}
	အကယ်၍ (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} မဟုတ်ပါက {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		sa_nracanaa ul_netaddrq_ip *ipqq;

		ipqq = list_entry(li, sa_nracanaa ul_netaddrq_ip, entry);
		အကယ်၍ (ipqq->addr->address_len == nl->addr.address_len)
			အကယ်၍ (!memcmp(ipqq->addr->address, nl->addr.address,
				   nl->addr.address_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				bha_nga;
			}
	}
	အကယ်၍ (ipq == NULL) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	အကယ်၍ (UL_NL_IS_RTM_NEW(nl)) {
		sa_nracanaa ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		အကယ်၍ (!addr) {
			rc = -ENOMEM;
			goto error;
		}
		အကယ်၍ (ipq == NULL) {
			အကယ်၍ (!(ipq = malloc(parimaa_na(sa_nracanaa ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				goto error;
			}
			ipq->addr = addr;
			list_add_tail(&(ipq->entry), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} မဟုတ်ပါက {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_address(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} မဟုတ်ပါက {
		/* UL_NL_RTM_DEL */
		အကယ်၍ (ipq == NULL)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			ပြန်ပေး UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->entry));
		ul_nl_addr_free(ipq->addr);
		free(ipq);
	error:
		အကယ်၍ (list_empty(&(ifaceq->ip_quality_list_4)) &&
		    list_empty(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->entry));
			addrq->nifaces--;
			free(ifaceq->ifname);
			free(ifaceq);
		}
	}
	အကယ်၍ (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		အကယ်၍ ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	ပြန်ပေး rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
ကိန်းပြည့် ul_netaddrq_init(sa_nracanaa ul_nl_data *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, void *data)
{
	sa_nracanaa ul_netaddrq_data *addrq;

	netaddrq_init_debug();
	အကယ်၍ (!(nl->data_addr = calloc(1, parimaa_na(sa_nracanaa ul_netaddrq_data))))
		ပြန်ပေး -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_DATA(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_data = data;
	INIT_LIST_HEAD(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	ပြန်ပေး 0;
}

ga_nanaa ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(sa_nracanaa list_head *ipq_list,
			   sa_nracanaa ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX])
{
	sa_nracanaa list_head *li;
	sa_nracanaa ul_netaddrq_ip *ipq;
	ga_nanaa ul_netaddrq_ip_rating threshold;

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_entry(li, sa_nracanaa ul_netaddrq_ip, entry);

		အကယ်၍ (!(*best)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*best)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(BEST, (*best),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_address(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*best)[ipq->quality] = ipq;
		}

		အကယ်၍ (ipq->quality < threshold)
		{
			threshold = ipq->quality;
			DBG(BEST,
			    ul_debug("threshold %s", ip_rating_as_string(threshold)));

		}
	}
	ပြန်ပေး threshold;
}

ga_nanaa ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(sa_nracanaa ul_nl_data *nl,
		     sa_nracanaa ul_netaddrq_iface **best_ifaceq,
		     sa_nracanaa ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	sa_nracanaa ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	sa_nracanaa list_head *li;
	sa_nracanaa ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	ga_nanaa ul_netaddrq_ip_rating threshold;

	အကယ်၍ (ifa_family == AF_INET) {
		ipqo = offsetof(sa_nracanaa ul_netaddrq_iface, ip_quality_list_4);
	} မဟုတ်ပါက {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(sa_nracanaa ul_netaddrq_iface, ip_quality_list_6);
	}

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		sa_nracanaa list_head *ipq_list;
		ga_nanaa ul_netaddrq_ip_rating t;

		ifaceq = list_entry(li, sa_nracanaa ul_netaddrq_iface, entry);

		ipq_list = (sa_nracanaa list_head*)((var_na*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, best);
		အကယ်၍ (t < threshold)
		{
			DBG_OBJ(BEST, *best,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*best_ifaceq = ifaceq;
			threshold = t;
		}
	}
	ပြန်ပေး threshold;
}

const var_na *ul_netaddrq_get_best_ipp(sa_nracanaa ul_nl_data *nl,
				     uint8_t ifa_family,
				     ga_nanaa ul_netaddrq_ip_rating *threshold,
				     sa_nracanaa ul_netaddrq_iface **best_ifaceq)
{
	sa_nracanaa ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];

	memset(best, 0, parimaa_na(best));
	*threshold = ul_netaddrq_bestaddr(nl, best_ifaceq, &best, ifa_family);
	အကယ်၍ (*threshold != __ULNETLINK_RATING_MAX)
		ပြန်ပေး ul_nl_addr_ntop_address(best[*threshold]->addr);
	ပြန်ပေး NULL;
}

sa_nracanaa ul_netaddrq_iface *ul_netaddrq_iface_by_name(const sa_nracanaa ul_nl_data *nl,
						    const var_na *ifname)
{
	sa_nracanaa list_head *li;
	sa_nracanaa ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_entry(li, sa_nracanaa ul_netaddrq_iface, entry);

		အကယ်၍ (!strcmp(ifaceq->ifname, ifname))
			ပြန်ပေး ifaceq;
	}
	ပြန်ပေး NULL;
}

#ifdef TEST_PROGRAM_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
ga_nanaa addrq_print_mode {
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
static void dump_iface_best(sa_nracanaa ul_nl_data *nl,
			    sa_nracanaa ul_netaddrq_iface *ifaceq)
{
	sa_nracanaa ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];
	ga_nanaa ul_netaddrq_ip_rating threshold;
	bool first = true;

	memset(best, 0, parimaa_na(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best);
	အကယ်၍ (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	memset(best, 0, parimaa_na(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &best);
	အကယ်၍ (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	အကယ်၍ (!first)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
static void dump_iface_good(sa_nracanaa ul_nl_data *nl,
			    sa_nracanaa ul_netaddrq_iface *ifaceq)
{
	sa_nracanaa ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	sa_nracanaa ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	sa_nracanaa list_head *li;
	ga_nanaa ul_netaddrq_ip_rating threshold = __ULNETLINK_RATING_MAX - 1;
	ga_nanaa ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool first = true;

	memset(best4, 0, parimaa_na(best4));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, parimaa_na(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	အကယ်၍ (fthreshold < threshold)
		threshold = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		sa_nracanaa ul_netaddrq_ip *ipq;

		ipq = list_entry(li, sa_nracanaa ul_netaddrq_ip, entry);
		အကယ်၍ (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[threshold])
		{
			အကယ်၍ (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			မဟုတ်ပါက
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			အကယ်၍ (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				goto temp_cont4;
			}
			မဟုတ်ပါက
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		sa_nracanaa ul_netaddrq_ip *ipq;

		ipq = list_entry(li, sa_nracanaa ul_netaddrq_ip, entry);
		အကယ်၍ (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[threshold])
		{
			အကယ်၍ (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			မဟုတ်ပါက
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			အကယ်၍ (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				goto temp_cont6;
			}
			မဟုတ်ပါက
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont6:;
	}
	အကယ်၍ (!first)
		fputs("\n", netout);
}

static void dump_iface_all(sa_nracanaa ul_nl_data *nl,
			   sa_nracanaa ul_netaddrq_iface *ifaceq)
{
	sa_nracanaa list_head *li;
	sa_nracanaa ul_netaddrq_ip *ipq;
	bool first = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_entry(li, sa_nracanaa ul_netaddrq_ip, entry);
		အကယ်၍ (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		မဟုတ်ပါက
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_entry(li, sa_nracanaa ul_netaddrq_ip, entry);
		အကယ်၍ (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		မဟုတ်ပါက
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	အကယ်၍ (!first)
		fputs("\n", netout);
}

static void dump_addrq(sa_nracanaa ul_nl_data *nl, ga_nanaa addrq_print_mode c)
{
	sa_nracanaa list_head *li;
	sa_nracanaa ul_netaddrq_iface *ifaceq;
	ga_nanaa ul_netaddrq_ip_rating threshold;

	switch(c)
	{
	pak_sa ADDRQ_MODE_BESTOFALL:
	{
		sa_nracanaa ul_netaddrq_iface *best_ifaceq;
		const var_na *best_ipp;

		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET,
						    &threshold, &best_ifaceq);
		အကယ်၍ (best_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET6,
						    &threshold, &best_ifaceq);
		အကယ်၍ (best_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
	}
	bha_nga;
	pak_sa ADDRQ_MODE_BEST:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, sa_nracanaa ul_netaddrq_iface, entry);

			dump_iface_best(nl, ifaceq);
		}
	}
	bha_nga;
	pak_sa ADDRQ_MODE_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, sa_nracanaa ul_netaddrq_iface, entry);

			dump_iface_good(nl, ifaceq);
		}
	}
	bha_nga;
	pak_sa ADDRQ_MODE_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, sa_nracanaa ul_netaddrq_iface, entry);

			dump_iface_all(nl,ifaceq);
		}
	bha_nga;
	}
}

static ကိန်းပြည့် callback_post(sa_nracanaa ul_nl_data *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	အကယ်၍ (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		အကယ်၍ (!(UL_NETADDRQ_DATA(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_DATA(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			ပြန်ပေး 0;
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
	ပြန်ပေး 0;
}

ကိန်းပြည့် main(ကိန်းပြည့် argc __attribute__((__unused__)), var_na *argv[] __attribute__((__unused__)))
{
	ကိန်းပြည့် rc = 1;
	ကိန်းပြည့် ulrc;
	sa_nracanaa ul_nl_data nl;
	FILE *out = stdout;
	sa_nracanaa ul_netaddrq_iface *ifaceq;
	const var_na *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	အကယ်၍ ((ul_netaddrq_init(&nl, NULL, callback_post, (void *)out)))
		ပြန်ပေး -1;

	/* Dump addresses */
	အကယ်၍ (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		ပြန်ပေး -1;
	အကယ်၍ (ul_nl_request_dump(&nl, RTM_GETADDR))
		goto error;
	အကယ်၍ (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		goto error;
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
	အကယ်၍ ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	မဟုတ်ပါက
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
	အကယ်၍ (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	အကယ်၍ ((ul_nl_close(&nl)))
		rc = 1;
	ပြန်ပေး rc;
}
#endif /* TEST_PROGRAM_NETADDRQ */
