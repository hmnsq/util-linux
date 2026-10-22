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

ಸಂರಚನಾ identry *get_id(ಸಂರಚನಾ idcache *ic, unsigned long ಪೂರ್ಣಾಙ್ಕ id)
{
	ಸಂರಚನಾ identry *ent;

	ಯದಿ (!ic)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತಿ (ent = ic->ent; ent; ent = ent->next) {
		ಯದಿ (ent->id == id)
			ಪ್ರತ್ಯಾಗಮ ent;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸಂರಚನಾ idcache *new_idcache(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ idcache));
}

ಶೂನ್ಯ free_idcache(ಸಂರಚನಾ idcache *ic)
{
	ಸಂರಚನಾ identry *ent;

	ಯದಿ (!ic)
		ಪ್ರತ್ಯಾಗಮ;

	ent = ic->ent;

	ಯಾವತ್ (ent) {
		ಸಂರಚನಾ identry *next = ent->next;
		ಮುಕ್ತ(ent->name);
		ಮುಕ್ತ(ent);
		ent = next;
	}

	ಮುಕ್ತ(ic);
}

ಸ್ಥಿರ ಶೂನ್ಯ add_id(ಸಂರಚನಾ idcache *ic, ವರ್ಣ *name, unsigned long ಪೂರ್ಣಾಙ್ಕ id)
{
	ಸಂರಚನಾ identry *ent, *x;
	ಪೂರ್ಣಾಙ್ಕ w = 0;

	ಯದಿ (!ic)
		ಪ್ರತ್ಯಾಗಮ;

	ent = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ identry));
	ಯದಿ (!ent)
		ಪ್ರತ್ಯಾಗಮ;
	ent->id = id;

	ಯದಿ (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		ಯದಿ (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		ಅನ್ಯಥಾ
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	ಯದಿ (w > 0) {
		ent->name = strdup(name);
		ಯದಿ (!ent->name) {
			ಮುಕ್ತ(ent);
			ಪ್ರತ್ಯಾಗಮ;
		}
	} ಅನ್ಯಥಾ {
		ಯದಿ (asprintf(&ent->name, "%lu", id) < 0) {
			ಮುಕ್ತ(ent);
			ಪ್ರತ್ಯಾಗಮ;
		}
	}

	ಪ್ರತಿ (x = ic->ent; x && x->next; x = x->next);

	ಯದಿ (x)
		x->next = ent;
	ಅನ್ಯಥಾ
		ic->ent = ent;

	ಯದಿ (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

ಶೂನ್ಯ add_uid(ಸಂರಚನಾ idcache *cache, unsigned long ಪೂರ್ಣಾಙ್ಕ id)
{
	ಸಂರಚನಾ identry *ent = get_id(cache, id);

	ಯದಿ (!ent) {
		ಸಂರಚನಾ passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : ರಿಕ್ತ, id);
	}
}

ಶೂನ್ಯ add_gid(ಸಂರಚನಾ idcache *cache, unsigned long ಪೂರ್ಣಾಙ್ಕ id)
{
	ಸಂರಚನಾ identry *ent = get_id(cache, id);

	ಯದಿ (!ent) {
		ಸಂರಚನಾ group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : ರಿಕ್ತ, id);
	}
}

