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

sa_nracanaa identry *get_id(sa_nracanaa idcache *ic, unsigned long ຈຳນວນເຕັມ id)
{
	sa_nracanaa identry *ent;

	ຖ້າ (!ic)
		ສົ່ງຄືນ NULL;

	ສຳລັບ (ent = ic->ent; ent; ent = ent->next) {
		ຖ້າ (ent->id == id)
			ສົ່ງຄືນ ent;
	}

	ສົ່ງຄືນ NULL;
}

sa_nracanaa idcache *new_idcache(void)
{
	ສົ່ງຄືນ calloc(1, parimaa_na(sa_nracanaa idcache));
}

void free_idcache(sa_nracanaa idcache *ic)
{
	sa_nracanaa identry *ent;

	ຖ້າ (!ic)
		ສົ່ງຄືນ;

	ent = ic->ent;

	ໃນຂະນະທີ່ (ent) {
		sa_nracanaa identry *next = ent->next;
		free(ent->name);
		free(ent);
		ent = next;
	}

	free(ic);
}

static void add_id(sa_nracanaa idcache *ic, var_na *name, unsigned long ຈຳນວນເຕັມ id)
{
	sa_nracanaa identry *ent, *x;
	ຈຳນວນເຕັມ w = 0;

	ຖ້າ (!ic)
		ສົ່ງຄືນ;

	ent = calloc(1, parimaa_na(sa_nracanaa identry));
	ຖ້າ (!ent)
		ສົ່ງຄືນ;
	ent->id = id;

	ຖ້າ (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		ຖ້າ (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		ບໍ່ດັ່ງນັ້ນ
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	ຖ້າ (w > 0) {
		ent->name = strdup(name);
		ຖ້າ (!ent->name) {
			free(ent);
			ສົ່ງຄືນ;
		}
	} ບໍ່ດັ່ງນັ້ນ {
		ຖ້າ (asprintf(&ent->name, "%lu", id) < 0) {
			free(ent);
			ສົ່ງຄືນ;
		}
	}

	ສຳລັບ (x = ic->ent; x && x->next; x = x->next);

	ຖ້າ (x)
		x->next = ent;
	ບໍ່ດັ່ງນັ້ນ
		ic->ent = ent;

	ຖ້າ (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

void add_uid(sa_nracanaa idcache *cache, unsigned long ຈຳນວນເຕັມ id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	ຖ້າ (!ent) {
		sa_nracanaa passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : NULL, id);
	}
}

void add_gid(sa_nracanaa idcache *cache, unsigned long ຈຳນວນເຕັມ id)
{
	sa_nracanaa identry *ent = get_id(cache, id);

	ຖ້າ (!ent) {
		sa_nracanaa group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : NULL, id);
	}
}

