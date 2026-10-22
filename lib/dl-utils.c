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
صحیح ul_dlopen_symbols(ثابت نویسه *libname, صحیح flags,
		      ثابت ساختار ul_dlsym *syms, size_t nsyms,
		      تهی *opers, تهی **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	اگر (!*dl_handle)
		برو failed;

	dlerror();

	برای (i = 0; i < nsyms; i++) {
		ثابت ساختار ul_dlsym *def = &syms[i];
		تهی **sym;

		sym = (تهی **) ((نویسه *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		اگر (dlerror())
			برو failed;
	}

	بازگشت 0;
failed:
	اگر (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = پوچ;
	}
	اگر (!errno)
		errno = ENOTSUP;
	بازگشت -errno;
}
