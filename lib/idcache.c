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

sa_nracanaa identry *get_id(sa_nracanaa idcache *ic, unsigned long ကိန်းပြည့် id)
{
	sa_nracanaa identry *ent;

	အကယ်၍ (!ic)
		ပြန်ပေး NULL;

	အတွက် (ent = ic->ent; ent; ent = ent->next) {
		အကယ်၍ (ent->id == id)
			ပြန်ပေး ent;
	}

	ပြန်ပေး NULL;
}

sa_nracanaa idcache *new_idcache(void)
{
	ပြန်ပေး calloc(1, parimaa_na(sa_nracanaa idcache));
}

void free_idcache(sa_nracanaa idcache *ic)
{
	sa_nracanaa identry *ent;

	အကယ်၍ (!ic)
		ပြန်ပေး;

	ent = ic->ent;

	နေစဉ် (ent) {
		sa_nracanaa identry *next = ent->next;
		free(ent->name);
		free(ent);
		ent = next;
	}

	free(ic);
}

static void add_id(sa_nracanaa idcache *ic, var_na *name, unsigned long ကိန်းပြည့် id)
{
	sa_nracanaa identry *ent, *x;
	ကိန်းပြည့် w = 0;

	အကယ်၍ (!ic)
		ပြန်ပေး;

	ent = calloc(1, parimaa_na(sa_nracanaa identry));
	အကယ်၍ (!ent)
		ပြန်ပေး;
	ent->id = id;

	အကယ်၍ (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		အကယ်၍ (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		မဟုတ်ပါက
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	အကယ်၍ (w > 0) {
		ent->name = strdup(name);
		အကယ်၍ (!ent->name) {
			free(ent);
			ပြန်ပေး;
		}
	} မဟုတ်ပါက {
		အကယ်၍ (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			ပြန်ပေး;
		}
	}

	အတွက် (x = ic->ent; x && x->next; x = x->next);

	အကယ်၍ (x)
		x->next = ent;
	မဟုတ်ပါက
		ic->ent = ent;

	အကယ်၍ (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(sa_nracanaa idcache *cache, unsigned long ကိန်းပြည့် id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	အကယ်၍ (!ent) {
		sa_nracanaa passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(sa_nracanaa idcache *cache, unsigned long ကိန်းပြည့် id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	အကယ်၍ (!ent) {
		sa_nracanaa group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

