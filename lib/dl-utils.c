/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ಅನ್ತರ್ಭಾವ <dlfcn.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

#ಅನ್ತರ್ಭಾವ "dl-utils.h"

/*
 * Open @libname with dlopen() and resolve all symbols listed in @syms
 * into the caller-provided @opers struct.  On success the handle is
 * stored in *@dl_handle and 0 is returned.  On failure *@dl_handle is
 * set to NULL and a negative errno value is returned (usually -ENOTSUP).
 */
ಪೂರ್ಣಾಂಕ ul_dlopen_symbols(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *libname, ಪೂರ್ಣಾಂಕ flags,
		      ಸ್ಥಿರಾಂಕ ರಚನೆ ul_dlsym *syms, size_t nsyms,
		      ಶೂನ್ಯ *opers, ಶೂನ್ಯ **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	ಆದರೆ (!*dl_handle)
		ಹೋಗು failed;

	dlerror();

	ಗಾಗಿ (i = 0; i < nsyms; i++) {
		ಸ್ಥಿರಾಂಕ ರಚನೆ ul_dlsym *def = &syms[i];
		ಶೂನ್ಯ **sym;

		sym = (ಶೂನ್ಯ **) ((ಅಕ್ಷರ *) opers + def->ಸ್ಥಾನಾಂತರ);
		*sym = dlsym(*dl_handle, def->name);

		ಆದರೆ (dlerror())
			ಹೋಗು failed;
	}

	ಹಿಂತಿರುಗಿ 0;
failed:
	ಆದರೆ (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = ಖಾಲಿ;
	}
	ಆದರೆ (!errno)
		errno = ENOTSUP;
	ಹಿಂತಿರುಗಿ -errno;
}
