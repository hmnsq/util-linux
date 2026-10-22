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

struct identry *get_id(struct idcache *ic, unsigned long целое id)
{
	struct identry *ent;

	если (!ic)
		вернуть NULL;

	для (ent = ic->ent; ent; ent = ent->next) {
		если (ent->id == id)
			вернуть ent;
	}

	вернуть NULL;
}

struct idcache *new_idcache(void)
{
	вернуть calloc(1, sizeof(struct idcache));
}

void free_idcache(struct idcache *ic)
{
	struct identry *ent;

	если (!ic)
		вернуть;

	ent = ic->ent;

	пока (ent) {
		struct identry *next = ent->next;
		free(ent->name);
		free(ent);
		ent = next;
	}

	free(ic);
}

static void add_id(struct idcache *ic, char *name, unsigned long целое id)
{
	struct identry *ent, *x;
	целое w = 0;

	если (!ic)
		вернуть;

	ent = calloc(1, sizeof(struct identry));
	если (!ent)
		вернуть;
	ent->id = id;

	если (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		если (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		иначе
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	если (w > 0) {
		ent->name = strdup(name);
		если (!ent->name) {
			free(ent);
			вернуть;
		}
	} иначе {
		если (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			вернуть;
		}
	}

	для (x = ic->ent; x && x->next; x = x->next);

	если (x)
		x->next = ent;
	иначе
		ic->ent = ent;

	если (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(struct idcache *cache, unsigned long целое id)
{
	struct identry *ent = get_id(cache, id);

	если (!ent) {
		struct passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(struct idcache *cache, unsigned long целое id)
{
	struct identry *ent = get_id(cache, id);

	если (!ent) {
		struct group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

