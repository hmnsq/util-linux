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
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <inttypes.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "fileutils.h"
#ಅನ್ತರ್ಭಾವ "all-io.h"
#ಅನ್ತರ್ಭಾವ "vfs.h"
#ಅನ್ತರ್ಭಾವ "path.h"
#ಅನ್ತರ್ಭಾವ "debug.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
ಸ್ಥಿರ UL_DEBUG_DEFINE_MASK(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPATH_DEBUG_INIT	(1 << 1)
#define ULPATH_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

ಶೂನ್ಯ ul_path_init_debug(ಶೂನ್ಯ)
{
	ಯದಿ (ulpath_debug_mask)
		ಪ್ರತ್ಯಾಗಮ;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

ಸಂರಚನಾ path_cxt *ul_new_path(ಧ್ರುವ ವರ್ಣ *dir, ...)
{
	ಸಂರಚನಾ path_cxt *pc = calloc(1, ಪರಿಮಾಣ(*pc));

	ಯದಿ (!pc)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	ಯದಿ (dir) {
		ಪೂರ್ಣಾಙ್ಕ rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		ಯದಿ (rc < 0 || !pc->dir_path)
			ಗಚ್ಛ fail;
	}
	ಪ್ರತ್ಯಾಗಮ pc;
fail:
	ul_unref_path(pc);
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಶೂನ್ಯ ul_ref_path(ಸಂರಚನಾ path_cxt *pc)
{
	ಯದಿ (pc)
		pc->refcount++;
}

ಶೂನ್ಯ ul_unref_path(ಸಂರಚನಾ path_cxt *pc)
{
	ಯದಿ (!pc)
		ಪ್ರತ್ಯಾಗಮ;

	pc->refcount--;

	ಯದಿ (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		ಯದಿ (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		ಮುಕ್ತ(pc->dir_path);
		ಮುಕ್ತ(pc->prefix);
		ಮುಕ್ತ(pc);
	}
}

ಶೂನ್ಯ ul_path_refer_vfs(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ಸಂರಚನಾ ul_vfs_ops *vfs)
{
	ಯದಿ (pc)
		pc->vfs = vfs;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_set_prefix(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *prefix)
{
	ವರ್ಣ *p = ರಿಕ್ತ;

	ಪ್ರತಿಜ್ಞಾ(pc->dir_fd < 0);

	ಯದಿ (prefix) {
		p = strdup(prefix);
		ಯದಿ (!p)
			ಪ್ರತ್ಯಾಗಮ -ENOMEM;
	}

	ಮುಕ್ತ(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಧ್ರುವ ವರ್ಣ *ul_path_get_prefix(ಸಂರಚನಾ path_cxt *pc)
{
	ಪ್ರತ್ಯಾಗಮ pc ? pc->prefix : ರಿಕ್ತ;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_set_dir(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *dir)
{
	ವರ್ಣ *p = ರಿಕ್ತ;

	ಯದಿ (dir) {
		p = strdup(dir);
		ಯದಿ (!p)
			ಪ್ರತ್ಯಾಗಮ -ENOMEM;
	}

	ಯದಿ (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	ಮುಕ್ತ(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಧ್ರುವ ವರ್ಣ *ul_path_get_dir(ಸಂರಚನಾ path_cxt *pc)
{
	ಪ್ರತ್ಯಾಗಮ pc ? pc->dir_path : ರಿಕ್ತ;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_set_dialect(ಸಂರಚನಾ path_cxt *pc, ಶೂನ್ಯ *data, ಶೂನ್ಯ free_data(ಸಂರಚನಾ path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ *ul_path_get_dialect(ಸಂರಚನಾ path_cxt *pc)
{
	ಪ್ರತ್ಯಾಗಮ pc ? pc->dialect : ರಿಕ್ತ;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_set_enoent_redirect(ಸಂರಚನಾ path_cxt *pc, ಪೂರ್ಣಾಙ್ಕ (*func)(ಸಂರಚನಾ path_cxt *, ಧ್ರುವ ವರ್ಣ *, ಪೂರ್ಣಾಙ್ಕ *))
{
	pc->redirect_on_enoent = func;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *get_absdir(ಸಂರಚನಾ path_cxt *pc)
{
	ಪೂರ್ಣಾಙ್ಕ rc;
	ಧ್ರುವ ವರ್ಣ *dirpath;

	ಯದಿ (!pc->prefix)
		ಪ್ರತ್ಯಾಗಮ pc->dir_path;

	dirpath = pc->dir_path;
	ಯದಿ (!dirpath)
		ಪ್ರತ್ಯಾಗಮ pc->prefix;
	ಯದಿ (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, ಪರಿಮಾಣ(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	ಯದಿ (rc < 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಯದಿ ((size_t)rc >= ಪರಿಮಾಣ(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ pc->path_buffer;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_is_accessible(ಸಂರಚನಾ path_cxt *pc)
{
	ಧ್ರುವ ವರ್ಣ *path;
	ಪ್ರತಿಜ್ಞಾ(pc);

	ಯದಿ (pc->dir_fd >= 0)
		ಪ್ರತ್ಯಾಗಮ 1;

	path = get_absdir(pc);
	ಯದಿ (!path)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಪ್ರತ್ಯಾಗಮ access(path, F_OK) == 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_get_dirfd(ಸಂರಚನಾ path_cxt *pc)
{
	ಪ್ರತಿಜ್ಞಾ(pc);
	ಪ್ರತಿಜ್ಞಾ(pc->dir_path);

	ಯದಿ (pc->dir_fd < 0) {
		ಧ್ರುವ ವರ್ಣ *path = get_absdir(pc);
		ಯದಿ (!path)
			ಪ್ರತ್ಯಾಗಮ -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	ಪ್ರತ್ಯಾಗಮ pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
ಶೂನ್ಯ ul_path_close_dirfd(ಸಂರಚನಾ path_cxt *pc)
{
	ಪ್ರತಿಜ್ಞಾ(pc);

	ಯದಿ (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

ಪೂರ್ಣಾಙ್ಕ ul_path_isopen_dirfd(ಸಂರಚನಾ path_cxt *pc)
{
	ಪ್ರತ್ಯಾಗಮ pc && pc->dir_fd >= 0;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ul_path_mkpath(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *path, va_list ap)
{
	ಪೂರ್ಣಾಙ್ಕ rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, ಪರಿಮಾಣ(pc->path_buffer), path, ap);
	ಯದಿ (rc < 0) {
		ಯದಿ (!errno)
			errno = EINVAL;
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಯದಿ ((size_t)rc >= ಪರಿಮಾಣ(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ಪ್ರತ್ಯಾಗಮ pc->path_buffer;
}

ವರ್ಣ *ul_path_get_abspath(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t bufsz, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	ಯದಿ (path) {
		ಪೂರ್ಣಾಙ್ಕ rc;
		va_list ap;
		ಧ್ರುವ ವರ್ಣ *tail = ರಿಕ್ತ, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		ಯದಿ (dirpath && *dirpath == '/')
			dirpath++;
		ಯದಿ (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		ಯದಿ ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
	} ಅನ್ಯಥಾ {
		ಧ್ರುವ ವರ್ಣ *tmp = get_absdir(pc);

		ಯದಿ (!tmp)
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		xstrncpy(buf, tmp, bufsz);
	}

	ಪ್ರತ್ಯಾಗಮ buf;
}


ಪೂರ್ಣಾಙ್ಕ ul_path_access(ಸಂರಚನಾ path_cxt *pc, ಪೂರ್ಣಾಙ್ಕ mode, ಧ್ರುವ ವರ್ಣ *path)
{
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (!path)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	ಯದಿ (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} ಅನ್ಯಥಾ {
		ಪೂರ್ಣಾಙ್ಕ dir = ul_path_get_dirfd(pc);
		ಯದಿ (dir < 0)
			ಪ್ರತ್ಯಾಗಮ dir;
		ಯದಿ (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		ಯದಿ (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_accessf(ಸಂರಚನಾ path_cxt *pc, ಪೂರ್ಣಾಙ್ಕ mode, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	va_list ap;
	ಧ್ರುವ ವರ್ಣ *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
ಪೂರ್ಣಾಙ್ಕ ul_path_stat(ಸಂರಚನಾ path_cxt *pc, ಸಂರಚನಾ stat *sb, ಪೂರ್ಣಾಙ್ಕ flags, ಧ್ರುವ ವರ್ಣ *path)
{
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} ಅನ್ಯಥಾ {
		ಪೂರ್ಣಾಙ್ಕ dir = ul_path_get_dirfd(pc);
		ಯದಿ (dir < 0)
			ಪ್ರತ್ಯಾಗಮ dir;
		ಯದಿ (path) {
			ಯದಿ  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} ಅನ್ಯಥಾ
			rc = fstat(dir, sb);	/* dir itself */

		ಯದಿ (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_vstatf(ಸಂರಚನಾ path_cxt *pc, ಸಂರಚನಾ stat *sb, ಪೂರ್ಣಾಙ್ಕ flags, ಧ್ರುವ ವರ್ಣ *path, va_list ap)
{
	ಧ್ರುವ ವರ್ಣ *p = ul_path_mkpath(pc, path, ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

ಪೂರ್ಣಾಙ್ಕ ul_path_statf(ಸಂರಚನಾ path_cxt *pc, ಸಂರಚನಾ stat *sb, ಪೂರ್ಣಾಙ್ಕ flags, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಙ್ಕ rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
ಪೂರ್ಣಾಙ್ಕ ul_path_statx(ಸಂರಚನಾ path_cxt *pc, ಸಂರಚನಾ statx *stx, ಪೂರ್ಣಾಙ್ಕ flags, unsigned ಪೂರ್ಣಾಙ್ಕ mask,
		  ಧ್ರುವ ವರ್ಣ *path)
{
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (!path)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	ಯದಿ (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	ಅನ್ಯಥಾ {
		ಪೂರ್ಣಾಙ್ಕ dir = ul_path_get_dirfd(pc);
		ಯದಿ (dir < 0)
			ಪ್ರತ್ಯಾಗಮ dir;
		ಯದಿ (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	ಪ್ರತ್ಯಾಗಮ rc;
}
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ ul_path_statx(ಸಂರಚನಾ path_cxt *pc __attribute__((__unused__)),
		  ಸಂರಚನಾ statx *stx __attribute__((__unused__)),
		  ಪೂರ್ಣಾಙ್ಕ flags __attribute__((__unused__)),
		  unsigned ಪೂರ್ಣಾಙ್ಕ mask __attribute__((__unused__)),
		  ಧ್ರುವ ವರ್ಣ *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	ಪ್ರತ್ಯಾಗಮ -1;
}
#endif /* HAVE_STATX */

ಪೂರ್ಣಾಙ್ಕ ul_path_vstatxf(ಸಂರಚನಾ path_cxt *pc, ಸಂರಚನಾ statx *stx, ಪೂರ್ಣಾಙ್ಕ flags, unsigned ಪೂರ್ಣಾಙ್ಕ mask,
		    ಧ್ರುವ ವರ್ಣ *path, va_list ap)
{
	ಧ್ರುವ ವರ್ಣ *p = ul_path_mkpath(pc, path, ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

ಪೂರ್ಣಾಙ್ಕ ul_path_statxf(ಸಂರಚನಾ path_cxt *pc, ಸಂರಚನಾ statx *stx, ಪೂರ್ಣಾಙ್ಕ flags, unsigned ಪೂರ್ಣಾಙ್ಕ mask,
		   ಧ್ರುವ ವರ್ಣ *path, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಙ್ಕ rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_open(ಸಂರಚನಾ path_cxt *pc, ಪೂರ್ಣಾಙ್ಕ flags, ಧ್ರುವ ವರ್ಣ *path)
{
	ಪೂರ್ಣಾಙ್ಕ fd;

	ಯದಿ (!path)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	ಯದಿ (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} ಅನ್ಯಥಾ {
		ಪೂರ್ಣಾಙ್ಕ fdx;
		ಪೂರ್ಣಾಙ್ಕ dir = ul_path_get_dirfd(pc);
		ಯದಿ (dir < 0)
			ಪ್ರತ್ಯಾಗಮ dir;

		ಯದಿ (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		ಯದಿ (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	ಪ್ರತ್ಯಾಗಮ fd;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_vopenf(ಸಂರಚನಾ path_cxt *pc, ಪೂರ್ಣಾಙ್ಕ flags, ಧ್ರುವ ವರ್ಣ *path, va_list ap)
{
	ಧ್ರುವ ವರ್ಣ *p = ul_path_mkpath(pc, path, ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_open(pc, flags, p);
}

ಪೂರ್ಣಾಙ್ಕ ul_path_openf(ಸಂರಚನಾ path_cxt *pc, ಪೂರ್ಣಾಙ್ಕ flags, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಙ್ಕ rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *mode, ಧ್ರುವ ವರ್ಣ *path)
{
	ಪೂರ್ಣಾಙ್ಕ flags = ul_mode_to_flags(mode);
	ಪೂರ್ಣಾಙ್ಕ fd = ul_path_open(pc, flags, path);

	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ ul_vfs_fdopen(pc ? pc->vfs : ರಿಕ್ತ, fd, mode);
}


FILE *ul_path_vfopenf(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *mode, ಧ್ರುವ ವರ್ಣ *path, va_list ap)
{
	ಧ್ರುವ ವರ್ಣ *p = ul_path_mkpath(pc, path, ap);

	ಪ್ರತ್ಯಾಗಮ !p ? ರಿಕ್ತ : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *mode, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *path)
{
	DIR *dir;
	ಪೂರ್ಣಾಙ್ಕ fd = -1;

	ಯದಿ (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	ಅನ್ಯಥಾ ಯದಿ (pc->dir_path) {
		ಪೂರ್ಣಾಙ್ಕ dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		ಯದಿ (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	dir = fdopendir(fd);
	ಯದಿ (!dir) {
		close(fd);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	ಯದಿ (!path)
		 rewinddir(dir);
	ಪ್ರತ್ಯಾಗಮ dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *path, va_list ap)
{
	ಧ್ರುವ ವರ್ಣ *p = ul_path_mkpath(pc, path, ap);

	ಪ್ರತ್ಯಾಗಮ !p ? ರಿಕ್ತ : ul_path_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendirf(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, path);
	dir = ul_path_vopendirf(pc, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t bufsiz, ಧ್ರುವ ವರ್ಣ *path)
{
	ಪೂರ್ಣಾಙ್ಕ dirfd;
	ssize_t ssz;

	ಯದಿ (!path) {
		ಧ್ರುವ ವರ್ಣ *p = get_absdir(pc);
		ಯದಿ (!p)
			ಪ್ರತ್ಯಾಗಮ -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} ಅನ್ಯಥಾ {
		dirfd = ul_path_get_dirfd(pc);
		ಯದಿ (dirfd < 0)
			ಪ್ರತ್ಯಾಗಮ dirfd;

		ಯದಿ (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	ಯದಿ (ssz >= 0)
		buf[ssz] = '\0';
	ಪ್ರತ್ಯಾಗಮ ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlinkf(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t bufsiz, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	ಧ್ರುವ ವರ್ಣ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

ಪೂರ್ಣಾಙ್ಕ ul_path_read(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t len, ಧ್ರುವ ವರ್ಣ *path)
{
	ಪೂರ್ಣಾಙ್ಕ rc, errsv;
	ಪೂರ್ಣಾಙ್ಕ fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : ರಿಕ್ತ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ರಿಕ್ತ, fd);
	errno = errsv;
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_vreadf(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t len, ಧ್ರುವ ವರ್ಣ *path, va_list ap)
{
	ಧ್ರುವ ವರ್ಣ *p = ul_path_mkpath(pc, path, ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_read(pc, buf, len, p);
}

ಪೂರ್ಣಾಙ್ಕ ul_path_readf(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t len, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಙ್ಕ rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
ಪೂರ್ಣಾಙ್ಕ ul_path_read_string(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ **str, ಧ್ರುವ ವರ್ಣ *path)
{
	ವರ್ಣ buf[BUFSIZ];
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (!str)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	*str = ರಿಕ್ತ;

	rc = ul_path_read_buffer(pc, buf, ಪರಿಮಾಣ(buf), path);
	ಯದಿ (rc < 0)
		ಪ್ರತ್ಯಾಗಮ rc;

	*str = strdup(buf);
	ಯದಿ (!*str)
		rc = -ENOMEM;

	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_readf_string(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ **str, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	ಧ್ರುವ ವರ್ಣ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_read_string(pc, str, p);
}

ಪೂರ್ಣಾಙ್ಕ ul_path_read_buffer(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t bufsz, ಧ್ರುವ ವರ್ಣ *path)
{
	ಪೂರ್ಣಾಙ್ಕ rc = ul_path_read(pc, buf, bufsz - 1, path);

	ಯದಿ (rc == 0)
		buf[0] = '\0';

	ಅನ್ಯಥಾ ಯದಿ (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		ಯದಿ (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		ಅನ್ಯಥಾ
			buf[rc] = '\0';
	}

	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_vreadf_buffer(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t bufsz, ಧ್ರುವ ವರ್ಣ *path, va_list ap)
{
	ಧ್ರುವ ವರ್ಣ *p;

	p = ul_path_mkpath(pc, path, ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

ಪೂರ್ಣಾಙ್ಕ ul_path_readf_buffer(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t bufsz, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಙ್ಕ rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_scanf(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *path, ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	ಪೂರ್ಣಾಙ್ಕ rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	ಯದಿ (!f)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_scanff(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *path, va_list ap, ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	ಪೂರ್ಣಾಙ್ಕ rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	ಯದಿ (!f)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ಪ್ರತ್ಯಾಗಮ rc;
}


ಪೂರ್ಣಾಙ್ಕ ul_path_read_s64(ಸಂರಚನಾ path_cxt *pc, int64_t *res, ಧ್ರುವ ವರ್ಣ *path)
{
	int64_t x = 0;
	ಪೂರ್ಣಾಙ್ಕ rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	ಯದಿ (rc != 1)
		ಪ್ರತ್ಯಾಗಮ -1;
	ಯದಿ (res)
		*res = x;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_readf_s64(ಸಂರಚನಾ path_cxt *pc, int64_t *res, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	ಧ್ರುವ ವರ್ಣ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_read_s64(pc, res, p);
}

ಪೂರ್ಣಾಙ್ಕ ul_path_read_u64(ಸಂರಚನಾ path_cxt *pc, uint64_t *res, ಧ್ರುವ ವರ್ಣ *path)
{
	uint64_t x = 0;
	ಪೂರ್ಣಾಙ್ಕ rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	ಯದಿ (rc != 1)
		ಪ್ರತ್ಯಾಗಮ -1;
	ಯದಿ (res)
		*res = x;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_readf_u64(ಸಂರಚನಾ path_cxt *pc, uint64_t *res, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	ಧ್ರುವ ವರ್ಣ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_read_u64(pc, res, p);
}

ಪೂರ್ಣಾಙ್ಕ ul_path_read_s32(ಸಂರಚನಾ path_cxt *pc, ಪೂರ್ಣಾಙ್ಕ *res, ಧ್ರುವ ವರ್ಣ *path)
{
	ಪೂರ್ಣಾಙ್ಕ rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	ಯದಿ (rc != 1)
		ಪ್ರತ್ಯಾಗಮ -1;
	ಯದಿ (res)
		*res = x;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_readf_s32(ಸಂರಚನಾ path_cxt *pc, ಪೂರ್ಣಾಙ್ಕ *res, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	ಧ್ರುವ ವರ್ಣ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_read_s32(pc, res, p);
}

ಪೂರ್ಣಾಙ್ಕ ul_path_read_u32(ಸಂರಚನಾ path_cxt *pc, unsigned ಪೂರ್ಣಾಙ್ಕ *res, ಧ್ರುವ ವರ್ಣ *path)
{
	ಪೂರ್ಣಾಙ್ಕ rc;
	unsigned ಪೂರ್ಣಾಙ್ಕ x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	ಯದಿ (rc != 1)
		ಪ್ರತ್ಯಾಗಮ -1;
	ಯದಿ (res)
		*res = x;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_readf_u32(ಸಂರಚನಾ path_cxt *pc, unsigned ಪೂರ್ಣಾಙ್ಕ *res, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	ಧ್ರುವ ವರ್ಣ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_read_u32(pc, res, p);
}

ಪೂರ್ಣಾಙ್ಕ ul_path_read_majmin(ಸಂರಚನಾ path_cxt *pc, dev_t *res, ಧ್ರುವ ವರ್ಣ *path)
{
	ಪೂರ್ಣಾಙ್ಕ rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	ಯದಿ (rc != 2)
		ಪ್ರತ್ಯಾಗಮ -1;
	ಯದಿ (res)
		*res = makedev(maj, min);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_readf_majmin(ಸಂರಚನಾ path_cxt *pc, dev_t *res, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	ಧ್ರುವ ವರ್ಣ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_read_majmin(pc, res, p);
}

ಪೂರ್ಣಾಙ್ಕ ul_path_write_string(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *str, ಧ್ರುವ ವರ್ಣ *path)
{
	ಪೂರ್ಣಾಙ್ಕ rc, errsv;
	ಪೂರ್ಣಾಙ್ಕ fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : ರಿಕ್ತ, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ರಿಕ್ತ, fd);
	errno = errsv;
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_writef_string(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *str, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	ಧ್ರುವ ವರ್ಣ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_write_string(pc, str, p);
}

ಪೂರ್ಣಾಙ್ಕ ul_path_write_s64(ಸಂರಚನಾ path_cxt *pc, int64_t ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *path)
{
	ವರ್ಣ buf[ಪರಿಮಾಣ(stringify_ಮೌಲ್ಯ(LLONG_MAX))];
	ಪೂರ್ಣಾಙ್ಕ rc, errsv;
	ಪೂರ್ಣಾಙ್ಕ fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ -errno;

	len = snprintf(buf, ಪರಿಮಾಣ(buf), "%" PRId64, ಅಂಕೆ);
	ಯದಿ (len < 0 || (size_t) len >= ಪರಿಮಾಣ(buf))
		rc = len < 0 ? -errno : -E2BIG;
	ಅನ್ಯಥಾ
		rc = ul_vfs_write_all(pc ? pc->vfs : ರಿಕ್ತ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ರಿಕ್ತ, fd);
	errno = errsv;
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_write_u64(ಸಂರಚನಾ path_cxt *pc, uint64_t ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *path)
{
	ವರ್ಣ buf[ಪರಿಮಾಣ(stringify_ಮೌಲ್ಯ(ULLONG_MAX))];
	ಪೂರ್ಣಾಙ್ಕ rc, errsv;
	ಪೂರ್ಣಾಙ್ಕ fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ -errno;

	len = snprintf(buf, ಪರಿಮಾಣ(buf), "%" PRIu64, ಅಂಕೆ);
	ಯದಿ (len < 0 || (size_t) len >= ಪರಿಮಾಣ(buf))
		rc = len < 0 ? -errno : -E2BIG;
	ಅನ್ಯಥಾ
		rc = ul_vfs_write_all(pc ? pc->vfs : ರಿಕ್ತ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ರಿಕ್ತ, fd);
	errno = errsv;
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_writef_u64(ಸಂರಚನಾ path_cxt *pc, uint64_t ಅಂಕೆ, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	ಧ್ರುವ ವರ್ಣ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_write_u64(pc, ಅಂಕೆ, p);

}

ಪೂರ್ಣಾಙ್ಕ ul_path_ಎಣಿಕೆ_dirents(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *path)
{
	DIR *dir;
	ಪೂರ್ಣಾಙ್ಕ r = 0;

	dir = ul_path_opendir(pc, path);
	ಯದಿ (!dir)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯಾವತ್ (xreaddir(dir)) r++;

	closedir(dir);
	ಪ್ರತ್ಯಾಗಮ r;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_countf_dirents(ಸಂರಚನಾ path_cxt *pc, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	ಧ್ರುವ ವರ್ಣ *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ !p ? -errno : ul_path_ಎಣಿಕೆ_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
ಪೂರ್ಣಾಙ್ಕ ul_path_next_dirent(ಸಂರಚನಾ path_cxt *pc, DIR **sub, ಧ್ರುವ ವರ್ಣ *dirname, ಸಂರಚನಾ dirent **d)
{
	ಯದಿ (!pc || !sub || !d)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ಯದಿ (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		ಯದಿ (!*sub)
			ಪ್ರತ್ಯಾಗಮ -errno;
	}

	*d = xreaddir(*sub);
	ಯದಿ (*d)
		ಪ್ರತ್ಯಾಗಮ 0;

	closedir(*sub);
	*sub = ರಿಕ್ತ;
	ಪ್ರತ್ಯಾಗಮ 1;
}

#ifdef HAVE_CPU_SET_T
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ul_path_cpuparse(ಸಂರಚನಾ path_cxt *pc, cpu_set_t **set, ಪೂರ್ಣಾಙ್ಕ maxcpus, ಪೂರ್ಣಾಙ್ಕ islist, ಧ್ರುವ ವರ್ಣ *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	ವರ್ಣ *buf;
	ಪೂರ್ಣಾಙ್ಕ rc;

	*set = ರಿಕ್ತ;

	buf = malloc(len);
	ಯದಿ (!buf)
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	ಯದಿ (rc < 0)
		ಗಚ್ಛ out;

	*set = cpuset_alloc(maxcpus, &setsize, ರಿಕ್ತ);
	ಯದಿ (!*set) {
		rc = -EINVAL;
		ಗಚ್ಛ out;
	}

	ಯದಿ (islist) {
		ಯದಿ (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			ಗಚ್ಛ out;
		}
	} ಅನ್ಯಥಾ {
		ಯದಿ (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			ಗಚ್ಛ out;
		}
	}
	rc = 0;

out:
	ಯದಿ (rc) {
		cpuset_free(*set);
		*set = ರಿಕ್ತ;
	}
	ಮುಕ್ತ(buf);
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_readf_cpuset(ಸಂರಚನಾ path_cxt *pc, cpu_set_t **set, ಪೂರ್ಣಾಙ್ಕ maxcpus, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ ul_path_readf_cpulist(ಸಂರಚನಾ path_cxt *pc, cpu_set_t **set, ಪೂರ್ಣಾಙ್ಕ maxcpus, ಧ್ರುವ ವರ್ಣ *path, ...)
{
	va_list ap;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	ಪ್ರತ್ಯಾಗಮ rc;
}

#endif /* HAVE_CPU_SET_T */


#ifdef TEST_PROGRAM_PATH
#ಅನ್ತರ್ಭಾವ <getopt.h>

ಸ್ಥಿರ ಶೂನ್ಯ __attribute__((__noreturn__)) usage(ಶೂನ್ಯ)
{
	fprintf(stdout, " %s [options] <dir> <command>\n\n", program_invocation_short_name);
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

	exit(EXIT_SUCCESS);
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಪೂರ್ಣಾಙ್ಕ c;
	ಧ್ರುವ ವರ್ಣ *prefix = ರಿಕ್ತ, *dir, *file, *command;
	ಸಂರಚನಾ path_cxt *pc = ರಿಕ್ತ;

	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
		{ "prefix",	1, ರಿಕ್ತ, 'p' },
		{ "help",       0, ರಿಕ್ತ, 'h' },
		{ ರಿಕ್ತ, 0, ರಿಕ್ತ, 0 },
	};

	ಯಾವತ್((c = getopt_long(argc, argv, "p:h", longopts, ರಿಕ್ತ)) != -1) {
		ವಿಕಲ್ಪ(c) {
		ಪಕ್ಷ 'p':
			prefix = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'h':
			usage();
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			err(EXIT_FAILURE, "try --help");
		}
	}

	ಯದಿ (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	ಯದಿ (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	ಯದಿ (prefix)
		ul_path_set_prefix(pc, prefix);

	ಯದಿ (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	ಯದಿ (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		ಯದಿ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ಯದಿ (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		ಯದಿ (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} ಅನ್ಯಥಾ ಯದಿ (strcmp(command, "read-s32") == 0) {
		int32_t res;

		ಯದಿ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ಯದಿ (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		ಯದಿ (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} ಅನ್ಯಥಾ ಯದಿ (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		ಯದಿ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ಯದಿ (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		ಯದಿ (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} ಅನ್ಯಥಾ ಯದಿ (strcmp(command, "read-s64") == 0) {
		int64_t res;

		ಯದಿ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ಯದಿ (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		ಯದಿ (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} ಅನ್ಯಥಾ ಯದಿ (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		ಯದಿ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ಯದಿ (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (ಪೂರ್ಣಾಙ್ಕ) res);

		ಯದಿ (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (ಪೂರ್ಣಾಙ್ಕ) res);

	} ಅನ್ಯಥಾ ಯದಿ (strcmp(command, "read-string") == 0) {
		ವರ್ಣ *res;

		ಯದಿ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ಯದಿ (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		ಯದಿ (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} ಅನ್ಯಥಾ ಯದಿ (strcmp(command, "read-link") == 0) {
		ವರ್ಣ res[PATH_MAX];

		ಯದಿ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ಯದಿ (ul_path_readlink(pc, res, ಪರಿಮಾಣ(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		ಯದಿ (ul_path_readlinkf(pc, res, ಪರಿಮಾಣ(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} ಅನ್ಯಥಾ ಯದಿ (strcmp(command, "write-string") == 0) {
		ವರ್ಣ *str;

		ಯದಿ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		ಯದಿ (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		ಯದಿ (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} ಅನ್ಯಥಾ ಯದಿ (strcmp(command, "write-u64") == 0) {
		uint64_t ಅಂಕೆ;

		ಯದಿ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		ಅಂಕೆ = strtoumax(argv[optind++], ರಿಕ್ತ, 0);

		ಯದಿ (ul_path_write_u64(pc, ಅಂಕೆ, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		ಯದಿ (ul_path_writef_u64(pc, ಅಂಕೆ, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
