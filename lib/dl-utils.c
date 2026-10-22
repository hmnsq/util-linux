/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#અન્તર્ભાવ <dlfcn.h>
#અન્તર્ભાવ <errno.h>

#અન્તર્ભાવ "dl-utils.h"

/*
 * Open @libname with dlopen() and resolve all symbols listed in @syms
 * into the caller-provided @opers struct.  On success the handle is
 * stored in *@dl_handle and 0 is returned.  On failure *@dl_handle is
 * set to NULL and a negative errno value is returned (usually -ENOTSUP).
 */
પૂર્ણાંક ul_dlopen_symbols(અચલ અક્ષર *libname, પૂર્ણાંક flags,
		      અચલ સંરચના ul_dlsym *syms, size_t nsyms,
		      શૂન્ય *opers, શૂન્ય **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	જો (!*dl_handle)
		જાઓ failed;

	dlerror();

	માટે (i = 0; i < nsyms; i++) {
		અચલ સંરચના ul_dlsym *def = &syms[i];
		શૂન્ય **sym;

		sym = (શૂન્ય **) ((અક્ષર *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		જો (dlerror())
			જાઓ failed;
	}

	પાછા 0;
failed:
	જો (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = ખાલી;
	}
	જો (!errno)
		errno = ENOTSUP;
	પાછા -errno;
}
