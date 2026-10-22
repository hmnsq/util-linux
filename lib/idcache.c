/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ಅನ್ತರ್ಭಾವ <pwd.h>
#ಅನ್ತರ್ಭಾವ <grp.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "widechar.h"
#ಅನ್ತರ್ಭಾವ "idcache.h"

ರಚನೆ identry *get_id(ರಚನೆ idcache *ic, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಪೂರ್ಣಾಂಕ id)
{
	ರಚನೆ identry *ent;

	ಆದರೆ (!ic)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಗಾಗಿ (ent = ic->ent; ent; ent = ent->ಮುಂದಿನ) {
		ಆದರೆ (ent->id == id)
			ಹಿಂತಿರುಗಿ ent;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ರಚನೆ idcache *new_idcache(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ idcache));
}

ಶೂನ್ಯ free_idcache(ರಚನೆ idcache *ic)
{
	ರಚನೆ identry *ent;

	ಆದರೆ (!ic)
		ಹಿಂತಿರುಗಿ;

	ent = ic->ent;

	ಆಗ (ent) {
		ರಚನೆ identry *ಮುಂದಿನ = ent->ಮುಂದಿನ;
		ಬಿಡುಗಡೆ(ent->name);
		ಬಿಡುಗಡೆ(ent);
		ent = ಮುಂದಿನ;
	}

	ಬಿಡುಗಡೆ(ic);
}

ಸ್ಥಿರ ಶೂನ್ಯ ಕೂಡಿಸು_id(ರಚನೆ idcache *ic, ಅಕ್ಷರ *name, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಪೂರ್ಣಾಂಕ id)
{
	ರಚನೆ identry *ent, *x;
	ಪೂರ್ಣಾಂಕ w = 0;

	ಆದರೆ (!ic)
		ಹಿಂತಿರುಗಿ;

	ent = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, ಗಾತ್ರ(ರಚನೆ identry));
	ಆದರೆ (!ent)
		ಹಿಂತಿರುಗಿ;
	ent->id = id;

	ಆದರೆ (name) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		ಆದರೆ (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
			w = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name);
	}

	/* note, we ignore names with non-printable widechars */
	ಆದರೆ (w > 0) {
		ent->name = strdup(name);
		ಆದರೆ (!ent->name) {
			ಬಿಡುಗಡೆ(ent);
			ಹಿಂತಿರುಗಿ;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (asprintf(&ent->name, "%lu", id) < 0) {
			ಬಿಡುಗಡೆ(ent);
			ಹಿಂತಿರುಗಿ;
		}
	}

	ಗಾಗಿ (x = ic->ent; x && x->ಮುಂದಿನ; x = x->ಮುಂದಿನ);

	ಆದರೆ (x)
		x->ಮುಂದಿನ = ent;
	ಇಲ್ಲದಿದ್ದರೆ
		ic->ent = ent;

	ಆದರೆ (w <= 0)
		w = ent->name ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ent->name) : 0;
	ic->ಅಗಲ = ic->ಅಗಲ < w ? w : ic->ಅಗಲ;
}

ಶೂನ್ಯ ಕೂಡಿಸು_uid(ರಚನೆ idcache *cache, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಪೂರ್ಣಾಂಕ id)
{
	ರಚನೆ identry *ent = get_id(cache, id);

	ಆದರೆ (!ent) {
		ರಚನೆ passwd *pw = getpwuid((uid_t) id);
		ಕೂಡಿಸು_id(cache, pw ? pw->pw_name : ಖಾಲಿ, id);
	}
}

ಶೂನ್ಯ ಕೂಡಿಸು_gid(ರಚನೆ idcache *cache, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಪೂರ್ಣಾಂಕ id)
{
	ರಚನೆ identry *ent = get_id(cache, id);

	ಆದರೆ (!ent) {
		ರಚನೆ group *gr = getgrgid((gid_t) id);
		ಕೂಡಿಸು_id(cache, gr ? gr->gr_name : ಖಾಲಿ, id);
	}
}

