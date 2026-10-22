/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#अन्तर्भाव <pwd.h>
#अन्तर्भाव <grp.h>
#अन्तर्भाव <sys/types.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "widechar.h"
#अन्तर्भाव "idcache.h"

संरचना identry *get_id(संरचना idcache *ic, अचिह्नित दीर्घ पूर्णाङ्क id)
{
	संरचना identry *ent;

	यदि (!ic)
		प्रत्यागम रिक्त;

	प्रति (ent = ic->ent; ent; ent = ent->अग्र) {
		यदि (ent->id == id)
			प्रत्यागम ent;
	}

	प्रत्यागम रिक्त;
}

संरचना idcache *new_idcache(शून्य)
{
	प्रत्यागम शून्यस्मृतिं_देहि(1, परिमाण(संरचना idcache));
}

शून्य free_idcache(संरचना idcache *ic)
{
	संरचना identry *ent;

	यदि (!ic)
		प्रत्यागम;

	ent = ic->ent;

	यावत् (ent) {
		संरचना identry *अग्र = ent->अग्र;
		मुक्त(ent->name);
		मुक्त(ent);
		ent = अग्र;
	}

	मुक्त(ic);
}

स्थिर शून्य सङ्कलय_id(संरचना idcache *ic, वर्ण *name, अचिह्नित दीर्घ पूर्णाङ्क id)
{
	संरचना identry *ent, *x;
	पूर्णाङ्क w = 0;

	यदि (!ic)
		प्रत्यागम;

	ent = शून्यस्मृतिं_देहि(1, परिमाण(संरचना identry));
	यदि (!ent)
		प्रत्यागम;
	ent->id = id;

	यदि (name) {
#यदि_परिभाषितम् HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		यदि (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		अन्यथा
#यदि_अन्तः
			w = वाक्यदैर्घ्यम्(name);
	}

	/* note, we ignore names with non-printable widechars */
	यदि (w > 0) {
		ent->name = strdup(name);
		यदि (!ent->name) {
			मुक्त(ent);
			प्रत्यागम;
		}
	} अन्यथा {
		यदि (asprintf(&ent->name, "%lu", id) < 0) {
			मुक्त(ent);
			प्रत्यागम;
		}
	}

	प्रति (x = ic->ent; x && x->अग्र; x = x->अग्र);

	यदि (x)
		x->अग्र = ent;
	अन्यथा
		ic->ent = ent;

	यदि (w <= 0)
		w = ent->name ? वाक्यदैर्घ्यम्(ent->name) : 0;
	ic->विस्तारः = ic->विस्तारः < w ? w : ic->विस्तारः;
}

शून्य सङ्कलय_uid(संरचना idcache *cache, अचिह्नित दीर्घ पूर्णाङ्क id)
{
	संरचना identry *ent = get_id(cache, id);

	यदि (!ent) {
		संरचना passwd *pw = getpwuid((uid_t) id);
		सङ्कलय_id(cache, pw ? pw->pw_name : रिक्त, id);
	}
}

शून्य सङ्कलय_gid(संरचना idcache *cache, अचिह्नित दीर्घ पूर्णाङ्क id)
{
	संरचना identry *ent = get_id(cache, id);

	यदि (!ent) {
		संरचना group *gr = getgrgid((gid_t) id);
		सङ्कलय_id(cache, gr ? gr->gr_name : रिक्त, id);
	}
}

