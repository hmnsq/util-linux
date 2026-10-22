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
ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ max_ifaces = 12;

/*
 * Debug stuff (based on include/debug.h)
 */
#ವ್ಯಾಖ್ಯಾನಿಸು ULNETADDRQ_DEBUG_HELP	(1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು ULNETADDRQ_DEBUG_INIT	(1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#ವ್ಯಾಖ್ಯಾನಿಸು ULNETADDRQ_DEBUG_LIST	(1 << 3)
#ವ್ಯಾಖ್ಯಾನಿಸು ULNETADDRQ_DEBUG_ಉತ್ತಮ	(1 << 4)

ಸ್ಥಿರ UL_DEBUG_DEFINE_ಮುಖವಾಡ(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_ಉತ್ತಮ,	"searching best address" },

	{ ಖಾಲಿ, 0 }
};

#ವ್ಯಾಖ್ಯಾನಿಸು DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#ವ್ಯಾಖ್ಯಾನಿಸು DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#ವ್ಯಾಖ್ಯಾನಿಸು ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

ಸ್ಥಿರ ಶೂನ್ಯ netaddrq_init_debug(ಶೂನ್ಯ)
{
	ಆದರೆ (netaddrq_debug_ಮುಖವಾಡ)
		ಹಿಂತಿರುಗಿ;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಎಣಿಕೆ ul_netaddrq_ip_rating
evaluate_ip_quality(ರಚನೆ ul_nl_addr *addr) {
	ಎಣಿಕೆ ul_netaddrq_ip_rating quality;

	ಆಯ್ಕೆ (addr->ifa_scope) {
	ಸ್ಥಿತಿ RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		quality = ULNETLINK_RATING_BAD;
		ನಿಲ್ಲಿಸು;
	}
	ಆದರೆ (addr->ifa_flags & IFA_F_TEMPORARY) {
		ಆದರೆ (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	ಹಿಂತಿರುಗಿ quality;
}

#ವ್ಯಾಖ್ಯಾನಿಸು DBG_CASE(x) ಸ್ಥಿತಿ x: str = #x; ನಿಲ್ಲಿಸು
#ವ್ಯಾಖ್ಯಾನಿಸು DBG_CASE_DEF8(x) ಪೂರ್ವನಿಯೋಜಿತ: snprintf(strx+2, 3, "%02hhx", x); str = strx; ನಿಲ್ಲಿಸು
ಸ್ಥಿರ ಅಕ್ಷರ *ip_rating_as_string(ಎಣಿಕೆ ul_netaddrq_ip_rating q)
{
	ಅಕ್ಷರ *str;
	ಸ್ಥಿರ ಅಕ್ಷರ strx[5] = "0x";
	ಆಯ್ಕೆ (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	ಹಿಂತಿರುಗಿ str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ callback_addrq(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl)
{
	ರಚನೆ ul_netaddrq_ದತ್ತಾಂಶ *addrq = UL_NETADDRQ_ದತ್ತಾಂಶ(nl);
	ರಚನೆ list_ತಲೆ *li, *ipq_list;
	ರಚನೆ ul_netaddrq_iface *ifaceq = ಖಾಲಿ;
	ರಚನೆ ul_netaddrq_ip *ipq = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_ವಿಳಾಸ(&(nl->addr)),
			      nl->addr.ifname));
	ಆದರೆ (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		ಆದರೆ ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		ರಚನೆ ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_ನಮೂದು(li, ರಚನೆ ul_netaddrq_iface, ನಮೂದು);
		ಆದರೆ (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			ನಿಲ್ಲಿಸು;
		}
		addrq->nifaces++;
	}

	ಆದರೆ (ifaceq == ಖಾಲಿ) {
		ಆದರೆ (nl->rtm_event) {
			ಆದರೆ (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->ಅತಿಪ್ರವಾಹ = true;
				ಹಿಂತಿರುಗಿ UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			ಆದರೆ (!(ifaceq = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ ul_netaddrq_iface))))
				ಹಿಂತಿರುಗಿ -ENOMEM;
			INIT_LIST_ತಲೆ(&(ifaceq->ip_quality_list_4));
			INIT_LIST_ತಲೆ(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			ಆದರೆ (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				ಬಿಡುಗಡೆ(ifaceq);
				ಹಿಂತಿರುಗಿ -ENOMEM;
			}
			list_ಕೂಡಿಸು_ಬಾಲ(&(ifaceq->ನಮೂದು), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			ಹಿಂತಿರುಗಿ UL_NL_SOFT_ERROR;
		}
	}
	ಆದರೆ (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} ಇಲ್ಲದಿದ್ದರೆ {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		ರಚನೆ ul_netaddrq_ip *ipqq;

		ipqq = list_ನಮೂದು(li, ರಚನೆ ul_netaddrq_ip, ನಮೂದು);
		ಆದರೆ (ipqq->addr->ವಿಳಾಸ_len == nl->addr.ವಿಳಾಸ_len)
			ಆದರೆ (!memcmp(ipqq->addr->ವಿಳಾಸ, nl->addr.ವಿಳಾಸ,
				   nl->addr.ವಿಳಾಸ_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				ನಿಲ್ಲಿಸು;
			}
	}
	ಆದರೆ (ipq == ಖಾಲಿ) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	ಆದರೆ (UL_NL_IS_RTM_NEW(nl)) {
		ರಚನೆ ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		ಆದರೆ (!addr) {
			rc = -ENOMEM;
			ಹೋಗು error;
		}
		ಆದರೆ (ipq == ಖಾಲಿ) {
			ಆದರೆ (!(ipq = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				ಹೋಗು error;
			}
			ipq->addr = addr;
			list_ಕೂಡಿಸು_ಬಾಲ(&(ipq->ನಮೂದು), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} ಇಲ್ಲದಿದ್ದರೆ {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_ವಿಳಾಸ(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* UL_NL_RTM_DEL */
		ಆದರೆ (ipq == ಖಾಲಿ)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			ಹಿಂತಿರುಗಿ UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->ನಮೂದು));
		ul_nl_addr_free(ipq->addr);
		ಬಿಡುಗಡೆ(ipq);
	error:
		ಆದರೆ (list_ಖಾಲಿಗಳು(&(ifaceq->ip_quality_list_4)) &&
		    list_ಖಾಲಿಗಳು(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->ನಮೂದು));
			addrq->nifaces--;
			ಬಿಡುಗಡೆ(ifaceq->ifname);
			ಬಿಡುಗಡೆ(ifaceq);
		}
	}
	ಆದರೆ (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		ಆದರೆ ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	ಹಿಂತಿರುಗಿ rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
ಪೂರ್ಣಾಂಕ ul_netaddrq_init(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
	ರಚನೆ ul_netaddrq_ದತ್ತಾಂಶ *addrq;

	netaddrq_init_debug();
	ಆದರೆ (!(nl->ದತ್ತಾಂಶ_addr = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ ul_netaddrq_ದತ್ತಾಂಶ))))
		ಹಿಂತಿರುಗಿ -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_ದತ್ತಾಂಶ(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
	INIT_LIST_ತಲೆ(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	ಹಿಂತಿರುಗಿ 0;
}

ಎಣಿಕೆ ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(ರಚನೆ list_ತಲೆ *ipq_list,
			   ರಚನೆ ul_netaddrq_ip *(*ಉತ್ತಮ)[__ULNETLINK_RATING_MAX])
{
	ರಚನೆ list_ತಲೆ *li;
	ರಚನೆ ul_netaddrq_ip *ipq;
	ಎಣಿಕೆ ul_netaddrq_ip_rating ಮಿತಿ;

	ಮಿತಿ = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_ನಮೂದು(li, ರಚನೆ ul_netaddrq_ip, ನಮೂದು);

		ಆದರೆ (!(*ಉತ್ತಮ)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*ಉತ್ತಮ)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(ಉತ್ತಮ_2, (*ಉತ್ತಮ),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_ವಿಳಾಸ(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*ಉತ್ತಮ)[ipq->quality] = ipq;
		}

		ಆದರೆ (ipq->quality < ಮಿತಿ)
		{
			ಮಿತಿ = ipq->quality;
			DBG(ಉತ್ತಮ_2,
			    ul_debug("threshold %s", ip_rating_as_string(ಮಿತಿ)));

		}
	}
	ಹಿಂತಿರುಗಿ ಮಿತಿ;
}

ಎಣಿಕೆ ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl,
		     ರಚನೆ ul_netaddrq_iface **ಉತ್ತಮ_ifaceq,
		     ರಚನೆ ul_netaddrq_ip *(*ಉತ್ತಮ)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	ರಚನೆ ul_netaddrq_ದತ್ತಾಂಶ *addrq = UL_NETADDRQ_ದತ್ತಾಂಶ(nl);
	ರಚನೆ list_ತಲೆ *li;
	ರಚನೆ ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	ಎಣಿಕೆ ul_netaddrq_ip_rating ಮಿತಿ;

	ಆದರೆ (ifa_family == AF_INET) {
		ipqo = offsetof(ರಚನೆ ul_netaddrq_iface, ip_quality_list_4);
	} ಇಲ್ಲದಿದ್ದರೆ {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(ರಚನೆ ul_netaddrq_iface, ip_quality_list_6);
	}

	ಮಿತಿ = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		ರಚನೆ list_ತಲೆ *ipq_list;
		ಎಣಿಕೆ ul_netaddrq_ip_rating t;

		ifaceq = list_ನಮೂದು(li, ರಚನೆ ul_netaddrq_iface, ನಮೂದು);

		ipq_list = (ರಚನೆ list_ತಲೆ*)((ಅಕ್ಷರ*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, ಉತ್ತಮ);
		ಆದರೆ (t < ಮಿತಿ)
		{
			DBG_OBJ(ಉತ್ತಮ_2, *ಉತ್ತಮ,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*ಉತ್ತಮ_ifaceq = ifaceq;
			ಮಿತಿ = t;
		}
	}
	ಹಿಂತಿರುಗಿ ಮಿತಿ;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ul_netaddrq_get_ಉತ್ತಮ_ipp(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl,
				     uint8_t ifa_family,
				     ಎಣಿಕೆ ul_netaddrq_ip_rating *ಮಿತಿ,
				     ರಚನೆ ul_netaddrq_iface **ಉತ್ತಮ_ifaceq)
{
	ರಚನೆ ul_netaddrq_ip *ಉತ್ತಮ[__ULNETLINK_RATING_MAX];

	memset(ಉತ್ತಮ, 0, ಗಾತ್ರ(ಉತ್ತಮ));
	*ಮಿತಿ = ul_netaddrq_bestaddr(nl, ಉತ್ತಮ_ifaceq, &ಉತ್ತಮ, ifa_family);
	ಆದರೆ (*ಮಿತಿ != __ULNETLINK_RATING_MAX)
		ಹಿಂತಿರುಗಿ ul_nl_addr_ntop_ವಿಳಾಸ(ಉತ್ತಮ[*ಮಿತಿ]->addr);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ರಚನೆ ul_netaddrq_iface *ul_netaddrq_iface_by_name(ಸ್ಥಿರಾಂಕ ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl,
						    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ifname)
{
	ರಚನೆ list_ತಲೆ *li;
	ರಚನೆ ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_ನಮೂದು(li, ರಚನೆ ul_netaddrq_iface, ನಮೂದು);

		ಆದರೆ (!strcmp(ifaceq->ifname, ifname))
			ಹಿಂತಿರುಗಿ ifaceq;
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
ಎಣಿಕೆ addrq_print_ಬಹುಲಕ {
	ADDRQ_ಬಹುಲಕ_BESTOFALL,	/* Best address of all interfaces */
	ADDRQ_ಬಹುಲಕ_ಉತ್ತಮ,	/* Best address per interface */
	ADDRQ_ಬಹುಲಕ_GOOD,	/* All global or site addresses, if none, the
				 * longest living temporary, if none, link */
	ADDRQ_ಬಹುಲಕ_ALL		/* All available addresses */
};

/* In our example addrq->callback_data is a simple FILE *. In more complex
 * programs it could be a pointer to an arbitrary struct */
#ವ್ಯಾಖ್ಯಾನಿಸು netout (FILE *)(UL_NETADDRQ_ದತ್ತಾಂಶ(nl)->callback_ದತ್ತಾಂಶ)

/* This example uses separate threshold for IPv4 and IPv6, so the best IPv4 and
 * best IPv6 addresses are printed. */
ಸ್ಥಿರ ಶೂನ್ಯ dump_iface_ಉತ್ತಮ(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl,
			    ರಚನೆ ul_netaddrq_iface *ifaceq)
{
	ರಚನೆ ul_netaddrq_ip *ಉತ್ತಮ[__ULNETLINK_RATING_MAX];
	ಎಣಿಕೆ ul_netaddrq_ip_rating ಮಿತಿ;
	bool ಮೊದಲ = true;

	memset(ಉತ್ತಮ, 0, ಗಾತ್ರ(ಉತ್ತಮ));
	ಮಿತಿ = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &ಉತ್ತಮ);
	ಆದರೆ (ಮಿತಿ != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (ಮೊದಲ ? "best address" : " "),
		       ul_nl_addr_ntop_ವಿಳಾಸ(ಉತ್ತಮ[ಮಿತಿ]->addr));
		ಮೊದಲ = false;
	}
	memset(ಉತ್ತಮ, 0, ಗಾತ್ರ(ಉತ್ತಮ));
	ಮಿತಿ = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &ಉತ್ತಮ);
	ಆದರೆ (ಮಿತಿ != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (ಮೊದಲ ? "best address" : " "),
		       ul_nl_addr_ntop_ವಿಳಾಸ(ಉತ್ತಮ[ಮಿತಿ]->addr));
		ಮೊದಲ = false;
	}
	ಆದರೆ (!ಮೊದಲ)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
ಸ್ಥಿರ ಶೂನ್ಯ dump_iface_good(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl,
			    ರಚನೆ ul_netaddrq_iface *ifaceq)
{
	ರಚನೆ ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	ರಚನೆ ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	ರಚನೆ list_ತಲೆ *li;
	ಎಣಿಕೆ ul_netaddrq_ip_rating ಮಿತಿ = __ULNETLINK_RATING_MAX - 1;
	ಎಣಿಕೆ ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool ಮೊದಲ = true;

	memset(best4, 0, ಗಾತ್ರ(best4));
	ಮಿತಿ = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, ಗಾತ್ರ(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	ಆದರೆ (fthreshold < ಮಿತಿ)
		ಮಿತಿ = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ರಚನೆ ul_netaddrq_ip *ipq;

		ipq = list_ನಮೂದು(li, ರಚನೆ ul_netaddrq_ip, ನಮೂದು);
		ಆದರೆ (ಮಿತಿ <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= ಮಿತಿ ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[ಮಿತಿ])
		{
			ಆದರೆ (ಮೊದಲ)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				ಮೊದಲ = false;
			}
			ಇಲ್ಲದಿದ್ದರೆ
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			ಆದರೆ (ಮಿತಿ == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_ವಿಳಾಸ(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				ಹೋಗು ತಾತ್ಕಾಲಿಕ_cont4;
			}
			ಇಲ್ಲದಿದ್ದರೆ
				fputs(ul_nl_addr_ntop_ವಿಳಾಸ(ipq->addr),
				      netout);
		}
	ತಾತ್ಕಾಲಿಕ_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ರಚನೆ ul_netaddrq_ip *ipq;

		ipq = list_ನಮೂದು(li, ರಚನೆ ul_netaddrq_ip, ನಮೂದು);
		ಆದರೆ (ಮಿತಿ <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= ಮಿತಿ ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[ಮಿತಿ])
		{
			ಆದರೆ (ಮೊದಲ)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				ಮೊದಲ = false;
			}
			ಇಲ್ಲದಿದ್ದರೆ
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			ಆದರೆ (ಮಿತಿ == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_ವಿಳಾಸ(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				ಹೋಗು ತಾತ್ಕಾಲಿಕ_cont6;
			}
			ಇಲ್ಲದಿದ್ದರೆ
				fputs(ul_nl_addr_ntop_ವಿಳಾಸ(ipq->addr),
				      netout);
		}
	ತಾತ್ಕಾಲಿಕ_cont6:;
	}
	ಆದರೆ (!ಮೊದಲ)
		fputs("\n", netout);
}

ಸ್ಥಿರ ಶೂನ್ಯ dump_iface_all(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl,
			   ರಚನೆ ul_netaddrq_iface *ifaceq)
{
	ರಚನೆ list_ತಲೆ *li;
	ರಚನೆ ul_netaddrq_ip *ipq;
	bool ಮೊದಲ = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_ನಮೂದು(li, ರಚನೆ ul_netaddrq_ip, ನಮೂದು);
		ಆದರೆ (ಮೊದಲ)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			ಮೊದಲ = false;
		}
		ಇಲ್ಲದಿದ್ದರೆ
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_ವಿಳಾಸ(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_ನಮೂದು(li, ರಚನೆ ul_netaddrq_ip, ನಮೂದು);
		ಆದರೆ (ಮೊದಲ)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			ಮೊದಲ = false;
		}
		ಇಲ್ಲದಿದ್ದರೆ
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_ವಿಳಾಸ(ipq->addr), netout);
	}
	ಆದರೆ (!ಮೊದಲ)
		fputs("\n", netout);
}

ಸ್ಥಿರ ಶೂನ್ಯ dump_addrq(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl, ಎಣಿಕೆ addrq_print_ಬಹುಲಕ c)
{
	ರಚನೆ list_ತಲೆ *li;
	ರಚನೆ ul_netaddrq_iface *ifaceq;
	ಎಣಿಕೆ ul_netaddrq_ip_rating ಮಿತಿ;

	ಆಯ್ಕೆ(c)
	{
	ಸ್ಥಿತಿ ADDRQ_ಬಹುಲಕ_BESTOFALL:
	{
		ರಚನೆ ul_netaddrq_iface *ಉತ್ತಮ_ifaceq;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಉತ್ತಮ_ipp;

		ಉತ್ತಮ_ipp = ul_netaddrq_get_ಉತ್ತಮ_ipp(nl, AF_INET,
						    &ಮಿತಿ, &ಉತ್ತಮ_ifaceq);
		ಆದರೆ (ಉತ್ತಮ_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				ಉತ್ತಮ_ipp, ಉತ್ತಮ_ifaceq->ifname);
		ಉತ್ತಮ_ipp = ul_netaddrq_get_ಉತ್ತಮ_ipp(nl, AF_INET6,
						    &ಮಿತಿ, &ಉತ್ತಮ_ifaceq);
		ಆದರೆ (ಉತ್ತಮ_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				ಉತ್ತಮ_ipp, ಉತ್ತಮ_ifaceq->ifname);
	}
	ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ ADDRQ_ಬಹುಲಕ_ಉತ್ತಮ:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_ನಮೂದು(li, ರಚನೆ ul_netaddrq_iface, ನಮೂದು);

			dump_iface_ಉತ್ತಮ(nl, ifaceq);
		}
	}
	ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ ADDRQ_ಬಹುಲಕ_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_ನಮೂದು(li, ರಚನೆ ul_netaddrq_iface, ನಮೂದು);

			dump_iface_good(nl, ifaceq);
		}
	}
	ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ ADDRQ_ಬಹುಲಕ_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_ನಮೂದು(li, ರಚನೆ ul_netaddrq_iface, ನಮೂದು);

			dump_iface_all(nl,ifaceq);
		}
	ನಿಲ್ಲಿಸು;
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ callback_post(ರಚನೆ ul_nl_ದತ್ತಾಂಶ *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	ಆದರೆ (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		ಆದರೆ (!(UL_NETADDRQ_ದತ್ತಾಂಶ(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_ದತ್ತಾಂಶ(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			ಹಿಂತಿರುಗಿ 0;
		}
		UL_NETADDRQ_ದತ್ತಾಂಶ(nl)->ifaces_change_4 = false;
		UL_NETADDRQ_ದತ್ತಾಂಶ(nl)->ifaces_change_6 = false;
		fputs("\n\nNetwork change detected:\n", netout);
		fputs("\nbest address:\n", netout);
		dump_addrq(nl, ADDRQ_ಬಹುಲಕ_BESTOFALL);

		fputs("\nbest addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_ಬಹುಲಕ_ಉತ್ತಮ);

		fputs("\ngood addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_ಬಹುಲಕ_GOOD);

		fputs("\nall addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_ಬಹುಲಕ_ALL);
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc __attribute__((__unused__)), ಅಕ್ಷರ *argv[] __attribute__((__unused__)))
{
	ಪೂರ್ಣಾಂಕ rc = 1;
	ಪೂರ್ಣಾಂಕ ulrc;
	ರಚನೆ ul_nl_ದತ್ತಾಂಶ nl;
	FILE *ಹೊರಹೋಗು = stdout;
	ರಚನೆ ul_netaddrq_iface *ifaceq;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	ಆದರೆ ((ul_netaddrq_init(&nl, ಖಾಲಿ, callback_post, (ಶೂನ್ಯ *)ಹೊರಹೋಗು)))
		ಹಿಂತಿರುಗಿ -1;

	/* Dump addresses */
	ಆದರೆ (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		ಹಿಂತಿರುಗಿ -1;
	ಆದರೆ (ul_nl_request_dump(&nl, RTM_GETADDR))
		ಹೋಗು error;
	ಆದರೆ (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_ಮುಗಿದ)
		ಹೋಗು error;
	fputs("RTM_GETADDR dump finished.", ಹೊರಹೋಗು);

	/* Example of several types of dump */
	fputs("\nbest address:\n", ಹೊರಹೋಗು);
	dump_addrq(&nl, ADDRQ_ಬಹುಲಕ_BESTOFALL);

	fputs("\nbest addresses dump:\n", ಹೊರಹೋಗು);
	dump_addrq(&nl, ADDRQ_ಬಹುಲಕ_ಉತ್ತಮ);

	fputs("\ngood addresses dump:\n", ಹೊರಹೋಗು);
	dump_addrq(&nl, ADDRQ_ಬಹುಲಕ_GOOD);

	fputs("\nall addresses dump:\n", ಹೊರಹೋಗು);
	dump_addrq(&nl, ADDRQ_ಬಹುಲಕ_ALL);

	fputs("\naddresses for interface ", ಹೊರಹೋಗು);
	ಆದರೆ ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	ಇಲ್ಲದಿದ್ದರೆ
		fprintf(ಹೊರಹೋಗು, "%s not found.", ifname);

	/* Monitor further changes */
	fputs("\nGoing to monitor mode.\n", ಹೊರಹೋಗು);

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
	ಆದರೆ (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	ಆದರೆ ((ul_nl_close(&nl)))
		rc = 1;
	ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_NETADDRQ */
