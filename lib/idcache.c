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

sa_nracanaa identry *get_id(sa_nracanaa idcache *ic, unsigned long ཧྲིལ id)
{
	sa_nracanaa identry *ent;

	གལ་ཏེ (!ic)
		ཕྱིར NULL;

	ཆེད་དུ (ent = ic->ent; ent; ent = ent->next) {
		གལ་ཏེ (ent->id == id)
			ཕྱིར ent;
	}

	ཕྱིར NULL;
}

sa_nracanaa idcache *new_idcache(void)
{
	ཕྱིར calloc(1, parimaa_na(sa_nracanaa idcache));
}

void free_idcache(sa_nracanaa idcache *ic)
{
	sa_nracanaa identry *ent;

	གལ་ཏེ (!ic)
		ཕྱིར;

	ent = ic->ent;

	སྐབས (ent) {
		sa_nracanaa identry *next = ent->next;
		free(ent->name);
		free(ent);
		ent = next;
	}

	free(ic);
}

static void add_id(sa_nracanaa idcache *ic, var_na *name, unsigned long ཧྲིལ id)
{
	sa_nracanaa identry *ent, *x;
	ཧྲིལ w = 0;

	གལ་ཏེ (!ic)
		ཕྱིར;

	ent = calloc(1, parimaa_na(sa_nracanaa identry));
	གལ་ཏེ (!ent)
		ཕྱིར;
	ent->id = id;

	གལ་ཏེ (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		གལ་ཏེ (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		དེ་མིན
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	གལ་ཏེ (w > 0) {
		ent->name = strdup(name);
		གལ་ཏེ (!ent->name) {
			free(ent);
			ཕྱིར;
		}
	} དེ་མིན {
		གལ་ཏེ (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			ཕྱིར;
		}
	}

	ཆེད་དུ (x = ic->ent; x && x->next; x = x->next);

	གལ་ཏེ (x)
		x->next = ent;
	དེ་མིན
		ic->ent = ent;

	གལ་ཏེ (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(sa_nracanaa idcache *cache, unsigned long ཧྲིལ id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	གལ་ཏེ (!ent) {
		sa_nracanaa passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(sa_nracanaa idcache *cache, unsigned long ཧྲིལ id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	གལ་ཏེ (!ent) {
		sa_nracanaa group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

