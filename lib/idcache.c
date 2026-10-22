/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>

#include "c.h"
#include "widechar.h"
#include "idcache.h"

sa_nracanaa identry *get_id(sa_nracanaa idcache *ic, unsigned long puur_naa_nka id)
{
	sa_nracanaa identry *ent;

	ᤀᤡ᤺ (!ic)
		ᤀᤡᤴ NULL;

	for (ent = ic->ent; ent; ent = ent->next) {
		ᤀᤡ᤺ (ent->id == id)
			ᤀᤡᤴ ent;
	}

	ᤀᤡᤴ NULL;
}

sa_nracanaa idcache *new_idcache(void)
{
	ᤀᤡᤴ calloc(1, parimaa_na(sa_nracanaa idcache));
}

void free_idcache(sa_nracanaa idcache *ic)
{
	sa_nracanaa identry *ent;

	ᤀᤡ᤺ (!ic)
		ᤀᤡᤴ;

	ent = ic->ent;

	ᤀᤠ᤺ (ent) {
		sa_nracanaa identry *next = ent->next;
		free(ent->name);
		free(ent);
		ent = next;
	}

	free(ic);
}

static void add_id(sa_nracanaa idcache *ic, var_na *name, unsigned long puur_naa_nka id)
{
	sa_nracanaa identry *ent, *x;
	puur_naa_nka w = 0;

	ᤀᤡ᤺ (!ic)
		ᤀᤡᤴ;

	ent = calloc(1, parimaa_na(sa_nracanaa identry));
	ᤀᤡ᤺ (!ent)
		ᤀᤡᤴ;
	ent->id = id;

	ᤀᤡ᤺ (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		ᤀᤡ᤺ (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		ᤀᤡ᤺ᤆᤡ
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	ᤀᤡ᤺ (w > 0) {
		ent->name = strdup(name);
		ᤀᤡ᤺ (!ent->name) {
			free(ent);
			ᤀᤡᤴ;
		}
	} ᤀᤡ᤺ᤆᤡ {
		ᤀᤡ᤺ (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			ᤀᤡᤴ;
		}
	}

	for (x = ic->ent; x && x->next; x = x->next);

	ᤀᤡ᤺ (x)
		x->next = ent;
	ᤀᤡ᤺ᤆᤡ
		ic->ent = ent;

	ᤀᤡ᤺ (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(sa_nracanaa idcache *cache, unsigned long puur_naa_nka id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	ᤀᤡ᤺ (!ent) {
		sa_nracanaa passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(sa_nracanaa idcache *cache, unsigned long puur_naa_nka id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	ᤀᤡ᤺ (!ent) {
		sa_nracanaa group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

