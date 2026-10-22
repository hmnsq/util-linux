/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#include <dlfcn.h>
#include <errno.h>

#include "dl-utils.h"

/*
 * Open @libname with dlopen() and resolve all symbols listed in @syms
 * into the caller-provided @opers struct.  On success the handle is
 * stored in *@dl_handle and 0 is returned.  On failure *@dl_handle is
 * set to NULL and a negative errno value is returned (usually -ENOTSUP).
 */
ཧྲིལ ul_dlopen_symbols(const var_na *libname, ཧྲིལ flags,
		      const sa_nracanaa ul_dlsym *syms, size_t nsyms,
		      void *opers, void **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	གལ་ཏེ (!*dl_handle)
		goto failed;

	dlerror();

	ཆེད་དུ (i = 0; i < nsyms; i++) {
		const sa_nracanaa ul_dlsym *def = &syms[i];
		void **sym;

		sym = (void **) ((var_na *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		གལ་ཏེ (dlerror())
			goto failed;
	}

	ཕྱིར 0;
failed:
	གལ་ཏེ (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = NULL;
	}
	གལ་ཏེ (!errno)
		errno = ENOTSUP;
	ཕྱིར -errno;
}
