/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#அந்தர்பாவ <dlfcn.h>
#அந்தர்பாவ <errno.h>

#அந்தர்பாவ "dl-utils.h"

/*
 * Open @libname with dlopen() and resolve all symbols listed in @syms
 * into the caller-provided @opers struct.  On success the handle is
 * stored in *@dl_handle and 0 is returned.  On failure *@dl_handle is
 * set to NULL and a negative errno value is returned (usually -ENOTSUP).
 */
முழு ul_dlopen_symbols(மாறிலி எழுத்து *libname, முழு flags,
		      மாறிலி கட்டமைப்பு ul_dlsym *syms, size_t nsyms,
		      வெறுமை *opers, வெறுமை **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	எனில் (!*dl_handle)
		செல் failed;

	dlerror();

	க்கு (i = 0; i < nsyms; i++) {
		மாறிலி கட்டமைப்பு ul_dlsym *def = &syms[i];
		வெறுமை **sym;

		sym = (வெறுமை **) ((எழுத்து *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		எனில் (dlerror())
			செல் failed;
	}

	திரும்பு 0;
failed:
	எனில் (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = வெற்று;
	}
	எனில் (!errno)
		errno = ENOTSUP;
	திரும்பு -errno;
}
