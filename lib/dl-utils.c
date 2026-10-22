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
	सचे (!*dl_handle)
		गच्छ failed;

	dlerror();

	अत्थाय (i = 0; i < nsyms; i++) {
		ध्रुव संरचना ul_dlsym *def = &syms[i];
		शून्य **sym;

		sym = (शून्य **) ((वर्ण *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		सचे (dlerror())
			गच्छ failed;
	}

	पटिनिवत्त 0;
failed:
	सचे (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = रिक्त;
	}
	सचे (!errno)
		errno = ENOTSUP;
	पटिनिवत्त -errno;
}
