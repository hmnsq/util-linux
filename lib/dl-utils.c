/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ضمن <dlfcn.h>
#ضمن <errno.h>

#ضمن "dl-utils.h"

/*
 * Open @libname with dlopen() and resolve all symbols listed in @syms
 * into the caller-provided @opers struct.  On success the handle is
 * stored in *@dl_handle and 0 is returned.  On failure *@dl_handle is
 * set to NULL and a negative errno value is returned (usually -ENOTSUP).
 */
صحيح ul_dlopen_symbols(ثابت حرف *libname, صحيح flags,
		      ثابت بنية ul_dlsym *syms, size_t nsyms,
		      فراغ *opers, فراغ **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	𐤀𐤌 (!*dl_handle)
		اذهب failed;

	dlerror();

	لكل (i = 0; i < nsyms; i++) {
		ثابت بنية ul_dlsym *def = &syms[i];
		فراغ **sym;

		sym = (فراغ **) ((حرف *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		𐤀𐤌 (dlerror())
			اذهب failed;
	}

	أرجع 0;
failed:
	𐤀𐤌 (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = عدم;
	}
	𐤀𐤌 (!errno)
		errno = ENOTSUP;
	أرجع -errno;
}
