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
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <inttypes.h>
#அந்தர்பாவ <errno.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "fileutils.h"
#அந்தர்பாவ "all-io.h"
#அந்தர்பாவ "vfs.h"
#அந்தர்பாவ "path.h"
#அந்தர்பாவ "debug.h"
#அந்தர்பாவ "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
நிலையான UL_DEBUG_DEFINE_MASK(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPATH_DEBUG_INIT	(1 << 1)
#define ULPATH_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

வெறுமை ul_path_init_debug(வெறுமை)
{
	எனில் (ulpath_debug_mask)
		திரும்பு;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

கட்டமைப்பு path_cxt *ul_new_path(மாறிலி எழுத்து *dir, ...)
{
	கட்டமைப்பு path_cxt *pc = calloc(1, அளவு(*pc));

	எனில் (!pc)
		திரும்பு வெற்று;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	எனில் (dir) {
		முழு rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		எனில் (rc < 0 || !pc->dir_path)
			செல் fail;
	}
	திரும்பு pc;
fail:
	ul_unref_path(pc);
	திரும்பு வெற்று;
}

வெறுமை ul_ref_path(கட்டமைப்பு path_cxt *pc)
{
	எனில் (pc)
		pc->refcount++;
}

வெறுமை ul_unref_path(கட்டமைப்பு path_cxt *pc)
{
	எனில் (!pc)
		திரும்பு;

	pc->refcount--;

	எனில் (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		எனில் (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		விடுவி(pc->dir_path);
		விடுவி(pc->prefix);
		விடுவி(pc);
	}
}

வெறுமை ul_path_refer_vfs(கட்டமைப்பு path_cxt *pc, மாறிலி கட்டமைப்பு ul_vfs_ops *vfs)
{
	எனில் (pc)
		pc->vfs = vfs;
}

முழு ul_path_set_prefix(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *prefix)
{
	எழுத்து *p = வெற்று;

	ப்ரதிஜ்ஞா(pc->dir_fd < 0);

	எனில் (prefix) {
		p = strdup(prefix);
		எனில் (!p)
			திரும்பு -ENOMEM;
	}

	விடுவி(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	திரும்பு 0;
}

மாறிலி எழுத்து *ul_path_get_prefix(கட்டமைப்பு path_cxt *pc)
{
	திரும்பு pc ? pc->prefix : வெற்று;
}

முழு ul_path_set_dir(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *dir)
{
	எழுத்து *p = வெற்று;

	எனில் (dir) {
		p = strdup(dir);
		எனில் (!p)
			திரும்பு -ENOMEM;
	}

	எனில் (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	விடுவி(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	திரும்பு 0;
}

மாறிலி எழுத்து *ul_path_get_dir(கட்டமைப்பு path_cxt *pc)
{
	திரும்பு pc ? pc->dir_path : வெற்று;
}

முழு ul_path_set_dialect(கட்டமைப்பு path_cxt *pc, வெறுமை *data, வெறுமை free_data(கட்டமைப்பு path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	திரும்பு 0;
}

வெறுமை *ul_path_get_dialect(கட்டமைப்பு path_cxt *pc)
{
	திரும்பு pc ? pc->dialect : வெற்று;
}

முழு ul_path_set_enoent_redirect(கட்டமைப்பு path_cxt *pc, முழு (*func)(கட்டமைப்பு path_cxt *, மாறிலி எழுத்து *, முழு *))
{
	pc->redirect_on_enoent = func;
	திரும்பு 0;
}

நிலையான மாறிலி எழுத்து *get_absdir(கட்டமைப்பு path_cxt *pc)
{
	முழு rc;
	மாறிலி எழுத்து *dirpath;

	எனில் (!pc->prefix)
		திரும்பு pc->dir_path;

	dirpath = pc->dir_path;
	எனில் (!dirpath)
		திரும்பு pc->prefix;
	எனில் (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, அளவு(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	எனில் (rc < 0)
		திரும்பு வெற்று;
	எனில் ((size_t)rc >= அளவு(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		திரும்பு வெற்று;
	}

	திரும்பு pc->path_buffer;
}

முழு ul_path_is_accessible(கட்டமைப்பு path_cxt *pc)
{
	மாறிலி எழுத்து *path;
	ப்ரதிஜ்ஞா(pc);

	எனில் (pc->dir_fd >= 0)
		திரும்பு 1;

	path = get_absdir(pc);
	எனில் (!path)
		திரும்பு 0;
	திரும்பு access(path, F_OK) == 0;
}

முழு ul_path_get_dirfd(கட்டமைப்பு path_cxt *pc)
{
	ப்ரதிஜ்ஞா(pc);
	ப்ரதிஜ்ஞா(pc->dir_path);

	எனில் (pc->dir_fd < 0) {
		மாறிலி எழுத்து *path = get_absdir(pc);
		எனில் (!path)
			திரும்பு -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	திரும்பு pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
வெறுமை ul_path_close_dirfd(கட்டமைப்பு path_cxt *pc)
{
	ப்ரதிஜ்ஞா(pc);

	எனில் (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

முழு ul_path_isopen_dirfd(கட்டமைப்பு path_cxt *pc)
{
	திரும்பு pc && pc->dir_fd >= 0;
}

நிலையான மாறிலி எழுத்து *ul_path_mkpath(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *path, va_list ap)
{
	முழு rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, அளவு(pc->path_buffer), path, ap);
	எனில் (rc < 0) {
		எனில் (!errno)
			errno = EINVAL;
		திரும்பு வெற்று;
	}

	எனில் ((size_t)rc >= அளவு(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		திரும்பு வெற்று;
	}

	திரும்பு pc->path_buffer;
}

எழுத்து *ul_path_get_abspath(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t bufsz, மாறிலி எழுத்து *path, ...)
{
	எனில் (path) {
		முழு rc;
		va_list ap;
		மாறிலி எழுத்து *tail = வெற்று, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		எனில் (dirpath && *dirpath == '/')
			dirpath++;
		எனில் (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		எனில் ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			திரும்பு வெற்று;
		}
	} இல்லையெனில் {
		மாறிலி எழுத்து *tmp = get_absdir(pc);

		எனில் (!tmp)
			திரும்பு வெற்று;
		xstrncpy(buf, tmp, bufsz);
	}

	திரும்பு buf;
}


முழு ul_path_access(கட்டமைப்பு path_cxt *pc, முழு mode, மாறிலி எழுத்து *path)
{
	முழு rc;

	எனில் (!path)
		திரும்பு -EINVAL;
	எனில் (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} இல்லையெனில் {
		முழு dir = ul_path_get_dirfd(pc);
		எனில் (dir < 0)
			திரும்பு dir;
		எனில் (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		எனில் (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	திரும்பு rc;
}

முழு ul_path_accessf(கட்டமைப்பு path_cxt *pc, முழு mode, மாறிலி எழுத்து *path, ...)
{
	va_list ap;
	மாறிலி எழுத்து *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	திரும்பு !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
முழு ul_path_stat(கட்டமைப்பு path_cxt *pc, கட்டமைப்பு stat *sb, முழு flags, மாறிலி எழுத்து *path)
{
	முழு rc;

	எனில் (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} இல்லையெனில் {
		முழு dir = ul_path_get_dirfd(pc);
		எனில் (dir < 0)
			திரும்பு dir;
		எனில் (path) {
			எனில்  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} இல்லையெனில்
			rc = fstat(dir, sb);	/* dir itself */

		எனில் (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	திரும்பு rc;
}

முழு ul_path_vstatf(கட்டமைப்பு path_cxt *pc, கட்டமைப்பு stat *sb, முழு flags, மாறிலி எழுத்து *path, va_list ap)
{
	மாறிலி எழுத்து *p = ul_path_mkpath(pc, path, ap);

	திரும்பு !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

முழு ul_path_statf(கட்டமைப்பு path_cxt *pc, கட்டமைப்பு stat *sb, முழு flags, மாறிலி எழுத்து *path, ...)
{
	va_list ap;
	முழு rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	திரும்பு rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
முழு ul_path_statx(கட்டமைப்பு path_cxt *pc, கட்டமைப்பு statx *stx, முழு flags, unsigned முழு mask,
		  மாறிலி எழுத்து *path)
{
	முழு rc;

	எனில் (!path)
		திரும்பு -EINVAL;
	எனில் (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	இல்லையெனில் {
		முழு dir = ul_path_get_dirfd(pc);
		எனில் (dir < 0)
			திரும்பு dir;
		எனில் (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	திரும்பு rc;
}
#இல்லையெனில்
முழு ul_path_statx(கட்டமைப்பு path_cxt *pc __attribute__((__unused__)),
		  கட்டமைப்பு statx *stx __attribute__((__unused__)),
		  முழு flags __attribute__((__unused__)),
		  unsigned முழு mask __attribute__((__unused__)),
		  மாறிலி எழுத்து *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	திரும்பு -1;
}
#endif /* HAVE_STATX */

முழு ul_path_vstatxf(கட்டமைப்பு path_cxt *pc, கட்டமைப்பு statx *stx, முழு flags, unsigned முழு mask,
		    மாறிலி எழுத்து *path, va_list ap)
{
	மாறிலி எழுத்து *p = ul_path_mkpath(pc, path, ap);

	திரும்பு !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

முழு ul_path_statxf(கட்டமைப்பு path_cxt *pc, கட்டமைப்பு statx *stx, முழு flags, unsigned முழு mask,
		   மாறிலி எழுத்து *path, ...)
{
	va_list ap;
	முழு rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	திரும்பு rc;
}

முழு ul_path_open(கட்டமைப்பு path_cxt *pc, முழு flags, மாறிலி எழுத்து *path)
{
	முழு fd;

	எனில் (!path)
		திரும்பு -EINVAL;
	எனில் (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} இல்லையெனில் {
		முழு fdx;
		முழு dir = ul_path_get_dirfd(pc);
		எனில் (dir < 0)
			திரும்பு dir;

		எனில் (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		எனில் (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	திரும்பு fd;
}

முழு ul_path_vopenf(கட்டமைப்பு path_cxt *pc, முழு flags, மாறிலி எழுத்து *path, va_list ap)
{
	மாறிலி எழுத்து *p = ul_path_mkpath(pc, path, ap);

	திரும்பு !p ? -errno : ul_path_open(pc, flags, p);
}

முழு ul_path_openf(கட்டமைப்பு path_cxt *pc, முழு flags, மாறிலி எழுத்து *path, ...)
{
	va_list ap;
	முழு rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	திரும்பு rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *mode, மாறிலி எழுத்து *path)
{
	முழு flags = ul_mode_to_flags(mode);
	முழு fd = ul_path_open(pc, flags, path);

	எனில் (fd < 0)
		திரும்பு வெற்று;

	திரும்பு ul_vfs_fdopen(pc ? pc->vfs : வெற்று, fd, mode);
}


FILE *ul_path_vfopenf(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *mode, மாறிலி எழுத்து *path, va_list ap)
{
	மாறிலி எழுத்து *p = ul_path_mkpath(pc, path, ap);

	திரும்பு !p ? வெற்று : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *mode, மாறிலி எழுத்து *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	திரும்பு f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *path)
{
	DIR *dir;
	முழு fd = -1;

	எனில் (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	இல்லையெனில் எனில் (pc->dir_path) {
		முழு dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		எனில் (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	எனில் (fd < 0)
		திரும்பு வெற்று;

	dir = fdopendir(fd);
	எனில் (!dir) {
		close(fd);
		திரும்பு வெற்று;
	}
	எனில் (!path)
		 rewinddir(dir);
	திரும்பு dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *path, va_list ap)
{
	மாறிலி எழுத்து *p = ul_path_mkpath(pc, path, ap);

	திரும்பு !p ? வெற்று : ul_path_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendirf(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *path, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, path);
	dir = ul_path_vopendirf(pc, path, ap);
	va_end(ap);

	திரும்பு dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t bufsiz, மாறிலி எழுத்து *path)
{
	முழு dirfd;
	ssize_t ssz;

	எனில் (!path) {
		மாறிலி எழுத்து *p = get_absdir(pc);
		எனில் (!p)
			திரும்பு -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} இல்லையெனில் {
		dirfd = ul_path_get_dirfd(pc);
		எனில் (dirfd < 0)
			திரும்பு dirfd;

		எனில் (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	எனில் (ssz >= 0)
		buf[ssz] = '\0';
	திரும்பு ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlinkf(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t bufsiz, மாறிலி எழுத்து *path, ...)
{
	மாறிலி எழுத்து *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	திரும்பு !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

முழு ul_path_read(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t len, மாறிலி எழுத்து *path)
{
	முழு rc, errsv;
	முழு fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	எனில் (fd < 0)
		திரும்பு -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : வெற்று, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : வெற்று, fd);
	errno = errsv;
	திரும்பு rc;
}

முழு ul_path_vreadf(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t len, மாறிலி எழுத்து *path, va_list ap)
{
	மாறிலி எழுத்து *p = ul_path_mkpath(pc, path, ap);

	திரும்பு !p ? -errno : ul_path_read(pc, buf, len, p);
}

முழு ul_path_readf(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t len, மாறிலி எழுத்து *path, ...)
{
	va_list ap;
	முழு rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	திரும்பு rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
முழு ul_path_read_string(கட்டமைப்பு path_cxt *pc, எழுத்து **str, மாறிலி எழுத்து *path)
{
	எழுத்து buf[BUFSIZ];
	முழு rc;

	எனில் (!str)
		திரும்பு -EINVAL;

	*str = வெற்று;

	rc = ul_path_read_buffer(pc, buf, அளவு(buf), path);
	எனில் (rc < 0)
		திரும்பு rc;

	*str = strdup(buf);
	எனில் (!*str)
		rc = -ENOMEM;

	திரும்பு rc;
}

முழு ul_path_readf_string(கட்டமைப்பு path_cxt *pc, எழுத்து **str, மாறிலி எழுத்து *path, ...)
{
	மாறிலி எழுத்து *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	திரும்பு !p ? -errno : ul_path_read_string(pc, str, p);
}

முழு ul_path_read_buffer(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t bufsz, மாறிலி எழுத்து *path)
{
	முழு rc = ul_path_read(pc, buf, bufsz - 1, path);

	எனில் (rc == 0)
		buf[0] = '\0';

	இல்லையெனில் எனில் (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		எனில் (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		இல்லையெனில்
			buf[rc] = '\0';
	}

	திரும்பு rc;
}

முழு ul_path_vreadf_buffer(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t bufsz, மாறிலி எழுத்து *path, va_list ap)
{
	மாறிலி எழுத்து *p;

	p = ul_path_mkpath(pc, path, ap);

	திரும்பு !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

முழு ul_path_readf_buffer(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t bufsz, மாறிலி எழுத்து *path, ...)
{
	va_list ap;
	முழு rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	திரும்பு rc;
}

முழு ul_path_scanf(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *path, மாறிலி எழுத்து *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	முழு rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	எனில் (!f)
		திரும்பு -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	திரும்பு rc;
}

முழு ul_path_scanff(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *path, va_list ap, மாறிலி எழுத்து *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	முழு rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	எனில் (!f)
		திரும்பு -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	திரும்பு rc;
}


முழு ul_path_read_s64(கட்டமைப்பு path_cxt *pc, int64_t *res, மாறிலி எழுத்து *path)
{
	int64_t x = 0;
	முழு rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	எனில் (rc != 1)
		திரும்பு -1;
	எனில் (res)
		*res = x;
	திரும்பு 0;
}

முழு ul_path_readf_s64(கட்டமைப்பு path_cxt *pc, int64_t *res, மாறிலி எழுத்து *path, ...)
{
	மாறிலி எழுத்து *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	திரும்பு !p ? -errno : ul_path_read_s64(pc, res, p);
}

முழு ul_path_read_u64(கட்டமைப்பு path_cxt *pc, uint64_t *res, மாறிலி எழுத்து *path)
{
	uint64_t x = 0;
	முழு rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	எனில் (rc != 1)
		திரும்பு -1;
	எனில் (res)
		*res = x;
	திரும்பு 0;
}

முழு ul_path_readf_u64(கட்டமைப்பு path_cxt *pc, uint64_t *res, மாறிலி எழுத்து *path, ...)
{
	மாறிலி எழுத்து *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	திரும்பு !p ? -errno : ul_path_read_u64(pc, res, p);
}

முழு ul_path_read_s32(கட்டமைப்பு path_cxt *pc, முழு *res, மாறிலி எழுத்து *path)
{
	முழு rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	எனில் (rc != 1)
		திரும்பு -1;
	எனில் (res)
		*res = x;
	திரும்பு 0;
}

முழு ul_path_readf_s32(கட்டமைப்பு path_cxt *pc, முழு *res, மாறிலி எழுத்து *path, ...)
{
	மாறிலி எழுத்து *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	திரும்பு !p ? -errno : ul_path_read_s32(pc, res, p);
}

முழு ul_path_read_u32(கட்டமைப்பு path_cxt *pc, unsigned முழு *res, மாறிலி எழுத்து *path)
{
	முழு rc;
	unsigned முழு x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	எனில் (rc != 1)
		திரும்பு -1;
	எனில் (res)
		*res = x;
	திரும்பு 0;
}

முழு ul_path_readf_u32(கட்டமைப்பு path_cxt *pc, unsigned முழு *res, மாறிலி எழுத்து *path, ...)
{
	மாறிலி எழுத்து *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	திரும்பு !p ? -errno : ul_path_read_u32(pc, res, p);
}

முழு ul_path_read_majmin(கட்டமைப்பு path_cxt *pc, dev_t *res, மாறிலி எழுத்து *path)
{
	முழு rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	எனில் (rc != 2)
		திரும்பு -1;
	எனில் (res)
		*res = makedev(maj, min);
	திரும்பு 0;
}

முழு ul_path_readf_majmin(கட்டமைப்பு path_cxt *pc, dev_t *res, மாறிலி எழுத்து *path, ...)
{
	மாறிலி எழுத்து *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	திரும்பு !p ? -errno : ul_path_read_majmin(pc, res, p);
}

முழு ul_path_write_string(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *str, மாறிலி எழுத்து *path)
{
	முழு rc, errsv;
	முழு fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	எனில் (fd < 0)
		திரும்பு -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : வெற்று, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : வெற்று, fd);
	errno = errsv;
	திரும்பு rc;
}

முழு ul_path_writef_string(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *str, மாறிலி எழுத்து *path, ...)
{
	மாறிலி எழுத்து *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	திரும்பு !p ? -errno : ul_path_write_string(pc, str, p);
}

முழு ul_path_write_s64(கட்டமைப்பு path_cxt *pc, int64_t இலக்கம், மாறிலி எழுத்து *path)
{
	எழுத்து buf[அளவு(stringify_மதிப்பு(LLONG_MAX))];
	முழு rc, errsv;
	முழு fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	எனில் (fd < 0)
		திரும்பு -errno;

	len = snprintf(buf, அளவு(buf), "%" PRId64, இலக்கம்);
	எனில் (len < 0 || (size_t) len >= அளவு(buf))
		rc = len < 0 ? -errno : -E2BIG;
	இல்லையெனில்
		rc = ul_vfs_write_all(pc ? pc->vfs : வெற்று, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : வெற்று, fd);
	errno = errsv;
	திரும்பு rc;
}

முழு ul_path_write_u64(கட்டமைப்பு path_cxt *pc, uint64_t இலக்கம், மாறிலி எழுத்து *path)
{
	எழுத்து buf[அளவு(stringify_மதிப்பு(ULLONG_MAX))];
	முழு rc, errsv;
	முழு fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	எனில் (fd < 0)
		திரும்பு -errno;

	len = snprintf(buf, அளவு(buf), "%" PRIu64, இலக்கம்);
	எனில் (len < 0 || (size_t) len >= அளவு(buf))
		rc = len < 0 ? -errno : -E2BIG;
	இல்லையெனில்
		rc = ul_vfs_write_all(pc ? pc->vfs : வெற்று, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : வெற்று, fd);
	errno = errsv;
	திரும்பு rc;
}

முழு ul_path_writef_u64(கட்டமைப்பு path_cxt *pc, uint64_t இலக்கம், மாறிலி எழுத்து *path, ...)
{
	மாறிலி எழுத்து *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	திரும்பு !p ? -errno : ul_path_write_u64(pc, இலக்கம், p);

}

முழு ul_path_மொத்தம்_dirents(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *path)
{
	DIR *dir;
	முழு r = 0;

	dir = ul_path_opendir(pc, path);
	எனில் (!dir)
		திரும்பு 0;

	போது (xreaddir(dir)) r++;

	closedir(dir);
	திரும்பு r;
}

முழு ul_path_countf_dirents(கட்டமைப்பு path_cxt *pc, மாறிலி எழுத்து *path, ...)
{
	மாறிலி எழுத்து *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	திரும்பு !p ? -errno : ul_path_மொத்தம்_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
முழு ul_path_next_dirent(கட்டமைப்பு path_cxt *pc, DIR **sub, மாறிலி எழுத்து *dirname, கட்டமைப்பு dirent **d)
{
	எனில் (!pc || !sub || !d)
		திரும்பு -EINVAL;

	எனில் (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		எனில் (!*sub)
			திரும்பு -errno;
	}

	*d = xreaddir(*sub);
	எனில் (*d)
		திரும்பு 0;

	closedir(*sub);
	*sub = வெற்று;
	திரும்பு 1;
}

#ifdef HAVE_CPU_SET_T
நிலையான முழு ul_path_cpuparse(கட்டமைப்பு path_cxt *pc, cpu_set_t **set, முழு maxcpus, முழு islist, மாறிலி எழுத்து *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	எழுத்து *buf;
	முழு rc;

	*set = வெற்று;

	buf = malloc(len);
	எனில் (!buf)
		திரும்பு -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	எனில் (rc < 0)
		செல் out;

	*set = cpuset_alloc(maxcpus, &setsize, வெற்று);
	எனில் (!*set) {
		rc = -EINVAL;
		செல் out;
	}

	எனில் (islist) {
		எனில் (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			செல் out;
		}
	} இல்லையெனில் {
		எனில் (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			செல் out;
		}
	}
	rc = 0;

out:
	எனில் (rc) {
		cpuset_free(*set);
		*set = வெற்று;
	}
	விடுவி(buf);
	திரும்பு rc;
}

முழு ul_path_readf_cpuset(கட்டமைப்பு path_cxt *pc, cpu_set_t **set, முழு maxcpus, மாறிலி எழுத்து *path, ...)
{
	va_list ap;
	முழு rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	திரும்பு rc;
}

முழு ul_path_readf_cpulist(கட்டமைப்பு path_cxt *pc, cpu_set_t **set, முழு maxcpus, மாறிலி எழுத்து *path, ...)
{
	va_list ap;
	முழு rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	திரும்பு rc;
}

#endif /* HAVE_CPU_SET_T */


#ifdef TEST_PROGRAM_PATH
#அந்தர்பாவ <getopt.h>

நிலையான வெறுமை __attribute__((__noreturn__)) usage(வெறுமை)
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

முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	முழு c;
	மாறிலி எழுத்து *prefix = வெற்று, *dir, *file, *command;
	கட்டமைப்பு path_cxt *pc = வெற்று;

	நிலையான மாறிலி கட்டமைப்பு option longopts[] = {
		{ "prefix",	1, வெற்று, 'p' },
		{ "help",       0, வெற்று, 'h' },
		{ வெற்று, 0, வெற்று, 0 },
	};

	போது((c = getopt_long(argc, argv, "p:h", longopts, வெற்று)) != -1) {
		தேர்வு(c) {
		நிலை 'p':
			prefix = optarg;
			நிறுத்து;
		நிலை 'h':
			usage();
			நிறுத்து;
		இயல்பு:
			err(EXIT_FAILURE, "try --help");
		}
	}

	எனில் (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	எனில் (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	எனில் (prefix)
		ul_path_set_prefix(pc, prefix);

	எனில் (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	எனில் (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		எனில் (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		எனில் (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		எனில் (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} இல்லையெனில் எனில் (strcmp(command, "read-s32") == 0) {
		int32_t res;

		எனில் (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		எனில் (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		எனில் (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} இல்லையெனில் எனில் (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		எனில் (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		எனில் (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		எனில் (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} இல்லையெனில் எனில் (strcmp(command, "read-s64") == 0) {
		int64_t res;

		எனில் (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		எனில் (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		எனில் (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} இல்லையெனில் எனில் (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		எனில் (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		எனில் (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (முழு) res);

		எனில் (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (முழு) res);

	} இல்லையெனில் எனில் (strcmp(command, "read-string") == 0) {
		எழுத்து *res;

		எனில் (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		எனில் (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		எனில் (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} இல்லையெனில் எனில் (strcmp(command, "read-link") == 0) {
		எழுத்து res[PATH_MAX];

		எனில் (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		எனில் (ul_path_readlink(pc, res, அளவு(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		எனில் (ul_path_readlinkf(pc, res, அளவு(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} இல்லையெனில் எனில் (strcmp(command, "write-string") == 0) {
		எழுத்து *str;

		எனில் (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		எனில் (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		எனில் (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} இல்லையெனில் எனில் (strcmp(command, "write-u64") == 0) {
		uint64_t இலக்கம்;

		எனில் (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		இலக்கம் = strtoumax(argv[optind++], வெற்று, 0);

		எனில் (ul_path_write_u64(pc, இலக்கம், file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		எனில் (ul_path_writef_u64(pc, இலக்கம், "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	திரும்பு EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
