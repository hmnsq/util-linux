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
पूर्णाङ्क ul_dlopen_symbols(अचर वर्ण *libname, पूर्णाङ्क flags,
		      अचर संरचना ul_dlsym *syms, माप_प्रकार nsyms,
		      शून्य *opers, शून्य **dl_handle)
{
	माप_प्रकार i;

	त्रुटि_सं = 0;

	*dl_handle = dlopen(libname, flags);
	यदि (!*dl_handle)
		गच्छ failed;

	dlerror();

	लागि (i = 0; i < nsyms; i++) {
		अचर संरचना ul_dlsym *def = &syms[i];
		शून्य **sym;

		sym = (शून्य **) ((वर्ण *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		यदि (dlerror())
			गच्छ failed;
	}

	फर्काऊ 0;
failed:
	यदि (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = रिक्त;
	}
	यदि (!त्रुटि_सं)
		त्रुटि_सं = ENOTSUP;
	फर्काऊ -त्रुटि_सं;
}
