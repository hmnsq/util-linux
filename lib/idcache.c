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

	𐡄𐡍 (!ic)
		أرجع عدم;

	لكل (ent = ic->ent; ent; ent = ent->next) {
		𐡄𐡍 (ent->id == id)
			أرجع ent;
	}

	أرجع عدم;
}

بنية idcache *new_idcache(فراغ)
{
	أرجع calloc(1, حجم(بنية idcache));
}

فراغ free_idcache(بنية idcache *ic)
{
	بنية identry *ent;

	𐡄𐡍 (!ic)
		أرجع;

	ent = ic->ent;

	طالما (ent) {
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

	𐡄𐡍 (!ic)
		أرجع;

	ent = calloc(1, حجم(بنية identry));
	𐡄𐡍 (!ent)
		أرجع;
	ent->id = id;

	𐡄𐡍 (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		𐡄𐡍 (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		وإلا
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	𐡄𐡍 (w > 0) {
		ent->name = strdup(name);
		𐡄𐡍 (!ent->name) {
			حرر(ent);
			أرجع;
		}
	} وإلا {
		𐡄𐡍 (asprintf(&ent->name, "%lu", id) < 0) {
			حرر(ent);
			أرجع;
		}
	}

	لكل (x = ic->ent; x && x->next; x = x->next);

	𐡄𐡍 (x)
		x->next = ent;
	وإلا
		ic->ent = ent;

	𐡄𐡍 (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

فراغ add_uid(بنية idcache *cache, unsigned long صحيح id)
{
	بنية identry *ent = get_id(cache, id);

	𐡄𐡍 (!ent) {
		بنية passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : عدم, id);
	}
}

فراغ add_gid(بنية idcache *cache, unsigned long صحيح id)
{
	بنية identry *ent = get_id(cache, id);

	𐡄𐡍 (!ent) {
		بنية group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : عدم, id);
	}
}

