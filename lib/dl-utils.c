/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#अन्तर्भाव <dlfcn.h>
#अन्तर्भाव <errno.h>

#अन्तर्भाव "dl-utils.h"

/*
 * Open @libname with dlopen() and resolve all symbols listed in @syms
 * into the caller-provided @opers struct.  On success the handle is
 * stored in *@dl_handle and 0 is returned.  On failure *@dl_handle is
 * set to NULL and a negative errno value is returned (usually -ENOTSUP).
 */
पूर्णांक ul_dlopen_symbols(स्थिरांक अक्षर *libname, पूर्णांक flags,
		      स्थिरांक संरचना ul_dlsym *syms, माप_प्रकार nsyms,
		      शून्य *opers, शून्य **dl_handle)
{
	माप_प्रकार i;

	त्रुटि_सं = 0;

	*dl_handle = dlopen(libname, flags);
	जर (!*dl_handle)
		जा failed;

	dlerror();

	साठी (i = 0; i < nsyms; i++) {
		स्थिरांक संरचना ul_dlsym *def = &syms[i];
		शून्य **sym;

		sym = (शून्य **) ((अक्षर *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		जर (dlerror())
			जा failed;
	}

	परत 0;
failed:
	जर (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = रिक्त;
	}
	जर (!त्रुटि_सं)
		त्रुटि_सं = ENOTSUP;
	परत -त्रुटि_सं;
}
