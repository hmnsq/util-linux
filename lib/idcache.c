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

بنية identry *get_id(بنية idcache *ic, بیعلامت دراز صحيح id)
{
	بنية identry *ent;

	إذا (!ic)
		أرجع عدم;

	لكل (ent = ic->ent; ent; ent = ent->بعدی) {
		إذا (ent->id == id)
			أرجع ent;
	}

	أرجع عدم;
}

بنية idcache *new_idcache(فراغ)
{
	أرجع حافظه_صفر_بده(1, حجم(بنية idcache));
}

فراغ free_idcache(بنية idcache *ic)
{
	بنية identry *ent;

	إذا (!ic)
		أرجع;

	ent = ic->ent;

	طالما (ent) {
		بنية identry *بعدی = ent->بعدی;
		حرر(ent->name);
		حرر(ent);
		ent = بعدی;
	}

	حرر(ic);
}

ساكن فراغ جمع_کن_id(بنية idcache *ic, حرف *name, بیعلامت دراز صحيح id)
{
	بنية identry *ent, *x;
	صحيح w = 0;

	إذا (!ic)
		أرجع;

	ent = حافظه_صفر_بده(1, حجم(بنية identry));
	إذا (!ent)
		أرجع;
	ent->id = id;

	إذا (name) {
#اگر_تعریف_شده HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		إذا (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		وإلا
#پایان_اگر
			w = طول_رشته(name);
	}

	/* note, we ignore names with non-printable widechars */
	إذا (w > 0) {
		ent->name = strdup(name);
		إذا (!ent->name) {
			حرر(ent);
			أرجع;
		}
	} وإلا {
		إذا (asprintf(&ent->name, "%lu", id) < 0) {
			حرر(ent);
			أرجع;
		}
	}

	لكل (x = ic->ent; x && x->بعدی; x = x->بعدی);

	إذا (x)
		x->بعدی = ent;
	وإلا
		ic->ent = ent;

	إذا (w <= 0)
		w = ent->name ? طول_رشته(ent->name) : 0;
	ic->عرض = ic->عرض < w ? w : ic->عرض;
}

فراغ جمع_کن_uid(بنية idcache *cache, بیعلامت دراز صحيح id)
{
	بنية identry *ent = get_id(cache, id);

	إذا (!ent) {
		بنية passwd *pw = getpwuid((uid_t) id);
		جمع_کن_id(cache, pw ? pw->pw_name : عدم, id);
	}
}

فراغ جمع_کن_gid(بنية idcache *cache, بیعلامت دراز صحيح id)
{
	بنية identry *ent = get_id(cache, id);

	إذا (!ent) {
		بنية group *gr = getgrgid((gid_t) id);
		جمع_کن_id(cache, gr ? gr->gr_name : عدم, id);
	}
}

