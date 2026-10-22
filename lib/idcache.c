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

sa_nracanaa identry *get_id(sa_nracanaa idcache *ic, unsigned long จำนวนเต็ม id)
{
	sa_nracanaa identry *ent;

	ถ้า (!ic)
		คืน NULL;

	สำหรับ (ent = ic->ent; ent; ent = ent->next) {
		ถ้า (ent->id == id)
			คืน ent;
	}

	คืน NULL;
}

sa_nracanaa idcache *new_idcache(void)
{
	คืน calloc(1, parimaa_na(sa_nracanaa idcache));
}

void free_idcache(sa_nracanaa idcache *ic)
{
	sa_nracanaa identry *ent;

	ถ้า (!ic)
		คืน;

	ent = ic->ent;

	ขณะ (ent) {
		sa_nracanaa identry *next = ent->next;
		free(ent->name);
		free(ent);
		ent = next;
	}

	free(ic);
}

static void add_id(sa_nracanaa idcache *ic, var_na *name, unsigned long จำนวนเต็ม id)
{
	sa_nracanaa identry *ent, *x;
	จำนวนเต็ม w = 0;

	ถ้า (!ic)
		คืน;

	ent = calloc(1, parimaa_na(sa_nracanaa identry));
	ถ้า (!ent)
		คืน;
	ent->id = id;

	ถ้า (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		ถ้า (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		มิฉะนั้น
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	ถ้า (w > 0) {
		ent->name = strdup(name);
		ถ้า (!ent->name) {
			free(ent);
			คืน;
		}
	} มิฉะนั้น {
		ถ้า (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			คืน;
		}
	}

	สำหรับ (x = ic->ent; x && x->next; x = x->next);

	ถ้า (x)
		x->next = ent;
	มิฉะนั้น
		ic->ent = ent;

	ถ้า (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(sa_nracanaa idcache *cache, unsigned long จำนวนเต็ม id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	ถ้า (!ent) {
		sa_nracanaa passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(sa_nracanaa idcache *cache, unsigned long จำนวนเต็ม id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	ถ้า (!ent) {
		sa_nracanaa group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

