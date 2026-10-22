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

struct identry *get_id(struct idcache *ic, unsigned long 整数 id)
{
	struct identry *ent;

	如果 (!ic)
		返回 NULL;

	为 (ent = ic->ent; ent; ent = ent->next) {
		如果 (ent->id == id)
			返回 ent;
	}

	返回 NULL;
}

struct idcache *new_idcache(void)
{
	返回 calloc(1, sizeof(struct idcache));
}

void free_idcache(struct idcache *ic)
{
	struct identry *ent;

	如果 (!ic)
		返回;

	ent = ic->ent;

	当 (ent) {
		struct identry *next = ent->next;
		free(ent->name);
		free(ent);
		ent = next;
	}

	free(ic);
}

static void add_id(struct idcache *ic, char *name, unsigned long 整数 id)
{
	struct identry *ent, *x;
	整数 w = 0;

	如果 (!ic)
		返回;

	ent = calloc(1, sizeof(struct identry));
	如果 (!ent)
		返回;
	ent->id = id;

	如果 (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		如果 (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		否则
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	如果 (w > 0) {
		ent->name = strdup(name);
		如果 (!ent->name) {
			free(ent);
			返回;
		}
	} 否则 {
		如果 (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			返回;
		}
	}

	为 (x = ic->ent; x && x->next; x = x->next);

	如果 (x)
		x->next = ent;
	否则
		ic->ent = ent;

	如果 (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(struct idcache *cache, unsigned long 整数 id)
{
	struct identry *ent = get_id(cache, id);

	如果 (!ent) {
		struct passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(struct idcache *cache, unsigned long 整数 id)
{
	struct identry *ent = get_id(cache, id);

	如果 (!ent) {
		struct group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

