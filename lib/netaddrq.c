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

#अन्तर्भाव <net/if.h>
#अन्तर्भाव <netinet/in.h>
#अन्तर्भाव <linux/rtnetlink.h>
#अन्तर्भाव <linux/if_addr.h>
#अन्तर्भाव "netaddrq.h"
#अन्तर्भाव "list.h"
#अन्तर्भाव "debug.h"

/* Maximal number of interfaces. The algorithm has a quadratic complexity,
 * don't overflood it. */
ध्रुव पूर्णाङ्क max_ifaces = 12;

/*
 * Debug stuff (based on include/debug.h)
 */
#परिभाषय ULNETADDRQ_DEBUG_HELP	(1 << 0)
#परिभाषय ULNETADDRQ_DEBUG_INIT	(1 << 1)
#परिभाषय ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#परिभाषय ULNETADDRQ_DEBUG_LIST	(1 << 3)
#परिभाषय ULNETADDRQ_DEBUG_BEST	(1 << 4)

स्थिर UL_DEBUG_DEFINE_आवरणम्(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_BEST,	"searching best address" },

	{ रिक्त, 0 }
};

#परिभाषय DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#परिभाषय DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#परिभाषय ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

स्थिर शून्य netaddrq_init_debug(शून्य)
{
	यदि (netaddrq_debug_आवरणम्)
		प्रत्यागम;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

स्थिर अन्तःस्थ गणना ul_netaddrq_ip_rating
evaluate_ip_quality(संरचना ul_nl_addr *addr) {
	गणना ul_netaddrq_ip_rating quality;

	विकल्प (addr->ifa_scope) {
	पक्ष RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		भङ्ग;
	पक्ष RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		भङ्ग;
	पक्ष RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		भङ्ग;
	स्वभाव:
		quality = ULNETLINK_RATING_BAD;
		भङ्ग;
	}
	यदि (addr->ifa_flags & IFA_F_TEMPORARY) {
		यदि (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	प्रत्यागम quality;
}

#परिभाषय DBG_CASE(x) पक्ष x: str = #x; भङ्ग
#परिभाषय DBG_CASE_DEF8(x) स्वभाव: snprintf(strx+2, 3, "%02hhx", x); str = strx; भङ्ग
स्थिर वर्ण *ip_rating_as_string(गणना ul_netaddrq_ip_rating q)
{
	वर्ण *str;
	स्थिर वर्ण strx[5] = "0x";
	विकल्प (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	प्रत्यागम str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
स्थिर पूर्णाङ्क callback_addrq(संरचना ul_nl_दत्त *nl)
{
	संरचना ul_netaddrq_दत्त *addrq = UL_NETADDRQ_दत्त(nl);
	संरचना list_शीर्ष *li, *ipq_list;
	संरचना ul_netaddrq_iface *ifaceq = रिक्त;
	संरचना ul_netaddrq_ip *ipq = रिक्त;
	पूर्णाङ्क rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_स्थानाङ्कः(&(nl->addr)),
			      nl->addr.ifname));
	यदि (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		यदि ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		संरचना ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_प्रविष्टिः(li, संरचना ul_netaddrq_iface, प्रविष्टिः);
		यदि (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			भङ्ग;
		}
		addrq->nifaces++;
	}

	यदि (ifaceq == रिक्त) {
		यदि (nl->rtm_event) {
			यदि (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->अतिप्रवाहः = true;
				प्रत्यागम UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			यदि (!(ifaceq = स्मृतिं_देहि(परिमाण(संरचना ul_netaddrq_iface))))
				प्रत्यागम -ENOMEM;
			INIT_LIST_शीर्ष(&(ifaceq->ip_quality_list_4));
			INIT_LIST_शीर्ष(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			यदि (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				मुक्त(ifaceq);
				प्रत्यागम -ENOMEM;
			}
			list_सङ्कलय_पुच्छ(&(ifaceq->प्रविष्टिः), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} अन्यथा {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			प्रत्यागम UL_NL_SOFT_ERROR;
		}
	}
	यदि (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} अन्यथा {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		संरचना ul_netaddrq_ip *ipqq;

		ipqq = list_प्रविष्टिः(li, संरचना ul_netaddrq_ip, प्रविष्टिः);
		यदि (ipqq->addr->स्थानाङ्कः_len == nl->addr.स्थानाङ्कः_len)
			यदि (!memcmp(ipqq->addr->स्थानाङ्कः, nl->addr.स्थानाङ्कः,
				   nl->addr.स्थानाङ्कः_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				भङ्ग;
			}
	}
	यदि (ipq == रिक्त) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	यदि (UL_NL_IS_RTM_NEW(nl)) {
		संरचना ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		यदि (!addr) {
			rc = -ENOMEM;
			गच्छ error;
		}
		यदि (ipq == रिक्त) {
			यदि (!(ipq = स्मृतिं_देहि(परिमाण(संरचना ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				गच्छ error;
			}
			ipq->addr = addr;
			list_सङ्कलय_पुच्छ(&(ipq->प्रविष्टिः), ipq_list);
			DBG_OBJ(LIST, ipq, ul_debug("new address"));
			*ifaces_change = true;
		} अन्यथा {
			DBG_OBJ(LIST, addrq, ul_debug("updating address data"));
			ul_nl_addr_free(ipq->addr);
			ipq->addr = addr;
		}
		ipq->quality = evaluate_ip_quality(addr);
		DBG_OBJ(ADDRQ, addrq,
		    ul_debug("%s rating: %s",
				ul_nl_addr_ntop_स्थानाङ्कः(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} अन्यथा {
		/* UL_NL_RTM_DEL */
		यदि (ipq == रिक्त)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			प्रत्यागम UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->प्रविष्टिः));
		ul_nl_addr_free(ipq->addr);
		मुक्त(ipq);
	error:
		यदि (list_रिक्तानि(&(ifaceq->ip_quality_list_4)) &&
		    list_रिक्तानि(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->प्रविष्टिः));
			addrq->nifaces--;
			मुक्त(ifaceq->ifname);
			मुक्त(ifaceq);
		}
	}
	यदि (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		यदि ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	प्रत्यागम rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
पूर्णाङ्क ul_netaddrq_init(संरचना ul_nl_दत्त *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, शून्य *दत्त)
{
	संरचना ul_netaddrq_दत्त *addrq;

	netaddrq_init_debug();
	यदि (!(nl->दत्त_addr = शून्यस्मृतिं_देहि(1, परिमाण(संरचना ul_netaddrq_दत्त))))
		प्रत्यागम -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_दत्त(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_दत्त = दत्त;
	INIT_LIST_शीर्ष(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	प्रत्यागम 0;
}

गणना ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(संरचना list_शीर्ष *ipq_list,
			   संरचना ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX])
{
	संरचना list_शीर्ष *li;
	संरचना ul_netaddrq_ip *ipq;
	गणना ul_netaddrq_ip_rating देहली;

	देहली = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_प्रविष्टिः(li, संरचना ul_netaddrq_ip, प्रविष्टिः);

		यदि (!(*best)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*best)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(BEST, (*best),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_स्थानाङ्कः(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*best)[ipq->quality] = ipq;
		}

		यदि (ipq->quality < देहली)
		{
			देहली = ipq->quality;
			DBG(BEST,
			    ul_debug("threshold %s", ip_rating_as_string(देहली)));

		}
	}
	प्रत्यागम देहली;
}

गणना ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(संरचना ul_nl_दत्त *nl,
		     संरचना ul_netaddrq_iface **best_ifaceq,
		     संरचना ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	संरचना ul_netaddrq_दत्त *addrq = UL_NETADDRQ_दत्त(nl);
	संरचना list_शीर्ष *li;
	संरचना ul_netaddrq_iface *ifaceq;
	size_t ipqo;
	गणना ul_netaddrq_ip_rating देहली;

	यदि (ifa_family == AF_INET) {
		ipqo = offsetof(संरचना ul_netaddrq_iface, ip_quality_list_4);
	} अन्यथा {
	/* if (ifa_family == AF_INET6) */
		ipqo = offsetof(संरचना ul_netaddrq_iface, ip_quality_list_6);
	}

	देहली = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		संरचना list_शीर्ष *ipq_list;
		गणना ul_netaddrq_ip_rating t;

		ifaceq = list_प्रविष्टिः(li, संरचना ul_netaddrq_iface, प्रविष्टिः);

		ipq_list = (संरचना list_शीर्ष*)((वर्ण*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, best);
		यदि (t < देहली)
		{
			DBG_OBJ(BEST, *best,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*best_ifaceq = ifaceq;
			देहली = t;
		}
	}
	प्रत्यागम देहली;
}

ध्रुव वर्ण *ul_netaddrq_get_best_ipp(संरचना ul_nl_दत्त *nl,
				     uint8_t ifa_family,
				     गणना ul_netaddrq_ip_rating *देहली,
				     संरचना ul_netaddrq_iface **best_ifaceq)
{
	संरचना ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];

	स्मृतिं_पूरय(best, 0, परिमाण(best));
	*देहली = ul_netaddrq_bestaddr(nl, best_ifaceq, &best, ifa_family);
	यदि (*देहली != __ULNETLINK_RATING_MAX)
		प्रत्यागम ul_nl_addr_ntop_स्थानाङ्कः(best[*देहली]->addr);
	प्रत्यागम रिक्त;
}

संरचना ul_netaddrq_iface *ul_netaddrq_iface_by_name(ध्रुव संरचना ul_nl_दत्त *nl,
						    ध्रुव वर्ण *ifname)
{
	संरचना list_शीर्ष *li;
	संरचना ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_प्रविष्टिः(li, संरचना ul_netaddrq_iface, प्रविष्टिः);

		यदि (!वाक्यं_तुलय(ifaceq->ifname, ifname))
			प्रत्यागम ifaceq;
	}
	प्रत्यागम रिक्त;
}

#यदि_परिभाषितम् TEST_क्रमादेशः_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
गणना addrq_print_बहुलकम् {
	ADDRQ_बहुलकम्_BESTOFALL,	/* Best address of all interfaces */
	ADDRQ_बहुलकम्_BEST,	/* Best address per interface */
	ADDRQ_बहुलकम्_GOOD,	/* All global or site addresses, if none, the
				 * longest living temporary, if none, link */
	ADDRQ_बहुलकम्_ALL		/* All available addresses */
};

/* In our example addrq->callback_data is a simple FILE *. In more complex
 * programs it could be a pointer to an arbitrary struct */
#परिभाषय netout (FILE *)(UL_NETADDRQ_दत्त(nl)->callback_दत्त)

/* This example uses separate threshold for IPv4 and IPv6, so the best IPv4 and
 * best IPv6 addresses are printed. */
स्थिर शून्य dump_iface_best(संरचना ul_nl_दत्त *nl,
			    संरचना ul_netaddrq_iface *ifaceq)
{
	संरचना ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];
	गणना ul_netaddrq_ip_rating देहली;
	bool first = true;

	स्मृतिं_पूरय(best, 0, परिमाण(best));
	देहली = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best);
	यदि (देहली != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv4: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_स्थानाङ्कः(best[देहली]->addr));
		first = false;
	}
	स्मृतिं_पूरय(best, 0, परिमाण(best));
	देहली = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &best);
	यदि (देहली != __ULNETLINK_RATING_MAX)
	{
		fprintf(netout, "%s IPv6: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_स्थानाङ्कः(best[देहली]->addr));
		first = false;
	}
	यदि (!first)
		fprintf(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
स्थिर शून्य dump_iface_good(संरचना ul_nl_दत्त *nl,
			    संरचना ul_netaddrq_iface *ifaceq)
{
	संरचना ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	संरचना ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	संरचना list_शीर्ष *li;
	गणना ul_netaddrq_ip_rating देहली = __ULNETLINK_RATING_MAX - 1;
	गणना ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool first = true;

	स्मृतिं_पूरय(best4, 0, परिमाण(best4));
	देहली = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	स्मृतिं_पूरय(best6, 0, परिमाण(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	यदि (fthreshold < देहली)
		देहली = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		संरचना ul_netaddrq_ip *ipq;

		ipq = list_प्रविष्टिः(li, संरचना ul_netaddrq_ip, प्रविष्टिः);
		यदि (देहली <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= देहली ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[देहली])
		{
			यदि (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			अन्यथा
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			यदि (देहली == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_स्थानाङ्कः(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				गच्छ अस्थायी_cont4;
			}
			अन्यथा
				fputs(ul_nl_addr_ntop_स्थानाङ्कः(ipq->addr),
				      netout);
		}
	अस्थायी_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		संरचना ul_netaddrq_ip *ipq;

		ipq = list_प्रविष्टिः(li, संरचना ul_netaddrq_ip, प्रविष्टिः);
		यदि (देहली <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= देहली ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[देहली])
		{
			यदि (first)
			{
				fprintf(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			अन्यथा
				fprintf(netout, " ");
			/* Write only the longest living temporary address */
			यदि (देहली == ULNETLINK_RATING_F_TEMPORARY)
			{
				fputs(ul_nl_addr_ntop_स्थानाङ्कः(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				गच्छ अस्थायी_cont6;
			}
			अन्यथा
				fputs(ul_nl_addr_ntop_स्थानाङ्कः(ipq->addr),
				      netout);
		}
	अस्थायी_cont6:;
	}
	यदि (!first)
		fputs("\n", netout);
}

स्थिर शून्य dump_iface_all(संरचना ul_nl_दत्त *nl,
			   संरचना ul_netaddrq_iface *ifaceq)
{
	संरचना list_शीर्ष *li;
	संरचना ul_netaddrq_ip *ipq;
	bool first = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_प्रविष्टिः(li, संरचना ul_netaddrq_ip, प्रविष्टिः);
		यदि (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		अन्यथा
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_स्थानाङ्कः(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_प्रविष्टिः(li, संरचना ul_netaddrq_ip, प्रविष्टिः);
		यदि (first)
		{
			fprintf(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		अन्यथा
			fprintf(netout, " ");
		fputs(ul_nl_addr_ntop_स्थानाङ्कः(ipq->addr), netout);
	}
	यदि (!first)
		fputs("\n", netout);
}

स्थिर शून्य dump_addrq(संरचना ul_nl_दत्त *nl, गणना addrq_print_बहुलकम् c)
{
	संरचना list_शीर्ष *li;
	संरचना ul_netaddrq_iface *ifaceq;
	गणना ul_netaddrq_ip_rating देहली;

	विकल्प(c)
	{
	पक्ष ADDRQ_बहुलकम्_BESTOFALL:
	{
		संरचना ul_netaddrq_iface *best_ifaceq;
		ध्रुव वर्ण *best_ipp;

		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET,
						    &देहली, &best_ifaceq);
		यदि (best_ipp)
			fprintf(netout, "best IPv4 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET6,
						    &देहली, &best_ifaceq);
		यदि (best_ipp)
			fprintf(netout, "best IPv6 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
	}
	भङ्ग;
	पक्ष ADDRQ_बहुलकम्_BEST:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_प्रविष्टिः(li, संरचना ul_netaddrq_iface, प्रविष्टिः);

			dump_iface_best(nl, ifaceq);
		}
	}
	भङ्ग;
	पक्ष ADDRQ_बहुलकम्_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_प्रविष्टिः(li, संरचना ul_netaddrq_iface, प्रविष्टिः);

			dump_iface_good(nl, ifaceq);
		}
	}
	भङ्ग;
	पक्ष ADDRQ_बहुलकम्_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_प्रविष्टिः(li, संरचना ul_netaddrq_iface, प्रविष्टिः);

			dump_iface_all(nl,ifaceq);
		}
	भङ्ग;
	}
}

स्थिर पूर्णाङ्क callback_post(संरचना ul_nl_दत्त *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	यदि (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		यदि (!(UL_NETADDRQ_दत्त(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_दत्त(nl)->ifaces_change_6))
		{
			fputs("\n\nNo changes in the address list.\n", netout);
			प्रत्यागम 0;
		}
		UL_NETADDRQ_दत्त(nl)->ifaces_change_4 = false;
		UL_NETADDRQ_दत्त(nl)->ifaces_change_6 = false;
		fputs("\n\nNetwork change detected:\n", netout);
		fputs("\nbest address:\n", netout);
		dump_addrq(nl, ADDRQ_बहुलकम्_BESTOFALL);

		fputs("\nbest addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_बहुलकम्_BEST);

		fputs("\ngood addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_बहुलकम्_GOOD);

		fputs("\nall addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_बहुलकम्_ALL);
	}
	प्रत्यागम 0;
}

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc __attribute__((__unused__)), वर्ण *argv[] __attribute__((__unused__)))
{
	पूर्णाङ्क rc = 1;
	पूर्णाङ्क ulrc;
	संरचना ul_nl_दत्त nl;
	FILE *बहिर्गमनम् = stdout;
	संरचना ul_netaddrq_iface *ifaceq;
	ध्रुव वर्ण *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	यदि ((ul_netaddrq_init(&nl, रिक्त, callback_post, (शून्य *)बहिर्गमनम्)))
		प्रत्यागम -1;

	/* Dump addresses */
	यदि (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		प्रत्यागम -1;
	यदि (ul_nl_request_dump(&nl, RTM_GETADDR))
		गच्छ error;
	यदि (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_समाप्तम्)
		गच्छ error;
	fputs("RTM_GETADDR dump finished.", बहिर्गमनम्);

	/* Example of several types of dump */
	fputs("\nbest address:\n", बहिर्गमनम्);
	dump_addrq(&nl, ADDRQ_बहुलकम्_BESTOFALL);

	fputs("\nbest addresses dump:\n", बहिर्गमनम्);
	dump_addrq(&nl, ADDRQ_बहुलकम्_BEST);

	fputs("\ngood addresses dump:\n", बहिर्गमनम्);
	dump_addrq(&nl, ADDRQ_बहुलकम्_GOOD);

	fputs("\nall addresses dump:\n", बहिर्गमनम्);
	dump_addrq(&nl, ADDRQ_बहुलकम्_ALL);

	fputs("\naddresses for interface ", बहिर्गमनम्);
	यदि ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	अन्यथा
		fprintf(बहिर्गमनम्, "%s not found.", ifname);

	/* Monitor further changes */
	fputs("\nGoing to monitor mode.\n", बहिर्गमनम्);

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
	यदि (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	यदि ((ul_nl_close(&nl)))
		rc = 1;
	प्रत्यागम rc;
}
#यदि_अन्तः /* TEST_PROGRAM_NETADDRQ */
