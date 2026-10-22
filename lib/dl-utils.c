/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#অন্তর্ভাৱ <dlfcn.h>
#অন্তর্ভাৱ <errno.h>

#অন্তর্ভাৱ "dl-utils.h"

/*
 * Open @libname with dlopen() and resolve all symbols listed in @syms
 * into the caller-provided @opers struct.  On success the handle is
 * stored in *@dl_handle and 0 is returned.  On failure *@dl_handle is
 * set to NULL and a negative errno value is returned (usually -ENOTSUP).
 */
পূর্ণাঙ্ক ul_dlopen_symbols(ধ্রুৱ ৱর্ণ *libname, পূর্ণাঙ্ক flags,
		      ধ্রুৱ সংরচনা ul_dlsym *syms, size_t nsyms,
		      শূন্য *opers, শূন্য **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	যদি (!*dl_handle)
		গচ্ছ failed;

	dlerror();

	বাবে (i = 0; i < nsyms; i++) {
		ধ্রুৱ সংরচনা ul_dlsym *def = &syms[i];
		শূন্য **sym;

		sym = (শূন্য **) ((ৱর্ণ *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		যদি (dlerror())
			গচ্ছ failed;
	}

	ঘূৰাই 0;
failed:
	যদি (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = রিক্ত;
	}
	যদি (!errno)
		errno = ENOTSUP;
	ঘূৰাই -errno;
}
