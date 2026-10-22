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
ចំនួនគត់ ul_dlopen_symbols(const var_na *libname, ចំនួនគត់ flags,
		      const sa_nracanaa ul_dlsym *syms, size_t nsyms,
		      void *opers, void **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	ប្រសិនបើ (!*dl_handle)
		goto failed;

	dlerror();

	សម្រាប់ (i = 0; i < nsyms; i++) {
		const sa_nracanaa ul_dlsym *def = &syms[i];
		void **sym;

		sym = (void **) ((var_na *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		ប្រសិនបើ (dlerror())
			goto failed;
	}

	ត្រឡប់ 0;
failed:
	ប្រសិនបើ (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = NULL;
	}
	ប្រសិនបើ (!errno)
		errno = ENOTSUP;
	ត្រឡប់ -errno;
}
