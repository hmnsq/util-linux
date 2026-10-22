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

ساخت identry *get_id(ساخت idcache *ic, غیرعلامتی طویل صحیح id)
{
	ساخت identry *ent;

	اگر (!ic)
		واپس تہی;

	برائے (ent = ic->ent; ent; ent = ent->اگلا) {
		اگر (ent->id == id)
			واپس ent;
	}

	واپس تہی;
}

ساخت idcache *new_idcache(خالی)
{
	واپس صفر_حافظہ_دو(1, حجم(ساخت idcache));
}

خالی free_idcache(ساخت idcache *ic)
{
	ساخت identry *ent;

	اگر (!ic)
		واپس;

	ent = ic->ent;

	جب (ent) {
		ساخت identry *اگلا = ent->اگلا;
		آزاد(ent->name);
		آزاد(ent);
		ent = اگلا;
	}

	آزاد(ic);
}

ساکن خالی جوڑو_id(ساخت idcache *ic, حرف *name, غیرعلامتی طویل صحیح id)
{
	ساخت identry *ent, *x;
	صحیح w = 0;

	اگر (!ic)
		واپس;

	ent = صفر_حافظہ_دو(1, حجم(ساخت identry));
	اگر (!ent)
		واپس;
	ent->id = id;

	اگر (name) {
#اگر_تعریف_شدہ HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		اگر (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		ورنہ
#اگر_ختم
			w = لڑی_لمبائی(name);
	}

	/* note, we ignore names with non-printable widechars */
	اگر (w > 0) {
		ent->name = strdup(name);
		اگر (!ent->name) {
			آزاد(ent);
			واپس;
		}
	} ورنہ {
		اگر (asprintf(&ent->name, "%lu", id) < 0) {
			آزاد(ent);
			واپس;
		}
	}

	برائے (x = ic->ent; x && x->اگلا; x = x->اگلا);

	اگر (x)
		x->اگلا = ent;
	ورنہ
		ic->ent = ent;

	اگر (w <= 0)
		w = ent->name ? لڑی_لمبائی(ent->name) : 0;
	ic->چوڑائی = ic->چوڑائی < w ? w : ic->چوڑائی;
}

خالی جوڑو_uid(ساخت idcache *cache, غیرعلامتی طویل صحیح id)
{
	ساخت identry *ent = get_id(cache, id);

	اگر (!ent) {
		ساخت passwd *pw = getpwuid((uid_t) id);
		جوڑو_id(cache, pw ? pw->pw_name : تہی, id);
	}
}

خالی جوڑو_gid(ساخت idcache *cache, غیرعلامتی طویل صحیح id)
{
	ساخت identry *ent = get_id(cache, id);

	اگر (!ent) {
		ساخت group *gr = getgrgid((gid_t) id);
		جوڑو_id(cache, gr ? gr->gr_name : تہی, id);
	}
}

