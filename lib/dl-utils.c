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
पूर्णाङ्क ul_dlopen_symbols(ध्रुव वर्ण *libname, पूर्णाङ्क flags,
		      ध्रुव संरचना ul_dlsym *syms, size_t nsyms,
		      शून्य *opers, शून्य **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	यदि (!*dl_handle)
		गच्छ failed;

	dlerror();

	प्रति (i = 0; i < nsyms; i++) {
		ध्रुव संरचना ul_dlsym *def = &syms[i];
		शून्य **sym;

		sym = (शून्य **) ((वर्ण *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		यदि (dlerror())
			गच्छ failed;
	}

	प्रत्यागम 0;
failed:
	यदि (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = रिक्त;
	}
	यदि (!errno)
		errno = ENOTSUP;
	प्रत्यागम -errno;
}
