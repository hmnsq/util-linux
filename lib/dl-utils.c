/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#ଅନ୍ତର୍ଭାଵ <dlfcn.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>

#ଅନ୍ତର୍ଭାଵ "dl-utils.h"

/*
 * Open @libname with dlopen() and resolve all symbols listed in @syms
 * into the caller-provided @opers struct.  On success the handle is
 * stored in *@dl_handle and 0 is returned.  On failure *@dl_handle is
 * set to NULL and a negative errno value is returned (usually -ENOTSUP).
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_dlopen_symbols(ଅଚଳ ଅକ୍ଷର *libname, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
		      ଅଚଳ ସଂରଚନା ul_dlsym *syms, size_t nsyms,
		      ଶୂନ୍ୟ *opers, ଶୂନ୍ୟ **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	ଯଦି (!*dl_handle)
		ଯାଅ failed;

	dlerror();

	ପାଇଁ (i = 0; i < nsyms; i++) {
		ଅଚଳ ସଂରଚନା ul_dlsym *def = &syms[i];
		ଶୂନ୍ୟ **sym;

		sym = (ଶୂନ୍ୟ **) ((ଅକ୍ଷର *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		ଯଦି (dlerror())
			ଯାଅ failed;
	}

	ଫେରନ୍ତୁ 0;
failed:
	ଯଦି (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = ଖାଲି;
	}
	ଯଦି (!errno)
		errno = ENOTSUP;
	ଫେରନ୍ତୁ -errno;
}
