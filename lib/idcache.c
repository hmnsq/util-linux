/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#অন্তর্ভাৱ <pwd.h>
#অন্তর্ভাৱ <grp.h>
#অন্তর্ভাৱ <sys/types.h>

#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "widechar.h"
#অন্তর্ভাৱ "idcache.h"

সংরচনা identry *get_id(সংরচনা idcache *ic, unsigned long পূর্ণাঙ্ক id)
{
	সংরচনা identry *ent;

	যদি (!ic)
		ঘূৰাই রিক্ত;

	বাবে (ent = ic->ent; ent; ent = ent->next) {
		যদি (ent->id == id)
			ঘূৰাই ent;
	}

	ঘূৰাই রিক্ত;
}

সংরচনা idcache *new_idcache(শূন্য)
{
	ঘূৰাই calloc(1, পরিমাণ(সংরচনা idcache));
}

শূন্য free_idcache(সংরচনা idcache *ic)
{
	সংরচনা identry *ent;

	যদি (!ic)
		ঘূৰাই;

	ent = ic->ent;

	যেতিয়ালৈ (ent) {
		সংরচনা identry *next = ent->next;
		মুক্ত(ent->name);
		মুক্ত(ent);
		ent = next;
	}

	মুক্ত(ic);
}

স্থির শূন্য add_id(সংরচনা idcache *ic, ৱর্ণ *name, unsigned long পূর্ণাঙ্ক id)
{
	সংরচনা identry *ent, *x;
	পূর্ণাঙ্ক w = 0;

	যদি (!ic)
		ঘূৰাই;

	ent = calloc(1, পরিমাণ(সংরচনা identry));
	যদি (!ent)
		ঘূৰাই;
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
			ঘূৰাই;
		}
	} অন্যথা {
		যদি (asprintf(&ent->name, "%lu", id) < 0) {
			মুক্ত(ent);
			ঘূৰাই;
		}
	}

	বাবে (x = ic->ent; x && x->next; x = x->next);

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

