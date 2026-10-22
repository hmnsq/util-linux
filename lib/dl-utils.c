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
ಪೂರ್ಣಾಙ್ಕ ul_dlopen_symbols(ಧ್ರುವ ವರ್ಣ *libname, ಪೂರ್ಣಾಙ್ಕ flags,
		      ಧ್ರುವ ಸಂರಚನಾ ul_dlsym *syms, size_t nsyms,
		      ಶೂನ್ಯ *opers, ಶೂನ್ಯ **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	ಯದಿ (!*dl_handle)
		ಗಚ್ಛ failed;

	dlerror();

	ಪ್ರತಿ (i = 0; i < nsyms; i++) {
		ಧ್ರುವ ಸಂರಚನಾ ul_dlsym *def = &syms[i];
		ಶೂನ್ಯ **sym;

		sym = (ಶೂನ್ಯ **) ((ವರ್ಣ *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		ಯದಿ (dlerror())
			ಗಚ್ಛ failed;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
failed:
	ಯದಿ (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = ರಿಕ್ತ;
	}
	ಯದಿ (!errno)
		errno = ENOTSUP;
	ಪ್ರತ್ಯಾಗಮ -errno;
}
