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

	ᮚᮦᮔ᮪ (!ic)
		ᮝᮜᮤ NULL;

	for (ent = ic->ent; ent; ent = ent->next) {
		ᮚᮦᮔ᮪ (ent->id == id)
			ᮝᮜᮤ ent;
	}

	ᮝᮜᮤ NULL;
}

sa_nracanaa idcache *new_idcache(void)
{
	ᮝᮜᮤ calloc(1, parimaa_na(sa_nracanaa idcache));
}

void free_idcache(sa_nracanaa idcache *ic)
{
	sa_nracanaa identry *ent;

	ᮚᮦᮔ᮪ (!ic)
		ᮝᮜᮤ;

	ent = ic->ent;

	ᮞᮨᮓᮨᮜ (ent) {
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

	ᮚᮦᮔ᮪ (!ic)
		ᮝᮜᮤ;

	ent = calloc(1, parimaa_na(sa_nracanaa identry));
	ᮚᮦᮔ᮪ (!ent)
		ᮝᮜᮤ;
	ent->id = id;

	ᮚᮦᮔ᮪ (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		ᮚᮦᮔ᮪ (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		ᮜᮤᮚ
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	ᮚᮦᮔ᮪ (w > 0) {
		ent->name = strdup(name);
		ᮚᮦᮔ᮪ (!ent->name) {
			free(ent);
			ᮝᮜᮤ;
		}
	} ᮜᮤᮚ {
		ᮚᮦᮔ᮪ (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			ᮝᮜᮤ;
		}
	}

	for (x = ic->ent; x && x->next; x = x->next);

	ᮚᮦᮔ᮪ (x)
		x->next = ent;
	ᮜᮤᮚ
		ic->ent = ent;

	ᮚᮦᮔ᮪ (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(sa_nracanaa idcache *cache, unsigned long puur_naa_nka id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	ᮚᮦᮔ᮪ (!ent) {
		sa_nracanaa passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(sa_nracanaa idcache *cache, unsigned long puur_naa_nka id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	ᮚᮦᮔ᮪ (!ent) {
		sa_nracanaa group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

