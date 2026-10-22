/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#شامل <dlfcn.h>
#شامل <errno.h>

#شامل "dl-utils.h"

/*
 * Open @libname with dlopen() and resolve all symbols listed in @syms
 * into the caller-provided @opers struct.  On success the handle is
 * stored in *@dl_handle and 0 is returned.  On failure *@dl_handle is
 * set to NULL and a negative errno value is returned (usually -ENOTSUP).
 */
صحیح ul_dlopen_symbols(ثابت حرف *libname, صحیح flags,
		      ثابت ساخت ul_dlsym *syms, size_t nsyms,
		      خالی *opers, خالی **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	اگر (!*dl_handle)
		جاؤ failed;

	dlerror();

	برائے (i = 0; i < nsyms; i++) {
		ثابت ساخت ul_dlsym *def = &syms[i];
		خالی **sym;

		sym = (خالی **) ((حرف *) opers + def->انحراف);
		*sym = dlsym(*dl_handle, def->name);

		اگر (dlerror())
			جاؤ failed;
	}

	واپس 0;
failed:
	اگر (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = تہی;
	}
	اگر (!errno)
		errno = ENOTSUP;
	واپس -errno;
}
