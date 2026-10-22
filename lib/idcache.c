/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#അന്തര്ഭാവ <pwd.h>
#അന്തര്ഭാവ <grp.h>
#അന്തര്ഭാവ <sys/types.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "widechar.h"
#അന്തര്ഭാവ "idcache.h"

ഘടന identry *get_id(ഘടന idcache *ic, unsigned long പൂർണ്ണസംഖ്യ id)
{
	ഘടന identry *ent;

	എങ്കിൽ (!ic)
		തിരികെ ശൂന്യ;

	വേണ്ടി (ent = ic->ent; ent; ent = ent->next) {
		എങ്കിൽ (ent->id == id)
			തിരികെ ent;
	}

	തിരികെ ശൂന്യ;
}

ഘടന idcache *new_idcache(ശൂന്യം)
{
	തിരികെ calloc(1, വലുപ്പം(ഘടന idcache));
}

ശൂന്യം free_idcache(ഘടന idcache *ic)
{
	ഘടന identry *ent;

	എങ്കിൽ (!ic)
		തിരികെ;

	ent = ic->ent;

	ആകുമ്പോൾ (ent) {
		ഘടന identry *next = ent->next;
		സ്വതന്ത്രം(ent->name);
		സ്വതന്ത്രം(ent);
		ent = next;
	}

	സ്വതന്ത്രം(ic);
}

സ്ഥിതം ശൂന്യം add_id(ഘടന idcache *ic, അക്ഷരം *name, unsigned long പൂർണ്ണസംഖ്യ id)
{
	ഘടന identry *ent, *x;
	പൂർണ്ണസംഖ്യ w = 0;

	എങ്കിൽ (!ic)
		തിരികെ;

	ent = calloc(1, വലുപ്പം(ഘടന identry));
	എങ്കിൽ (!ent)
		തിരികെ;
	ent->id = id;

	എങ്കിൽ (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		എങ്കിൽ (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		അല്ലെങ്കിൽ
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	എങ്കിൽ (w > 0) {
		ent->name = strdup(name);
		എങ്കിൽ (!ent->name) {
			സ്വതന്ത്രം(ent);
			തിരികെ;
		}
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (asprintf(&ent->name, "%lu", id) < 0) {
			സ്വതന്ത്രം(ent);
			തിരികെ;
		}
	}

	വേണ്ടി (x = ic->ent; x && x->next; x = x->next);

	എങ്കിൽ (x)
		x->next = ent;
	അല്ലെങ്കിൽ
		ic->ent = ent;

	എങ്കിൽ (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

ശൂന്യം add_uid(ഘടന idcache *cache, unsigned long പൂർണ്ണസംഖ്യ id)
{
	ഘടന identry *ent = get_id(cache, id);

	എങ്കിൽ (!ent) {
		ഘടന passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : ശൂന്യ, id);
	}
}

ശൂന്യം add_gid(ഘടന idcache *cache, unsigned long പൂർണ്ണസംഖ്യ id)
{
	ഘടന identry *ent = get_id(cache, id);

	എങ്കിൽ (!ent) {
		ഘടന group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : ശൂന്യ, id);
	}
}

