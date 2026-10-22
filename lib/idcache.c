/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#అన్తర్భావ <pwd.h>
#అన్తర్భావ <grp.h>
#అన్తర్భావ <sys/types.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "widechar.h"
#అన్తర్భావ "idcache.h"

నిర్మాణం identry *get_id(నిర్మాణం idcache *ic, గుర్తులేని దీర్ఘ పూర్ణాంకం id)
{
	నిర్మాణం identry *ent;

	ఒకవేళ (!ic)
		తిరిగి ఖాళీ;

	కొరకు (ent = ic->ent; ent; ent = ent->తదుపరి) {
		ఒకవేళ (ent->id == id)
			తిరిగి ent;
	}

	తిరిగి ఖాళీ;
}

నిర్మాణం idcache *new_idcache(శూన్య)
{
	తిరిగి సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం idcache));
}

శూన్య free_idcache(నిర్మాణం idcache *ic)
{
	నిర్మాణం identry *ent;

	ఒకవేళ (!ic)
		తిరిగి;

	ent = ic->ent;

	అయితే (ent) {
		నిర్మాణం identry *తదుపరి = ent->తదుపరి;
		విడుదల(ent->name);
		విడుదల(ent);
		ent = తదుపరి;
	}

	విడుదల(ic);
}

స్థిర శూన్య కూడు_id(నిర్మాణం idcache *ic, అక్షరం *name, గుర్తులేని దీర్ఘ పూర్ణాంకం id)
{
	నిర్మాణం identry *ent, *x;
	పూర్ణాంకం w = 0;

	ఒకవేళ (!ic)
		తిరిగి;

	ent = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(నిర్మాణం identry));
	ఒకవేళ (!ent)
		తిరిగి;
	ent->id = id;

	ఒకవేళ (name) {
#నిర్వచితమైతే HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		ఒకవేళ (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		లేకపోతే
#అయితే_ముగింపు
			w = అక్షరశ్రేణి_పొడవు(name);
	}

	/* note, we ignore names with non-printable widechars */
	ఒకవేళ (w > 0) {
		ent->name = strdup(name);
		ఒకవేళ (!ent->name) {
			విడుదల(ent);
			తిరిగి;
		}
	} లేకపోతే {
		ఒకవేళ (asprintf(&ent->name, "%lu", id) < 0) {
			విడుదల(ent);
			తిరిగి;
		}
	}

	కొరకు (x = ic->ent; x && x->తదుపరి; x = x->తదుపరి);

	ఒకవేళ (x)
		x->తదుపరి = ent;
	లేకపోతే
		ic->ent = ent;

	ఒకవేళ (w <= 0)
		w = ent->name ? అక్షరశ్రేణి_పొడవు(ent->name) : 0;
	ic->వెడల్పు = ic->వెడల్పు < w ? w : ic->వెడల్పు;
}

శూన్య కూడు_uid(నిర్మాణం idcache *cache, గుర్తులేని దీర్ఘ పూర్ణాంకం id)
{
	నిర్మాణం identry *ent = get_id(cache, id);

	ఒకవేళ (!ent) {
		నిర్మాణం passwd *pw = getpwuid((uid_t) id);
		కూడు_id(cache, pw ? pw->pw_name : ఖాళీ, id);
	}
}

శూన్య కూడు_gid(నిర్మాణం idcache *cache, గుర్తులేని దీర్ఘ పూర్ణాంకం id)
{
	నిర్మాణం identry *ent = get_id(cache, id);

	ఒకవేళ (!ent) {
		నిర్మాణం group *gr = getgrgid((gid_t) id);
		కూడు_id(cache, gr ? gr->gr_name : ఖాళీ, id);
	}
}

