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

#అన్తర్భావ <net/if.h>
#అన్తర్భావ <netinet/in.h>
#అన్తర్భావ <linux/rtnetlink.h>
#అన్తర్భావ <linux/if_addr.h>
#అన్తర్భావ "netaddrq.h"
#అన్తర్భావ "list.h"
#అన్తర్భావ "debug.h"

/* Maximal number of interfaces. The algorithm has a quadratic complexity,
 * don't overflood it. */
స్థిరరాశి పూర్ణాంకం max_ifaces = 12;

/*
 * Debug stuff (based on include/debug.h)
 */
#నిర్వచించు ULNETADDRQ_DEBUG_HELP	(1 << 0)
#నిర్వచించు ULNETADDRQ_DEBUG_INIT	(1 << 1)
#నిర్వచించు ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#నిర్వచించు ULNETADDRQ_DEBUG_LIST	(1 << 3)
#నిర్వచించు ULNETADDRQ_DEBUG_ఉత్తమ	(1 << 4)

స్థిర UL_DEBUG_DEFINE_ముసుగు(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_ఉత్తమ,	"searching best address" },

	{ ఖాళీ, 0 }
};

#నిర్వచించు DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#నిర్వచించు DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#నిర్వచించు ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

స్థిర శూన్య netaddrq_init_debug(శూన్య)
{
	ఒకవేళ (netaddrq_debug_ముసుగు)
		తిరిగి;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

స్థిర అంతఃవరుస గణన ul_netaddrq_ip_rating
evaluate_ip_quality(నిర్మాణం ul_nl_addr *addr) {
	గణన ul_netaddrq_ip_rating quality;

	ఎంపిక (addr->ifa_scope) {
	స్థితి RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		ఆపు;
	స్థితి RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		ఆపు;
	స్థితి RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		ఆపు;
	అప్రమేయ:
		quality = ULNETLINK_RATING_BAD;
		ఆపు;
	}
	ఒకవేళ (addr->ifa_flags & IFA_F_TEMPORARY) {
		ఒకవేళ (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	తిరిగి quality;
}

#నిర్వచించు DBG_CASE(x) స్థితి x: str = #x; ఆపు
#నిర్వచించు DBG_CASE_DEF8(x) అప్రమేయ: snprintf(strx+2, 3, "%02hhx", x); str = strx; ఆపు
స్థిర అక్షరం *ip_rating_as_string(గణన ul_netaddrq_ip_rating q)
{
	అక్షరం *str;
	స్థిర అక్షరం strx[5] = "0x";
	ఎంపిక (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	తిరిగి str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
స్థిర పూర్ణాంకం callback_addrq(నిర్మాణం ul_nl_దత్తాంశం *nl)
{
	నిర్మాణం ul_netaddrq_దత్తాంశం *addrq = UL_NETADDRQ_దత్తాంశం(nl);
	నిర్మాణం list_తల *li, *ipq_list;
	నిర్మాణం ul_netaddrq_iface *ifaceq = ఖాళీ;
	నిర్మాణం ul_netaddrq_ip *ipq = ఖాళీ;
	పూర్ణాంకం rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_చిరునామా(&(nl->addr)),
			      nl->addr.ifname));
	ఒకవేళ (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		ఒకవేళ ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		నిర్మాణం ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_నమోదు(li, నిర్మాణం ul_netaddrq_iface, నమోదు_2);
		ఒకవేళ (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			ఆపు;
		}
		addrq->nifaces++;
	}

	ఒకవేళ (ifaceq == ఖాళీ) {
		ఒకవేళ (nl->rtm_event) {
			ఒకవేళ (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->అధికప్రవాహం = true;
				తిరిగి UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			ఒకవేళ (!(ifaceq = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం ul_netaddrq_iface))))
				తిరిగి -ENOMEM;
			INIT_LIST_తల(&(ifaceq->ip_quality_list_4));
			INIT_LIST_తల(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			ఒకవేళ (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				విడుదల(ifaceq);
				తిరిగి -ENOMEM;
			}
			list_కూడు_తోక(&(ifaceq->నమోదు_2), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} లేకపోతే {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			తిరిగి UL_NL_SOFT_ERROR;
		}
	}
	ఒకవేళ (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} లేకపోతే {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		నిర్మాణం ul_netaddrq_ip *ipqq;

		ipqq = list_నమోదు(li, నిర్మాణం ul_netaddrq_ip, నమోదు_2);
		ఒకవేళ (ipqq->addr->చిరునామా_len == nl->addr.చిరునామా_len)
			ఒకవేళ (!memcmp(ipqq->addr->చిరునామా, nl->addr.చిరునామా,
				   nl->addr.చిరునామా_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				ఆపు;
			}
	}
	ఒకవేళ (ipq == ఖాళీ) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	ఒకవేళ (UL_NL_IS_RTM_NEW(nl)) {
		నిర్మాణం ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		ఒకవేళ (!addr) {
			rc = -ENOMEM;
			వెళ్ళు error;
		}
		ఒకవేళ (ipq == ఖాళీ) {
			ఒకవేళ (!(ipq = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				వెళ్ళు error;
			}
			ipq->addr = addr;
			list_కూడు_తోక(&(ipq->నమోదు_2), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} లేకపోతే {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_చిరునామా(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} లేకపోతే {
		/* UL_NL_RTM_DEL */
		ఒకవేళ (ipq == ఖాళీ)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			తిరిగి UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->నమోదు_2));
		ul_nl_addr_free(ipq->addr);
		విడుదల(ipq);
	error:
		ఒకవేళ (list_ఖాళీలు(&(ifaceq->ip_quality_list_4)) &&
		    list_ఖాళీలు(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->నమోదు_2));
			addrq->nifaces--;
			విడుదల(ifaceq->ifname);
			విడుదల(ifaceq);
		}
	}
	ఒకవేళ (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		ఒకవేళ ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	తిరిగి rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
పూర్ణాంకం ul_netaddrq_init(నిర్మాణం ul_nl_దత్తాంశం *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, శూన్య *దత్తాంశం)
{
	నిర్మాణం ul_netaddrq_దత్తాంశం *addrq;

	netaddrq_init_debug();
	ఒకవేళ (!(nl->దత్తాంశం_addr = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం ul_netaddrq_దత్తాంశం))))
		తిరిగి -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_దత్తాంశం(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_దత్తాంశం = దత్తాంశం;
	INIT_LIST_తల(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	తిరిగి 0;
}

గణన ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(నిర్మాణం list_తల *ipq_list,
			   నిర్మాణం ul_netaddrq_ip *(*ఉత్తమ)[__ULNETLINK_RATING_MAX])
{
	నిర్మాణం list_తల *li;
	నిర్మాణం ul_netaddrq_ip *ipq;
	గణన ul_netaddrq_ip_rating పరిమితి;

	పరిమితి = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_నమోదు(li, నిర్మాణం ul_netaddrq_ip, నమోదు_2);

		ఒకవేళ (!(*ఉత్తమ)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*ఉత్తమ)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(ఉత్తమ_2, (*ఉత్తమ),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_చిరునామా(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*ఉత్తమ)[ipq->quality] = ipq;
		}

		ఒకవేళ (ipq->quality < పరిమితి)
		{
			పరిమితి = ipq->quality;
			DBG(ఉత్తమ_2,
			    ul_debug("threshold %s", ip_rating_as_string(పరిమితి)));

		}
	}
	తిరిగి పరిమితి;
}

గణన ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(నిర్మాణం ul_nl_దత్తాంశం *nl,
		     నిర్మాణం ul_netaddrq_iface **ఉత్తమ_ifaceq,
		     నిర్మాణం ul_netaddrq_ip *(*ఉత్తమ)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	నిర్మాణం ul_netaddrq_దత్తాంశం *addrq = UL_NETADDRQ_దత్తాంశం(nl);
	నిర్మాణం list_తల *li;
	నిర్మాణం ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	గణన ul_netaddrq_ip_rating పరిమితి;

	ఒకవేళ (ifa_family == AF_INET) {
		ipqo = offsetof(నిర్మాణం ul_netaddrq_iface, ip_quality_list_4);
	} లేకపోతే {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(నిర్మాణం ul_netaddrq_iface, ip_quality_list_6);
	}

	పరిమితి = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		నిర్మాణం list_తల *ipq_list;
		గణన ul_netaddrq_ip_rating t;

		ifaceq = list_నమోదు(li, నిర్మాణం ul_netaddrq_iface, నమోదు_2);

		ipq_list = (నిర్మాణం list_తల*)((అక్షరం*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, ఉత్తమ);
		ఒకవేళ (t < పరిమితి)
		{
			DBG_OBJ(ఉత్తమ_2, *ఉత్తమ,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*ఉత్తమ_ifaceq = ifaceq;
			పరిమితి = t;
		}
	}
	తిరిగి పరిమితి;
}

స్థిరరాశి అక్షరం *ul_netaddrq_get_ఉత్తమ_ipp(నిర్మాణం ul_nl_దత్తాంశం *nl,
				     uint8_t ifa_family,
				     గణన ul_netaddrq_ip_rating *పరిమితి,
				     నిర్మాణం ul_netaddrq_iface **ఉత్తమ_ifaceq)
{
	నిర్మాణం ul_netaddrq_ip *ఉత్తమ[__ULNETLINK_RATING_MAX];

	memset(ఉత్తమ, 0, పరిమాణం(ఉత్తమ));
	*పరిమితి = ul_netaddrq_bestaddr(nl, ఉత్తమ_ifaceq, &ఉత్తమ, ifa_family);
	ఒకవేళ (*పరిమితి != __ULNETLINK_RATING_MAX)
		తిరిగి ul_nl_addr_ntop_చిరునామా(ఉత్తమ[*పరిమితి]->addr);
	తిరిగి ఖాళీ;
}

నిర్మాణం ul_netaddrq_iface *ul_netaddrq_iface_by_name(స్థిరరాశి నిర్మాణం ul_nl_దత్తాంశం *nl,
						    స్థిరరాశి అక్షరం *ifname)
{
	నిర్మాణం list_తల *li;
	నిర్మాణం ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_నమోదు(li, నిర్మాణం ul_netaddrq_iface, నమోదు_2);

		ఒకవేళ (!strcmp(ifaceq->ifname, ifname))
			తిరిగి ifaceq;
	}
	తిరిగి ఖాళీ;
}

#నిర్వచితమైతే TEST_కార్యక్రమం_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
గణన addrq_print_బహుళకం {
	ADDRQ_బహుళకం_BESTOFALL,	/* Best address of all interfaces */
	ADDRQ_బహుళకం_ఉత్తమ,	/* Best address per interface */
	ADDRQ_బహుళకం_GOOD,	/* All global or site addresses, if none, the
				 * longest living temporary, if none, link */
	ADDRQ_బహుళకం_ALL		/* All available addresses */
};

/* In our example addrq->callback_data is a simple FILE *. In more complex
 * programs it could be a pointer to an arbitrary struct */
#నిర్వచించు netout (FILE *)(UL_NETADDRQ_దత్తాంశం(nl)->callback_దత్తాంశం)

/* This example uses separate threshold for IPv4 and IPv6, so the best IPv4 and
 * best IPv6 addresses are printed. */
స్థిర శూన్య dump_iface_ఉత్తమ(నిర్మాణం ul_nl_దత్తాంశం *nl,
			    నిర్మాణం ul_netaddrq_iface *ifaceq)
{
	నిర్మాణం ul_netaddrq_ip *ఉత్తమ[__ULNETLINK_RATING_MAX];
	గణన ul_netaddrq_ip_rating పరిమితి;
	bool మొదటి = true;

	memset(ఉత్తమ, 0, పరిమాణం(ఉత్తమ));
	పరిమితి = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &ఉత్తమ);
	ఒకవేళ (పరిమితి != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (మొదటి ? "best address" : " "),
		       ul_nl_addr_ntop_చిరునామా(ఉత్తమ[పరిమితి]->addr));
		మొదటి = false;
	}
	memset(ఉత్తమ, 0, పరిమాణం(ఉత్తమ));
	పరిమితి = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &ఉత్తమ);
	ఒకవేళ (పరిమితి != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (మొదటి ? "best address" : " "),
		       ul_nl_addr_ntop_చిరునామా(ఉత్తమ[పరిమితి]->addr));
		మొదటి = false;
	}
	ఒకవేళ (!మొదటి)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
స్థిర శూన్య dump_iface_good(నిర్మాణం ul_nl_దత్తాంశం *nl,
			    నిర్మాణం ul_netaddrq_iface *ifaceq)
{
	నిర్మాణం ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	నిర్మాణం ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	నిర్మాణం list_తల *li;
	గణన ul_netaddrq_ip_rating పరిమితి = __ULNETLINK_RATING_MAX - 1;
	గణన ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool మొదటి = true;

	memset(best4, 0, పరిమాణం(best4));
	పరిమితి = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	memset(best6, 0, పరిమాణం(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	ఒకవేళ (fthreshold < పరిమితి)
		పరిమితి = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		నిర్మాణం ul_netaddrq_ip *ipq;

		ipq = list_నమోదు(li, నిర్మాణం ul_netaddrq_ip, నమోదు_2);
		ఒకవేళ (పరిమితి <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= పరిమితి ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[పరిమితి])
		{
			ఒకవేళ (మొదటి)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				మొదటి = false;
			}
			లేకపోతే
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			ఒకవేళ (పరిమితి == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_చిరునామా(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				వెళ్ళు తాత్కాలిక_cont4;
			}
			లేకపోతే
				fputs(ul_nl_addr_ntop_చిరునామా(ipq->addr),
				      netout);
		}
	తాత్కాలిక_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		నిర్మాణం ul_netaddrq_ip *ipq;

		ipq = list_నమోదు(li, నిర్మాణం ul_netaddrq_ip, నమోదు_2);
		ఒకవేళ (పరిమితి <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= పరిమితి ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[పరిమితి])
		{
			ఒకవేళ (మొదటి)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				మొదటి = false;
			}
			లేకపోతే
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			ఒకవేళ (పరిమితి == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_చిరునామా(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				వెళ్ళు తాత్కాలిక_cont6;
			}
			లేకపోతే
				fputs(ul_nl_addr_ntop_చిరునామా(ipq->addr),
				      netout);
		}
	తాత్కాలిక_cont6:;
	}
	ఒకవేళ (!మొదటి)
		fputs("\n", netout);
}

స్థిర శూన్య dump_iface_all(నిర్మాణం ul_nl_దత్తాంశం *nl,
			   నిర్మాణం ul_netaddrq_iface *ifaceq)
{
	నిర్మాణం list_తల *li;
	నిర్మాణం ul_netaddrq_ip *ipq;
	bool మొదటి = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_నమోదు(li, నిర్మాణం ul_netaddrq_ip, నమోదు_2);
		ఒకవేళ (మొదటి)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			మొదటి = false;
		}
		లేకపోతే
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_చిరునామా(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_నమోదు(li, నిర్మాణం ul_netaddrq_ip, నమోదు_2);
		ఒకవేళ (మొదటి)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			మొదటి = false;
		}
		లేకపోతే
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_చిరునామా(ipq->addr), netout);
	}
	ఒకవేళ (!మొదటి)
		fputs("\n", netout);
}

స్థిర శూన్య dump_addrq(నిర్మాణం ul_nl_దత్తాంశం *nl, గణన addrq_print_బహుళకం c)
{
	నిర్మాణం list_తల *li;
	నిర్మాణం ul_netaddrq_iface *ifaceq;
	గణన ul_netaddrq_ip_rating పరిమితి;

	ఎంపిక(c)
	{
	స్థితి ADDRQ_బహుళకం_BESTOFALL:
	{
		నిర్మాణం ul_netaddrq_iface *ఉత్తమ_ifaceq;
		స్థిరరాశి అక్షరం *ఉత్తమ_ipp;

		ఉత్తమ_ipp = ul_netaddrq_get_ఉత్తమ_ipp(nl, AF_INET,
						    &పరిమితి, &ఉత్తమ_ifaceq);
		ఒకవేళ (ఉత్తమ_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				ఉత్తమ_ipp, ఉత్తమ_ifaceq->ifname);
		ఉత్తమ_ipp = ul_netaddrq_get_ఉత్తమ_ipp(nl, AF_INET6,
						    &పరిమితి, &ఉత్తమ_ifaceq);
		ఒకవేళ (ఉత్తమ_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				ఉత్తమ_ipp, ఉత్తమ_ifaceq->ifname);
	}
	ఆపు;
	స్థితి ADDRQ_బహుళకం_ఉత్తమ:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_నమోదు(li, నిర్మాణం ul_netaddrq_iface, నమోదు_2);

			dump_iface_ఉత్తమ(nl, ifaceq);
		}
	}
	ఆపు;
	స్థితి ADDRQ_బహుళకం_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_నమోదు(li, నిర్మాణం ul_netaddrq_iface, నమోదు_2);

			dump_iface_good(nl, ifaceq);
		}
	}
	ఆపు;
	స్థితి ADDRQ_బహుళకం_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_నమోదు(li, నిర్మాణం ul_netaddrq_iface, నమోదు_2);

			dump_iface_all(nl,ifaceq);
		}
	ఆపు;
	}
}

స్థిర పూర్ణాంకం callback_post(నిర్మాణం ul_nl_దత్తాంశం *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	ఒకవేళ (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		ఒకవేళ (!(UL_NETADDRQ_దత్తాంశం(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_దత్తాంశం(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			తిరిగి 0;
		}
		UL_NETADDRQ_దత్తాంశం(nl)->ifaces_change_4 = false;
		UL_NETADDRQ_దత్తాంశం(nl)->ifaces_change_6 = false;
		fputs("\n\nNetwork change detected:\n", netout);
		fputs("\nbest address:\n", netout);
		dump_addrq(nl, ADDRQ_బహుళకం_BESTOFALL);

		fputs("\nbest addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_బహుళకం_ఉత్తమ);

		fputs("\ngood addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_బహుళకం_GOOD);

		fputs("\nall addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_బహుళకం_ALL);
	}
	తిరిగి 0;
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc __attribute__((__unused__)), అక్షరం *argv[] __attribute__((__unused__)))
{
	పూర్ణాంకం rc = 1;
	పూర్ణాంకం ulrc;
	నిర్మాణం ul_nl_దత్తాంశం nl;
	FILE *బహిర్గతం = stdout;
	నిర్మాణం ul_netaddrq_iface *ifaceq;
	స్థిరరాశి అక్షరం *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	ఒకవేళ ((ul_netaddrq_init(&nl, ఖాళీ, callback_post, (శూన్య *)బహిర్గతం)))
		తిరిగి -1;

	/* Dump addresses */
	ఒకవేళ (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		తిరిగి -1;
	ఒకవేళ (ul_nl_request_dump(&nl, RTM_GETADDR))
		వెళ్ళు error;
	ఒకవేళ (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_పూర్తి)
		వెళ్ళు error;
	fputs("RTM_GETADDR dump finished.", బహిర్గతం);

	/* Example of several types of dump */
	fputs("\nbest address:\n", బహిర్గతం);
	dump_addrq(&nl, ADDRQ_బహుళకం_BESTOFALL);

	fputs("\nbest addresses dump:\n", బహిర్గతం);
	dump_addrq(&nl, ADDRQ_బహుళకం_ఉత్తమ);

	fputs("\ngood addresses dump:\n", బహిర్గతం);
	dump_addrq(&nl, ADDRQ_బహుళకం_GOOD);

	fputs("\nall addresses dump:\n", బహిర్గతం);
	dump_addrq(&nl, ADDRQ_బహుళకం_ALL);

	fputs("\naddresses for interface ", బహిర్గతం);
	ఒకవేళ ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	లేకపోతే
		fprintf(బహిర్గతం, "%s not found.", ifname);

	/* Monitor further changes */
	fputs("\nGoing to monitor mode.\n", బహిర్గతం);

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
	ఒకవేళ (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	ఒకవేళ ((ul_nl_close(&nl)))
		rc = 1;
	తిరిగి rc;
}
#అయితే_ముగింపు /* TEST_PROGRAM_NETADDRQ */
