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

sa_nracanaa identry *get_id(sa_nracanaa idcache *ic, unsigned long පූර්ණ_සංඛ්‍යා id)
{
	sa_nracanaa identry *ent;

	නම් (!ic)
		ආපසු NULL;

	සඳහා (ent = ic->ent; ent; ent = ent->next) {
		නම් (ent->id == id)
			ආපසු ent;
	}

	ආපසු NULL;
}

sa_nracanaa idcache *new_idcache(void)
{
	ආපසු calloc(1, parimaa_na(sa_nracanaa idcache));
}

void free_idcache(sa_nracanaa idcache *ic)
{
	sa_nracanaa identry *ent;

	නම් (!ic)
		ආපසු;

	ent = ic->ent;

	අතරතුර (ent) {
		sa_nracanaa identry *next = ent->next;
		free(ent->name);
		free(ent);
		ent = next;
	}

	free(ic);
}

static void add_id(sa_nracanaa idcache *ic, var_na *name, unsigned long පූර්ණ_සංඛ්‍යා id)
{
	sa_nracanaa identry *ent, *x;
	පූර්ණ_සංඛ්‍යා w = 0;

	නම් (!ic)
		ආපසු;

	ent = calloc(1, parimaa_na(sa_nracanaa identry));
	නම් (!ent)
		ආපසු;
	ent->id = id;

	නම් (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		නම් (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		නැතහොත්
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	නම් (w > 0) {
		ent->name = strdup(name);
		නම් (!ent->name) {
			free(ent);
			ආපසු;
		}
	} නැතහොත් {
		නම් (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			ආපසු;
		}
	}

	සඳහා (x = ic->ent; x && x->next; x = x->next);

	නම් (x)
		x->next = ent;
	නැතහොත්
		ic->ent = ent;

	නම් (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(sa_nracanaa idcache *cache, unsigned long පූර්ණ_සංඛ්‍යා id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	නම් (!ent) {
		sa_nracanaa passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(sa_nracanaa idcache *cache, unsigned long පූර්ණ_සංඛ්‍යා id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	නම් (!ent) {
		sa_nracanaa group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

