/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 */
#అన్తర్భావ <dlfcn.h>
#అన్తర్భావ <errno.h>

#అన్తర్భావ "dl-utils.h"

/*
 * Open @libname with dlopen() and resolve all symbols listed in @syms
 * into the caller-provided @opers struct.  On success the handle is
 * stored in *@dl_handle and 0 is returned.  On failure *@dl_handle is
 * set to NULL and a negative errno value is returned (usually -ENOTSUP).
 */
పూర్ణాంకం ul_dlopen_symbols(స్థిరరాశి అక్షరం *libname, పూర్ణాంకం flags,
		      స్థిరరాశి నిర్మాణం ul_dlsym *syms, size_t nsyms,
		      శూన్య *opers, శూన్య **dl_handle)
{
	size_t i;

	errno = 0;

	*dl_handle = dlopen(libname, flags);
	ఒకవేళ (!*dl_handle)
		వెళ్ళు failed;

	dlerror();

	కొరకు (i = 0; i < nsyms; i++) {
		స్థిరరాశి నిర్మాణం ul_dlsym *def = &syms[i];
		శూన్య **sym;

		sym = (శూన్య **) ((అక్షరం *) opers + def->స్థానభ్రంశం);
		*sym = dlsym(*dl_handle, def->name);

		ఒకవేళ (dlerror())
			వెళ్ళు failed;
	}

	తిరిగి 0;
failed:
	ఒకవేళ (*dl_handle) {
		dlclose(*dl_handle);
		*dl_handle = ఖాళీ;
	}
	ఒకవేళ (!errno)
		errno = ENOTSUP;
	తిరిగి -errno;
}
