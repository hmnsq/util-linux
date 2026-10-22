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

संरचना identry *get_id(संरचना idcache *ic, अचिन्हित दीर्घ पूर्णाङ्क id)
{
	संरचना identry *ent;

	यदि (!ic)
		प्रत्यागम रिक्त;

	प्रति (ent = ic->ent; ent; ent = ent->next) {
		यदि (ent->id == id)
			प्रत्यागम ent;
	}

	प्रत्यागम रिक्त;
}

संरचना idcache *new_idcache(शून्य)
{
	प्रत्यागम सुस्मृति(1, परिमाण(संरचना idcache));
}

शून्य free_idcache(संरचना idcache *ic)
{
	संरचना identry *ent;

	यदि (!ic)
		प्रत्यागम;

	ent = ic->ent;

	यावत् (ent) {
		संरचना identry *next = ent->next;
		मुक्त(ent->name);
		मुक्त(ent);
		ent = next;
	}

	मुक्त(ic);
}

स्थिर शून्य add_id(संरचना idcache *ic, वर्ण *name, अचिन्हित दीर्घ पूर्णाङ्क id)
{
	संरचना identry *ent, *x;
	पूर्णाङ्क w = 0;

	यदि (!ic)
		प्रत्यागम;

	ent = सुस्मृति(1, परिमाण(संरचना identry));
	यदि (!ent)
		प्रत्यागम;
	ent->id = id;

	यदि (name) {
#ifdef HAVE_WIDECHAR
		ब_अक्षर_प्रकार wc[LOGIN_NAME_MAX + 1];

		यदि (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		अन्यथा
#endif
			w = म_माप(name);
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

	प्रति (x = ic->ent; x && x->next; x = x->next);

	यदि (x)
		x->next = ent;
	अन्यथा
		ic->ent = ent;

	यदि (w <= 0)
		w = ent->name ? म_माप(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

शून्य add_uid(संरचना idcache *cache, अचिन्हित दीर्घ पूर्णाङ्क id)
{
	संरचना identry *ent = get_id(cache, id);

	यदि (!ent) {
		संरचना passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : रिक्त, id);
	}
}

शून्य add_gid(संरचना idcache *cache, अचिन्हित दीर्घ पूर्णाङ्क id)
{
	संरचना identry *ent = get_id(cache, id);

	यदि (!ent) {
		संरचना group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : रिक्त, id);
	}
}

