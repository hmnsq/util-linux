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
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
#include <errno.h>

#include "c.h"
#include "fileutils.h"
#include "all-io.h"
#include "vfs.h"
#include "path.h"
#include "debug.h"
#include "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
static UL_DEBUG_DEFINE_MASK(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPATH_DEBUG_INIT	(1 << 1)
#define ULPATH_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

void ul_path_init_debug(void)
{
	གལ་ཏེ (ulpath_debug_mask)
		ཕྱིར;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

sa_nracanaa path_cxt *ul_new_path(const var_na *dir, ...)
{
	sa_nracanaa path_cxt *pc = calloc(1, parimaa_na(*pc));

	གལ་ཏེ (!pc)
		ཕྱིར NULL;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	གལ་ཏེ (dir) {
		ཧྲིལ rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		གལ་ཏེ (rc < 0 || !pc->dir_path)
			goto fail;
	}
	ཕྱིར pc;
fail:
	ul_unref_path(pc);
	ཕྱིར NULL;
}

void ul_ref_path(sa_nracanaa path_cxt *pc)
{
	གལ་ཏེ (pc)
		pc->refcount++;
}

void ul_unref_path(sa_nracanaa path_cxt *pc)
{
	གལ་ཏེ (!pc)
		ཕྱིར;

	pc->refcount--;

	གལ་ཏེ (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		གལ་ཏེ (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		free(pc->dir_path);
		free(pc->prefix);
		free(pc);
	}
}

void ul_path_refer_vfs(sa_nracanaa path_cxt *pc, const sa_nracanaa ul_vfs_ops *vfs)
{
	གལ་ཏེ (pc)
		pc->vfs = vfs;
}

ཧྲིལ ul_path_set_prefix(sa_nracanaa path_cxt *pc, const var_na *prefix)
{
	var_na *p = NULL;

	assert(pc->dir_fd < 0);

	གལ་ཏེ (prefix) {
		p = strdup(prefix);
		གལ་ཏེ (!p)
			ཕྱིར -ENOMEM;
	}

	free(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	ཕྱིར 0;
}

const var_na *ul_path_get_prefix(sa_nracanaa path_cxt *pc)
{
	ཕྱིར pc ? pc->prefix : NULL;
}

ཧྲིལ ul_path_set_dir(sa_nracanaa path_cxt *pc, const var_na *dir)
{
	var_na *p = NULL;

	གལ་ཏེ (dir) {
		p = strdup(dir);
		གལ་ཏེ (!p)
			ཕྱིར -ENOMEM;
	}

	གལ་ཏེ (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	free(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	ཕྱིར 0;
}

const var_na *ul_path_get_dir(sa_nracanaa path_cxt *pc)
{
	ཕྱིར pc ? pc->dir_path : NULL;
}

ཧྲིལ ul_path_set_dialect(sa_nracanaa path_cxt *pc, void *data, void free_data(sa_nracanaa path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	ཕྱིར 0;
}

void *ul_path_get_dialect(sa_nracanaa path_cxt *pc)
{
	ཕྱིར pc ? pc->dialect : NULL;
}

ཧྲིལ ul_path_set_enoent_redirect(sa_nracanaa path_cxt *pc, ཧྲིལ (*func)(sa_nracanaa path_cxt *, const var_na *, ཧྲིལ *))
{
	pc->redirect_on_enoent = func;
	ཕྱིར 0;
}

static const var_na *get_absdir(sa_nracanaa path_cxt *pc)
{
	ཧྲིལ rc;
	const var_na *dirpath;

	གལ་ཏེ (!pc->prefix)
		ཕྱིར pc->dir_path;

	dirpath = pc->dir_path;
	གལ་ཏེ (!dirpath)
		ཕྱིར pc->prefix;
	གལ་ཏེ (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, parimaa_na(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	གལ་ཏེ (rc < 0)
		ཕྱིར NULL;
	གལ་ཏེ ((size_t)rc >= parimaa_na(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		ཕྱིར NULL;
	}

	ཕྱིར pc->path_buffer;
}

ཧྲིལ ul_path_is_accessible(sa_nracanaa path_cxt *pc)
{
	const var_na *path;
	assert(pc);

	གལ་ཏེ (pc->dir_fd >= 0)
		ཕྱིར 1;

	path = get_absdir(pc);
	གལ་ཏེ (!path)
		ཕྱིར 0;
	ཕྱིར access(path, F_OK) == 0;
}

ཧྲིལ ul_path_get_dirfd(sa_nracanaa path_cxt *pc)
{
	assert(pc);
	assert(pc->dir_path);

	གལ་ཏེ (pc->dir_fd < 0) {
		const var_na *path = get_absdir(pc);
		གལ་ཏེ (!path)
			ཕྱིར -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	ཕྱིར pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
void ul_path_close_dirfd(sa_nracanaa path_cxt *pc)
{
	assert(pc);

	གལ་ཏེ (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

ཧྲིལ ul_path_isopen_dirfd(sa_nracanaa path_cxt *pc)
{
	ཕྱིར pc && pc->dir_fd >= 0;
}

static const var_na *ul_path_mkpath(sa_nracanaa path_cxt *pc, const var_na *path, va_list ap)
{
	ཧྲིལ rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, parimaa_na(pc->path_buffer), path, ap);
	གལ་ཏེ (rc < 0) {
		གལ་ཏེ (!errno)
			errno = EINVAL;
		ཕྱིར NULL;
	}

	གལ་ཏེ ((size_t)rc >= parimaa_na(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		ཕྱིར NULL;
	}

	ཕྱིར pc->path_buffer;
}

var_na *ul_path_get_abspath(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz, const var_na *path, ...)
{
	གལ་ཏེ (path) {
		ཧྲིལ rc;
		va_list ap;
		const var_na *tail = NULL, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		གལ་ཏེ (dirpath && *dirpath == '/')
			dirpath++;
		གལ་ཏེ (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		གལ་ཏེ ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			ཕྱིར NULL;
		}
	} དེ་མིན {
		const var_na *tmp = get_absdir(pc);

		གལ་ཏེ (!tmp)
			ཕྱིར NULL;
		xstrncpy(buf, tmp, bufsz);
	}

	ཕྱིར buf;
}


ཧྲིལ ul_path_access(sa_nracanaa path_cxt *pc, ཧྲིལ mode, const var_na *path)
{
	ཧྲིལ rc;

	གལ་ཏེ (!path)
		ཕྱིར -EINVAL;
	གལ་ཏེ (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} དེ་མིན {
		ཧྲིལ dir = ul_path_get_dirfd(pc);
		གལ་ཏེ (dir < 0)
			ཕྱིར dir;
		གལ་ཏེ (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		གལ་ཏེ (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	ཕྱིར rc;
}

ཧྲིལ ul_path_accessf(sa_nracanaa path_cxt *pc, ཧྲིལ mode, const var_na *path, ...)
{
	va_list ap;
	const var_na *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ཕྱིར !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
ཧྲིལ ul_path_stat(sa_nracanaa path_cxt *pc, sa_nracanaa stat *sb, ཧྲིལ flags, const var_na *path)
{
	ཧྲིལ rc;

	གལ་ཏེ (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} དེ་མིན {
		ཧྲིལ dir = ul_path_get_dirfd(pc);
		གལ་ཏེ (dir < 0)
			ཕྱིར dir;
		གལ་ཏེ (path) {
			གལ་ཏེ  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} དེ་མིན
			rc = fstat(dir, sb);	/* dir itself */

		གལ་ཏེ (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	ཕྱིར rc;
}

ཧྲིལ ul_path_vstatf(sa_nracanaa path_cxt *pc, sa_nracanaa stat *sb, ཧྲིལ flags, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ཕྱིར !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

ཧྲིལ ul_path_statf(sa_nracanaa path_cxt *pc, sa_nracanaa stat *sb, ཧྲིལ flags, const var_na *path, ...)
{
	va_list ap;
	ཧྲིལ rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	ཕྱིར rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
ཧྲིལ ul_path_statx(sa_nracanaa path_cxt *pc, sa_nracanaa statx *stx, ཧྲིལ flags, unsigned ཧྲིལ mask,
		  const var_na *path)
{
	ཧྲིལ rc;

	གལ་ཏེ (!path)
		ཕྱིར -EINVAL;
	གལ་ཏེ (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	དེ་མིན {
		ཧྲིལ dir = ul_path_get_dirfd(pc);
		གལ་ཏེ (dir < 0)
			ཕྱིར dir;
		གལ་ཏེ (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	ཕྱིར rc;
}
#དེ་མིན
ཧྲིལ ul_path_statx(sa_nracanaa path_cxt *pc __attribute__((__unused__)),
		  sa_nracanaa statx *stx __attribute__((__unused__)),
		  ཧྲིལ flags __attribute__((__unused__)),
		  unsigned ཧྲིལ mask __attribute__((__unused__)),
		  const var_na *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	ཕྱིར -1;
}
#endif /* HAVE_STATX */

ཧྲིལ ul_path_vstatxf(sa_nracanaa path_cxt *pc, sa_nracanaa statx *stx, ཧྲིལ flags, unsigned ཧྲིལ mask,
		    const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ཕྱིར !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

ཧྲིལ ul_path_statxf(sa_nracanaa path_cxt *pc, sa_nracanaa statx *stx, ཧྲིལ flags, unsigned ཧྲིལ mask,
		   const var_na *path, ...)
{
	va_list ap;
	ཧྲིལ rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	ཕྱིར rc;
}

ཧྲིལ ul_path_open(sa_nracanaa path_cxt *pc, ཧྲིལ flags, const var_na *path)
{
	ཧྲིལ fd;

	གལ་ཏེ (!path)
		ཕྱིར -EINVAL;
	གལ་ཏེ (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} དེ་མིན {
		ཧྲིལ fdx;
		ཧྲིལ dir = ul_path_get_dirfd(pc);
		གལ་ཏེ (dir < 0)
			ཕྱིར dir;

		གལ་ཏེ (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		གལ་ཏེ (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	ཕྱིར fd;
}

ཧྲིལ ul_path_vopenf(sa_nracanaa path_cxt *pc, ཧྲིལ flags, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ཕྱིར !p ? -errno : ul_path_open(pc, flags, p);
}

ཧྲིལ ul_path_openf(sa_nracanaa path_cxt *pc, ཧྲིལ flags, const var_na *path, ...)
{
	va_list ap;
	ཧྲིལ rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	ཕྱིར rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(sa_nracanaa path_cxt *pc, const var_na *mode, const var_na *path)
{
	ཧྲིལ flags = ul_mode_to_flags(mode);
	ཧྲིལ fd = ul_path_open(pc, flags, path);

	གལ་ཏེ (fd < 0)
		ཕྱིར NULL;

	ཕྱིར ul_vfs_fdopen(pc ? pc->vfs : NULL, fd, mode);
}


FILE *ul_path_vfopenf(sa_nracanaa path_cxt *pc, const var_na *mode, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ཕྱིར !p ? NULL : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(sa_nracanaa path_cxt *pc, const var_na *mode, const var_na *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	ཕྱིར f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(sa_nracanaa path_cxt *pc, const var_na *path)
{
	DIR *dir;
	ཧྲིལ fd = -1;

	གལ་ཏེ (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	དེ་མིན གལ་ཏེ (pc->dir_path) {
		ཧྲིལ dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		གལ་ཏེ (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	གལ་ཏེ (fd < 0)
		ཕྱིར NULL;

	dir = fdopendir(fd);
	གལ་ཏེ (!dir) {
		close(fd);
		ཕྱིར NULL;
	}
	གལ་ཏེ (!path)
		 rewinddir(dir);
	ཕྱིར dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(sa_nracanaa path_cxt *pc, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ཕྱིར !p ? NULL : ul_path_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendirf(sa_nracanaa path_cxt *pc, const var_na *path, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, path);
	dir = ul_path_vopendirf(pc, path, ap);
	va_end(ap);

	ཕྱིར dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsiz, const var_na *path)
{
	ཧྲིལ dirfd;
	ssize_t ssz;

	གལ་ཏེ (!path) {
		const var_na *p = get_absdir(pc);
		གལ་ཏེ (!p)
			ཕྱིར -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} དེ་མིན {
		dirfd = ul_path_get_dirfd(pc);
		གལ་ཏེ (dirfd < 0)
			ཕྱིར dirfd;

		གལ་ཏེ (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	གལ་ཏེ (ssz >= 0)
		buf[ssz] = '\0';
	ཕྱིར ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlinkf(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsiz, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ཕྱིར !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

ཧྲིལ ul_path_read(sa_nracanaa path_cxt *pc, var_na *buf, size_t len, const var_na *path)
{
	ཧྲིལ rc, errsv;
	ཧྲིལ fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	གལ་ཏེ (fd < 0)
		ཕྱིར -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	ཕྱིར rc;
}

ཧྲིལ ul_path_vreadf(sa_nracanaa path_cxt *pc, var_na *buf, size_t len, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ཕྱིར !p ? -errno : ul_path_read(pc, buf, len, p);
}

ཧྲིལ ul_path_readf(sa_nracanaa path_cxt *pc, var_na *buf, size_t len, const var_na *path, ...)
{
	va_list ap;
	ཧྲིལ rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	ཕྱིར rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
ཧྲིལ ul_path_read_string(sa_nracanaa path_cxt *pc, var_na **str, const var_na *path)
{
	var_na buf[BUFSIZ];
	ཧྲིལ rc;

	གལ་ཏེ (!str)
		ཕྱིར -EINVAL;

	*str = NULL;

	rc = ul_path_read_buffer(pc, buf, parimaa_na(buf), path);
	གལ་ཏེ (rc < 0)
		ཕྱིར rc;

	*str = strdup(buf);
	གལ་ཏེ (!*str)
		rc = -ENOMEM;

	ཕྱིར rc;
}

ཧྲིལ ul_path_readf_string(sa_nracanaa path_cxt *pc, var_na **str, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ཕྱིར !p ? -errno : ul_path_read_string(pc, str, p);
}

ཧྲིལ ul_path_read_buffer(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz, const var_na *path)
{
	ཧྲིལ rc = ul_path_read(pc, buf, bufsz - 1, path);

	གལ་ཏེ (rc == 0)
		buf[0] = '\0';

	དེ་མིན གལ་ཏེ (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		གལ་ཏེ (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		དེ་མིན
			buf[rc] = '\0';
	}

	ཕྱིར rc;
}

ཧྲིལ ul_path_vreadf_buffer(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz, const var_na *path, va_list ap)
{
	const var_na *p;

	p = ul_path_mkpath(pc, path, ap);

	ཕྱིར !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

ཧྲིལ ul_path_readf_buffer(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz, const var_na *path, ...)
{
	va_list ap;
	ཧྲིལ rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	ཕྱིར rc;
}

ཧྲིལ ul_path_scanf(sa_nracanaa path_cxt *pc, const var_na *path, const var_na *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	ཧྲིལ rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	གལ་ཏེ (!f)
		ཕྱིར -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ཕྱིར rc;
}

ཧྲིལ ul_path_scanff(sa_nracanaa path_cxt *pc, const var_na *path, va_list ap, const var_na *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	ཧྲིལ rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	གལ་ཏེ (!f)
		ཕྱིར -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ཕྱིར rc;
}


ཧྲིལ ul_path_read_s64(sa_nracanaa path_cxt *pc, int64_t *res, const var_na *path)
{
	int64_t x = 0;
	ཧྲིལ rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	གལ་ཏེ (rc != 1)
		ཕྱིར -1;
	གལ་ཏེ (res)
		*res = x;
	ཕྱིར 0;
}

ཧྲིལ ul_path_readf_s64(sa_nracanaa path_cxt *pc, int64_t *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ཕྱིར !p ? -errno : ul_path_read_s64(pc, res, p);
}

ཧྲིལ ul_path_read_u64(sa_nracanaa path_cxt *pc, uint64_t *res, const var_na *path)
{
	uint64_t x = 0;
	ཧྲིལ rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	གལ་ཏེ (rc != 1)
		ཕྱིར -1;
	གལ་ཏེ (res)
		*res = x;
	ཕྱིར 0;
}

ཧྲིལ ul_path_readf_u64(sa_nracanaa path_cxt *pc, uint64_t *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ཕྱིར !p ? -errno : ul_path_read_u64(pc, res, p);
}

ཧྲིལ ul_path_read_s32(sa_nracanaa path_cxt *pc, ཧྲིལ *res, const var_na *path)
{
	ཧྲིལ rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	གལ་ཏེ (rc != 1)
		ཕྱིར -1;
	གལ་ཏེ (res)
		*res = x;
	ཕྱིར 0;
}

ཧྲིལ ul_path_readf_s32(sa_nracanaa path_cxt *pc, ཧྲིལ *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ཕྱིར !p ? -errno : ul_path_read_s32(pc, res, p);
}

ཧྲིལ ul_path_read_u32(sa_nracanaa path_cxt *pc, unsigned ཧྲིལ *res, const var_na *path)
{
	ཧྲིལ rc;
	unsigned ཧྲིལ x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	གལ་ཏེ (rc != 1)
		ཕྱིར -1;
	གལ་ཏེ (res)
		*res = x;
	ཕྱིར 0;
}

ཧྲིལ ul_path_readf_u32(sa_nracanaa path_cxt *pc, unsigned ཧྲིལ *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ཕྱིར !p ? -errno : ul_path_read_u32(pc, res, p);
}

ཧྲིལ ul_path_read_majmin(sa_nracanaa path_cxt *pc, dev_t *res, const var_na *path)
{
	ཧྲིལ rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	གལ་ཏེ (rc != 2)
		ཕྱིར -1;
	གལ་ཏེ (res)
		*res = makedev(maj, min);
	ཕྱིར 0;
}

ཧྲིལ ul_path_readf_majmin(sa_nracanaa path_cxt *pc, dev_t *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ཕྱིར !p ? -errno : ul_path_read_majmin(pc, res, p);
}

ཧྲིལ ul_path_write_string(sa_nracanaa path_cxt *pc, const var_na *str, const var_na *path)
{
	ཧྲིལ rc, errsv;
	ཧྲིལ fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	གལ་ཏེ (fd < 0)
		ཕྱིར -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	ཕྱིར rc;
}

ཧྲིལ ul_path_writef_string(sa_nracanaa path_cxt *pc, const var_na *str, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ཕྱིར !p ? -errno : ul_path_write_string(pc, str, p);
}

ཧྲིལ ul_path_write_s64(sa_nracanaa path_cxt *pc, int64_t num, const var_na *path)
{
	var_na buf[parimaa_na(stringify_value(LLONG_MAX))];
	ཧྲིལ rc, errsv;
	ཧྲིལ fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	གལ་ཏེ (fd < 0)
		ཕྱིར -errno;

	len = snprintf(buf, parimaa_na(buf), "%" PRId64, num);
	གལ་ཏེ (len < 0 || (size_t) len >= parimaa_na(buf))
		rc = len < 0 ? -errno : -E2BIG;
	དེ་མིན
		rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	ཕྱིར rc;
}

ཧྲིལ ul_path_write_u64(sa_nracanaa path_cxt *pc, uint64_t num, const var_na *path)
{
	var_na buf[parimaa_na(stringify_value(ULLONG_MAX))];
	ཧྲིལ rc, errsv;
	ཧྲིལ fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	གལ་ཏེ (fd < 0)
		ཕྱིར -errno;

	len = snprintf(buf, parimaa_na(buf), "%" PRIu64, num);
	གལ་ཏེ (len < 0 || (size_t) len >= parimaa_na(buf))
		rc = len < 0 ? -errno : -E2BIG;
	དེ་མིན
		rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	ཕྱིར rc;
}

ཧྲིལ ul_path_writef_u64(sa_nracanaa path_cxt *pc, uint64_t num, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ཕྱིར !p ? -errno : ul_path_write_u64(pc, num, p);

}

ཧྲིལ ul_path_count_dirents(sa_nracanaa path_cxt *pc, const var_na *path)
{
	DIR *dir;
	ཧྲིལ r = 0;

	dir = ul_path_opendir(pc, path);
	གལ་ཏེ (!dir)
		ཕྱིར 0;

	སྐབས (xreaddir(dir)) r++;

	closedir(dir);
	ཕྱིར r;
}

ཧྲིལ ul_path_countf_dirents(sa_nracanaa path_cxt *pc, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ཕྱིར !p ? -errno : ul_path_count_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
ཧྲིལ ul_path_next_dirent(sa_nracanaa path_cxt *pc, DIR **sub, const var_na *dirname, sa_nracanaa dirent **d)
{
	གལ་ཏེ (!pc || !sub || !d)
		ཕྱིར -EINVAL;

	གལ་ཏེ (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		གལ་ཏེ (!*sub)
			ཕྱིར -errno;
	}

	*d = xreaddir(*sub);
	གལ་ཏེ (*d)
		ཕྱིར 0;

	closedir(*sub);
	*sub = NULL;
	ཕྱིར 1;
}

#ifdef HAVE_CPU_SET_T
static ཧྲིལ ul_path_cpuparse(sa_nracanaa path_cxt *pc, cpu_set_t **set, ཧྲིལ maxcpus, ཧྲིལ islist, const var_na *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	var_na *buf;
	ཧྲིལ rc;

	*set = NULL;

	buf = malloc(len);
	གལ་ཏེ (!buf)
		ཕྱིར -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	གལ་ཏེ (rc < 0)
		goto out;

	*set = cpuset_alloc(maxcpus, &setsize, NULL);
	གལ་ཏེ (!*set) {
		rc = -EINVAL;
		goto out;
	}

	གལ་ཏེ (islist) {
		གལ་ཏེ (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			goto out;
		}
	} དེ་མིན {
		གལ་ཏེ (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			goto out;
		}
	}
	rc = 0;

out:
	གལ་ཏེ (rc) {
		cpuset_free(*set);
		*set = NULL;
	}
	free(buf);
	ཕྱིར rc;
}

ཧྲིལ ul_path_readf_cpuset(sa_nracanaa path_cxt *pc, cpu_set_t **set, ཧྲིལ maxcpus, const var_na *path, ...)
{
	va_list ap;
	ཧྲིལ rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	ཕྱིར rc;
}

ཧྲིལ ul_path_readf_cpulist(sa_nracanaa path_cxt *pc, cpu_set_t **set, ཧྲིལ maxcpus, const var_na *path, ...)
{
	va_list ap;
	ཧྲིལ rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	ཕྱིར rc;
}

#endif /* HAVE_CPU_SET_T */


#ifdef TEST_PROGRAM_PATH
#include <getopt.h>

static void __attribute__((__noreturn__)) usage(void)
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

ཧྲིལ གཙོ(ཧྲིལ argc, var_na *argv[])
{
	ཧྲིལ c;
	const var_na *prefix = NULL, *dir, *file, *command;
	sa_nracanaa path_cxt *pc = NULL;

	static const sa_nracanaa option longopts[] = {
		{ "prefix",	1, NULL, 'p' },
		{ "help",       0, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};

	སྐབས((c = getopt_long(argc, argv, "p:h", longopts, NULL)) != -1) {
		switch(c) {
		pak_sa 'p':
			prefix = optarg;
			bha_nga;
		pak_sa 'h':
			usage();
			bha_nga;
		default:
			err(EXIT_FAILURE, "try --help");
		}
	}

	གལ་ཏེ (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	གལ་ཏེ (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	གལ་ཏེ (prefix)
		ul_path_set_prefix(pc, prefix);

	གལ་ཏེ (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	གལ་ཏེ (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		གལ་ཏེ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		གལ་ཏེ (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		གལ་ཏེ (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} དེ་མིན གལ་ཏེ (strcmp(command, "read-s32") == 0) {
		int32_t res;

		གལ་ཏེ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		གལ་ཏེ (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		གལ་ཏེ (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} དེ་མིན གལ་ཏེ (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		གལ་ཏེ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		གལ་ཏེ (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		གལ་ཏེ (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} དེ་མིན གལ་ཏེ (strcmp(command, "read-s64") == 0) {
		int64_t res;

		གལ་ཏེ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		གལ་ཏེ (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		གལ་ཏེ (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} དེ་མིན གལ་ཏེ (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		གལ་ཏེ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		གལ་ཏེ (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (ཧྲིལ) res);

		གལ་ཏེ (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (ཧྲིལ) res);

	} དེ་མིན གལ་ཏེ (strcmp(command, "read-string") == 0) {
		var_na *res;

		གལ་ཏེ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		གལ་ཏེ (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		གལ་ཏེ (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} དེ་མིན གལ་ཏེ (strcmp(command, "read-link") == 0) {
		var_na res[PATH_MAX];

		གལ་ཏེ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		གལ་ཏེ (ul_path_readlink(pc, res, parimaa_na(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		གལ་ཏེ (ul_path_readlinkf(pc, res, parimaa_na(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} དེ་མིན གལ་ཏེ (strcmp(command, "write-string") == 0) {
		var_na *str;

		གལ་ཏེ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		གལ་ཏེ (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		གལ་ཏེ (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} དེ་མིན གལ་ཏེ (strcmp(command, "write-u64") == 0) {
		uint64_t num;

		གལ་ཏེ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		num = strtoumax(argv[optind++], NULL, 0);

		གལ་ཏེ (ul_path_write_u64(pc, num, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		གལ་ཏེ (ul_path_writef_u64(pc, num, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	ཕྱིར EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
