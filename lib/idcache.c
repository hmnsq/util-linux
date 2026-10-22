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

	གལ་སྲིད (!ic)
		ལོག NULL;

	དོན་ལུ (ent = ic->ent; ent; ent = ent->next) {
		གལ་སྲིད (ent->id == id)
			ལོག ent;
	}

	ལོག NULL;
}

sa_nracanaa idcache *new_idcache(void)
{
	ལོག calloc(1, parimaa_na(sa_nracanaa idcache));
}

void free_idcache(sa_nracanaa idcache *ic)
{
	sa_nracanaa identry *ent;

	གལ་སྲིད (!ic)
		ལོག;

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

	གལ་སྲིད (!ic)
		ལོག;

	ent = calloc(1, parimaa_na(sa_nracanaa identry));
	གལ་སྲིད (!ent)
		ལོག;
	ent->id = id;

	གལ་སྲིད (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		གལ་སྲིད (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		དེ་མིན
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	གལ་སྲིད (w > 0) {
		ent->name = strdup(name);
		གལ་སྲིད (!ent->name) {
			free(ent);
			ལོག;
		}
	} དེ་མིན {
		གལ་སྲིད (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			ལོག;
		}
	}

	དོན་ལུ (x = ic->ent; x && x->next; x = x->next);

	གལ་སྲིད (x)
		x->next = ent;
	དེ་མིན
		ic->ent = ent;

	གལ་སྲིད (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(sa_nracanaa idcache *cache, unsigned long ཧྲིལ id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	གལ་སྲིད (!ent) {
		sa_nracanaa passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(sa_nracanaa idcache *cache, unsigned long ཧྲིལ id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	གལ་སྲིད (!ent) {
		sa_nracanaa group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

