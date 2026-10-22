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

بنية identry *get_id(بنية idcache *ic, بنانشانی لما صحيح id)
{
	بنية identry *ent;

	إذا (!ic)
		أرجع عدم;

	لكل (ent = ic->ent; ent; ent = ent->اگلا) {
		إذا (ent->id == id)
			أرجع ent;
	}

	أرجع عدم;
}

بنية idcache *new_idcache(فراغ)
{
	أرجع صفر_حافظہ_دیو(1, حجم(بنية idcache));
}

فراغ free_idcache(بنية idcache *ic)
{
	بنية identry *ent;

	إذا (!ic)
		أرجع;

	ent = ic->ent;

	طالما (ent) {
		بنية identry *اگلا = ent->اگلا;
		حرر(ent->name);
		حرر(ent);
		ent = اگلا;
	}

	حرر(ic);
}

ساكن فراغ جوڑو_id(بنية idcache *ic, حرف *name, بنانشانی لما صحيح id)
{
	بنية identry *ent, *x;
	صحيح w = 0;

	إذا (!ic)
		أرجع;

	ent = صفر_حافظہ_دیو(1, حجم(بنية identry));
	إذا (!ent)
		أرجع;
	ent->id = id;

	إذا (name) {
#جے_تعریف_شدہ HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		إذا (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		وإلا
#جے_انت
			w = لڑی_لمبائی(name);
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

	لكل (x = ic->ent; x && x->اگلا; x = x->اگلا);

	إذا (x)
		x->اگلا = ent;
	وإلا
		ic->ent = ent;

	إذا (w <= 0)
		w = ent->name ? لڑی_لمبائی(ent->name) : 0;
	ic->چوڑائی = ic->چوڑائی < w ? w : ic->چوڑائی;
}

فراغ جوڑو_uid(بنية idcache *cache, بنانشانی لما صحيح id)
{
	بنية identry *ent = get_id(cache, id);

	إذا (!ent) {
		بنية passwd *pw = getpwuid((uid_t) id);
		جوڑو_id(cache, pw ? pw->pw_name : عدم, id);
	}
}

فراغ جوڑو_gid(بنية idcache *cache, بنانشانی لما صحيح id)
{
	بنية identry *ent = get_id(cache, id);

	إذا (!ent) {
		بنية group *gr = getgrgid((gid_t) id);
		جوڑو_id(cache, gr ? gr->gr_name : عدم, id);
	}
}

