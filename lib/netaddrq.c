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
const ціле max_ifaces = 12;

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
	якщо (netaddrq_debug_mask)
		повернути;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

static inline enum ul_netaddrq_ip_rating
evaluate_ip_quality(struct ul_nl_addr *addr) {
	enum ul_netaddrq_ip_rating quality;

	switch (addr->ifa_scope) {
	case RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		break;
	case RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		break;
	case RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		break;
	default:
		quality = ULNETLINK_RATING_BAD;
		break;
	}
	якщо (addr->ifa_flags & IFA_F_TEMPORARY) {
		якщо (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	повернути quality;
}

#define DBG_CASE(x) case x: str = #x; break
#define DBG_CASE_DEF8(x) default: snprintf(strx+2, 3, "%02hhx", x); str = strx; break
static char *ip_rating_as_string(enum ul_netaddrq_ip_rating q)
{
	char *str;
	static char strx[5] = "0x";
	switch (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	повернути str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
static ціле callback_addrq(struct ul_nl_data *nl)
{
	struct ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	struct list_head *li, *ipq_list;
	struct ul_netaddrq_iface *ifaceq = NULL;
	struct ul_netaddrq_ip *ipq = NULL;
	ціле rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_address(&(nl->addr)),
			      nl->addr.ifname));
	якщо (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		якщо ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		struct ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_entry(li, struct ul_netaddrq_iface, entry);
		якщо (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			break;
		}
		addrq->nifaces++;
	}

	якщо (ifaceq == NULL) {
		якщо (nl->rtm_event) {
			якщо (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->overflow = true;
				повернути UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			якщо (!(ifaceq = malloc(sizeof(struct ul_netaddrq_iface))))
				повернути -ENOMEM;
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_4));
			INIT_LIST_HEAD(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			якщо (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				free(ifaceq);
				повернути -ENOMEM;
			}
			list_add_tail(&(ifaceq->entry), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} інакше {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			повернути UL_NL_SOFT_ERROR;
		}
	}
	якщо (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} інакше {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		struct ul_netaddrq_ip *ipqq;

		ipqq = list_entry(li, struct ul_netaddrq_ip, entry);
		якщо (ipqq->addr->address_len == nl->addr.address_len)
			якщо (!memcmp(ipqq->addr->address, nl->addr.address,
				   nl->addr.address_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				break;
			}
	}
	якщо (ipq == NULL) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	якщо (UL_NL_IS_RTM_NEW(nl)) {
		struct ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		якщо (!addr) {
			rc = -ENOMEM;
			goto error;
		}
		якщо (ipq == NULL) {
			якщо (!(ipq = malloc(sizeof(struct ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				goto error;
			}
			ipq->addr = addr;
			list_add_tail(&(ipq->entry), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} інакше {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_address(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} інакше {
		/* UL_NL_RTM_DEL */
		якщо (ipq == NULL)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			повернути UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->entry));
		ul_nl_addr_free(ipq->addr);
		free(ipq);
	error:
		якщо (list_empty(&(ifaceq->ip_quality_list_4)) &&
		    list_empty(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->entry));
			addrq->nifaces--;
			free(ifaceq->ifname);
			free(ifaceq);
		}
	}
	якщо (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		якщо ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	повернути rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
ціле ul_netaddrq_init(struct ul_nl_data *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, void *data)
{
	struct ul_netaddrq_data *addrq;

	netaddrq_init_debug();
	якщо (!(nl->data_addr = calloc(1, sizeof(struct ul_netaddrq_data))))
		повернути -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_DATA(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_data = data;
	INIT_LIST_HEAD(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	повернути 0;
}

enum ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(struct list_head *ipq_list,
			   struct ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX])
{
	struct list_head *li;
	struct ul_netaddrq_ip *ipq;
	enum ul_netaddrq_ip_rating threshold;

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_entry(li, struct ul_netaddrq_ip, entry);

		якщо (!(*best)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*best)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(BEST, (*best),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_address(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*best)[ipq->quality] = ipq;
		}

		якщо (ipq->quality < threshold)
		{
			threshold = ipq->quality;
			DBG(BEST,
			    ul_debug("threshold %s", ip_rating_as_string(threshold)));

		}
	}
	повернути threshold;
}

enum ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(struct ul_nl_data *nl,
		     struct ul_netaddrq_iface **best_ifaceq,
		     struct ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	struct ul_netaddrq_data *addrq = UL_NETADDRQ_DATA(nl);
	struct list_head *li;
	struct ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	enum ul_netaddrq_ip_rating threshold;

	якщо (ifa_family == AF_INET) {
		ipqo = offsetof(struct ul_netaddrq_iface, ip_quality_list_4);
	} інакше {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(struct ul_netaddrq_iface, ip_quality_list_6);
	}

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		struct list_head *ipq_list;
		enum ul_netaddrq_ip_rating t;

		ifaceq = list_entry(li, struct ul_netaddrq_iface, entry);

		ipq_list = (struct list_head*)((char*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, best);
		якщо (t < threshold)
		{
			DBG_OBJ(BEST, *best,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*best_ifaceq = ifaceq;
			threshold = t;
		}
	}
	повернути threshold;
}

const char *ul_netaddrq_get_best_ipp(struct ul_nl_data *nl,
				     uint8_t ifa_family,
				     enum ul_netaddrq_ip_rating *threshold,
				     struct ul_netaddrq_iface **best_ifaceq)
{
	struct ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];

	memset(best, 0, sizeof(best));
	*threshold = ul_netaddrq_bestaddr(nl, best_ifaceq, &best, ifa_family);
	якщо (*threshold != __ULNETLINK_RATING_MAX)
		повернути ul_nl_addr_ntop_address(best[*threshold]->addr);
	повернути NULL;
}

struct ul_netaddrq_iface *ul_netaddrq_iface_by_name(const struct ul_nl_data *nl,
						    const char *ifname)
{
	struct list_head *li;
	struct ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_entry(li, struct ul_netaddrq_iface, entry);

		якщо (!strcmp(ifaceq->ifname, ifname))
			повернути ifaceq;
	}
	повернути NULL;
}

#ifdef TEST_PROGRAM_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
enum addrq_print_mode {
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
static void dump_iface_best(struct ul_nl_data *nl,
			    struct ul_netaddrq_iface *ifaceq)
{
	struct ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];
	enum ul_netaddrq_ip_rating threshold;
	bool first = true;

	memset(best, 0, sizeof(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best);
	якщо (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	memset(best, 0, sizeof(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &best);
	якщо (threshold != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	якщо (!first)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
static void dump_iface_good(struct ul_nl_data *nl,
			    struct ul_netaddrq_iface *ifaceq)
{
	struct ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	struct ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	struct list_head *li;
	enum ul_netaddrq_ip_rating threshold = __ULNETLINK_RATING_MAX - 1;
	enum ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool first = true;

	memset(best4, 0, sizeof(best4));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, sizeof(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	якщо (fthreshold < threshold)
		threshold = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		struct ul_netaddrq_ip *ipq;

		ipq = list_entry(li, struct ul_netaddrq_ip, entry);
		якщо (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[threshold])
		{
			якщо (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			інакше
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			якщо (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				goto temp_cont4;
			}
			інакше
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		struct ul_netaddrq_ip *ipq;

		ipq = list_entry(li, struct ul_netaddrq_ip, entry);
		якщо (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[threshold])
		{
			якщо (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			інакше
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			якщо (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_address(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				goto temp_cont6;
			}
			інакше
				fputs(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	temp_cont6:;
	}
	якщо (!first)
		fputs("\n", netout);
}

static void dump_iface_all(struct ul_nl_data *nl,
			   struct ul_netaddrq_iface *ifaceq)
{
	struct list_head *li;
	struct ul_netaddrq_ip *ipq;
	bool first = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_entry(li, struct ul_netaddrq_ip, entry);
		якщо (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		інакше
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_entry(li, struct ul_netaddrq_ip, entry);
		якщо (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		інакше
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	якщо (!first)
		fputs("\n", netout);
}

static void dump_addrq(struct ul_nl_data *nl, enum addrq_print_mode c)
{
	struct list_head *li;
	struct ul_netaddrq_iface *ifaceq;
	enum ul_netaddrq_ip_rating threshold;

	switch(c)
	{
	case ADDRQ_MODE_BESTOFALL:
	{
		struct ul_netaddrq_iface *best_ifaceq;
		const char *best_ipp;

		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET,
						    &threshold, &best_ifaceq);
		якщо (best_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET6,
						    &threshold, &best_ifaceq);
		якщо (best_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
	}
	break;
	case ADDRQ_MODE_BEST:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, struct ul_netaddrq_iface, entry);

			dump_iface_best(nl, ifaceq);
		}
	}
	break;
	case ADDRQ_MODE_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, struct ul_netaddrq_iface, entry);

			dump_iface_good(nl, ifaceq);
		}
	}
	break;
	case ADDRQ_MODE_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, struct ul_netaddrq_iface, entry);

			dump_iface_all(nl,ifaceq);
		}
	break;
	}
}

static ціле callback_post(struct ul_nl_data *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	якщо (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		якщо (!(UL_NETADDRQ_DATA(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_DATA(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			повернути 0;
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
	повернути 0;
}

ціле main(ціле argc __attribute__((__unused__)), char *argv[] __attribute__((__unused__)))
{
	ціле rc = 1;
	ціле ulrc;
	struct ul_nl_data nl;
	FILE *out = stdout;
	struct ul_netaddrq_iface *ifaceq;
	const char *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	якщо ((ul_netaddrq_init(&nl, NULL, callback_post, (void *)out)))
		повернути -1;

	/* Dump addresses */
	якщо (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		повернути -1;
	якщо (ul_nl_request_dump(&nl, RTM_GETADDR))
		goto error;
	якщо (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
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
	якщо ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	інакше
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
	якщо (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	якщо ((ul_nl_close(&nl)))
		rc = 1;
	повернути rc;
}
#endif /* TEST_PROGRAM_NETADDRQ */
