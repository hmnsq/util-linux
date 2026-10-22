/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#شامل <pwd.h>
#شامل <grp.h>
#شامل <sys/types.h>

#شامل "c.h"
#شامل "widechar.h"
#شامل "idcache.h"

ساختار identry *get_id(ساختار idcache *ic, unsigned long صحیح id)
{
	ساختار identry *ent;

	اگر (!ic)
		بازگشت پوچ;

	برای (ent = ic->ent; ent; ent = ent->next) {
		اگر (ent->id == id)
			بازگشت ent;
	}

	بازگشت پوچ;
}

ساختار idcache *new_idcache(تهی)
{
	بازگشت calloc(1, اندازه(ساختار idcache));
}

تهی free_idcache(ساختار idcache *ic)
{
	ساختار identry *ent;

	اگر (!ic)
		بازگشت;

	ent = ic->ent;

	وقتی (ent) {
		ساختار identry *next = ent->next;
		آزاد(ent->name);
		آزاد(ent);
		ent = next;
	}

	آزاد(ic);
}

ایستا تهی add_id(ساختار idcache *ic, نویسه *name, unsigned long صحیح id)
{
	ساختار identry *ent, *x;
	صحیح w = 0;

	اگر (!ic)
		بازگشت;

	ent = calloc(1, اندازه(ساختار identry));
	اگر (!ent)
		بازگشت;
	ent->id = id;

	اگر (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		اگر (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		وگرنه
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	اگر (w > 0) {
		ent->name = strdup(name);
		اگر (!ent->name) {
			آزاد(ent);
			بازگشت;
		}
	} وگرنه {
		اگر (asprintf(&ent->name, "%lu", id) < 0) {
			آزاد(ent);
			بازگشت;
		}
	}

	برای (x = ic->ent; x && x->next; x = x->next);

	اگر (x)
		x->next = ent;
	وگرنه
		ic->ent = ent;

	اگر (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

تهی add_uid(ساختار idcache *cache, unsigned long صحیح id)
{
	ساختار identry *ent = get_id(cache, id);

	اگر (!ent) {
		ساختار passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : پوچ, id);
	}
}

تهی add_gid(ساختار idcache *cache, unsigned long صحیح id)
{
	ساختار identry *ent = get_id(cache, id);

	اگر (!ent) {
		ساختار group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : پوچ, id);
	}
}

