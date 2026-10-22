/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#அந்தர்பாவ <pwd.h>
#அந்தர்பாவ <grp.h>
#அந்தர்பாவ <sys/types.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "widechar.h"
#அந்தர்பாவ "idcache.h"

கட்டமைப்பு identry *get_id(கட்டமைப்பு idcache *ic, unsigned long முழு id)
{
	கட்டமைப்பு identry *ent;

	எனில் (!ic)
		திரும்பு வெற்று;

	க்கு (ent = ic->ent; ent; ent = ent->next) {
		எனில் (ent->id == id)
			திரும்பு ent;
	}

	திரும்பு வெற்று;
}

கட்டமைப்பு idcache *new_idcache(வெறுமை)
{
	திரும்பு calloc(1, அளவு(கட்டமைப்பு idcache));
}

வெறுமை free_idcache(கட்டமைப்பு idcache *ic)
{
	கட்டமைப்பு identry *ent;

	எனில் (!ic)
		திரும்பு;

	ent = ic->ent;

	போது (ent) {
		கட்டமைப்பு identry *next = ent->next;
		விடுவி(ent->name);
		விடுவி(ent);
		ent = next;
	}

	விடுவி(ic);
}

நிலையான வெறுமை add_id(கட்டமைப்பு idcache *ic, எழுத்து *name, unsigned long முழு id)
{
	கட்டமைப்பு identry *ent, *x;
	முழு w = 0;

	எனில் (!ic)
		திரும்பு;

	ent = calloc(1, அளவு(கட்டமைப்பு identry));
	எனில் (!ent)
		திரும்பு;
	ent->id = id;

	எனில் (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		எனில் (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		இல்லையெனில்
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	எனில் (w > 0) {
		ent->name = strdup(name);
		எனில் (!ent->name) {
			விடுவி(ent);
			திரும்பு;
		}
	} இல்லையெனில் {
		எனில் (asprintf(&ent->name, "%lu", id) < 0) {
			விடுவி(ent);
			திரும்பு;
		}
	}

	க்கு (x = ic->ent; x && x->next; x = x->next);

	எனில் (x)
		x->next = ent;
	இல்லையெனில்
		ic->ent = ent;

	எனில் (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

வெறுமை add_uid(கட்டமைப்பு idcache *cache, unsigned long முழு id)
{
	கட்டமைப்பு identry *ent = get_id(cache, id);

	எனில் (!ent) {
		கட்டமைப்பு passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : வெற்று, id);
	}
}

வெறுமை add_gid(கட்டமைப்பு idcache *cache, unsigned long முழு id)
{
	கட்டமைப்பு identry *ent = get_id(cache, id);

	எனில் (!ent) {
		கட்டமைப்பு group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : வெற்று, id);
	}
}

