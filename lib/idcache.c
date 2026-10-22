/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#ଅନ୍ତର୍ଭାଵ <pwd.h>
#ଅନ୍ତର୍ଭାଵ <grp.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "widechar.h"
#ଅନ୍ତର୍ଭାଵ "idcache.h"

ସଂରଚନା identry *get_id(ସଂରଚନା idcache *ic, unsigned long ପୂର୍ଣ୍ଣସଂଖ୍ୟା id)
{
	ସଂରଚନା identry *ent;

	ଯଦି (!ic)
		ଫେରନ୍ତୁ ଖାଲି;

	ପାଇଁ (ent = ic->ent; ent; ent = ent->next) {
		ଯଦି (ent->id == id)
			ଫେରନ୍ତୁ ent;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

ସଂରଚନା idcache *new_idcache(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ calloc(1, ଆକାର(ସଂରଚନା idcache));
}

ଶୂନ୍ୟ free_idcache(ସଂରଚନା idcache *ic)
{
	ସଂରଚନା identry *ent;

	ଯଦି (!ic)
		ଫେରନ୍ତୁ;

	ent = ic->ent;

	ଯେତେବେଳେ (ent) {
		ସଂରଚନା identry *next = ent->next;
		ମୁକ୍ତ(ent->name);
		ମୁକ୍ତ(ent);
		ent = next;
	}

	ମୁକ୍ତ(ic);
}

ସ୍ଥିର ଶୂନ୍ୟ add_id(ସଂରଚନା idcache *ic, ଅକ୍ଷର *name, unsigned long ପୂର୍ଣ୍ଣସଂଖ୍ୟା id)
{
	ସଂରଚନା identry *ent, *x;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା w = 0;

	ଯଦି (!ic)
		ଫେରନ୍ତୁ;

	ent = calloc(1, ଆକାର(ସଂରଚନା identry));
	ଯଦି (!ent)
		ଫେରନ୍ତୁ;
	ent->id = id;

	ଯଦି (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		ଯଦି (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		ଅନ୍ୟଥା
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	ଯଦି (w > 0) {
		ent->name = strdup(name);
		ଯଦି (!ent->name) {
			ମୁକ୍ତ(ent);
			ଫେରନ୍ତୁ;
		}
	} ଅନ୍ୟଥା {
		ଯଦି (asprintf(&ent->name, "%lu", id) < 0) {
			ମୁକ୍ତ(ent);
			ଫେରନ୍ତୁ;
		}
	}

	ପାଇଁ (x = ic->ent; x && x->next; x = x->next);

	ଯଦି (x)
		x->next = ent;
	ଅନ୍ୟଥା
		ic->ent = ent;

	ଯଦି (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

ଶୂନ୍ୟ add_uid(ସଂରଚନା idcache *cache, unsigned long ପୂର୍ଣ୍ଣସଂଖ୍ୟା id)
{
	ସଂରଚନା identry *ent = get_id(cache, id);

	ଯଦି (!ent) {
		ସଂରଚନା passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : ଖାଲି, id);
	}
}

ଶୂନ୍ୟ add_gid(ସଂରଚନା idcache *cache, unsigned long ପୂର୍ଣ୍ଣସଂଖ୍ୟା id)
{
	ସଂରଚନା identry *ent = get_id(cache, id);

	ଯଦି (!ent) {
		ସଂରଚନା group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : ଖାଲି, id);
	}
}

