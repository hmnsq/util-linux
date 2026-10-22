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
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <inttypes.h>
#অন্তর্ভাব <errno.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "fileutils.h"
#অন্তর্ভাব "all-io.h"
#অন্তর্ভাব "vfs.h"
#অন্তর্ভাব "path.h"
#অন্তর্ভাব "debug.h"
#অন্তর্ভাব "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
স্থির UL_DEBUG_DEFINE_MASK(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPATH_DEBUG_INIT	(1 << 1)
#define ULPATH_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

শূন্য ul_path_init_debug(শূন্য)
{
	যদি (ulpath_debug_mask)
		ফেরত;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

কাঠামো path_cxt *ul_new_path(ধ্রুবক অক্ষর *dir, ...)
{
	কাঠামো path_cxt *pc = calloc(1, আকার(*pc));

	যদি (!pc)
		ফেরত ফাঁকা;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	যদি (dir) {
		পূর্ণসংখ্যা rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		যদি (rc < 0 || !pc->dir_path)
			যাও fail;
	}
	ফেরত pc;
fail:
	ul_unref_path(pc);
	ফেরত ফাঁকা;
}

শূন্য ul_ref_path(কাঠামো path_cxt *pc)
{
	যদি (pc)
		pc->refcount++;
}

শূন্য ul_unref_path(কাঠামো path_cxt *pc)
{
	যদি (!pc)
		ফেরত;

	pc->refcount--;

	যদি (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		যদি (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		মুক্ত(pc->dir_path);
		মুক্ত(pc->prefix);
		মুক্ত(pc);
	}
}

শূন্য ul_path_refer_vfs(কাঠামো path_cxt *pc, ধ্রুবক কাঠামো ul_vfs_ops *vfs)
{
	যদি (pc)
		pc->vfs = vfs;
}

পূর্ণসংখ্যা ul_path_set_prefix(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *prefix)
{
	অক্ষর *p = ফাঁকা;

	প্রতিজ্ঞা(pc->dir_fd < 0);

	যদি (prefix) {
		p = strdup(prefix);
		যদি (!p)
			ফেরত -ENOMEM;
	}

	মুক্ত(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	ফেরত 0;
}

ধ্রুবক অক্ষর *ul_path_get_prefix(কাঠামো path_cxt *pc)
{
	ফেরত pc ? pc->prefix : ফাঁকা;
}

পূর্ণসংখ্যা ul_path_set_dir(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *dir)
{
	অক্ষর *p = ফাঁকা;

	যদি (dir) {
		p = strdup(dir);
		যদি (!p)
			ফেরত -ENOMEM;
	}

	যদি (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	মুক্ত(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	ফেরত 0;
}

ধ্রুবক অক্ষর *ul_path_get_dir(কাঠামো path_cxt *pc)
{
	ফেরত pc ? pc->dir_path : ফাঁকা;
}

পূর্ণসংখ্যা ul_path_set_dialect(কাঠামো path_cxt *pc, শূন্য *তথ্য, শূন্য free_তথ্য(কাঠামো path_cxt *))
{
	pc->dialect = তথ্য;
	pc->free_dialect = free_তথ্য;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	ফেরত 0;
}

শূন্য *ul_path_get_dialect(কাঠামো path_cxt *pc)
{
	ফেরত pc ? pc->dialect : ফাঁকা;
}

পূর্ণসংখ্যা ul_path_set_enoent_redirect(কাঠামো path_cxt *pc, পূর্ণসংখ্যা (*func)(কাঠামো path_cxt *, ধ্রুবক অক্ষর *, পূর্ণসংখ্যা *))
{
	pc->redirect_on_enoent = func;
	ফেরত 0;
}

স্থির ধ্রুবক অক্ষর *get_absdir(কাঠামো path_cxt *pc)
{
	পূর্ণসংখ্যা rc;
	ধ্রুবক অক্ষর *dirpath;

	যদি (!pc->prefix)
		ফেরত pc->dir_path;

	dirpath = pc->dir_path;
	যদি (!dirpath)
		ফেরত pc->prefix;
	যদি (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_বাফার, আকার(pc->path_বাফার), "%s/%s", pc->prefix, dirpath);
	যদি (rc < 0)
		ফেরত ফাঁকা;
	যদি ((size_t)rc >= আকার(pc->path_বাফার)) {
		errno = ENAMETOOLONG;
		ফেরত ফাঁকা;
	}

	ফেরত pc->path_বাফার;
}

পূর্ণসংখ্যা ul_path_is_accessible(কাঠামো path_cxt *pc)
{
	ধ্রুবক অক্ষর *path;
	প্রতিজ্ঞা(pc);

	যদি (pc->dir_fd >= 0)
		ফেরত 1;

	path = get_absdir(pc);
	যদি (!path)
		ফেরত 0;
	ফেরত access(path, F_OK) == 0;
}

পূর্ণসংখ্যা ul_path_get_dirfd(কাঠামো path_cxt *pc)
{
	প্রতিজ্ঞা(pc);
	প্রতিজ্ঞা(pc->dir_path);

	যদি (pc->dir_fd < 0) {
		ধ্রুবক অক্ষর *path = get_absdir(pc);
		যদি (!path)
			ফেরত -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	ফেরত pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
শূন্য ul_path_close_dirfd(কাঠামো path_cxt *pc)
{
	প্রতিজ্ঞা(pc);

	যদি (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

পূর্ণসংখ্যা ul_path_isopen_dirfd(কাঠামো path_cxt *pc)
{
	ফেরত pc && pc->dir_fd >= 0;
}

স্থির ধ্রুবক অক্ষর *ul_path_mkpath(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *path, va_list ap)
{
	পূর্ণসংখ্যা rc;

	errno = 0;

	rc = vsnprintf(pc->path_বাফার, আকার(pc->path_বাফার), path, ap);
	যদি (rc < 0) {
		যদি (!errno)
			errno = EINVAL;
		ফেরত ফাঁকা;
	}

	যদি ((size_t)rc >= আকার(pc->path_বাফার)) {
		errno = ENAMETOOLONG;
		ফেরত ফাঁকা;
	}

	ফেরত pc->path_বাফার;
}

অক্ষর *ul_path_get_abspath(কাঠামো path_cxt *pc, অক্ষর *buf, size_t bufsz, ধ্রুবক অক্ষর *path, ...)
{
	যদি (path) {
		পূর্ণসংখ্যা rc;
		va_list ap;
		ধ্রুবক অক্ষর *লেজ = ফাঁকা, *dirpath = pc->dir_path;

		va_start(ap, path);
		লেজ = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		যদি (dirpath && *dirpath == '/')
			dirpath++;
		যদি (লেজ && *লেজ == '/')
			লেজ++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				লেজ ? লেজ : "");

		যদি ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			ফেরত ফাঁকা;
		}
	} অন্যথা {
		ধ্রুবক অক্ষর *tmp = get_absdir(pc);

		যদি (!tmp)
			ফেরত ফাঁকা;
		xstrncpy(buf, tmp, bufsz);
	}

	ফেরত buf;
}


পূর্ণসংখ্যা ul_path_access(কাঠামো path_cxt *pc, পূর্ণসংখ্যা mode, ধ্রুবক অক্ষর *path)
{
	পূর্ণসংখ্যা rc;

	যদি (!path)
		ফেরত -EINVAL;
	যদি (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} অন্যথা {
		পূর্ণসংখ্যা dir = ul_path_get_dirfd(pc);
		যদি (dir < 0)
			ফেরত dir;
		যদি (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		যদি (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	ফেরত rc;
}

পূর্ণসংখ্যা ul_path_accessf(কাঠামো path_cxt *pc, পূর্ণসংখ্যা mode, ধ্রুবক অক্ষর *path, ...)
{
	va_list ap;
	ধ্রুবক অক্ষর *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ফেরত !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
পূর্ণসংখ্যা ul_path_stat(কাঠামো path_cxt *pc, কাঠামো stat *sb, পূর্ণসংখ্যা flags, ধ্রুবক অক্ষর *path)
{
	পূর্ণসংখ্যা rc;

	যদি (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} অন্যথা {
		পূর্ণসংখ্যা dir = ul_path_get_dirfd(pc);
		যদি (dir < 0)
			ফেরত dir;
		যদি (path) {
			যদি  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} অন্যথা
			rc = fstat(dir, sb);	/* dir itself */

		যদি (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	ফেরত rc;
}

পূর্ণসংখ্যা ul_path_vstatf(কাঠামো path_cxt *pc, কাঠামো stat *sb, পূর্ণসংখ্যা flags, ধ্রুবক অক্ষর *path, va_list ap)
{
	ধ্রুবক অক্ষর *p = ul_path_mkpath(pc, path, ap);

	ফেরত !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

পূর্ণসংখ্যা ul_path_statf(কাঠামো path_cxt *pc, কাঠামো stat *sb, পূর্ণসংখ্যা flags, ধ্রুবক অক্ষর *path, ...)
{
	va_list ap;
	পূর্ণসংখ্যা rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	ফেরত rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
পূর্ণসংখ্যা ul_path_statx(কাঠামো path_cxt *pc, কাঠামো statx *stx, পূর্ণসংখ্যা flags, unsigned পূর্ণসংখ্যা mask,
		  ধ্রুবক অক্ষর *path)
{
	পূর্ণসংখ্যা rc;

	যদি (!path)
		ফেরত -EINVAL;
	যদি (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	অন্যথা {
		পূর্ণসংখ্যা dir = ul_path_get_dirfd(pc);
		যদি (dir < 0)
			ফেরত dir;
		যদি (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	ফেরত rc;
}
#অন্যথা
পূর্ণসংখ্যা ul_path_statx(কাঠামো path_cxt *pc __attribute__((__unused__)),
		  কাঠামো statx *stx __attribute__((__unused__)),
		  পূর্ণসংখ্যা flags __attribute__((__unused__)),
		  unsigned পূর্ণসংখ্যা mask __attribute__((__unused__)),
		  ধ্রুবক অক্ষর *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	ফেরত -1;
}
#endif /* HAVE_STATX */

পূর্ণসংখ্যা ul_path_vstatxf(কাঠামো path_cxt *pc, কাঠামো statx *stx, পূর্ণসংখ্যা flags, unsigned পূর্ণসংখ্যা mask,
		    ধ্রুবক অক্ষর *path, va_list ap)
{
	ধ্রুবক অক্ষর *p = ul_path_mkpath(pc, path, ap);

	ফেরত !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

পূর্ণসংখ্যা ul_path_statxf(কাঠামো path_cxt *pc, কাঠামো statx *stx, পূর্ণসংখ্যা flags, unsigned পূর্ণসংখ্যা mask,
		   ধ্রুবক অক্ষর *path, ...)
{
	va_list ap;
	পূর্ণসংখ্যা rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	ফেরত rc;
}

পূর্ণসংখ্যা ul_path_open(কাঠামো path_cxt *pc, পূর্ণসংখ্যা flags, ধ্রুবক অক্ষর *path)
{
	পূর্ণসংখ্যা fd;

	যদি (!path)
		ফেরত -EINVAL;
	যদি (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} অন্যথা {
		পূর্ণসংখ্যা fdx;
		পূর্ণসংখ্যা dir = ul_path_get_dirfd(pc);
		যদি (dir < 0)
			ফেরত dir;

		যদি (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		যদি (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	ফেরত fd;
}

পূর্ণসংখ্যা ul_path_vopenf(কাঠামো path_cxt *pc, পূর্ণসংখ্যা flags, ধ্রুবক অক্ষর *path, va_list ap)
{
	ধ্রুবক অক্ষর *p = ul_path_mkpath(pc, path, ap);

	ফেরত !p ? -errno : ul_path_open(pc, flags, p);
}

পূর্ণসংখ্যা ul_path_openf(কাঠামো path_cxt *pc, পূর্ণসংখ্যা flags, ধ্রুবক অক্ষর *path, ...)
{
	va_list ap;
	পূর্ণসংখ্যা rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	ফেরত rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *mode, ধ্রুবক অক্ষর *path)
{
	পূর্ণসংখ্যা flags = ul_mode_to_flags(mode);
	পূর্ণসংখ্যা fd = ul_path_open(pc, flags, path);

	যদি (fd < 0)
		ফেরত ফাঁকা;

	ফেরত ul_vfs_fdopen(pc ? pc->vfs : ফাঁকা, fd, mode);
}


FILE *ul_path_vfopenf(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *mode, ধ্রুবক অক্ষর *path, va_list ap)
{
	ধ্রুবক অক্ষর *p = ul_path_mkpath(pc, path, ap);

	ফেরত !p ? ফাঁকা : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *mode, ধ্রুবক অক্ষর *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	ফেরত f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *path)
{
	DIR *dir;
	পূর্ণসংখ্যা fd = -1;

	যদি (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	অন্যথা যদি (pc->dir_path) {
		পূর্ণসংখ্যা dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		যদি (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	যদি (fd < 0)
		ফেরত ফাঁকা;

	dir = fdopendir(fd);
	যদি (!dir) {
		close(fd);
		ফেরত ফাঁকা;
	}
	যদি (!path)
		 rewinddir(dir);
	ফেরত dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *path, va_list ap)
{
	ধ্রুবক অক্ষর *p = ul_path_mkpath(pc, path, ap);

	ফেরত !p ? ফাঁকা : ul_path_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendirf(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *path, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, path);
	dir = ul_path_vopendirf(pc, path, ap);
	va_end(ap);

	ফেরত dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(কাঠামো path_cxt *pc, অক্ষর *buf, size_t bufsiz, ধ্রুবক অক্ষর *path)
{
	পূর্ণসংখ্যা dirfd;
	ssize_t ssz;

	যদি (!path) {
		ধ্রুবক অক্ষর *p = get_absdir(pc);
		যদি (!p)
			ফেরত -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} অন্যথা {
		dirfd = ul_path_get_dirfd(pc);
		যদি (dirfd < 0)
			ফেরত dirfd;

		যদি (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	যদি (ssz >= 0)
		buf[ssz] = '\0';
	ফেরত ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlinkf(কাঠামো path_cxt *pc, অক্ষর *buf, size_t bufsiz, ধ্রুবক অক্ষর *path, ...)
{
	ধ্রুবক অক্ষর *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ফেরত !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

পূর্ণসংখ্যা ul_path_read(কাঠামো path_cxt *pc, অক্ষর *buf, size_t len, ধ্রুবক অক্ষর *path)
{
	পূর্ণসংখ্যা rc, errsv;
	পূর্ণসংখ্যা fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	যদি (fd < 0)
		ফেরত -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : ফাঁকা, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ফাঁকা, fd);
	errno = errsv;
	ফেরত rc;
}

পূর্ণসংখ্যা ul_path_vreadf(কাঠামো path_cxt *pc, অক্ষর *buf, size_t len, ধ্রুবক অক্ষর *path, va_list ap)
{
	ধ্রুবক অক্ষর *p = ul_path_mkpath(pc, path, ap);

	ফেরত !p ? -errno : ul_path_read(pc, buf, len, p);
}

পূর্ণসংখ্যা ul_path_readf(কাঠামো path_cxt *pc, অক্ষর *buf, size_t len, ধ্রুবক অক্ষর *path, ...)
{
	va_list ap;
	পূর্ণসংখ্যা rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	ফেরত rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
পূর্ণসংখ্যা ul_path_read_string(কাঠামো path_cxt *pc, অক্ষর **str, ধ্রুবক অক্ষর *path)
{
	অক্ষর buf[BUFSIZ];
	পূর্ণসংখ্যা rc;

	যদি (!str)
		ফেরত -EINVAL;

	*str = ফাঁকা;

	rc = ul_path_read_বাফার(pc, buf, আকার(buf), path);
	যদি (rc < 0)
		ফেরত rc;

	*str = strdup(buf);
	যদি (!*str)
		rc = -ENOMEM;

	ফেরত rc;
}

পূর্ণসংখ্যা ul_path_readf_string(কাঠামো path_cxt *pc, অক্ষর **str, ধ্রুবক অক্ষর *path, ...)
{
	ধ্রুবক অক্ষর *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ফেরত !p ? -errno : ul_path_read_string(pc, str, p);
}

পূর্ণসংখ্যা ul_path_read_বাফার(কাঠামো path_cxt *pc, অক্ষর *buf, size_t bufsz, ধ্রুবক অক্ষর *path)
{
	পূর্ণসংখ্যা rc = ul_path_read(pc, buf, bufsz - 1, path);

	যদি (rc == 0)
		buf[0] = '\0';

	অন্যথা যদি (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		যদি (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		অন্যথা
			buf[rc] = '\0';
	}

	ফেরত rc;
}

পূর্ণসংখ্যা ul_path_vreadf_বাফার(কাঠামো path_cxt *pc, অক্ষর *buf, size_t bufsz, ধ্রুবক অক্ষর *path, va_list ap)
{
	ধ্রুবক অক্ষর *p;

	p = ul_path_mkpath(pc, path, ap);

	ফেরত !p ? -errno : ul_path_read_বাফার(pc, buf, bufsz, p);
}

পূর্ণসংখ্যা ul_path_readf_বাফার(কাঠামো path_cxt *pc, অক্ষর *buf, size_t bufsz, ধ্রুবক অক্ষর *path, ...)
{
	va_list ap;
	পূর্ণসংখ্যা rc;

	va_start(ap, path);
	rc = ul_path_vreadf_বাফার(pc, buf, bufsz, path, ap);
	va_end(ap);

	ফেরত rc;
}

পূর্ণসংখ্যা ul_path_scanf(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *path, ধ্রুবক অক্ষর *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	পূর্ণসংখ্যা rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	যদি (!f)
		ফেরত -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ফেরত rc;
}

পূর্ণসংখ্যা ul_path_scanff(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *path, va_list ap, ধ্রুবক অক্ষর *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	পূর্ণসংখ্যা rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	যদি (!f)
		ফেরত -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ফেরত rc;
}


পূর্ণসংখ্যা ul_path_read_s64(কাঠামো path_cxt *pc, int64_t *res, ধ্রুবক অক্ষর *path)
{
	int64_t x = 0;
	পূর্ণসংখ্যা rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	যদি (rc != 1)
		ফেরত -1;
	যদি (res)
		*res = x;
	ফেরত 0;
}

পূর্ণসংখ্যা ul_path_readf_s64(কাঠামো path_cxt *pc, int64_t *res, ধ্রুবক অক্ষর *path, ...)
{
	ধ্রুবক অক্ষর *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ফেরত !p ? -errno : ul_path_read_s64(pc, res, p);
}

পূর্ণসংখ্যা ul_path_read_u64(কাঠামো path_cxt *pc, uint64_t *res, ধ্রুবক অক্ষর *path)
{
	uint64_t x = 0;
	পূর্ণসংখ্যা rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	যদি (rc != 1)
		ফেরত -1;
	যদি (res)
		*res = x;
	ফেরত 0;
}

পূর্ণসংখ্যা ul_path_readf_u64(কাঠামো path_cxt *pc, uint64_t *res, ধ্রুবক অক্ষর *path, ...)
{
	ধ্রুবক অক্ষর *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ফেরত !p ? -errno : ul_path_read_u64(pc, res, p);
}

পূর্ণসংখ্যা ul_path_read_s32(কাঠামো path_cxt *pc, পূর্ণসংখ্যা *res, ধ্রুবক অক্ষর *path)
{
	পূর্ণসংখ্যা rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	যদি (rc != 1)
		ফেরত -1;
	যদি (res)
		*res = x;
	ফেরত 0;
}

পূর্ণসংখ্যা ul_path_readf_s32(কাঠামো path_cxt *pc, পূর্ণসংখ্যা *res, ধ্রুবক অক্ষর *path, ...)
{
	ধ্রুবক অক্ষর *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ফেরত !p ? -errno : ul_path_read_s32(pc, res, p);
}

পূর্ণসংখ্যা ul_path_read_u32(কাঠামো path_cxt *pc, unsigned পূর্ণসংখ্যা *res, ধ্রুবক অক্ষর *path)
{
	পূর্ণসংখ্যা rc;
	unsigned পূর্ণসংখ্যা x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	যদি (rc != 1)
		ফেরত -1;
	যদি (res)
		*res = x;
	ফেরত 0;
}

পূর্ণসংখ্যা ul_path_readf_u32(কাঠামো path_cxt *pc, unsigned পূর্ণসংখ্যা *res, ধ্রুবক অক্ষর *path, ...)
{
	ধ্রুবক অক্ষর *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ফেরত !p ? -errno : ul_path_read_u32(pc, res, p);
}

পূর্ণসংখ্যা ul_path_read_majmin(কাঠামো path_cxt *pc, dev_t *res, ধ্রুবক অক্ষর *path)
{
	পূর্ণসংখ্যা rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	যদি (rc != 2)
		ফেরত -1;
	যদি (res)
		*res = makedev(maj, min);
	ফেরত 0;
}

পূর্ণসংখ্যা ul_path_readf_majmin(কাঠামো path_cxt *pc, dev_t *res, ধ্রুবক অক্ষর *path, ...)
{
	ধ্রুবক অক্ষর *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ফেরত !p ? -errno : ul_path_read_majmin(pc, res, p);
}

পূর্ণসংখ্যা ul_path_write_string(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *str, ধ্রুবক অক্ষর *path)
{
	পূর্ণসংখ্যা rc, errsv;
	পূর্ণসংখ্যা fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	যদি (fd < 0)
		ফেরত -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : ফাঁকা, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ফাঁকা, fd);
	errno = errsv;
	ফেরত rc;
}

পূর্ণসংখ্যা ul_path_writef_string(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *str, ধ্রুবক অক্ষর *path, ...)
{
	ধ্রুবক অক্ষর *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ফেরত !p ? -errno : ul_path_write_string(pc, str, p);
}

পূর্ণসংখ্যা ul_path_write_s64(কাঠামো path_cxt *pc, int64_t অঙ্ক, ধ্রুবক অক্ষর *path)
{
	অক্ষর buf[আকার(stringify_মান(LLONG_MAX))];
	পূর্ণসংখ্যা rc, errsv;
	পূর্ণসংখ্যা fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	যদি (fd < 0)
		ফেরত -errno;

	len = snprintf(buf, আকার(buf), "%" PRId64, অঙ্ক);
	যদি (len < 0 || (size_t) len >= আকার(buf))
		rc = len < 0 ? -errno : -E2BIG;
	অন্যথা
		rc = ul_vfs_write_all(pc ? pc->vfs : ফাঁকা, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ফাঁকা, fd);
	errno = errsv;
	ফেরত rc;
}

পূর্ণসংখ্যা ul_path_write_u64(কাঠামো path_cxt *pc, uint64_t অঙ্ক, ধ্রুবক অক্ষর *path)
{
	অক্ষর buf[আকার(stringify_মান(ULLONG_MAX))];
	পূর্ণসংখ্যা rc, errsv;
	পূর্ণসংখ্যা fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	যদি (fd < 0)
		ফেরত -errno;

	len = snprintf(buf, আকার(buf), "%" PRIu64, অঙ্ক);
	যদি (len < 0 || (size_t) len >= আকার(buf))
		rc = len < 0 ? -errno : -E2BIG;
	অন্যথা
		rc = ul_vfs_write_all(pc ? pc->vfs : ফাঁকা, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ফাঁকা, fd);
	errno = errsv;
	ফেরত rc;
}

পূর্ণসংখ্যা ul_path_writef_u64(কাঠামো path_cxt *pc, uint64_t অঙ্ক, ধ্রুবক অক্ষর *path, ...)
{
	ধ্রুবক অক্ষর *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ফেরত !p ? -errno : ul_path_write_u64(pc, অঙ্ক, p);

}

পূর্ণসংখ্যা ul_path_গুনতি_dirents(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *path)
{
	DIR *dir;
	পূর্ণসংখ্যা r = 0;

	dir = ul_path_opendir(pc, path);
	যদি (!dir)
		ফেরত 0;

	যখন (xreaddir(dir)) r++;

	closedir(dir);
	ফেরত r;
}

পূর্ণসংখ্যা ul_path_countf_dirents(কাঠামো path_cxt *pc, ধ্রুবক অক্ষর *path, ...)
{
	ধ্রুবক অক্ষর *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ফেরত !p ? -errno : ul_path_গুনতি_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
পূর্ণসংখ্যা ul_path_পরবর্তী_dirent(কাঠামো path_cxt *pc, DIR **sub, ধ্রুবক অক্ষর *dirname, কাঠামো dirent **d)
{
	যদি (!pc || !sub || !d)
		ফেরত -EINVAL;

	যদি (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		যদি (!*sub)
			ফেরত -errno;
	}

	*d = xreaddir(*sub);
	যদি (*d)
		ফেরত 0;

	closedir(*sub);
	*sub = ফাঁকা;
	ফেরত 1;
}

#ifdef HAVE_CPU_SET_T
স্থির পূর্ণসংখ্যা ul_path_cpuparse(কাঠামো path_cxt *pc, cpu_set_t **set, পূর্ণসংখ্যা maxcpus, পূর্ণসংখ্যা islist, ধ্রুবক অক্ষর *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	অক্ষর *buf;
	পূর্ণসংখ্যা rc;

	*set = ফাঁকা;

	buf = malloc(len);
	যদি (!buf)
		ফেরত -ENOMEM;

	rc = ul_path_vreadf_বাফার(pc, buf, len, path, ap);
	যদি (rc < 0)
		যাও out;

	*set = cpuset_alloc(maxcpus, &setsize, ফাঁকা);
	যদি (!*set) {
		rc = -EINVAL;
		যাও out;
	}

	যদি (islist) {
		যদি (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			যাও out;
		}
	} অন্যথা {
		যদি (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			যাও out;
		}
	}
	rc = 0;

out:
	যদি (rc) {
		cpuset_free(*set);
		*set = ফাঁকা;
	}
	মুক্ত(buf);
	ফেরত rc;
}

পূর্ণসংখ্যা ul_path_readf_cpuset(কাঠামো path_cxt *pc, cpu_set_t **set, পূর্ণসংখ্যা maxcpus, ধ্রুবক অক্ষর *path, ...)
{
	va_list ap;
	পূর্ণসংখ্যা rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	ফেরত rc;
}

পূর্ণসংখ্যা ul_path_readf_cpulist(কাঠামো path_cxt *pc, cpu_set_t **set, পূর্ণসংখ্যা maxcpus, ধ্রুবক অক্ষর *path, ...)
{
	va_list ap;
	পূর্ণসংখ্যা rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	ফেরত rc;
}

#endif /* HAVE_CPU_SET_T */


#ifdef TEST_PROGRAM_PATH
#অন্তর্ভাব <getopt.h>

স্থির শূন্য __attribute__((__noreturn__)) usage(শূন্য)
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

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc, অক্ষর *argv[])
{
	পূর্ণসংখ্যা c;
	ধ্রুবক অক্ষর *prefix = ফাঁকা, *dir, *file, *command;
	কাঠামো path_cxt *pc = ফাঁকা;

	স্থির ধ্রুবক কাঠামো option longopts[] = {
		{ "prefix",	1, ফাঁকা, 'p' },
		{ "help",       0, ফাঁকা, 'h' },
		{ ফাঁকা, 0, ফাঁকা, 0 },
	};

	যখন((c = getopt_long(argc, argv, "p:h", longopts, ফাঁকা)) != -1) {
		নির্বাচন(c) {
		ক্ষেত্র 'p':
			prefix = optarg;
			বিরতি;
		ক্ষেত্র 'h':
			usage();
			বিরতি;
		পূর্বনির্ধারিত:
			err(EXIT_FAILURE, "try --help");
		}
	}

	যদি (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	যদি (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	যদি (prefix)
		ul_path_set_prefix(pc, prefix);

	যদি (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	যদি (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		যদি (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		যদি (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		যদি (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} অন্যথা যদি (strcmp(command, "read-s32") == 0) {
		int32_t res;

		যদি (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		যদি (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		যদি (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} অন্যথা যদি (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		যদি (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		যদি (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		যদি (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} অন্যথা যদি (strcmp(command, "read-s64") == 0) {
		int64_t res;

		যদি (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		যদি (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		যদি (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} অন্যথা যদি (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		যদি (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		যদি (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (পূর্ণসংখ্যা) res);

		যদি (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (পূর্ণসংখ্যা) res);

	} অন্যথা যদি (strcmp(command, "read-string") == 0) {
		অক্ষর *res;

		যদি (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		যদি (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		যদি (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} অন্যথা যদি (strcmp(command, "read-link") == 0) {
		অক্ষর res[PATH_MAX];

		যদি (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		যদি (ul_path_readlink(pc, res, আকার(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		যদি (ul_path_readlinkf(pc, res, আকার(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} অন্যথা যদি (strcmp(command, "write-string") == 0) {
		অক্ষর *str;

		যদি (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		যদি (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		যদি (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} অন্যথা যদি (strcmp(command, "write-u64") == 0) {
		uint64_t অঙ্ক;

		যদি (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		অঙ্ক = strtoumax(argv[optind++], ফাঁকা, 0);

		যদি (ul_path_write_u64(pc, অঙ্ক, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		যদি (ul_path_writef_u64(pc, অঙ্ক, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	ফেরত EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
