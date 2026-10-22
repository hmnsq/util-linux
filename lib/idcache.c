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

struct identry *get_id(struct idcache *ic, unsigned long số_nguyên id)
{
	struct identry *ent;

	nếu (!ic)
		trả_về NULL;

	cho (ent = ic->ent; ent; ent = ent->next) {
		nếu (ent->id == id)
			trả_về ent;
	}

	trả_về NULL;
}

struct idcache *new_idcache(void)
{
	trả_về calloc(1, sizeof(struct idcache));
}

void free_idcache(struct idcache *ic)
{
	struct identry *ent;

	nếu (!ic)
		trả_về;

	ent = ic->ent;

	trong_khi (ent) {
		struct identry *next = ent->next;
		free(ent->name);
		free(ent);
		ent = next;
	}

	free(ic);
}

static void add_id(struct idcache *ic, char *name, unsigned long số_nguyên id)
{
	struct identry *ent, *x;
	số_nguyên w = 0;

	nếu (!ic)
		trả_về;

	ent = calloc(1, sizeof(struct identry));
	nếu (!ent)
		trả_về;
	ent->id = id;

	nếu (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		nếu (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		không_thì
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	nếu (w > 0) {
		ent->name = strdup(name);
		nếu (!ent->name) {
			free(ent);
			trả_về;
		}
	} không_thì {
		nếu (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			trả_về;
		}
	}

	cho (x = ic->ent; x && x->next; x = x->next);

	nếu (x)
		x->next = ent;
	không_thì
		ic->ent = ent;

	nếu (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(struct idcache *cache, unsigned long số_nguyên id)
{
	struct identry *ent = get_id(cache, id);

	nếu (!ent) {
		struct passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(struct idcache *cache, unsigned long số_nguyên id)
{
	struct identry *ent = get_id(cache, id);

	nếu (!ent) {
		struct group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

