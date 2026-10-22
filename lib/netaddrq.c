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

#समावेश <net/if.h>
#समावेश <netinet/in.h>
#समावेश <linux/rtnetlink.h>
#समावेश <linux/if_addr.h>
#समावेश "netaddrq.h"
#समावेश "list.h"
#समावेश "debug.h"

/* Maximal number of interfaces. The algorithm has a quadratic complexity,
 * don't overflood it. */
स्थिर पूर्णांक max_ifaces = 12;

/*
 * Debug stuff (based on include/debug.h)
 */
#घोषणा ULNETADDRQ_DEBUG_HELP	(1 << 0)
#घोषणा ULNETADDRQ_DEBUG_INIT	(1 << 1)
#घोषणा ULNETADDRQ_DEBUG_ADDRQ	(1 << 2)
#घोषणा ULNETADDRQ_DEBUG_LIST	(1 << 3)
#घोषणा ULNETADDRQ_DEBUG_BEST	(1 << 4)

अटल UL_DEBUG_DEFINE_MASK(netaddrq);
UL_DEBUG_DEFINE_MASKNAMES(netaddrq) =
{
	{ "all",   UL_DEBUG_ALL,		"complete address processing" },
	{ "help",  ULNETADDRQ_DEBUG_HELP,	"this help" },
	{ "addrq", ULNETADDRQ_DEBUG_ADDRQ,	"address rating" },
	{ "list",  ULNETADDRQ_DEBUG_LIST,	"list processing" },
	{ "best",  ULNETADDRQ_DEBUG_BEST,	"searching best address" },

	{ शून्य, 0 }
};

#घोषणा DBG(m, x)		__UL_DBG(netaddrq, ULNETADDRQ_DEBUG_, m, x)
#घोषणा DBG_OBJ(m, h, x)	__UL_DBG_OBJ(netaddrq, ULNETADDRQ_DEBUG_, m, h, x)
#घोषणा ON_DBG(m, x)		__UL_DBG_CALL(netaddrq, ULNETADDRQ_DEBUG_, m, x)

अटल व्योम netaddrq_init_debug(व्योम)
{
	अगर (netaddrq_debug_mask)
		वापस;

	__UL_INIT_DEBUG_FROM_ENV(netaddrq, ULNETADDRQ_DEBUG_, 0,
				 ULNETADDRQ_DEBUG);

	ON_DBG(HELP, ul_debug_print_masks("ULNETADDRQ_DEBUG",
				UL_DEBUG_MASKNAMES(netaddrq)));
}

अटल अंतरभूत क्रमागत ul_netaddrq_ip_rating
evaluate_ip_quality(काष्ठा ul_nl_addr *addr) {
	क्रमागत ul_netaddrq_ip_rating quality;

	चयन (addr->ifa_scope) {
	हाल RT_SCOPE_UNIVERSE:
		quality = ULNETLINK_RATING_SCOPE_UNIVERSE;
		अवरोध;
	हाल RT_SCOPE_LINK:
		quality = ULNETLINK_RATING_SCOPE_LINK;
		अवरोध;
	हाल RT_SCOPE_SITE:
		quality = ULNETLINK_RATING_SCOPE_SITE;
		अवरोध;
	शेष:
		quality = ULNETLINK_RATING_BAD;
		अवरोध;
	}
	अगर (addr->ifa_flags & IFA_F_TEMPORARY) {
		अगर (quality <= ULNETLINK_RATING_F_TEMPORARY)
			quality = ULNETLINK_RATING_F_TEMPORARY;
	}
	वापस quality;
}

#घोषणा DBG_CASE(x) हाल x: str = #x; अवरोध
#घोषणा DBG_CASE_DEF8(x) शेष: snprintf(strx+2, 3, "%02hhx", x); str = strx; अवरोध
अटल अक्षर *ip_rating_as_string(क्रमागत ul_netaddrq_ip_rating q)
{
	अक्षर *str;
	अटल अक्षर strx[5] = "0x";
	चयन (q) {
		DBG_CASE(ULNETLINK_RATING_SCOPE_UNIVERSE);
		DBG_CASE(ULNETLINK_RATING_SCOPE_SITE);
		DBG_CASE(ULNETLINK_RATING_F_TEMPORARY);
		DBG_CASE(ULNETLINK_RATING_SCOPE_LINK);
		DBG_CASE(ULNETLINK_RATING_BAD);
		DBG_CASE_DEF8(q);
	}
	वापस str;
}

/* Netlink callback evaluating the address quality and building the list of
 * interface lists */
अटल पूर्णांक callback_addrq(काष्ठा ul_nl_तथ्य *nl)
{
	काष्ठा ul_netaddrq_तथ्य *addrq = UL_NETADDRQ_तथ्य(nl);
	काष्ठा list_शीर्ष *li, *ipq_list;
	काष्ठा ul_netaddrq_iface *ifaceq = शून्य;
	काष्ठा ul_netaddrq_ip *ipq = शून्य;
	पूर्णांक rc;
	bool *ifaces_change;

	DBG_OBJ(LIST, addrq, ul_debug("callback_addrq() for %s on %s",
			      ul_nl_addr_ntop_address(&(nl->addr)),
			      nl->addr.ifname));
	अगर (addrq->callback_pre)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_pre"));
		अगर ((rc = (*(addrq->callback_pre))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_pre rc != 0"));
	}

	/* Search for interface in ifaces */
	addrq->nifaces = 0;

	list_for_each(li, &(addrq->ifaces)) {
		काष्ठा ul_netaddrq_iface *ifaceqq;

		ifaceqq = list_entry(li, काष्ठा ul_netaddrq_iface, entry);
		अगर (ifaceqq->ifa_index == nl->addr.ifa_index) {
			ifaceq = ifaceqq;
			DBG_OBJ(LIST, ifaceq, ul_debug(
					      "%s found in addrq",
					      nl->addr.ifname));
			अवरोध;
		}
		addrq->nifaces++;
	}

	अगर (ifaceq == शून्य) {
		अगर (nl->rtm_event) {
			अगर (addrq->nifaces >= max_ifaces) {
				DBG_OBJ(LIST, addrq, ul_debug(
						       "too many interfaces"));
				addrq->overflow = true;
				वापस UL_NL_IFACES_MAX;
			}
			DBG_OBJ(LIST, addrq, ul_debug(
					       "new ifa_index in addrq"));
			अगर (!(ifaceq = दो_स्मृति(माप(काष्ठा ul_netaddrq_iface))))
				वापस -ENOMEM;
			INIT_LIST_शीर्ष(&(ifaceq->ip_quality_list_4));
			INIT_LIST_शीर्ष(&(ifaceq->ip_quality_list_6));
			ifaceq->ifa_index = nl->addr.ifa_index;
			अगर (!(ifaceq->ifname = strdup(nl->addr.ifname)))
			{
				DBG_OBJ(LIST, addrq, ul_debug(
						      "malloc() failed"));
				मुक्त(ifaceq);
				वापस -ENOMEM;
			}
			list_add_पुच्छ(&(ifaceq->entry), &(addrq->ifaces));
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "new interface"));
		} अन्यथा {
			/* Should never happen. */
			DBG_OBJ(LIST, ifaceq, ul_debug(
					       "interface not found"));
			वापस UL_NL_SOFT_ERROR;
		}
	}
	अगर (nl->addr.ifa_family == AF_INET) {
		ipq_list = &(ifaceq->ip_quality_list_4);
		ifaces_change = &(addrq->ifaces_change_4);
	} अन्यथा {
	/* if (nl->addr.ifa_family == AF_INET6) */
		ipq_list = &(ifaceq->ip_quality_list_6);
		ifaces_change = &(addrq->ifaces_change_6);
	}

	list_for_each(li, ipq_list) {
		काष्ठा ul_netaddrq_ip *ipqq;

		ipqq = list_entry(li, काष्ठा ul_netaddrq_ip, entry);
		अगर (ipqq->addr->address_len == nl->addr.address_len)
			अगर (!स_भेद(ipqq->addr->address, nl->addr.address,
				   nl->addr.address_len)) {
				ipq = ipqq;
				DBG_OBJ(LIST, ipq, ul_debug(
						      "address found in ipq_list"));
				अवरोध;
			}
	}
	अगर (ipq == शून्य) {
			DBG_OBJ(LIST, ipq_list, ul_debug(
					      "address not found in the list"));
	}

	/* From now on, rc is return code */
	rc = 0;
	अगर (UL_NL_IS_RTM_NEW(nl)) {
		काष्ठा ul_nl_addr *addr;

		addr = ul_nl_addr_dup(&(nl->addr));
		अगर (!addr) {
			rc = -ENOMEM;
			जाओ error;
		}
		अगर (ipq == शून्य) {
			अगर (!(ipq = दो_स्मृति(माप(काष्ठा ul_netaddrq_ip))))
			{
				rc = -ENOMEM;
				ul_nl_addr_free(addr);
				जाओ error;
			}
			ipq->addr = addr;
			list_add_पुच्छ(&(ipq->entry), ipq_list);
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
				ul_nl_addr_ntop_address(&(nl->addr)),
				ip_rating_as_string(ipq->quality)));
	} अन्यथा {
		/* UL_NL_RTM_DEL */
		अगर (ipq == शून्य)
		{
			/* Should not happen. */
			DBG_OBJ(LIST, nl, ul_debug(
					      "UL_NL_RTM_DEL: unknown address"));
			वापस UL_NL_SOFT_ERROR;
		}
		/* Delist the address */
		DBG_OBJ(LIST, ipq, ul_debug("removing address"));
		*ifaces_change = true;
		list_del(&(ipq->entry));
		ul_nl_addr_free(ipq->addr);
		मुक्त(ipq);
	error:
		अगर (list_empty(&(ifaceq->ip_quality_list_4)) &&
		    list_empty(&(ifaceq->ip_quality_list_6))) {
		DBG_OBJ(LIST, ifaceq,
		    ul_debug("deleted last address, removing interface"));
			list_del(&(ifaceq->entry));
			addrq->nifaces--;
			मुक्त(ifaceq->ifname);
			मुक्त(ifaceq);
		}
	}
	अगर (!rc && addrq->callback_post)
	{
		DBG_OBJ(LIST, addrq, ul_debug("callback_post"));
		अगर ((rc = (*(addrq->callback_post))(nl)))
			DBG_OBJ(LIST, nl, ul_debug("callback_post rc != 0"));
	}
	वापस rc;
}

/* Initialize ul_nl_data for use with netlink-addr-quality */
पूर्णांक ul_netaddrq_init(काष्ठा ul_nl_तथ्य *nl, ul_nl_callback callback_pre,
		     ul_nl_callback callback_post, व्योम *तथ्य)
{
	काष्ठा ul_netaddrq_तथ्य *addrq;

	netaddrq_init_debug();
	अगर (!(nl->तथ्य_addr = सुस्मृति(1, माप(काष्ठा ul_netaddrq_तथ्य))))
		वापस -ENOMEM;
	nl->callback_addr = callback_addrq;
	addrq = UL_NETADDRQ_तथ्य(nl);
	addrq->callback_pre = callback_pre;
	addrq->callback_post = callback_post;
	addrq->callback_तथ्य = तथ्य;
	INIT_LIST_शीर्ष(&(addrq->ifaces));
	DBG_OBJ(LIST, addrq, ul_debug("callback initialized"));
	वापस 0;
}

क्रमागत ul_netaddrq_ip_rating
ul_netaddrq_iface_bestaddr(काष्ठा list_शीर्ष *ipq_list,
			   काष्ठा ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX])
{
	काष्ठा list_शीर्ष *li;
	काष्ठा ul_netaddrq_ip *ipq;
	क्रमागत ul_netaddrq_ip_rating threshold;

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, ipq_list)
	{
		ipq = list_entry(li, काष्ठा ul_netaddrq_ip, entry);

		अगर (!(*best)[ipq->quality] ||
		    ipq->addr->ifa_valid >
		    (*best)[ipq->quality]->addr->ifa_valid)
		{
			DBG_OBJ(BEST, (*best),
			    ul_debug("%s -> best[%s]",
					ul_nl_addr_ntop_address(ipq->addr),
					ip_rating_as_string(ipq->quality)));
			(*best)[ipq->quality] = ipq;
		}

		अगर (ipq->quality < threshold)
		{
			threshold = ipq->quality;
			DBG(BEST,
			    ul_debug("threshold %s", ip_rating_as_string(threshold)));

		}
	}
	वापस threshold;
}

क्रमागत ul_netaddrq_ip_rating
ul_netaddrq_bestaddr(काष्ठा ul_nl_तथ्य *nl,
		     काष्ठा ul_netaddrq_iface **best_ifaceq,
		     काष्ठा ul_netaddrq_ip *(*best)[__ULNETLINK_RATING_MAX],
		     uint8_t ifa_family)
{
	काष्ठा ul_netaddrq_तथ्य *addrq = UL_NETADDRQ_तथ्य(nl);
	काष्ठा list_शीर्ष *li;
	काष्ठा ul_netaddrq_iface *ifaceq;
	माप_प्रकार ipqo;
	क्रमागत ul_netaddrq_ip_rating threshold;

	अगर (ifa_family == AF_INET) {
		ipqo = दुरत्व(काष्ठा ul_netaddrq_iface, ip_quality_list_4);
	} अन्यथा {
	/* if (ifa_family == AF_INET6) */
		ipqo = दुरत्व(काष्ठा ul_netaddrq_iface, ip_quality_list_6);
	}

	threshold = __ULNETLINK_RATING_MAX;
	list_for_each(li, &(addrq->ifaces))
	{
		काष्ठा list_शीर्ष *ipq_list;
		क्रमागत ul_netaddrq_ip_rating t;

		ifaceq = list_entry(li, काष्ठा ul_netaddrq_iface, entry);

		ipq_list = (काष्ठा list_शीर्ष*)((अक्षर*)ifaceq + ipqo);

		t = ul_netaddrq_iface_bestaddr(ipq_list, best);
		अगर (t < threshold)
		{
			DBG_OBJ(BEST, *best,
			    ul_debug("best iface %s, threshold %u",
					ifaceq->ifname, t));
			*best_ifaceq = ifaceq;
			threshold = t;
		}
	}
	वापस threshold;
}

स्थिर अक्षर *ul_netaddrq_get_best_ipp(काष्ठा ul_nl_तथ्य *nl,
				     uint8_t ifa_family,
				     क्रमागत ul_netaddrq_ip_rating *threshold,
				     काष्ठा ul_netaddrq_iface **best_ifaceq)
{
	काष्ठा ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];

	स_रखो(best, 0, माप(best));
	*threshold = ul_netaddrq_bestaddr(nl, best_ifaceq, &best, ifa_family);
	अगर (*threshold != __ULNETLINK_RATING_MAX)
		वापस ul_nl_addr_ntop_address(best[*threshold]->addr);
	वापस शून्य;
}

काष्ठा ul_netaddrq_iface *ul_netaddrq_iface_by_name(स्थिर काष्ठा ul_nl_तथ्य *nl,
						    स्थिर अक्षर *ifname)
{
	काष्ठा list_शीर्ष *li;
	काष्ठा ul_netaddrq_iface *ifaceq;

	list_for_each_netaddrq_iface(li, nl)
	{
		ifaceq = list_entry(li, काष्ठा ul_netaddrq_iface, entry);

		अगर (!म_भेद(ifaceq->ifname, ifname))
			वापस ifaceq;
	}
	वापस शून्य;
}

#अगर_घोषित TEST_PROGRAM_NETADDRQ
/* This test program shows several possibilities for cherry-picking of IP
 * addresses based on its rating. But there are many more possibilities in the
 * criteria selection. ADDRQ_MODE_GOOD is the most smart one. */
क्रमागत addrq_print_mode {
	ADDRQ_MODE_BESTOFALL,	/* Best address of all interfaces */
	ADDRQ_MODE_BEST,	/* Best address per interface */
	ADDRQ_MODE_GOOD,	/* All global or site addresses, if none, the
				 * longest living temporary, if none, link */
	ADDRQ_MODE_ALL		/* All available addresses */
};

/* In our example addrq->callback_data is a simple FILE *. In more complex
 * programs it could be a pointer to an arbitrary struct */
#घोषणा netout (खाता *)(UL_NETADDRQ_तथ्य(nl)->callback_तथ्य)

/* This example uses separate threshold for IPv4 and IPv6, so the best IPv4 and
 * best IPv6 addresses are printed. */
अटल व्योम dump_iface_best(काष्ठा ul_nl_तथ्य *nl,
			    काष्ठा ul_netaddrq_iface *ifaceq)
{
	काष्ठा ul_netaddrq_ip *best[__ULNETLINK_RATING_MAX];
	क्रमागत ul_netaddrq_ip_rating threshold;
	bool first = true;

	स_रखो(best, 0, माप(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best);
	अगर (threshold != __ULNETLINK_RATING_MAX)
	{
		ख_लिखो(netout, "%s IPv4: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	स_रखो(best, 0, माप(best));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
					       &best);
	अगर (threshold != __ULNETLINK_RATING_MAX)
	{
		ख_लिखो(netout, "%s IPv6: %s", (first ? "best address" : " "),
		       ul_nl_addr_ntop_address(best[threshold]->addr));
		first = false;
	}
	अगर (!first)
		ख_लिखो(netout, " on interface %s\n",
		       ifaceq->ifname);
}

/* This example uses common threshold for IPv4 and IPv6, so e. g. worse rated
 * IPv6 are completely ignored. */
अटल व्योम dump_iface_good(काष्ठा ul_nl_तथ्य *nl,
			    काष्ठा ul_netaddrq_iface *ifaceq)
{
	काष्ठा ul_netaddrq_ip *best4[__ULNETLINK_RATING_MAX];
	काष्ठा ul_netaddrq_ip *best6[__ULNETLINK_RATING_MAX];
	काष्ठा list_शीर्ष *li;
	क्रमागत ul_netaddrq_ip_rating threshold = __ULNETLINK_RATING_MAX - 1;
	क्रमागत ul_netaddrq_ip_rating fthreshold; /* per family threshold */
	bool first = true;

	स_रखो(best4, 0, माप(best4));
	threshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_4),
					       &best4);
	स_रखो(best6, 0, माप(best6));
	fthreshold = ul_netaddrq_iface_bestaddr(&(ifaceq->ip_quality_list_6),
						&best6);
	अगर (fthreshold < threshold)
		threshold = fthreshold;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		काष्ठा ul_netaddrq_ip *ipq;

		ipq = list_entry(li, काष्ठा ul_netaddrq_ip, entry);
		अगर (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best4[threshold])
		{
			अगर (first)
			{
				ख_लिखो(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			अन्यथा
				ख_लिखो(netout, " ");
			/* Write only the longest living temporary address */
			अगर (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				ख_माला_दो(ul_nl_addr_ntop_address(best4[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				जाओ अस्थायी_cont4;
			}
			अन्यथा
				ख_माला_दो(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	अस्थायी_cont4:;
	}

	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		काष्ठा ul_netaddrq_ip *ipq;

		ipq = list_entry(li, काष्ठा ul_netaddrq_ip, entry);
		अगर (threshold <= ULNETLINK_RATING_SCOPE_LINK &&
		    ( ipq->quality <= threshold ||
		      /* Consider site addresses equally good as global */
		      ipq->quality == ULNETLINK_RATING_SCOPE_SITE) &&
		    best6[threshold])
		{
			अगर (first)
			{
				ख_लिखो(netout, "%s: ", ifaceq->ifname);
				first = false;
			}
			अन्यथा
				ख_लिखो(netout, " ");
			/* Write only the longest living temporary address */
			अगर (threshold == ULNETLINK_RATING_F_TEMPORARY)
			{
				ख_माला_दो(ul_nl_addr_ntop_address(best6[ULNETLINK_RATING_F_TEMPORARY]->addr),
				      netout);
				जाओ अस्थायी_cont6;
			}
			अन्यथा
				ख_माला_दो(ul_nl_addr_ntop_address(ipq->addr),
				      netout);
		}
	अस्थायी_cont6:;
	}
	अगर (!first)
		ख_माला_दो("\n", netout);
}

अटल व्योम dump_iface_all(काष्ठा ul_nl_तथ्य *nl,
			   काष्ठा ul_netaddrq_iface *ifaceq)
{
	काष्ठा list_शीर्ष *li;
	काष्ठा ul_netaddrq_ip *ipq;
	bool first = true;

	list_for_each(li, &(ifaceq->ip_quality_list_4))
	{
		ipq = list_entry(li, काष्ठा ul_netaddrq_ip, entry);
		अगर (first)
		{
			ख_लिखो(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		अन्यथा
			ख_लिखो(netout, " ");
		ख_माला_दो(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	list_for_each(li, &(ifaceq->ip_quality_list_6))
	{
		ipq = list_entry(li, काष्ठा ul_netaddrq_ip, entry);
		अगर (first)
		{
			ख_लिखो(netout, "%s: ", ifaceq->ifname);
			first = false;
		}
		अन्यथा
			ख_लिखो(netout, " ");
		ख_माला_दो(ul_nl_addr_ntop_address(ipq->addr), netout);
	}
	अगर (!first)
		ख_माला_दो("\n", netout);
}

अटल व्योम dump_addrq(काष्ठा ul_nl_तथ्य *nl, क्रमागत addrq_print_mode c)
{
	काष्ठा list_शीर्ष *li;
	काष्ठा ul_netaddrq_iface *ifaceq;
	क्रमागत ul_netaddrq_ip_rating threshold;

	चयन(c)
	{
	हाल ADDRQ_MODE_BESTOFALL:
	{
		काष्ठा ul_netaddrq_iface *best_ifaceq;
		स्थिर अक्षर *best_ipp;

		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET,
						    &threshold, &best_ifaceq);
		अगर (best_ipp)
			ख_लिखो(netout, "best IPv4 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
		best_ipp = ul_netaddrq_get_best_ipp(nl, AF_INET6,
						    &threshold, &best_ifaceq);
		अगर (best_ipp)
			ख_लिखो(netout, "best IPv6 address: %s on %s\n",
				best_ipp, best_ifaceq->ifname);
	}
	अवरोध;
	हाल ADDRQ_MODE_BEST:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, काष्ठा ul_netaddrq_iface, entry);

			dump_iface_best(nl, ifaceq);
		}
	}
	अवरोध;
	हाल ADDRQ_MODE_GOOD:
	{
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, काष्ठा ul_netaddrq_iface, entry);

			dump_iface_good(nl, ifaceq);
		}
	}
	अवरोध;
	हाल ADDRQ_MODE_ALL:
		list_for_each_netaddrq_iface(li, nl)
		{
			ifaceq = list_entry(li, काष्ठा ul_netaddrq_iface, entry);

			dump_iface_all(nl,ifaceq);
		}
	अवरोध;
	}
}

अटल पूर्णांक callback_post(काष्ठा ul_nl_तथ्य *nl)
{
	/* If not processing dump, process the change immediately by the
	 * callback. */
	अगर (!nl->dumping)
	{
		/* If there is no change in the list, do nothing */
		अगर (!(UL_NETADDRQ_तथ्य(nl)->ifaces_change_4 ||
		      UL_NETADDRQ_तथ्य(nl)->ifaces_change_6))
		{
			ख_माला_दो("\n\nNo changes in the address list.\n", netout);
			वापस 0;
		}
		UL_NETADDRQ_तथ्य(nl)->ifaces_change_4 = false;
		UL_NETADDRQ_तथ्य(nl)->ifaces_change_6 = false;
		ख_माला_दो("\n\nNetwork change detected:\n", netout);
		ख_माला_दो("\nbest address:\n", netout);
		dump_addrq(nl, ADDRQ_MODE_BESTOFALL);

		ख_माला_दो("\nbest addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_MODE_BEST);

		ख_माला_दो("\ngood addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_MODE_GOOD);

		ख_माला_दो("\nall addresses dump:\n", netout);
		dump_addrq(nl, ADDRQ_MODE_ALL);
	}
	वापस 0;
}

पूर्णांक मुख्य(पूर्णांक argc __attribute__((__unused__)), अक्षर *argv[] __attribute__((__unused__)))
{
	पूर्णांक rc = 1;
	पूर्णांक ulrc;
	काष्ठा ul_nl_तथ्य nl;
	खाता *out = मानक_निकास;
	काष्ठा ul_netaddrq_iface *ifaceq;
	स्थिर अक्षर *ifname = "eth0";

	/* Prepare netlink. */
	ul_nl_init(&nl);
	अगर ((ul_netaddrq_init(&nl, शून्य, callback_post, (व्योम *)out)))
		वापस -1;

	/* Dump addresses */
	अगर (ul_nl_open(&nl,
		       RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR))
		वापस -1;
	अगर (ul_nl_request_dump(&nl, RTM_GETADDR))
		जाओ error;
	अगर (ul_nl_process(&nl, UL_NL_SYNC, UL_NL_LOOP) != UL_NL_DONE)
		जाओ error;
	ख_माला_दो("RTM_GETADDR dump finished.", out);

	/* Example of several types of dump */
	ख_माला_दो("\nbest address:\n", out);
	dump_addrq(&nl, ADDRQ_MODE_BESTOFALL);

	ख_माला_दो("\nbest addresses dump:\n", out);
	dump_addrq(&nl, ADDRQ_MODE_BEST);

	ख_माला_दो("\ngood addresses dump:\n", out);
	dump_addrq(&nl, ADDRQ_MODE_GOOD);

	ख_माला_दो("\nall addresses dump:\n", out);
	dump_addrq(&nl, ADDRQ_MODE_ALL);

	ख_माला_दो("\naddresses for interface ", out);
	अगर ((ifaceq = ul_netaddrq_iface_by_name(&nl, ifname)))
		dump_iface_all(&nl, ifaceq);
	अन्यथा
		ख_लिखो(out, "%s not found.", ifname);

	/* Monitor further changes */
	ख_माला_दो("\nGoing to monitor mode.\n", out);

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
	अगर (!ulrc || ulrc == UL_NL_RETURN)
		rc = 0;
error:
	अगर ((ul_nl_close(&nl)))
		rc = 1;
	वापस rc;
}
#पूर्ण_अगर /* TEST_PROGRAM_NETADDRQ */
