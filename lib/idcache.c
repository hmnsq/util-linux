/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#અન્તર્ભાવ <pwd.h>
#અન્તર્ભાવ <grp.h>
#અન્તર્ભાવ <sys/types.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "widechar.h"
#અન્તર્ભાવ "idcache.h"

સંરચના identry *get_id(સંરચના idcache *ic, unsigned long પૂર્ણાંક id)
{
	સંરચના identry *ent;

	જો (!ic)
		પાછા ખાલી;

	માટે (ent = ic->ent; ent; ent = ent->next) {
		જો (ent->id == id)
			પાછા ent;
	}

	પાછા ખાલી;
}

સંરચના idcache *new_idcache(શૂન્ય)
{
	પાછા calloc(1, કદ(સંરચના idcache));
}

શૂન્ય free_idcache(સંરચના idcache *ic)
{
	સંરચના identry *ent;

	જો (!ic)
		પાછા;

	ent = ic->ent;

	જ્યારે (ent) {
		સંરચના identry *next = ent->next;
		મુક્ત(ent->name);
		મુક્ત(ent);
		ent = next;
	}

	મુક્ત(ic);
}

સ્થિર શૂન્ય add_id(સંરચના idcache *ic, અક્ષર *name, unsigned long પૂર્ણાંક id)
{
	સંરચના identry *ent, *x;
	પૂર્ણાંક w = 0;

	જો (!ic)
		પાછા;

	ent = calloc(1, કદ(સંરચના identry));
	જો (!ent)
		પાછા;
	ent->id = id;

	જો (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		જો (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		નહીંતર
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	જો (w > 0) {
		ent->name = strdup(name);
		જો (!ent->name) {
			મુક્ત(ent);
			પાછા;
		}
	} નહીંતર {
		જો (asprintf(&ent->name, "%lu", id) < 0) {
			મુક્ત(ent);
			પાછા;
		}
	}

	માટે (x = ic->ent; x && x->next; x = x->next);

	જો (x)
		x->next = ent;
	નહીંતર
		ic->ent = ent;

	જો (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

શૂન્ય add_uid(સંરચના idcache *cache, unsigned long પૂર્ણાંક id)
{
	સંરચના identry *ent = get_id(cache, id);

	જો (!ent) {
		સંરચના passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : ખાલી, id);
	}
}

શૂન્ય add_gid(સંરચના idcache *cache, unsigned long પૂર્ણાંક id)
{
	સંરચના identry *ent = get_id(cache, id);

	જો (!ent) {
		સંરચના group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : ખાલી, id);
	}
}

