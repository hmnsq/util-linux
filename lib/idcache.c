/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ضمن <pwd.h>
#ضمن <grp.h>
#ضمن <sys/types.h>

#ضمن "c.h"
#ضمن "widechar.h"
#ضمن "idcache.h"

بنية identry *get_id(بنية idcache *ic, unsigned long صحيح id)
{
	بنية identry *ent;

	אם (!ic)
		החזר عدم;

	עבור (ent = ic->ent; ent; ent = ent->next) {
		אם (ent->id == id)
			החזר ent;
	}

	החזר عدم;
}

بنية idcache *new_idcache(فراغ)
{
	החזר calloc(1, حجم(بنية idcache));
}

فراغ free_idcache(بنية idcache *ic)
{
	بنية identry *ent;

	אם (!ic)
		החזר;

	ent = ic->ent;

	כלעוד (ent) {
		بنية identry *next = ent->next;
		حرر(ent->name);
		حرر(ent);
		ent = next;
	}

	حرر(ic);
}

ساكن فراغ add_id(بنية idcache *ic, حرف *name, unsigned long صحيح id)
{
	بنية identry *ent, *x;
	صحيح w = 0;

	אם (!ic)
		החזר;

	ent = calloc(1, حجم(بنية identry));
	אם (!ent)
		החזר;
	ent->id = id;

	אם (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		אם (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		אחרת
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	אם (w > 0) {
		ent->name = strdup(name);
		אם (!ent->name) {
			حرر(ent);
			החזר;
		}
	} אחרת {
		אם (asprintf(&ent->name, "%lu", id) < 0) {
			حرر(ent);
			החזר;
		}
	}

	עבור (x = ic->ent; x && x->next; x = x->next);

	אם (x)
		x->next = ent;
	אחרת
		ic->ent = ent;

	אם (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

فراغ add_uid(بنية idcache *cache, unsigned long صحيح id)
{
	بنية identry *ent = get_id(cache, id);

	אם (!ent) {
		بنية passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : عدم, id);
	}
}

فراغ add_gid(بنية idcache *cache, unsigned long صحيح id)
{
	بنية identry *ent = get_id(cache, id);

	אם (!ent) {
		بنية group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : عدم, id);
	}
}

