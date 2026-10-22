/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2018]
 *
 *
 * Simple functions to access files. Paths can be globally prefixed to read
 * data from an alternative source (e.g. a /proc dump for regression tests).
 *
 * The paths is possible to format by printf-like way for functions with "f"
 * postfix in the name (e.g. readf, openf, ... ul_path_readf_u64()).
 *
 * The ul_path_read_* API is possible to use without path_cxt handler. In this
 * case is not possible to use global prefix and printf-like formatting.
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <inttypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "fileutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "all-io.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "vfs.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "path.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
ਸਥਿਰ UL_DEBUG_DEFINE_ਨਕਾਬ(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_ਖਾਲੀ_ਥਾਵਾਂ_MASKNAMES;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULPATH_DEBUG_INIT	(1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ULPATH_DEBUG_CXT	(1 << 2)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

ਖਾਲੀ ul_ਰਸਤਾ_init_debug(ਖਾਲੀ)
{
	ਜੇ (ulpath_debug_ਨਕਾਬ)
		ਵਾਪਸ;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

ਬਣਤਰ ਰਸਤਾ_cxt *ul_new_ਰਸਤਾ(ਅਚਲ ਅੱਖਰ *dir, ...)
{
	ਬਣਤਰ ਰਸਤਾ_cxt *pc = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*pc));

	ਜੇ (!pc)
		ਵਾਪਸ ਨਿਰਾ;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	ਜੇ (dir) {
		ਪੂਰਨ rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_ਰਸਤਾ, dir, ap);
		va_end(ap);

		ਜੇ (rc < 0 || !pc->dir_ਰਸਤਾ)
			ਜਾਓ fail;
	}
	ਵਾਪਸ pc;
fail:
	ul_unref_ਰਸਤਾ(pc);
	ਵਾਪਸ ਨਿਰਾ;
}

ਖਾਲੀ ul_ref_ਰਸਤਾ(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਜੇ (pc)
		pc->refcount++;
}

ਖਾਲੀ ul_unref_ਰਸਤਾ(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਜੇ (!pc)
		ਵਾਪਸ;

	pc->refcount--;

	ਜੇ (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		ਜੇ (pc->dialect)
			pc->free_dialect(pc);
		ul_ਰਸਤਾ_close_dirfd(pc);
		ਮੁਕਤ(pc->dir_ਰਸਤਾ);
		ਮੁਕਤ(pc->ਅਗੇਤਰ);
		ਮੁਕਤ(pc);
	}
}

ਖਾਲੀ ul_ਰਸਤਾ_refer_vfs(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਬਣਤਰ ul_vfs_ops *vfs)
{
	ਜੇ (pc)
		pc->vfs = vfs;
}

ਪੂਰਨ ul_ਰਸਤਾ_ਸੈੱਟ_ਅਗੇਤਰ(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ)
{
	ਅੱਖਰ *p = ਨਿਰਾ;

	ਪ੍ਰਤਿਜ੍ਞਾ(pc->dir_fd < 0);

	ਜੇ (ਅਗੇਤਰ) {
		p = strdup(ਅਗੇਤਰ);
		ਜੇ (!p)
			ਵਾਪਸ -ENOMEM;
	}

	ਮੁਕਤ(pc->ਅਗੇਤਰ);
	pc->ਅਗੇਤਰ = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	ਵਾਪਸ 0;
}

ਅਚਲ ਅੱਖਰ *ul_ਰਸਤਾ_get_ਅਗੇਤਰ(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਵਾਪਸ pc ? pc->ਅਗੇਤਰ : ਨਿਰਾ;
}

ਪੂਰਨ ul_ਰਸਤਾ_ਸੈੱਟ_dir(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *dir)
{
	ਅੱਖਰ *p = ਨਿਰਾ;

	ਜੇ (dir) {
		p = strdup(dir);
		ਜੇ (!p)
			ਵਾਪਸ -ENOMEM;
	}

	ਜੇ (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	ਮੁਕਤ(pc->dir_ਰਸਤਾ);
	pc->dir_ਰਸਤਾ = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	ਵਾਪਸ 0;
}

ਅਚਲ ਅੱਖਰ *ul_ਰਸਤਾ_get_dir(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਵਾਪਸ pc ? pc->dir_ਰਸਤਾ : ਨਿਰਾ;
}

ਪੂਰਨ ul_ਰਸਤਾ_ਸੈੱਟ_dialect(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਖਾਲੀ *ਅੰਕੜੇ, ਖਾਲੀ free_ਅੰਕੜੇ(ਬਣਤਰ ਰਸਤਾ_cxt *))
{
	pc->dialect = ਅੰਕੜੇ;
	pc->free_dialect = free_ਅੰਕੜੇ;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	ਵਾਪਸ 0;
}

ਖਾਲੀ *ul_ਰਸਤਾ_get_dialect(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਵਾਪਸ pc ? pc->dialect : ਨਿਰਾ;
}

ਪੂਰਨ ul_ਰਸਤਾ_ਸੈੱਟ_enoent_redirect(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਪੂਰਨ (*func)(ਬਣਤਰ ਰਸਤਾ_cxt *, ਅਚਲ ਅੱਖਰ *, ਪੂਰਨ *))
{
	pc->redirect_on_enoent = func;
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *get_absdir(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਪੂਰਨ rc;
	ਅਚਲ ਅੱਖਰ *dirpath;

	ਜੇ (!pc->ਅਗੇਤਰ)
		ਵਾਪਸ pc->dir_ਰਸਤਾ;

	dirpath = pc->dir_ਰਸਤਾ;
	ਜੇ (!dirpath)
		ਵਾਪਸ pc->ਅਗੇਤਰ;
	ਜੇ (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->ਰਸਤਾ_ਬਫਰ, ਆਕਾਰ(pc->ਰਸਤਾ_ਬਫਰ), "%s/%s", pc->ਅਗੇਤਰ, dirpath);
	ਜੇ (rc < 0)
		ਵਾਪਸ ਨਿਰਾ;
	ਜੇ ((size_t)rc >= ਆਕਾਰ(pc->ਰਸਤਾ_ਬਫਰ)) {
		errno = ENAMETOOLONG;
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ pc->ਰਸਤਾ_ਬਫਰ;
}

ਪੂਰਨ ul_ਰਸਤਾ_is_accessible(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਅਚਲ ਅੱਖਰ *ਰਸਤਾ;
	ਪ੍ਰਤਿਜ੍ਞਾ(pc);

	ਜੇ (pc->dir_fd >= 0)
		ਵਾਪਸ 1;

	ਰਸਤਾ = get_absdir(pc);
	ਜੇ (!ਰਸਤਾ)
		ਵਾਪਸ 0;
	ਵਾਪਸ access(ਰਸਤਾ, F_OK) == 0;
}

ਪੂਰਨ ul_ਰਸਤਾ_get_dirfd(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(pc);
	ਪ੍ਰਤਿਜ੍ਞਾ(pc->dir_ਰਸਤਾ);

	ਜੇ (pc->dir_fd < 0) {
		ਅਚਲ ਅੱਖਰ *ਰਸਤਾ = get_absdir(pc);
		ਜੇ (!ਰਸਤਾ)
			ਵਾਪਸ -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", ਰਸਤਾ));
		pc->dir_fd = open(ਰਸਤਾ, O_RDONLY|O_CLOEXEC);
	}

	ਵਾਪਸ pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
ਖਾਲੀ ul_ਰਸਤਾ_close_dirfd(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਪ੍ਰਤਿਜ੍ਞਾ(pc);

	ਜੇ (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

ਪੂਰਨ ul_ਰਸਤਾ_isopen_dirfd(ਬਣਤਰ ਰਸਤਾ_cxt *pc)
{
	ਵਾਪਸ pc && pc->dir_fd >= 0;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ul_ਰਸਤਾ_mkpath(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, va_list ap)
{
	ਪੂਰਨ rc;

	errno = 0;

	rc = vsnprintf(pc->ਰਸਤਾ_ਬਫਰ, ਆਕਾਰ(pc->ਰਸਤਾ_ਬਫਰ), ਰਸਤਾ, ap);
	ਜੇ (rc < 0) {
		ਜੇ (!errno)
			errno = EINVAL;
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ ((size_t)rc >= ਆਕਾਰ(pc->ਰਸਤਾ_ਬਫਰ)) {
		errno = ENAMETOOLONG;
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਵਾਪਸ pc->ਰਸਤਾ_ਬਫਰ;
}

ਅੱਖਰ *ul_ਰਸਤਾ_get_abspath(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t bufsz, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	ਜੇ (ਰਸਤਾ) {
		ਪੂਰਨ rc;
		va_list ap;
		ਅਚਲ ਅੱਖਰ *ਪੂਛ = ਨਿਰਾ, *dirpath = pc->dir_ਰਸਤਾ;

		va_start(ap, ਰਸਤਾ);
		ਪੂਛ = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);
		va_end(ap);

		ਜੇ (dirpath && *dirpath == '/')
			dirpath++;
		ਜੇ (ਪੂਛ && *ਪੂਛ == '/')
			ਪੂਛ++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->ਅਗੇਤਰ ? pc->ਅਗੇਤਰ : "",
				dirpath ? dirpath : "",
				ਪੂਛ ? ਪੂਛ : "");

		ਜੇ ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			ਵਾਪਸ ਨਿਰਾ;
		}
	} ਨਹੀਂਤਾਂ {
		ਅਚਲ ਅੱਖਰ *tmp = get_absdir(pc);

		ਜੇ (!tmp)
			ਵਾਪਸ ਨਿਰਾ;
		xstrncpy(buf, tmp, bufsz);
	}

	ਵਾਪਸ buf;
}


ਪੂਰਨ ul_ਰਸਤਾ_access(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਪੂਰਨ ਬਹੁਲਕ, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਪੂਰਨ rc;

	ਜੇ (!ਰਸਤਾ)
		ਵਾਪਸ -EINVAL;
	ਜੇ (!pc) {
		rc = access(ਰਸਤਾ, ਬਹੁਲਕ);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", ਰਸਤਾ, rc));
	} ਨਹੀਂਤਾਂ {
		ਪੂਰਨ dir = ul_ਰਸਤਾ_get_dirfd(pc);
		ਜੇ (dir < 0)
			ਵਾਪਸ dir;
		ਜੇ (*ਰਸਤਾ == '/')
			ਰਸਤਾ++;

		rc = faccessat(dir, ਰਸਤਾ, ਬਹੁਲਕ, 0);

		ਜੇ (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, ਰਸਤਾ, &dir) == 0)
			rc = faccessat(dir, ਰਸਤਾ, ਬਹੁਲਕ, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", ਰਸਤਾ, rc));
	}
	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_ਰਸਤਾ_accessf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਪੂਰਨ ਬਹੁਲਕ, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	va_list ap;
	ਅਚਲ ਅੱਖਰ *p;

	va_start(ap, ਰਸਤਾ);
	p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_access(pc, ਬਹੁਲਕ, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
ਪੂਰਨ ul_ਰਸਤਾ_stat(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਬਣਤਰ stat *sb, ਪੂਰਨ flags, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਪੂਰਨ rc;

	ਜੇ (!pc) {
		rc = ਰਸਤਾ ? stat(ਰਸਤਾ, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", ਰਸਤਾ, rc));
	} ਨਹੀਂਤਾਂ {
		ਪੂਰਨ dir = ul_ਰਸਤਾ_get_dirfd(pc);
		ਜੇ (dir < 0)
			ਵਾਪਸ dir;
		ਜੇ (ਰਸਤਾ) {
			ਜੇ  (*ਰਸਤਾ == '/')
				ਰਸਤਾ++;
			rc = fstatat(dir, ਰਸਤਾ, sb, flags);

		} ਨਹੀਂਤਾਂ
			rc = fstat(dir, sb);	/* dir itself */

		ਜੇ (rc && errno == ENOENT
		    && ਰਸਤਾ
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, ਰਸਤਾ, &dir) == 0)
			rc = fstatat(dir, ਰਸਤਾ, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", ਰਸਤਾ, rc));
	}
	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_ਰਸਤਾ_vstatf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਬਣਤਰ stat *sb, ਪੂਰਨ flags, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, va_list ap)
{
	ਅਚਲ ਅੱਖਰ *p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_stat(pc, sb, flags, p);
}

ਪੂਰਨ ul_ਰਸਤਾ_statf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਬਣਤਰ stat *sb, ਪੂਰਨ flags, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	va_list ap;
	ਪੂਰਨ rc;

	va_start(ap, ਰਸਤਾ);
	rc = ul_ਰਸਤਾ_vstatf(pc, sb, flags, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ rc;
}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
ਪੂਰਨ ul_ਰਸਤਾ_statx(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਬਣਤਰ statx *stx, ਪੂਰਨ flags, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਨਕਾਬ,
		  ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਪੂਰਨ rc;

	ਜੇ (!ਰਸਤਾ)
		ਵਾਪਸ -EINVAL;
	ਜੇ (!pc)
		rc = ਰਸਤਾ ? statx(AT_FDCWD, ਰਸਤਾ, flags, ਨਕਾਬ, stx) : - EINVAL;
	ਨਹੀਂਤਾਂ {
		ਪੂਰਨ dir = ul_ਰਸਤਾ_get_dirfd(pc);
		ਜੇ (dir < 0)
			ਵਾਪਸ dir;
		ਜੇ (*ਰਸਤਾ == '/')
			ਰਸਤਾ++;

		rc = statx(dir, ਰਸਤਾ, flags, ਨਕਾਬ, stx);
	}

	ਵਾਪਸ rc;
}
#ਨਹੀਂਤਾਂ
ਪੂਰਨ ul_ਰਸਤਾ_statx(ਬਣਤਰ ਰਸਤਾ_cxt *pc __attribute__((__unused__)),
		  ਬਣਤਰ statx *stx __attribute__((__unused__)),
		  ਪੂਰਨ flags __attribute__((__unused__)),
		  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਨਕਾਬ __attribute__((__unused__)),
		  ਅਚਲ ਅੱਖਰ *ਰਸਤਾ __attribute__((__unused__)))
{
	errno = ENOSYS;
	ਵਾਪਸ -1;
}
#ਜੇ_ਅੰਤ /* HAVE_STATX */

ਪੂਰਨ ul_ਰਸਤਾ_vstatxf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਬਣਤਰ statx *stx, ਪੂਰਨ flags, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਨਕਾਬ,
		    ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, va_list ap)
{
	ਅਚਲ ਅੱਖਰ *p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_statx(pc, stx, flags, ਨਕਾਬ, p);
}

ਪੂਰਨ ul_ਰਸਤਾ_statxf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਬਣਤਰ statx *stx, ਪੂਰਨ flags, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਨਕਾਬ,
		   ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	va_list ap;
	ਪੂਰਨ rc;

	va_start(ap, ਰਸਤਾ);
	rc = ul_ਰਸਤਾ_vstatxf(pc, stx, flags, ਨਕਾਬ, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_ਰਸਤਾ_open(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਪੂਰਨ flags, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਪੂਰਨ fd;

	ਜੇ (!ਰਸਤਾ)
		ਵਾਪਸ -EINVAL;
	ਜੇ (!pc) {
		fd = open(ਰਸਤਾ, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", ਰਸਤਾ));
	} ਨਹੀਂਤਾਂ {
		ਪੂਰਨ fdx;
		ਪੂਰਨ dir = ul_ਰਸਤਾ_get_dirfd(pc);
		ਜੇ (dir < 0)
			ਵਾਪਸ dir;

		ਜੇ (*ਰਸਤਾ == '/')
			ਰਸਤਾ++;

		fdx = fd = openat(dir, ਰਸਤਾ, flags);

		ਜੇ (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, ਰਸਤਾ, &dir) == 0)
			fd = openat(dir, ਰਸਤਾ, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", ਰਸਤਾ, fdx != fd ? " [redirected]" : ""));
	}
	ਵਾਪਸ fd;
}

ਪੂਰਨ ul_ਰਸਤਾ_vopenf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਪੂਰਨ flags, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, va_list ap)
{
	ਅਚਲ ਅੱਖਰ *p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_open(pc, flags, p);
}

ਪੂਰਨ ul_ਰਸਤਾ_openf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਪੂਰਨ flags, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	va_list ap;
	ਪੂਰਨ rc;

	va_start(ap, ਰਸਤਾ);
	rc = ul_ਰਸਤਾ_vopenf(pc, flags, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_ਰਸਤਾ_fopen(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਪੂਰਨ flags = ul_ਬਹੁਲਕ_ਮੰਜ਼ਿਲ_flags(ਬਹੁਲਕ);
	ਪੂਰਨ fd = ul_ਰਸਤਾ_open(pc, flags, ਰਸਤਾ);

	ਜੇ (fd < 0)
		ਵਾਪਸ ਨਿਰਾ;

	ਵਾਪਸ ul_vfs_fdopen(pc ? pc->vfs : ਨਿਰਾ, fd, ਬਹੁਲਕ);
}


FILE *ul_ਰਸਤਾ_vfopenf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, va_list ap)
{
	ਅਚਲ ਅੱਖਰ *p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);

	ਵਾਪਸ !p ? ਨਿਰਾ : ul_ਰਸਤਾ_fopen(pc, ਬਹੁਲਕ, p);
}

FILE *ul_ਰਸਤਾ_fopenf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਬਹੁਲਕ, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, ਰਸਤਾ);
	f = ul_ਰਸਤਾ_vfopenf(pc, ਬਹੁਲਕ, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_ਰਸਤਾ_opendir(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	DIR *dir;
	ਪੂਰਨ fd = -1;

	ਜੇ (ਰਸਤਾ)
		fd = ul_ਰਸਤਾ_open(pc, O_RDONLY|O_CLOEXEC, ਰਸਤਾ);
	ਨਹੀਂਤਾਂ ਜੇ (pc->dir_ਰਸਤਾ) {
		ਪੂਰਨ dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_ਰਸਤਾ_get_dirfd(pc);
		ਜੇ (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	ਜੇ (fd < 0)
		ਵਾਪਸ ਨਿਰਾ;

	dir = fdopendir(fd);
	ਜੇ (!dir) {
		close(fd);
		ਵਾਪਸ ਨਿਰਾ;
	}
	ਜੇ (!ਰਸਤਾ)
		 rewinddir(dir);
	ਵਾਪਸ dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_ਰਸਤਾ_vopendirf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, va_list ap)
{
	ਅਚਲ ਅੱਖਰ *p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);

	ਵਾਪਸ !p ? ਨਿਰਾ : ul_ਰਸਤਾ_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_ਰਸਤਾ_opendirf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, ਰਸਤਾ);
	dir = ul_ਰਸਤਾ_vopendirf(pc, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_ਰਸਤਾ_readlink(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t bufsiz, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਪੂਰਨ dirfd;
	ssize_t ssz;

	ਜੇ (!ਰਸਤਾ) {
		ਅਚਲ ਅੱਖਰ *p = get_absdir(pc);
		ਜੇ (!p)
			ਵਾਪਸ -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} ਨਹੀਂਤਾਂ {
		dirfd = ul_ਰਸਤਾ_get_dirfd(pc);
		ਜੇ (dirfd < 0)
			ਵਾਪਸ dirfd;

		ਜੇ (*ਰਸਤਾ == '/')
			ਰਸਤਾ++;

		ssz = readlinkat(dirfd, ਰਸਤਾ, buf, bufsiz - 1);
	}

	ਜੇ (ssz >= 0)
		buf[ssz] = '\0';
	ਵਾਪਸ ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_ਰਸਤਾ_readlinkf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t bufsiz, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	ਅਚਲ ਅੱਖਰ *p;
	va_list ap;

	va_start(ap, ਰਸਤਾ);
	p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_readlink(pc, buf, bufsiz, p);
}

ਪੂਰਨ ul_ਰਸਤਾ_read(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t len, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਪੂਰਨ rc, errsv;
	ਪੂਰਨ fd;

	fd = ul_ਰਸਤਾ_open(pc, O_RDONLY|O_CLOEXEC, ਰਸਤਾ);
	ਜੇ (fd < 0)
		ਵਾਪਸ -errno;

	DBG(CXT, ul_debug(" reading '%s'", ਰਸਤਾ));
	rc = ul_vfs_read_all(pc ? pc->vfs : ਨਿਰਾ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ਨਿਰਾ, fd);
	errno = errsv;
	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_ਰਸਤਾ_vreadf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t len, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, va_list ap)
{
	ਅਚਲ ਅੱਖਰ *p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_read(pc, buf, len, p);
}

ਪੂਰਨ ul_ਰਸਤਾ_readf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t len, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	va_list ap;
	ਪੂਰਨ rc;

	va_start(ap, ਰਸਤਾ);
	rc = ul_ਰਸਤਾ_vreadf(pc, buf, len, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
ਪੂਰਨ ul_ਰਸਤਾ_read_string(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ **str, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਅੱਖਰ buf[BUFSIZ];
	ਪੂਰਨ rc;

	ਜੇ (!str)
		ਵਾਪਸ -EINVAL;

	*str = ਨਿਰਾ;

	rc = ul_ਰਸਤਾ_read_ਬਫਰ(pc, buf, ਆਕਾਰ(buf), ਰਸਤਾ);
	ਜੇ (rc < 0)
		ਵਾਪਸ rc;

	*str = strdup(buf);
	ਜੇ (!*str)
		rc = -ENOMEM;

	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_ਰਸਤਾ_readf_string(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ **str, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	ਅਚਲ ਅੱਖਰ *p;
	va_list ap;

	va_start(ap, ਰਸਤਾ);
	p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_read_string(pc, str, p);
}

ਪੂਰਨ ul_ਰਸਤਾ_read_ਬਫਰ(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t bufsz, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਪੂਰਨ rc = ul_ਰਸਤਾ_read(pc, buf, bufsz - 1, ਰਸਤਾ);

	ਜੇ (rc == 0)
		buf[0] = '\0';

	ਨਹੀਂਤਾਂ ਜੇ (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		ਜੇ (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		ਨਹੀਂਤਾਂ
			buf[rc] = '\0';
	}

	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_ਰਸਤਾ_vreadf_ਬਫਰ(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t bufsz, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, va_list ap)
{
	ਅਚਲ ਅੱਖਰ *p;

	p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_read_ਬਫਰ(pc, buf, bufsz, p);
}

ਪੂਰਨ ul_ਰਸਤਾ_readf_ਬਫਰ(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅੱਖਰ *buf, size_t bufsz, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	va_list ap;
	ਪੂਰਨ rc;

	va_start(ap, ਰਸਤਾ);
	rc = ul_ਰਸਤਾ_vreadf_ਬਫਰ(pc, buf, bufsz, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_ਰਸਤਾ_scanf(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	ਪੂਰਨ rc;

	f = ul_ਰਸਤਾ_fopen(pc, "r" UL_CLOEXECSTR, ਰਸਤਾ);
	ਜੇ (!f)
		ਵਾਪਸ -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, ਰਸਤਾ));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_ਰਸਤਾ_scanff(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, va_list ap, ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	ਪੂਰਨ rc;

	f = ul_ਰਸਤਾ_vfopenf(pc, "r" UL_CLOEXECSTR, ਰਸਤਾ, ap);
	ਜੇ (!f)
		ਵਾਪਸ -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ਵਾਪਸ rc;
}


ਪੂਰਨ ul_ਰਸਤਾ_read_s64(ਬਣਤਰ ਰਸਤਾ_cxt *pc, int64_t *res, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	int64_t x = 0;
	ਪੂਰਨ rc;

	rc = ul_ਰਸਤਾ_scanf(pc, ਰਸਤਾ, "%"SCNd64, &x);
	ਜੇ (rc != 1)
		ਵਾਪਸ -1;
	ਜੇ (res)
		*res = x;
	ਵਾਪਸ 0;
}

ਪੂਰਨ ul_ਰਸਤਾ_readf_s64(ਬਣਤਰ ਰਸਤਾ_cxt *pc, int64_t *res, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	ਅਚਲ ਅੱਖਰ *p;
	va_list ap;

	va_start(ap, ਰਸਤਾ);
	p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_read_s64(pc, res, p);
}

ਪੂਰਨ ul_ਰਸਤਾ_read_u64(ਬਣਤਰ ਰਸਤਾ_cxt *pc, uint64_t *res, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	uint64_t x = 0;
	ਪੂਰਨ rc;

	rc = ul_ਰਸਤਾ_scanf(pc, ਰਸਤਾ, "%"SCNu64, &x);
	ਜੇ (rc != 1)
		ਵਾਪਸ -1;
	ਜੇ (res)
		*res = x;
	ਵਾਪਸ 0;
}

ਪੂਰਨ ul_ਰਸਤਾ_readf_u64(ਬਣਤਰ ਰਸਤਾ_cxt *pc, uint64_t *res, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	ਅਚਲ ਅੱਖਰ *p;
	va_list ap;

	va_start(ap, ਰਸਤਾ);
	p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_read_u64(pc, res, p);
}

ਪੂਰਨ ul_ਰਸਤਾ_read_s32(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਪੂਰਨ *res, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਪੂਰਨ rc, x = 0;

	rc = ul_ਰਸਤਾ_scanf(pc, ਰਸਤਾ, "%d", &x);
	ਜੇ (rc != 1)
		ਵਾਪਸ -1;
	ਜੇ (res)
		*res = x;
	ਵਾਪਸ 0;
}

ਪੂਰਨ ul_ਰਸਤਾ_readf_s32(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਪੂਰਨ *res, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	ਅਚਲ ਅੱਖਰ *p;
	va_list ap;

	va_start(ap, ਰਸਤਾ);
	p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_read_s32(pc, res, p);
}

ਪੂਰਨ ul_ਰਸਤਾ_read_u32(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *res, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਪੂਰਨ rc;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ x = 0;

	rc = ul_ਰਸਤਾ_scanf(pc, ਰਸਤਾ, "%u", &x);
	ਜੇ (rc != 1)
		ਵਾਪਸ -1;
	ਜੇ (res)
		*res = x;
	ਵਾਪਸ 0;
}

ਪੂਰਨ ul_ਰਸਤਾ_readf_u32(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *res, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	ਅਚਲ ਅੱਖਰ *p;
	va_list ap;

	va_start(ap, ਰਸਤਾ);
	p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_read_u32(pc, res, p);
}

ਪੂਰਨ ul_ਰਸਤਾ_read_majmin(ਬਣਤਰ ਰਸਤਾ_cxt *pc, dev_t *res, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਪੂਰਨ rc, maj = 0, min = 0;

	rc = ul_ਰਸਤਾ_scanf(pc, ਰਸਤਾ, "%d:%d", &maj, &min);
	ਜੇ (rc != 2)
		ਵਾਪਸ -1;
	ਜੇ (res)
		*res = makedev(maj, min);
	ਵਾਪਸ 0;
}

ਪੂਰਨ ul_ਰਸਤਾ_readf_majmin(ਬਣਤਰ ਰਸਤਾ_cxt *pc, dev_t *res, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	ਅਚਲ ਅੱਖਰ *p;
	va_list ap;

	va_start(ap, ਰਸਤਾ);
	p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_read_majmin(pc, res, p);
}

ਪੂਰਨ ul_ਰਸਤਾ_write_string(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *str, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਪੂਰਨ rc, errsv;
	ਪੂਰਨ fd;

	fd = ul_ਰਸਤਾ_open(pc, O_WRONLY|O_CLOEXEC, ਰਸਤਾ);
	ਜੇ (fd < 0)
		ਵਾਪਸ -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : ਨਿਰਾ, fd, str, ਲੜੀ_ਲੰਬਾਈ(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ਨਿਰਾ, fd);
	errno = errsv;
	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_ਰਸਤਾ_writef_string(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *str, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	ਅਚਲ ਅੱਖਰ *p;
	va_list ap;

	va_start(ap, ਰਸਤਾ);
	p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_write_string(pc, str, p);
}

ਪੂਰਨ ul_ਰਸਤਾ_write_s64(ਬਣਤਰ ਰਸਤਾ_cxt *pc, int64_t ਅੰਕ, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਅੱਖਰ buf[ਆਕਾਰ(stringify_ਮੁੱਲ(LLONG_MAX))];
	ਪੂਰਨ rc, errsv;
	ਪੂਰਨ fd, len;

	fd = ul_ਰਸਤਾ_open(pc, O_WRONLY|O_CLOEXEC, ਰਸਤਾ);
	ਜੇ (fd < 0)
		ਵਾਪਸ -errno;

	len = snprintf(buf, ਆਕਾਰ(buf), "%" PRId64, ਅੰਕ);
	ਜੇ (len < 0 || (size_t) len >= ਆਕਾਰ(buf))
		rc = len < 0 ? -errno : -E2BIG;
	ਨਹੀਂਤਾਂ
		rc = ul_vfs_write_all(pc ? pc->vfs : ਨਿਰਾ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ਨਿਰਾ, fd);
	errno = errsv;
	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_ਰਸਤਾ_write_u64(ਬਣਤਰ ਰਸਤਾ_cxt *pc, uint64_t ਅੰਕ, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	ਅੱਖਰ buf[ਆਕਾਰ(stringify_ਮੁੱਲ(ULLONG_MAX))];
	ਪੂਰਨ rc, errsv;
	ਪੂਰਨ fd, len;

	fd = ul_ਰਸਤਾ_open(pc, O_WRONLY|O_CLOEXEC, ਰਸਤਾ);
	ਜੇ (fd < 0)
		ਵਾਪਸ -errno;

	len = snprintf(buf, ਆਕਾਰ(buf), "%" PRIu64, ਅੰਕ);
	ਜੇ (len < 0 || (size_t) len >= ਆਕਾਰ(buf))
		rc = len < 0 ? -errno : -E2BIG;
	ਨਹੀਂਤਾਂ
		rc = ul_vfs_write_all(pc ? pc->vfs : ਨਿਰਾ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ਨਿਰਾ, fd);
	errno = errsv;
	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_ਰਸਤਾ_writef_u64(ਬਣਤਰ ਰਸਤਾ_cxt *pc, uint64_t ਅੰਕ, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	ਅਚਲ ਅੱਖਰ *p;
	va_list ap;

	va_start(ap, ਰਸਤਾ);
	p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_write_u64(pc, ਅੰਕ, p);

}

ਪੂਰਨ ul_ਰਸਤਾ_ਗਿਣਤੀ_dirents(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ)
{
	DIR *dir;
	ਪੂਰਨ r = 0;

	dir = ul_ਰਸਤਾ_opendir(pc, ਰਸਤਾ);
	ਜੇ (!dir)
		ਵਾਪਸ 0;

	ਜਦੋਂ (xreaddir(dir)) r++;

	closedir(dir);
	ਵਾਪਸ r;
}

ਪੂਰਨ ul_ਰਸਤਾ_countf_dirents(ਬਣਤਰ ਰਸਤਾ_cxt *pc, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	ਅਚਲ ਅੱਖਰ *p;
	va_list ap;

	va_start(ap, ਰਸਤਾ);
	p = ul_ਰਸਤਾ_mkpath(pc, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ !p ? -errno : ul_ਰਸਤਾ_ਗਿਣਤੀ_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
ਪੂਰਨ ul_ਰਸਤਾ_ਅਗਲਾ_dirent(ਬਣਤਰ ਰਸਤਾ_cxt *pc, DIR **sub, ਅਚਲ ਅੱਖਰ *dirname, ਬਣਤਰ dirent **d)
{
	ਜੇ (!pc || !sub || !d)
		ਵਾਪਸ -EINVAL;

	ਜੇ (!*sub) {
		*sub = ul_ਰਸਤਾ_opendir(pc, dirname);
		ਜੇ (!*sub)
			ਵਾਪਸ -errno;
	}

	*d = xreaddir(*sub);
	ਜੇ (*d)
		ਵਾਪਸ 0;

	closedir(*sub);
	*sub = ਨਿਰਾ;
	ਵਾਪਸ 1;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_CPU_ਸੈੱਟ_T
ਸਥਿਰ ਪੂਰਨ ul_ਰਸਤਾ_cpuparse(ਬਣਤਰ ਰਸਤਾ_cxt *pc, cpu_set_t **ਸੈੱਟ, ਪੂਰਨ maxcpus, ਪੂਰਨ islist, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	ਅੱਖਰ *buf;
	ਪੂਰਨ rc;

	*ਸੈੱਟ = ਨਿਰਾ;

	buf = ਸਮ੍ਰਿਤੀ_ਦਿਓ(len);
	ਜੇ (!buf)
		ਵਾਪਸ -ENOMEM;

	rc = ul_ਰਸਤਾ_vreadf_ਬਫਰ(pc, buf, len, ਰਸਤਾ, ap);
	ਜੇ (rc < 0)
		ਜਾਓ ਬਾਹਰਲਾ;

	*ਸੈੱਟ = cpuset_alloc(maxcpus, &setsize, ਨਿਰਾ);
	ਜੇ (!*ਸੈੱਟ) {
		rc = -EINVAL;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (islist) {
		ਜੇ (cpulist_parse(buf, *ਸੈੱਟ, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			ਜਾਓ ਬਾਹਰਲਾ;
		}
	} ਨਹੀਂਤਾਂ {
		ਜੇ (cpumask_parse(buf, *ਸੈੱਟ, setsize)) {
			errno = EINVAL;
			rc = -errno;
			ਜਾਓ ਬਾਹਰਲਾ;
		}
	}
	rc = 0;

ਬਾਹਰਲਾ:
	ਜੇ (rc) {
		cpuset_free(*ਸੈੱਟ);
		*ਸੈੱਟ = ਨਿਰਾ;
	}
	ਮੁਕਤ(buf);
	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_ਰਸਤਾ_readf_cpuset(ਬਣਤਰ ਰਸਤਾ_cxt *pc, cpu_set_t **ਸੈੱਟ, ਪੂਰਨ maxcpus, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	va_list ap;
	ਪੂਰਨ rc = 0;

	va_start(ap, ਰਸਤਾ);
	rc = ul_ਰਸਤਾ_cpuparse(pc, ਸੈੱਟ, maxcpus, 0, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ rc;
}

ਪੂਰਨ ul_ਰਸਤਾ_readf_cpulist(ਬਣਤਰ ਰਸਤਾ_cxt *pc, cpu_set_t **ਸੈੱਟ, ਪੂਰਨ maxcpus, ਅਚਲ ਅੱਖਰ *ਰਸਤਾ, ...)
{
	va_list ap;
	ਪੂਰਨ rc = 0;

	va_start(ap, ਰਸਤਾ);
	rc = ul_ਰਸਤਾ_cpuparse(pc, ਸੈੱਟ, maxcpus, 1, ਰਸਤਾ, ap);
	va_end(ap);

	ਵਾਪਸ rc;
}

#ਜੇ_ਅੰਤ /* HAVE_CPU_SET_T */


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_ਰਸਤਾ
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>

ਸਥਿਰ ਖਾਲੀ __attribute__((__noreturn__)) usage(ਖਾਲੀ)
{
	fprintf(stdout, " %s [options] <dir> <command>\n\n", ਪ੍ਰੋਗਰਾਮ_invocation_short_name);
	fputs(" -p, --prefix <dir>      redirect hardcoded paths to <dir>\n", stdout);

	fputs(" Commands:\n", stdout);
	fputs(" read-u64 <file>            read uint64_t from file\n", stdout);
	fputs(" read-s64 <file>            read  int64_t from file\n", stdout);
	fputs(" read-u32 <file>            read uint32_t from file\n", stdout);
	fputs(" read-s32 <file>            read  int32_t from file\n", stdout);
	fputs(" read-string <file>         read string  from file\n", stdout);
	fputs(" read-majmin <file>         read devno from file\n", stdout);
	fputs(" read-link <file>           read symlink\n", stdout);
	fputs(" write-string <file> <str>  write string from file\n", stdout);
	fputs(" write-u64 <file> <str>     write uint64_t from file\n", stdout);

	ਬਾਹਰ_ਨਿਕਲੋ(EXIT_SUCCESS);
}

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	ਪੂਰਨ c;
	ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ = ਨਿਰਾ, *dir, *file, *command;
	ਬਣਤਰ ਰਸਤਾ_cxt *pc = ਨਿਰਾ;

	ਸਥਿਰ ਅਚਲ ਬਣਤਰ option longopts[] = {
		{ "prefix",	1, ਨਿਰਾ, 'p' },
		{ "help",       0, ਨਿਰਾ, 'h' },
		{ ਨਿਰਾ, 0, ਨਿਰਾ, 0 },
	};

	ਜਦੋਂ((c = getopt_long(argc, argv, "p:h", longopts, ਨਿਰਾ)) != -1) {
		ਚੋਣ(c) {
		ਹਾਲਤ 'p':
			ਅਗੇਤਰ = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ 'h':
			usage();
			ਰੋਕੋ;
		ਮੂਲ:
			err(EXIT_FAILURE, "try --help");
		}
	}

	ਜੇ (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_ਰਸਤਾ_init_debug();

	pc = ul_new_ਰਸਤਾ("%s", dir);
	ਜੇ (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	ਜੇ (ਅਗੇਤਰ)
		ul_ਰਸਤਾ_ਸੈੱਟ_ਅਗੇਤਰ(pc, ਅਗੇਤਰ);

	ਜੇ (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	ਜੇ (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		ਜੇ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ਜੇ (ul_ਰਸਤਾ_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		ਲਿਖੋ("read:  %s: %u\n", file, res);

		ਜੇ (ul_ਰਸਤਾ_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		ਲਿਖੋ("readf: %s: %u\n", file, res);

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(command, "read-s32") == 0) {
		int32_t res;

		ਜੇ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ਜੇ (ul_ਰਸਤਾ_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		ਲਿਖੋ("read:  %s: %d\n", file, res);

		ਜੇ (ul_ਰਸਤਾ_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		ਲਿਖੋ("readf: %s: %d\n", file, res);

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		ਜੇ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ਜੇ (ul_ਰਸਤਾ_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		ਲਿਖੋ("read:  %s: %" PRIu64 "\n", file, res);

		ਜੇ (ul_ਰਸਤਾ_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		ਲਿਖੋ("readf: %s: %" PRIu64 "\n", file, res);

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(command, "read-s64") == 0) {
		int64_t res;

		ਜੇ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ਜੇ (ul_ਰਸਤਾ_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		ਲਿਖੋ("read:  %s: %" PRId64 "\n", file, res);

		ਜੇ (ul_ਰਸਤਾ_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		ਲਿਖੋ("readf: %s: %" PRId64 "\n", file, res);

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		ਜੇ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ਜੇ (ul_ਰਸਤਾ_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		ਲਿਖੋ("read:  %s: %d\n", file, (ਪੂਰਨ) res);

		ਜੇ (ul_ਰਸਤਾ_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		ਲਿਖੋ("readf: %s: %d\n", file, (ਪੂਰਨ) res);

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(command, "read-string") == 0) {
		ਅੱਖਰ *res;

		ਜੇ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ਜੇ (ul_ਰਸਤਾ_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		ਲਿਖੋ("read:  %s: %s\n", file, res);

		ਜੇ (ul_ਰਸਤਾ_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		ਲਿਖੋ("readf: %s: %s\n", file, res);

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(command, "read-link") == 0) {
		ਅੱਖਰ res[PATH_MAX];

		ਜੇ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ਜੇ (ul_ਰਸਤਾ_readlink(pc, res, ਆਕਾਰ(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		ਲਿਖੋ("read:  %s: %s\n", file, res);

		ਜੇ (ul_ਰਸਤਾ_readlinkf(pc, res, ਆਕਾਰ(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		ਲਿਖੋ("readf: %s: %s\n", file, res);

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(command, "write-string") == 0) {
		ਅੱਖਰ *str;

		ਜੇ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		ਜੇ (ul_ਰਸਤਾ_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		ਜੇ (ul_ਰਸਤਾ_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} ਨਹੀਂਤਾਂ ਜੇ (strcmp(command, "write-u64") == 0) {
		uint64_t ਅੰਕ;

		ਜੇ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		ਅੰਕ = strtoumax(argv[optind++], ਨਿਰਾ, 0);

		ਜੇ (ul_ਰਸਤਾ_write_u64(pc, ਅੰਕ, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		ਜੇ (ul_ਰਸਤਾ_writef_u64(pc, ਅੰਕ, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_ਰਸਤਾ(pc);
	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_PATH */
