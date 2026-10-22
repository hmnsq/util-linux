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

بنية identry *get_id(بنية idcache *ic, بنانشان ڊگهو صحيح id)
{
	بنية identry *ent;

	إذا (!ic)
		أرجع عدم;

	لكل (ent = ic->ent; ent; ent = ent->ايندڙ) {
		إذا (ent->id == id)
			أرجع ent;
	}

	أرجع عدم;
}

بنية idcache *new_idcache(فراغ)
{
	أرجع صفر_يادگيري_ڏيو(1, حجم(بنية idcache));
}

فراغ free_idcache(بنية idcache *ic)
{
	بنية identry *ent;

	إذا (!ic)
		أرجع;

	ent = ic->ent;

	طالما (ent) {
		بنية identry *ايندڙ = ent->ايندڙ;
		حرر(ent->name);
		حرر(ent);
		ent = ايندڙ;
	}

	حرر(ic);
}

ساكن فراغ جوڙيو_id(بنية idcache *ic, حرف *name, بنانشان ڊگهو صحيح id)
{
	بنية identry *ent, *x;
	صحيح w = 0;

	إذا (!ic)
		أرجع;

	ent = صفر_يادگيري_ڏيو(1, حجم(بنية identry));
	إذا (!ent)
		أرجع;
	ent->id = id;

	إذا (name) {
#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		إذا (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		وإلا
#جيڪڏهن_پورو
			w = لڙي_ڊيگهه(name);
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

	لكل (x = ic->ent; x && x->ايندڙ; x = x->ايندڙ);

	إذا (x)
		x->ايندڙ = ent;
	وإلا
		ic->ent = ent;

	إذا (w <= 0)
		w = ent->name ? لڙي_ڊيگهه(ent->name) : 0;
	ic->ويڪر = ic->ويڪر < w ? w : ic->ويڪر;
}

فراغ جوڙيو_uid(بنية idcache *cache, بنانشان ڊگهو صحيح id)
{
	بنية identry *ent = get_id(cache, id);

	إذا (!ent) {
		بنية passwd *pw = getpwuid((uid_t) id);
		جوڙيو_id(cache, pw ? pw->pw_name : عدم, id);
	}
}

فراغ جوڙيو_gid(بنية idcache *cache, بنانشان ڊگهو صحيح id)
{
	بنية identry *ent = get_id(cache, id);

	إذا (!ent) {
		بنية group *gr = getgrgid((gid_t) id);
		جوڙيو_id(cache, gr ? gr->gr_name : عدم, id);
	}
}

