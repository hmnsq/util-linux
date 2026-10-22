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

	ᱡᱩᱫᱤ (!ic)
		ᱨᱩᱣᱟᱹᱲ NULL;

	for (ent = ic->ent; ent; ent = ent->next) {
		ᱡᱩᱫᱤ (ent->id == id)
			ᱨᱩᱣᱟᱹᱲ ent;
	}

	ᱨᱩᱣᱟᱹᱲ NULL;
}

sa_nracanaa idcache *new_idcache(void)
{
	ᱨᱩᱣᱟᱹᱲ calloc(1, parimaa_na(sa_nracanaa idcache));
}

void free_idcache(sa_nracanaa idcache *ic)
{
	sa_nracanaa identry *ent;

	ᱡᱩᱫᱤ (!ic)
		ᱨᱩᱣᱟᱹᱲ;

	ent = ic->ent;

	ᱡᱚᱠᱷᱚᱱ (ent) {
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

	ᱡᱩᱫᱤ (!ic)
		ᱨᱩᱣᱟᱹᱲ;

	ent = calloc(1, parimaa_na(sa_nracanaa identry));
	ᱡᱩᱫᱤ (!ent)
		ᱨᱩᱣᱟᱹᱲ;
	ent->id = id;

	ᱡᱩᱫᱤ (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		ᱡᱩᱫᱤ (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		ᱵᱟᱝ
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	ᱡᱩᱫᱤ (w > 0) {
		ent->name = strdup(name);
		ᱡᱩᱫᱤ (!ent->name) {
			free(ent);
			ᱨᱩᱣᱟᱹᱲ;
		}
	} ᱵᱟᱝ {
		ᱡᱩᱫᱤ (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			ᱨᱩᱣᱟᱹᱲ;
		}
	}

	for (x = ic->ent; x && x->next; x = x->next);

	ᱡᱩᱫᱤ (x)
		x->next = ent;
	ᱵᱟᱝ
		ic->ent = ent;

	ᱡᱩᱫᱤ (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(sa_nracanaa idcache *cache, unsigned long puur_naa_nka id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	ᱡᱩᱫᱤ (!ent) {
		sa_nracanaa passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(sa_nracanaa idcache *cache, unsigned long puur_naa_nka id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	ᱡᱩᱫᱤ (!ent) {
		sa_nracanaa group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

