/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#അന്തര്ഭാവ <dlfcn.h>
#അന്തര്ഭാവ <errno.h>

#അന്തര്ഭാവ "dl-utils.h"

/*
 * Open @libname with dlopen() and resolve all symbols listed in @syms
 * into the caller-provided @opers struct.  On success the handle is
 * stored in *@dl_handle and 0 is returned.  On failure *@dl_handle is
 * set to NULL and a negative errno value is returned (usually -ENOTSUP).
 */
പൂർണ്ണസംഖ്യ ul_dlopen_symbols(സ്ഥിരം അക്ഷരം *libname, പൂർണ്ണസംഖ്യ flags,
		      സ്ഥിരം ഘടന ul_dlsym *syms, size_t nsyms,
		      ശൂന്യം *opers, ശൂന്യം **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	എങ്കിൽ (!*dl_handle)
		പോകുക failed;

	dlerror();

	വേണ്ടി (i = 0; i < nsyms; i++) {
		സ്ഥിരം ഘടന ul_dlsym *def = &syms[i];
		ശൂന്യം **sym;

		sym = (ശൂന്യം **) ((അക്ഷരം *) opers + def->offset);
		*sym = dlsym(*dl_handle, def->name);

		എങ്കിൽ (dlerror())
			പോകുക failed;
	}

	തിരികെ 0;
failed:
	എങ്കിൽ (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = ശൂന്യ;
	}
	എങ്കിൽ (!errno)
		errno = ENOTSUP;
	തിരികെ -errno;
}
