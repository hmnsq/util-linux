/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#समावेश <dlfcn.h>
#समावेश <errno.h>

#समावेश "dl-utils.h"

/*
 * Open @libname with dlopen() and resolve all symbols listed in @syms
 * into the caller-provided @opers struct.  On success the handle is
 * stored in *@dl_handle and 0 is returned.  On failure *@dl_handle is
 * set to NULL and a negative errno value is returned (usually -ENOTSUP).
 */
पूर्णांक ul_dlopen_symbols(स्थिर अक्षर *libname, पूर्णांक flags,
		      स्थिर काष्ठा ul_dlsym *syms, माप_प्रकार nsyms,
		      व्योम *opers, व्योम **dl_handle)
{
	माप_प्रकार i;

	त्रुटि_सं = 0;

	*dl_handle = dlopen(libname, flags);
	अगर (!*dl_handle)
		जाओ failed;

	dlerror();

	क्रम (i = 0; i < nsyms; i++) {
		स्थिर काष्ठा ul_dlsym *def = &syms[i];
		व्योम **sym;

		sym = (व्योम **) ((अक्षर *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		अगर (dlerror())
			जाओ failed;
	}

	वापस 0;
failed:
	अगर (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = शून्य;
	}
	अगर (!त्रुटि_सं)
		त्रुटि_सं = ENOTSUP;
	वापस -त्रुटि_सं;
}
