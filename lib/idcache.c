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

struct identry *get_id(struct idcache *ic, unsigned long ціле id)
{
	struct identry *ent;

	якщо (!ic)
		повернути NULL;

	для (ent = ic->ent; ent; ent = ent->next) {
		якщо (ent->id == id)
			повернути ent;
	}

	повернути NULL;
}

struct idcache *new_idcache(void)
{
	повернути calloc(1, sizeof(struct idcache));
}

void free_idcache(struct idcache *ic)
{
	struct identry *ent;

	якщо (!ic)
		повернути;

	ent = ic->ent;

	поки (ent) {
		struct identry *next = ent->next;
		free(ent->name);
		free(ent);
		ent = next;
	}

	free(ic);
}

static void add_id(struct idcache *ic, char *name, unsigned long ціле id)
{
	struct identry *ent, *x;
	ціле w = 0;

	якщо (!ic)
		повернути;

	ent = calloc(1, sizeof(struct identry));
	якщо (!ent)
		повернути;
	ent->id = id;

	якщо (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		якщо (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		інакше
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	якщо (w > 0) {
		ent->name = strdup(name);
		якщо (!ent->name) {
			free(ent);
			повернути;
		}
	} інакше {
		якщо (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			повернути;
		}
	}

	для (x = ic->ent; x && x->next; x = x->next);

	якщо (x)
		x->next = ent;
	інакше
		ic->ent = ent;

	якщо (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(struct idcache *cache, unsigned long ціле id)
{
	struct identry *ent = get_id(cache, id);

	якщо (!ent) {
		struct passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(struct idcache *cache, unsigned long ціле id)
{
	struct identry *ent = get_id(cache, id);

	якщо (!ent) {
		struct group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

