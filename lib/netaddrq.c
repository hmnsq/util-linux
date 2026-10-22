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

#ਅਨ੍ਤਰ੍ਭਾਵ <net/if.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <netinet/in.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/rtnetlink.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/if_addr.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "netaddrq.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "list.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "debug.h"

/* Maximal number of interfaces. The algorithm has a quadratic complexity,
 * don't overflood it. */
ਅਚਲ ਪੂਰਨ max_ifaces = 12;

/*
 * Debug stuff (based on include/debug.h)
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULNETADDRQ_DEBUG_HELP	(1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULNETADDRQ_DEBUG_INIT	(1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULNETADDRQ_DEBUG_LIST	(1 << 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULNETADDRQ_DEBUG_ਉੱਤਮ	(1 << 4)

ਸਥਿਰ UL_DEBUG_DEFINE_ਨਕਾਬ(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_ਉੱਤਮ,	"searching best address" },

	{ ਨਿਰਾ, 0 }
};

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

ਸਥਿਰ ਖਾਲੀ netaddrq_init_debug(ਖਾਲੀ)
{
	ਜੇ (netaddrq_debug_ਨਕਾਬ)
		ਵਾਪਸ;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਗਿਣਤੀ ul_netaddrq_ip_rating
evaluate_ip_quality(ਬਣਤਰ ul_nl_addr *addr) {
	ਗਿਣਤੀ ul_netaddrq_ip_rating quality;

	ਚੋਣ (addr->ifa_scope) {
	ਹਾਲਤ RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		ਰੋਕੋ;
	ਹਾਲਤ RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		ਰੋਕੋ;
	ਹਾਲਤ RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		ਰੋਕੋ;
	ਮੂਲ:
		quality = ULNETLINK_RATING_BAD;
		ਰੋਕੋ;
	}
	ਜੇ (addr->ifa_flags & IFA_F_TEMPORARY) {
		ਜੇ (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	ਵਾਪਸ quality;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG_CASE(x) ਹਾਲਤ x: str = #x; ਰੋਕੋ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG_CASE_DEF8(x) ਮੂਲ: snprintf(strx+2, 3, "%02hhx", x); str = strx; ਰੋਕੋ
ਸਥਿਰ ਅੱਖਰ *ip_rating_as_string(ਗਿਣਤੀ ul_netaddrq_ip_rating q)
{
	ਅੱਖਰ *str;
	ਸਥਿਰ ਅੱਖਰ strx[5] = "0x";
	ਚੋਣ (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	ਵਾਪਸ str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
ਸਥਿਰ ਪੂਰਨ callback_addrq(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl)
{
	ਬਣਤਰ ul_netaddrq_ਅੰਕੜੇ *addrq = UL_NETADDRQ_ਅੰਕੜੇ(nl);
	ਬਣਤਰ list_ਸਿਰ *li, *ipq_list;
	ਬਣਤਰ ul_netaddrq_iface *ifaceq = ਨਿਰਾ;
	ਬਣਤਰ ul_netaddrq_ip *ipq = ਨਿਰਾ;
	ਪੂਰਨ rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_ਪਤਾ(&(nl->addr)),
			      nl->addr.ifname));
	ਜੇ (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		ਜੇ ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		ਬਣਤਰ ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_ਇੰਦਰਾਜ(li, ਬਣਤਰ ul_netaddrq_iface, ਇੰਦਰਾਜ);
		ਜੇ (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			ਰੋਕੋ;
		}
		addrq->nifaces++;
	}

	ਜੇ (ifaceq == ਨਿਰਾ) {
		ਜੇ (nl->rtm_event) {
			ਜੇ (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->ਉਛਾਲ = true;
				ਵਾਪਸ UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			ਜੇ (!(ifaceq = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ ul_netaddrq_iface))))
				ਵਾਪਸ -ENOMEM;
			INIT_LIST_ਸਿਰ(&(ifaceq->ip_quality_list_4));
			INIT_LIST_ਸਿਰ(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			ਜੇ (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				ਮੁਕਤ(ifaceq);
				ਵਾਪਸ -ENOMEM;
			}
			list_ਜੋੜੋ_ਪੂਛ(&(ifaceq->ਇੰਦਰਾਜ), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} ਨਹੀਂਤਾਂ {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			ਵਾਪਸ UL_NL_SOFT_ERROR;
		}
	}
	ਜੇ (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} ਨਹੀਂਤਾਂ {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		ਬਣਤਰ ul_netaddrq_ip *ipqq;

		ipqq = list_ਇੰਦਰਾਜ(li, ਬਣਤਰ ul_netaddrq_ip, ਇੰਦਰਾਜ);
		ਜੇ (ipqq->addr->ਪਤਾ_len == nl->addr.ਪਤਾ_len)
			ਜੇ (!memcmp(ipqq->addr->ਪਤਾ, nl->addr.ਪਤਾ,
				   nl->addr.ਪਤਾ_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				ਰੋਕੋ;
			}
	}
	ਜੇ (ipq == ਨਿਰਾ) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	ਜੇ (UL_NL_IS_RTM_NEW(nl)) {
		ਬਣਤਰ ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		ਜੇ (!addr) {
			rc = -ENOMEM;
			ਜਾਓ error;
		}
		ਜੇ (ipq == ਨਿਰਾ) {
			ਜੇ (!(ipq = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				ਜਾਓ error;
			}
			ipq->addr = addr;
			list_ਜੋੜੋ_ਪੂਛ(&(ipq->ਇੰਦਰਾਜ), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} ਨਹੀਂਤਾਂ {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_ਪਤਾ(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} ਨਹੀਂਤਾਂ {
		/* UL_NL_RTM_DEL */
		ਜੇ (ipq == ਨਿਰਾ)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			ਵਾਪਸ UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->ਇੰਦਰਾਜ));
		ul_nl_addr_free(ipq->addr);
		ਮੁਕਤ(ipq);
	error:
		ਜੇ (list_ਖਾਲੀ_ਥਾਵਾਂ(&(ifaceq->ip_quality_list_4)) &&
		    list_ਖਾਲੀ_ਥਾਵਾਂ(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->ਇੰਦਰਾਜ));
			addrq->nifaces--;
			ਮੁਕਤ(ifaceq->ifname);
			ਮੁਕਤ(ifaceq);
		}
	}
	ਜੇ (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		ਜੇ ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	ਵਾਪਸ rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
ਪੂਰਨ ul_netaddrq_init(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, ਖਾਲੀ *ਅੰਕੜੇ)
{
	ਬਣਤਰ ul_netaddrq_ਅੰਕੜੇ *addrq;

	netaddrq_init_debug();
	ਜੇ (!(nl->ਅੰਕੜੇ_addr = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ ul_netaddrq_ਅੰਕੜੇ))))
		ਵਾਪਸ -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_ਅੰਕੜੇ(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_ਅੰਕੜੇ = ਅੰਕੜੇ;
	INIT_LIST_ਸਿਰ(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	ਵਾਪਸ 0;
}

ਗਿਣਤੀ ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(ਬਣਤਰ list_ਸਿਰ *ipq_list,
			   ਬਣਤਰ ul_netaddrq_ip *(*ਉੱਤਮ)[__ULNETLINK_RATING_MAX])
{
	ਬਣਤਰ list_ਸਿਰ *li;
	ਬਣਤਰ ul_netaddrq_ip *ipq;
	ਗਿਣਤੀ ul_netaddrq_ip_rating ਹੱਦ;

	ਹੱਦ = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_ਇੰਦਰਾਜ(li, ਬਣਤਰ ul_netaddrq_ip, ਇੰਦਰਾਜ);

		ਜੇ (!(*ਉੱਤਮ)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*ਉੱਤਮ)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(ਉੱਤਮ_2, (*ਉੱਤਮ),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_ਪਤਾ(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*ਉੱਤਮ)[ipq->quality] = ipq;
		}

		ਜੇ (ipq->quality < ਹੱਦ)
		{
			ਹੱਦ = ipq->quality;
			DBG(ਉੱਤਮ_2,
			    ul_debug("threshold %s", ip_rating_as_string(ਹੱਦ)));

		}
	}
	ਵਾਪਸ ਹੱਦ;
}

ਗਿਣਤੀ ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl,
		     ਬਣਤਰ ul_netaddrq_iface **ਉੱਤਮ_ifaceq,
		     ਬਣਤਰ ul_netaddrq_ip *(*ਉੱਤਮ)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	ਬਣਤਰ ul_netaddrq_ਅੰਕੜੇ *addrq = UL_NETADDRQ_ਅੰਕੜੇ(nl);
	ਬਣਤਰ list_ਸਿਰ *li;
	ਬਣਤਰ ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	ਗਿਣਤੀ ul_netaddrq_ip_rating ਹੱਦ;

	ਜੇ (ifa_family == AF_INET) {
		ipqo = offsetof(ਬਣਤਰ ul_netaddrq_iface, ip_quality_list_4);
	} ਨਹੀਂਤਾਂ {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(ਬਣਤਰ ul_netaddrq_iface, ip_quality_list_6);
	}

	ਹੱਦ = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		ਬਣਤਰ list_ਸਿਰ *ipq_list;
		ਗਿਣਤੀ ul_netaddrq_ip_rating t;

		ifaceq = list_ਇੰਦਰਾਜ(li, ਬਣਤਰ ul_netaddrq_iface, ਇੰਦਰਾਜ);

		ipq_list = (ਬਣਤਰ list_ਸਿਰ*)((ਅੱਖਰ*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, ਉੱਤਮ);
		ਜੇ (t < ਹੱਦ)
		{
			DBG_OBJ(ਉੱਤਮ_2, *ਉੱਤਮ,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*ਉੱਤਮ_ifaceq = ifaceq;
			ਹੱਦ = t;
		}
	}
	ਵਾਪਸ ਹੱਦ;
}

ਅਚਲ ਅੱਖਰ *ul_netaddrq_get_ਉੱਤਮ_ipp(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl,
				     uint8_t ifa_family,
				     ਗਿਣਤੀ ul_netaddrq_ip_rating *ਹੱਦ,
				     ਬਣਤਰ ul_netaddrq_iface **ਉੱਤਮ_ifaceq)
{
	ਬਣਤਰ ul_netaddrq_ip *ਉੱਤਮ[__ULNETLINK_RATING_MAX];

	memset(ਉੱਤਮ, 0, ਆਕਾਰ(ਉੱਤਮ));
	*ਹੱਦ = ul_netaddrq_bestaddr(nl, ਉੱਤਮ_ifaceq, &ਉੱਤਮ, ifa_family);
	ਜੇ (*ਹੱਦ != __ULNETLINK_RATING_MAX)
		ਵਾਪਸ ul_nl_addr_ntop_ਪਤਾ(ਉੱਤਮ[*ਹੱਦ]->addr);
	ਵਾਪਸ ਨਿਰਾ;
}

ਬਣਤਰ ul_netaddrq_iface *ul_netaddrq_iface_by_name(ਅਚਲ ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl,
						    ਅਚਲ ਅੱਖਰ *ifname)
{
	ਬਣਤਰ list_ਸਿਰ *li;
	ਬਣਤਰ ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_ਇੰਦਰਾਜ(li, ਬਣਤਰ ul_netaddrq_iface, ਇੰਦਰਾਜ);

		ਜੇ (!strcmp(ifaceq->ifname, ifname))
			ਵਾਪਸ ifaceq;
	}
	ਵਾਪਸ ਨਿਰਾ;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
ਗਿਣਤੀ addrq_print_ਬਹੁਲਕ {
	ADDRQ_ਬਹੁਲਕ_BESTOFALL,	/* Best address of all interfaces */
	ADDRQ_ਬਹੁਲਕ_ਉੱਤਮ,	/* Best address per interface */
	ADDRQ_ਬਹੁਲਕ_GOOD,	/* All global or site addresses, if none, the
				 * longest living temporary, if none, link */
	ADDRQ_ਬਹੁਲਕ_ALL		/* All available addresses */
};

/* In our example addrq->callback_data is a simple FILE *. In more complex
 * programs it could be a pointer to an arbitrary struct */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ netout (FILE *)(UL_NETADDRQ_ਅੰਕੜੇ(nl)->callback_ਅੰਕੜੇ)

/* This example uses separate threshold for IPv4 and IPv6, so the best IPv4 and
 * best IPv6 addresses are printed. */
ਸਥਿਰ ਖਾਲੀ dump_iface_ਉੱਤਮ(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl,
			    ਬਣਤਰ ul_netaddrq_iface *ifaceq)
{
	ਬਣਤਰ ul_netaddrq_ip *ਉੱਤਮ[__ULNETLINK_RATING_MAX];
	ਗਿਣਤੀ ul_netaddrq_ip_rating ਹੱਦ;
	bool ਪਹਿਲਾ = true;

	memset(ਉੱਤਮ, 0, ਆਕਾਰ(ਉੱਤਮ));
	ਹੱਦ = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &ਉੱਤਮ);
	ਜੇ (ਹੱਦ != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (ਪਹਿਲਾ ? "best address" : " "),
		       ul_nl_addr_ntop_ਪਤਾ(ਉੱਤਮ[ਹੱਦ]->addr));
		ਪਹਿਲਾ = false;
	}
	memset(ਉੱਤਮ, 0, ਆਕਾਰ(ਉੱਤਮ));
	ਹੱਦ = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &ਉੱਤਮ);
	ਜੇ (ਹੱਦ != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (ਪਹਿਲਾ ? "best address" : " "),
		       ul_nl_addr_ntop_ਪਤਾ(ਉੱਤਮ[ਹੱਦ]->addr));
		ਪਹਿਲਾ = false;
	}
	ਜੇ (!ਪਹਿਲਾ)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
ਸਥਿਰ ਖਾਲੀ dump_iface_good(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl,
			    ਬਣਤਰ ul_netaddrq_iface *ifaceq)
{
	ਬਣਤਰ ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	ਬਣਤਰ ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	ਬਣਤਰ list_ਸਿਰ *li;
	ਗਿਣਤੀ ul_netaddrq_ip_rating ਹੱਦ = __ULNETLINK_RATING_MAX - 1;
	ਗਿਣਤੀ ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool ਪਹਿਲਾ = true;

	memset(best4, 0, ਆਕਾਰ(best4));
	ਹੱਦ = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, ਆਕਾਰ(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	ਜੇ (fthreshold < ਹੱਦ)
		ਹੱਦ = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ਬਣਤਰ ul_netaddrq_ip *ipq;

		ipq = list_ਇੰਦਰਾਜ(li, ਬਣਤਰ ul_netaddrq_ip, ਇੰਦਰਾਜ);
		ਜੇ (ਹੱਦ <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= ਹੱਦ ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[ਹੱਦ])
		{
			ਜੇ (ਪਹਿਲਾ)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				ਪਹਿਲਾ = false;
			}
			ਨਹੀਂਤਾਂ
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			ਜੇ (ਹੱਦ == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_ਪਤਾ(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				ਜਾਓ ਆਰਜ਼ੀ_cont4;
			}
			ਨਹੀਂਤਾਂ
				fputs(ul_nl_addr_ntop_ਪਤਾ(ipq->addr),
				      netout);
		}
	ਆਰਜ਼ੀ_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ਬਣਤਰ ul_netaddrq_ip *ipq;

		ipq = list_ਇੰਦਰਾਜ(li, ਬਣਤਰ ul_netaddrq_ip, ਇੰਦਰਾਜ);
		ਜੇ (ਹੱਦ <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= ਹੱਦ ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[ਹੱਦ])
		{
			ਜੇ (ਪਹਿਲਾ)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				ਪਹਿਲਾ = false;
			}
			ਨਹੀਂਤਾਂ
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			ਜੇ (ਹੱਦ == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_ਪਤਾ(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				ਜਾਓ ਆਰਜ਼ੀ_cont6;
			}
			ਨਹੀਂਤਾਂ
				fputs(ul_nl_addr_ntop_ਪਤਾ(ipq->addr),
				      netout);
		}
	ਆਰਜ਼ੀ_cont6:;
	}
	ਜੇ (!ਪਹਿਲਾ)
		fputs("\n", netout);
}

ਸਥਿਰ ਖਾਲੀ dump_iface_all(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl,
			   ਬਣਤਰ ul_netaddrq_iface *ifaceq)
{
	ਬਣਤਰ list_ਸਿਰ *li;
	ਬਣਤਰ ul_netaddrq_ip *ipq;
	bool ਪਹਿਲਾ = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_ਇੰਦਰਾਜ(li, ਬਣਤਰ ul_netaddrq_ip, ਇੰਦਰਾਜ);
		ਜੇ (ਪਹਿਲਾ)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			ਪਹਿਲਾ = false;
		}
		ਨਹੀਂਤਾਂ
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_ਪਤਾ(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_ਇੰਦਰਾਜ(li, ਬਣਤਰ ul_netaddrq_ip, ਇੰਦਰਾਜ);
		ਜੇ (ਪਹਿਲਾ)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			ਪਹਿਲਾ = false;
		}
		ਨਹੀਂਤਾਂ
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_ਪਤਾ(ipq->addr), netout);
	}
	ਜੇ (!ਪਹਿਲਾ)
		fputs("\n", netout);
}

ਸਥਿਰ ਖਾਲੀ dump_addrq(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl, ਗਿਣਤੀ addrq_print_ਬਹੁਲਕ c)
{
	ਬਣਤਰ list_ਸਿਰ *li;
	ਬਣਤਰ ul_netaddrq_iface *ifaceq;
	ਗਿਣਤੀ ul_netaddrq_ip_rating ਹੱਦ;

	ਚੋਣ(c)
	{
	ਹਾਲਤ ADDRQ_ਬਹੁਲਕ_BESTOFALL:
	{
		ਬਣਤਰ ul_netaddrq_iface *ਉੱਤਮ_ifaceq;
		ਅਚਲ ਅੱਖਰ *ਉੱਤਮ_ipp;

		ਉੱਤਮ_ipp = ul_netaddrq_get_ਉੱਤਮ_ipp(nl, AF_INET,
						    &ਹੱਦ, &ਉੱਤਮ_ifaceq);
		ਜੇ (ਉੱਤਮ_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				ਉੱਤਮ_ipp, ਉੱਤਮ_ifaceq->ifname);
		ਉੱਤਮ_ipp = ul_netaddrq_get_ਉੱਤਮ_ipp(nl, AF_INET6,
						    &ਹੱਦ, &ਉੱਤਮ_ifaceq);
		ਜੇ (ਉੱਤਮ_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				ਉੱਤਮ_ipp, ਉੱਤਮ_ifaceq->ifname);
	}
	ਰੋਕੋ;
	ਹਾਲਤ ADDRQ_ਬਹੁਲਕ_ਉੱਤਮ:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_ਇੰਦਰਾਜ(li, ਬਣਤਰ ul_netaddrq_iface, ਇੰਦਰਾਜ);

			dump_iface_ਉੱਤਮ(nl, ifaceq);
		}
	}
	ਰੋਕੋ;
	ਹਾਲਤ ADDRQ_ਬਹੁਲਕ_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_ਇੰਦਰਾਜ(li, ਬਣਤਰ ul_netaddrq_iface, ਇੰਦਰਾਜ);

			dump_iface_good(nl, ifaceq);
		}
	}
	ਰੋਕੋ;
	ਹਾਲਤ ADDRQ_ਬਹੁਲਕ_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_ਇੰਦਰਾਜ(li, ਬਣਤਰ ul_netaddrq_iface, ਇੰਦਰਾਜ);

			dump_iface_all(nl,ifaceq);
		}
	ਰੋਕੋ;
	}
}

ਸਥਿਰ ਪੂਰਨ callback_post(ਬਣਤਰ ul_nl_ਅੰਕੜੇ *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	ਜੇ (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		ਜੇ (!(UL_NETADDRQ_ਅੰਕੜੇ(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_ਅੰਕੜੇ(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			ਵਾਪਸ 0;
		}
		UL_NETADDRQ_ਅੰਕੜੇ(nl)->ifaces_change_4 = false;
		UL_NETADDRQ_ਅੰਕੜੇ(nl)->ifaces_change_6 = false;
		fputs("\n\nNetwork change detected:\n", netout);
		fputs("\nbest address:\n", netout);
		dump_addrq(nl, ADDRQ_ਬਹੁਲਕ_BESTOFALL);

		fputs("\nbest addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_ਬਹੁਲਕ_ਉੱਤਮ);

		fputs("\ngood addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_ਬਹੁਲਕ_GOOD);

		fputs("\nall addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_ਬਹੁਲਕ_ALL);
	}
	ਵਾਪਸ 0;
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc __attribute__((__unused__)), ਅੱਖਰ *argv[] __attribute__((__unused__)))
{
	ਪੂਰਨ rc = 1;
	ਪੂਰਨ ulrc;
	ਬਣਤਰ ul_nl_ਅੰਕੜੇ nl;
	FILE *ਬਾਹਰਲਾ = stdout;
	ਬਣਤਰ ul_netaddrq_iface *ifaceq;
	ਅਚਲ ਅੱਖਰ *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	ਜੇ ((ul_netaddrq_init(&nl, ਨਿਰਾ, callback_post, (ਖਾਲੀ *)ਬਾਹਰਲਾ)))
		ਵਾਪਸ -1;

	/* Dump addresses */
	ਜੇ (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		ਵਾਪਸ -1;
	ਜੇ (ul_nl_request_dump(&nl, RTM_GETADDR))
		ਜਾਓ error;
	ਜੇ (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_ਪੂਰਾ)
		ਜਾਓ error;
	fputs("RTM_GETADDR dump finished.", ਬਾਹਰਲਾ);

	/* Example of several types of dump */
	fputs("\nbest address:\n", ਬਾਹਰਲਾ);
	dump_addrq(&nl, ADDRQ_ਬਹੁਲਕ_BESTOFALL);

	fputs("\nbest addresses dump:\n", ਬਾਹਰਲਾ);
	dump_addrq(&nl, ADDRQ_ਬਹੁਲਕ_ਉੱਤਮ);

	fputs("\ngood addresses dump:\n", ਬਾਹਰਲਾ);
	dump_addrq(&nl, ADDRQ_ਬਹੁਲਕ_GOOD);

	fputs("\nall addresses dump:\n", ਬਾਹਰਲਾ);
	dump_addrq(&nl, ADDRQ_ਬਹੁਲਕ_ALL);

	fputs("\naddresses for interface ", ਬਾਹਰਲਾ);
	ਜੇ ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	ਨਹੀਂਤਾਂ
		fprintf(ਬਾਹਰਲਾ, "%s not found.", ifname);

	/* Monitor further changes */
	fputs("\nGoing to monitor mode.\n", ਬਾਹਰਲਾ);

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
	ਜੇ (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	ਜੇ ((ul_nl_close(&nl)))
		rc = 1;
	ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_NETADDRQ */
