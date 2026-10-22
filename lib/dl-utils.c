/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <dlfcn.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "dl-utils.h"

/*
 * Open @libname with dlopen() and resolve all symbols listed in @syms
 * into the caller-provided @opers struct.  On success the handle is
 * stored in *@dl_handle and 0 is returned.  On failure *@dl_handle is
 * set to NULL and a negative errno value is returned (usually -ENOTSUP).
 */
ਪੂਰਨ ul_dlopen_symbols(ਅਚਲ ਅੱਖਰ *libname, ਪੂਰਨ flags,
		      ਅਚਲ ਬਣਤਰ ul_dlsym *syms, size_t nsyms,
		      ਖਾਲੀ *opers, ਖਾਲੀ **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	ਜੇ (!*dl_handle)
		ਜਾਓ failed;

	dlerror();

	ਲਈ (i = 0; i < nsyms; i++) {
		ਅਚਲ ਬਣਤਰ ul_dlsym *def = &syms[i];
		ਖਾਲੀ **sym;

		sym = (ਖਾਲੀ **) ((ਅੱਖਰ *) opers + def->ਵਿੱਥ);
		*sym = dlsym(*dl_handle, def->name);

		ਜੇ (dlerror())
			ਜਾਓ failed;
	}

	ਵਾਪਸ 0;
failed:
	ਜੇ (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = ਨਿਰਾ;
	}
	ਜੇ (!errno)
		errno = ENOTSUP;
	ਵਾਪਸ -errno;
}
