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

কাঠামো identry *get_id(কাঠামো idcache *ic, unsigned long পূর্ণসংখ্যা id)
{
	কাঠামো identry *ent;

	যদি (!ic)
		ফেরত ফাঁকা;

	জন্য (ent = ic->ent; ent; ent = ent->পরবর্তী) {
		যদি (ent->id == id)
			ফেরত ent;
	}

	ফেরত ফাঁকা;
}

কাঠামো idcache *new_idcache(শূন্য)
{
	ফেরত calloc(1, আকার(কাঠামো idcache));
}

শূন্য free_idcache(কাঠামো idcache *ic)
{
	কাঠামো identry *ent;

	যদি (!ic)
		ফেরত;

	ent = ic->ent;

	যখন (ent) {
		কাঠামো identry *পরবর্তী = ent->পরবর্তী;
		মুক্ত(ent->name);
		মুক্ত(ent);
		ent = পরবর্তী;
	}

	মুক্ত(ic);
}

স্থির শূন্য add_id(কাঠামো idcache *ic, অক্ষর *name, unsigned long পূর্ণসংখ্যা id)
{
	কাঠামো identry *ent, *x;
	পূর্ণসংখ্যা w = 0;

	যদি (!ic)
		ফেরত;

	ent = calloc(1, আকার(কাঠামো identry));
	যদি (!ent)
		ফেরত;
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
			ফেরত;
		}
	} অন্যথা {
		যদি (asprintf(&ent->name, "%lu", id) < 0) {
			মুক্ত(ent);
			ফেরত;
		}
	}

	জন্য (x = ic->ent; x && x->পরবর্তী; x = x->পরবর্তী);

	যদি (x)
		x->পরবর্তী = ent;
	অন্যথা
		ic->ent = ent;

	যদি (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

শূন্য add_uid(কাঠামো idcache *cache, unsigned long পূর্ণসংখ্যা id)
{
	কাঠামো identry *ent = get_id(cache, id);

	যদি (!ent) {
		কাঠামো passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : ফাঁকা, id);
	}
}

শূন্য add_gid(কাঠামো idcache *cache, unsigned long পূর্ণসংখ্যা id)
{
	কাঠামো identry *ent = get_id(cache, id);

	যদি (!ent) {
		কাঠামো group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : ফাঁকা, id);
	}
}

