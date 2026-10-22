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
ціле ul_dlopen_symbols(const char *libname, ціле flags,
		      const struct ul_dlsym *syms, size_t nsyms,
		      void *opers, void **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	якщо (!*dl_handle)
		goto failed;

	dlerror();

	для (i = 0; i < nsyms; i++) {
		const struct ul_dlsym *def = &syms[i];
		void **sym;

		sym = (void **) ((char *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		якщо (dlerror())
			goto failed;
	}

	повернути 0;
failed:
	якщо (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = NULL;
	}
	якщо (!errno)
		errno = ENOTSUP;
	повернути -errno;
}
