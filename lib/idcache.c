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

struct identry *get_id(struct idcache *ic, unsigned long цео id)
{
	struct identry *ent;

	ако (!ic)
		врати NULL;

	за (ent = ic->ent; ent; ent = ent->next) {
		ако (ent->id == id)
			врати ent;
	}

	врати NULL;
}

struct idcache *new_idcache(void)
{
	врати calloc(1, sizeof(struct idcache));
}

void free_idcache(struct idcache *ic)
{
	struct identry *ent;

	ако (!ic)
		врати;

	ent = ic->ent;

	док (ent) {
		struct identry *next = ent->next;
		free(ent->name);
		free(ent);
		ent = next;
	}

	free(ic);
}

static void add_id(struct idcache *ic, char *name, unsigned long цео id)
{
	struct identry *ent, *x;
	цео w = 0;

	ако (!ic)
		врати;

	ent = calloc(1, sizeof(struct identry));
	ако (!ent)
		врати;
	ent->id = id;

	ако (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		ако (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		инако
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	ако (w > 0) {
		ent->name = strdup(name);
		ако (!ent->name) {
			free(ent);
			врати;
		}
	} инако {
		ако (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			врати;
		}
	}

	за (x = ic->ent; x && x->next; x = x->next);

	ако (x)
		x->next = ent;
	инако
		ic->ent = ent;

	ако (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(struct idcache *cache, unsigned long цео id)
{
	struct identry *ent = get_id(cache, id);

	ако (!ent) {
		struct passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(struct idcache *cache, unsigned long цео id)
{
	struct identry *ent = get_id(cache, id);

	ако (!ent) {
		struct group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

