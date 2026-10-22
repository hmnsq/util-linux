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

sa_nracanaa identry *get_id(sa_nracanaa idcache *ic, unsigned long ចំនួនគត់ id)
{
	sa_nracanaa identry *ent;

	ប្រសិនបើ (!ic)
		ត្រឡប់ NULL;

	សម្រាប់ (ent = ic->ent; ent; ent = ent->next) {
		ប្រសិនបើ (ent->id == id)
			ត្រឡប់ ent;
	}

	ត្រឡប់ NULL;
}

sa_nracanaa idcache *new_idcache(void)
{
	ត្រឡប់ calloc(1, parimaa_na(sa_nracanaa idcache));
}

void free_idcache(sa_nracanaa idcache *ic)
{
	sa_nracanaa identry *ent;

	ប្រសិនបើ (!ic)
		ត្រឡប់;

	ent = ic->ent;

	ខណៈពេល (ent) {
		sa_nracanaa identry *next = ent->next;
		free(ent->name);
		free(ent);
		ent = next;
	}

	free(ic);
}

static void add_id(sa_nracanaa idcache *ic, var_na *name, unsigned long ចំនួនគត់ id)
{
	sa_nracanaa identry *ent, *x;
	ចំនួនគត់ w = 0;

	ប្រសិនបើ (!ic)
		ត្រឡប់;

	ent = calloc(1, parimaa_na(sa_nracanaa identry));
	ប្រសិនបើ (!ent)
		ត្រឡប់;
	ent->id = id;

	ប្រសិនបើ (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		ប្រសិនបើ (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		បើមិនដូច្នេះ
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	ប្រសិនបើ (w > 0) {
		ent->name = strdup(name);
		ប្រសិនបើ (!ent->name) {
			free(ent);
			ត្រឡប់;
		}
	} បើមិនដូច្នេះ {
		ប្រសិនបើ (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			ត្រឡប់;
		}
	}

	សម្រាប់ (x = ic->ent; x && x->next; x = x->next);

	ប្រសិនបើ (x)
		x->next = ent;
	បើមិនដូច្នេះ
		ic->ent = ent;

	ប្រសិនបើ (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(sa_nracanaa idcache *cache, unsigned long ចំនួនគត់ id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	ប្រសិនបើ (!ent) {
		sa_nracanaa passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(sa_nracanaa idcache *cache, unsigned long ចំនួនគត់ id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	ប្រសិនបើ (!ent) {
		sa_nracanaa group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

