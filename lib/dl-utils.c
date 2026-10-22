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
정수 ul_dlopen_symbols(const char *libname, 정수 flags,
		      const struct ul_dlsym *syms, size_t nsyms,
		      void *opers, void **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	만약 (!*dl_handle)
		goto failed;

	dlerror();

	위해 (i = 0; i < nsyms; i++) {
		const struct ul_dlsym *def = &syms[i];
		void **sym;

		sym = (void **) ((char *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		만약 (dlerror())
			goto failed;
	}

	반환 0;
failed:
	만약 (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = NULL;
	}
	만약 (!errno)
		errno = ENOTSUP;
	반환 -errno;
}
