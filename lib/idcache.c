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

संरचना identry *get_id(संरचना idcache *ic, unsigned long पूर्णाङ्क id)
{
	संरचना identry *ent;

	सचे (!ic)
		पटिनिवत्त रिक्त;

	अत्थाय (ent = ic->ent; ent; ent = ent->next) {
		सचे (ent->id == id)
			पटिनिवत्त ent;
	}

	पटिनिवत्त रिक्त;
}

संरचना idcache *new_idcache(शून्य)
{
	पटिनिवत्त calloc(1, परिमाण(संरचना idcache));
}

शून्य free_idcache(संरचना idcache *ic)
{
	संरचना identry *ent;

	सचे (!ic)
		पटिनिवत्त;

	ent = ic->ent;

	याव (ent) {
		संरचना identry *next = ent->next;
		मुक्त(ent->name);
		मुक्त(ent);
		ent = next;
	}

	मुक्त(ic);
}

स्थिर शून्य add_id(संरचना idcache *ic, वर्ण *name, unsigned long पूर्णाङ्क id)
{
	संरचना identry *ent, *x;
	पूर्णाङ्क w = 0;

	सचे (!ic)
		पटिनिवत्त;

	ent = calloc(1, परिमाण(संरचना identry));
	सचे (!ent)
		पटिनिवत्त;
	ent->id = id;

	सचे (name) {
#ifdef HAVE_WIDECHAR
		wchar_t wc[LOGIN_NAME_MAX + 1];

		सचे (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		अञ्ञथा
#endif
			w = strlen(name);
	}

	/* note, we ignore names with non-printable widechars */
	सचे (w > 0) {
		ent->name = strdup(name);
		सचे (!ent->name) {
			मुक्त(ent);
			पटिनिवत्त;
		}
	} अञ्ञथा {
		सचे (asprintf(&ent->name, "%lu", id) < 0) {
			मुक्त(ent);
			पटिनिवत्त;
		}
	}

	अत्थाय (x = ic->ent; x && x->next; x = x->next);

	सचे (x)
		x->next = ent;
	अञ्ञथा
		ic->ent = ent;

	सचे (w <= 0)
		w = ent->name ? strlen(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

शून्य add_uid(संरचना idcache *cache, unsigned long पूर्णाङ्क id)
{
	संरचना identry *ent = get_id(cache, id);

	सचे (!ent) {
		संरचना passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : रिक्त, id);
	}
}

शून्य add_gid(संरचना idcache *cache, unsigned long पूर्णाङ्क id)
{
	संरचना identry *ent = get_id(cache, id);

	सचे (!ent) {
		संरचना group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : रिक्त, id);
	}
}

