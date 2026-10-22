/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com>
 */
#समावेश <pwd.h>
#समावेश <grp.h>
#समावेश <sys/types.h>

#समावेश "c.h"
#समावेश "widechar.h"
#समावेश "idcache.h"

काष्ठा identry *get_id(काष्ठा idcache *ic, अचिन्हित दीर्घ पूर्णांक id)
{
	काष्ठा identry *ent;

	अगर (!ic)
		वापस शून्य;

	क्रम (ent = ic->ent; ent; ent = ent->अग्र) {
		अगर (ent->id == id)
			वापस ent;
	}

	वापस शून्य;
}

काष्ठा idcache *new_idcache(व्योम)
{
	वापस सुस्मृति(1, माप(काष्ठा idcache));
}

व्योम free_idcache(काष्ठा idcache *ic)
{
	काष्ठा identry *ent;

	अगर (!ic)
		वापस;

	ent = ic->ent;

	जबतक (ent) {
		काष्ठा identry *अग्र = ent->अग्र;
		मुक्त(ent->name);
		मुक्त(ent);
		ent = अग्र;
	}

	मुक्त(ic);
}

अटल व्योम add_id(काष्ठा idcache *ic, अक्षर *name, अचिन्हित दीर्घ पूर्णांक id)
{
	काष्ठा identry *ent, *x;
	पूर्णांक w = 0;

	अगर (!ic)
		वापस;

	ent = सुस्मृति(1, माप(काष्ठा identry));
	अगर (!ent)
		वापस;
	ent->id = id;

	अगर (name) {
#अगर_घोषित HAVE_WIDECHAR
		ब_अक्षर_प्रकार wc[LOGIN_NAME_MAX + 1];

		अगर (mbstowcs(wc, name, LOGIN_NAME_MAX) > 0) {
			wc[LOGIN_NAME_MAX] = '\0';
			w = wcswidth(wc, LOGIN_NAME_MAX);
		}
		अन्यथा
#पूर्ण_अगर
			w = म_माप(name);
	}

	/* note, we ignore names with non-printable widechars */
	अगर (w > 0) {
		ent->name = strdup(name);
		अगर (!ent->name) {
			मुक्त(ent);
			वापस;
		}
	} अन्यथा {
		अगर (asprintf(&ent->name, "%lu", id) < 0) {
			मुक्त(ent);
			वापस;
		}
	}

	क्रम (x = ic->ent; x && x->अग्र; x = x->अग्र);

	अगर (x)
		x->अग्र = ent;
	अन्यथा
		ic->ent = ent;

	अगर (w <= 0)
		w = ent->name ? म_माप(ent->name) : 0;
	ic->width = ic->width < w ? w : ic->width;
}

व्योम add_uid(काष्ठा idcache *cache, अचिन्हित दीर्घ पूर्णांक id)
{
	काष्ठा identry *ent = get_id(cache, id);

	अगर (!ent) {
		काष्ठा passwd *pw = getpwuid((uid_t) id);
		add_id(cache, pw ? pw->pw_name : शून्य, id);
	}
}

व्योम add_gid(काष्ठा idcache *cache, अचिन्हित दीर्घ पूर्णांक id)
{
	काष्ठा identry *ent = get_id(cache, id);

	अगर (!ent) {
		काष्ठा group *gr = getgrgid((gid_t) id);
		add_id(cache, gr ? gr->gr_name : शून्य, id);
	}
}

