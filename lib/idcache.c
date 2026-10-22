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

struct identry *get_id(struct idcache *ic, unsigned long 정수 id)
{
	struct identry *ent;

	만약 (!ic)
		반환 NULL;

	위해 (ent = ic->ent; ent; ent = ent->next) {
		만약 (ent->id == id)
			반환 ent;
	}

	반환 NULL;
}

struct idcache *new_idcache(void)
{
	반환 calloc(1, sizeof(struct idcache));
}

void free_idcache(struct idcache *ic)
{
	struct identry *ent;

	만약 (!ic)
		반환;

	ent = ic->ent;

	동안 (ent) {
		struct identry *next = ent->next;
		free(ent->name);
		free(ent);
		ent = next;
	}

	free(ic);
}

static void add_id(struct idcache *ic, char *name, unsigned long 정수 id)
{
	struct identry *ent, *x;
	정수 w = 0;

	만약 (!ic)
		반환;

	ent = calloc(1, sizeof(struct identry));
	만약 (!ent)
		반환;
	ent->id = id;

	만약 (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		만약 (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		아니면
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	만약 (w > 0) {
		ent->name = strdup(name);
		만약 (!ent->name) {
			free(ent);
			반환;
		}
	} 아니면 {
		만약 (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			반환;
		}
	}

	위해 (x = ic->ent; x && x->next; x = x->next);

	만약 (x)
		x->next = ent;
	아니면
		ic->ent = ent;

	만약 (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(struct idcache *cache, unsigned long 정수 id)
{
	struct identry *ent = get_id(cache, id);

	만약 (!ent) {
		struct passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(struct idcache *cache, unsigned long 정수 id)
{
	struct identry *ent = get_id(cache, id);

	만약 (!ent) {
		struct group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

