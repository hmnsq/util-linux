/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাব <pwd.h>
#অন্তর্ভাব <grp.h>
#অন্তর্ভাব <sys/types.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "widechar.h"
#অন্তর্ভাব "idcache.h"

সংরচনা identry *get_id(সংরচনা idcache *ic, unsigned long পূর্ণাঙ্ক id)
{
	সংরচনা identry *ent;

	যদি (!ic)
		প্রত্যাগম রিক্ত;

	প্রতি (ent = ic->ent; ent; ent = ent->next) {
		যদি (ent->id == id)
			প্রত্যাগম ent;
	}

	প্রত্যাগম রিক্ত;
}

সংরচনা idcache *new_idcache(শূন্য)
{
	প্রত্যাগম calloc(1, পরিমাণ(সংরচনা idcache));
}

শূন্য free_idcache(সংরচনা idcache *ic)
{
	সংরচনা identry *ent;

	যদি (!ic)
		প্রত্যাগম;

	ent = ic->ent;

	যাবত্ (ent) {
		সংরচনা identry *next = ent->next;
		মুক্ত(ent->name);
		মুক্ত(ent);
		ent = next;
	}

	মুক্ত(ic);
}

স্থির শূন্য add_id(সংরচনা idcache *ic, বর্ণ *name, unsigned long পূর্ণাঙ্ক id)
{
	সংরচনা identry *ent, *x;
	পূর্ণাঙ্ক w = 0;

	যদি (!ic)
		প্রত্যাগম;

	ent = calloc(1, পরিমাণ(সংরচনা identry));
	যদি (!ent)
		প্রত্যাগম;
	ent->id = id;

	যদি (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		যদি (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		অন্যথা
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	যদি (w > 0) {
		ent->name = strdup(name);
		যদি (!ent->name) {
			মুক্ত(ent);
			প্রত্যাগম;
		}
	} অন্যথা {
		যদি (asprintf(&ent->name, "%lu", id) < 0) {
			মুক্ত(ent);
			প্রত্যাগম;
		}
	}

	প্রতি (x = ic->ent; x && x->next; x = x->next);

	যদি (x)
		x->next = ent;
	অন্যথা
		ic->ent = ent;

	যদি (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

শূন্য add_uid(সংরচনা idcache *cache, unsigned long পূর্ণাঙ্ক id)
{
	সংরচনা identry *ent = get_id(cache, id);

	যদি (!ent) {
		সংরচনা passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : রিক্ত, id);
	}
}

শূন্য add_gid(সংরচনা idcache *cache, unsigned long পূর্ণাঙ্ক id)
{
	সংরচনা identry *ent = get_id(cache, id);

	যদি (!ent) {
		সংরচনা group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : রিক্ত, id);
	}
}

