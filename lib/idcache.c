/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <pwd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <grp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "widechar.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "idcache.h"

ਬਣਤਰ identry *get_id(ਬਣਤਰ idcache *ic, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਪੂਰਨ id)
{
	ਬਣਤਰ identry *ent;

	ਜੇ (!ic)
		ਵਾਪਸ ਨਿਰਾ;

	ਲਈ (ent = ic->ent; ent; ent = ent->ਅਗਲਾ) {
		ਜੇ (ent->id == id)
			ਵਾਪਸ ent;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

ਬਣਤਰ idcache *new_idcache(ਖਾਲੀ)
{
	ਵਾਪਸ ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ idcache));
}

ਖਾਲੀ free_idcache(ਬਣਤਰ idcache *ic)
{
	ਬਣਤਰ identry *ent;

	ਜੇ (!ic)
		ਵਾਪਸ;

	ent = ic->ent;

	ਜਦੋਂ (ent) {
		ਬਣਤਰ identry *ਅਗਲਾ = ent->ਅਗਲਾ;
		ਮੁਕਤ(ent->name);
		ਮੁਕਤ(ent);
		ent = ਅਗਲਾ;
	}

	ਮੁਕਤ(ic);
}

ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_id(ਬਣਤਰ idcache *ic, ਅੱਖਰ *name, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਪੂਰਨ id)
{
	ਬਣਤਰ identry *ent, *x;
	ਪੂਰਨ w = 0;

	ਜੇ (!ic)
		ਵਾਪਸ;

	ent = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(ਬਣਤਰ identry));
	ਜੇ (!ent)
		ਵਾਪਸ;
	ent->id = id;

	ਜੇ (name) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		ਜੇ (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
			w = ਲੜੀ_ਲੰਬਾਈ(name);
	}

	/* note, we ignore names with non-printable widechars */
	ਜੇ (w > 0) {
		ent->name = strdup(name);
		ਜੇ (!ent->name) {
			ਮੁਕਤ(ent);
			ਵਾਪਸ;
		}
	} ਨਹੀਂਤਾਂ {
		ਜੇ (asprintf(&ent->name, "%lu", id) < 0) {
			ਮੁਕਤ(ent);
			ਵਾਪਸ;
		}
	}

	ਲਈ (x = ic->ent; x && x->ਅਗਲਾ; x = x->ਅਗਲਾ);

	ਜੇ (x)
		x->ਅਗਲਾ = ent;
	ਨਹੀਂਤਾਂ
		ic->ent = ent;

	ਜੇ (w <= 0)
		w = ent->name ? ਲੜੀ_ਲੰਬਾਈ(ent->name) : 0;
	ic->ਚੌੜਾਈ = ic->ਚੌੜਾਈ < w ? w : ic->ਚੌੜਾਈ;
}

ਖਾਲੀ ਜੋੜੋ_uid(ਬਣਤਰ idcache *cache, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਪੂਰਨ id)
{
	ਬਣਤਰ identry *ent = get_id(cache, id);

	ਜੇ (!ent) {
		ਬਣਤਰ passwd *pw = getpwuid((uid_t) id);
		ਜੋੜੋ_id(cache, pw ? pw->pw_name : ਨਿਰਾ, id);
	}
}

ਖਾਲੀ ਜੋੜੋ_gid(ਬਣਤਰ idcache *cache, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਪੂਰਨ id)
{
	ਬਣਤਰ identry *ent = get_id(cache, id);

	ਜੇ (!ent) {
		ਬਣਤਰ group *gr = getgrgid((gid_t) id);
		ਜੋੜੋ_id(cache, gr ? gr->gr_name : ਨਿਰਾ, id);
	}
}

