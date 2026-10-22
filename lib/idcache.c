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

संरचना identry *get_id(संरचना idcache *ic, अचिन्हित दीर्घ पूर्णांक id)
{
	संरचना identry *ent;

	जर (!ic)
		परत रिक्त;

	साठी (ent = ic->ent; ent; ent = ent->next) {
		जर (ent->id == id)
			परत ent;
	}

	परत रिक्त;
}

संरचना idcache *new_idcache(शून्य)
{
	परत सुस्मृति(1, आकार(संरचना idcache));
}

शून्य free_idcache(संरचना idcache *ic)
{
	संरचना identry *ent;

	जर (!ic)
		परत;

	ent = ic->ent;

	पर्यंत (ent) {
		संरचना identry *next = ent->next;
		मुक्त(ent->name);
		मुक्त(ent);
		ent = next;
	}

	मुक्त(ic);
}

स्थिर शून्य add_id(संरचना idcache *ic, अक्षर *name, अचिन्हित दीर्घ पूर्णांक id)
{
	संरचना identry *ent, *x;
	पूर्णांक w = 0;

	जर (!ic)
		परत;

	ent = सुस्मृति(1, आकार(संरचना identry));
	जर (!ent)
		परत;
	ent->id = id;

	जर (name) {
#ifdef HAVE_WIDECHAR
		ब_अक्षर_प्रकार wc[LOGIN_NAME_MAX + 1];

		जर (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		अन्यथा
#endif
			w = म_माप(name);
	}

	/* note, we ignore names with non-printable widechars */
	जर (w > 0) {
		ent->name = strdup(name);
		जर (!ent->name) {
			मुक्त(ent);
			परत;
		}
	} अन्यथा {
		जर (asprintf(&ent->name, "%lu", id) < 0) {
			मुक्त(ent);
			परत;
		}
	}

	साठी (x = ic->ent; x && x->next; x = x->next);

	जर (x)
		x->next = ent;
	अन्यथा
		ic->ent = ent;

	जर (w <= 0)
		w = ent->name ? म_माप(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

शून्य add_uid(संरचना idcache *cache, अचिन्हित दीर्घ पूर्णांक id)
{
	संरचना identry *ent = get_id(cache, id);

	जर (!ent) {
		संरचना passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : रिक्त, id);
	}
}

शून्य add_gid(संरचना idcache *cache, अचिन्हित दीर्घ पूर्णांक id)
{
	संरचना identry *ent = get_id(cache, id);

	जर (!ent) {
		संरचना group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : रिक्त, id);
	}
}

