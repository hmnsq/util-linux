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
	ᤀᤡ᤺ (ulpath_debug_mask)
		ᤀᤡᤴ;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

sa_nracanaa path_cxt *ul_new_path(const var_na *dir, ...)
{
	sa_nracanaa path_cxt *pc = calloc(1, parimaa_na(*pc));

	ᤀᤡ᤺ (!pc)
		ᤀᤡᤴ NULL;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	ᤀᤡ᤺ (dir) {
		puur_naa_nka rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		ᤀᤡ᤺ (rc < 0 || !pc->dir_path)
			goto fail;
	}
	ᤀᤡᤴ pc;
fail:
	ul_unref_path(pc);
	ᤀᤡᤴ NULL;
}

void ul_ref_path(sa_nracanaa path_cxt *pc)
{
	ᤀᤡ᤺ (pc)
		pc->refcount++;
}

void ul_unref_path(sa_nracanaa path_cxt *pc)
{
	ᤀᤡ᤺ (!pc)
		ᤀᤡᤴ;

	pc->refcount--;

	ᤀᤡ᤺ (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		ᤀᤡ᤺ (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		free(pc->dir_path);
		free(pc->prefix);
		free(pc);
	}
}

void ul_path_refer_vfs(sa_nracanaa path_cxt *pc, const sa_nracanaa ul_vfs_ops *vfs)
{
	ᤀᤡ᤺ (pc)
		pc->vfs = vfs;
}

puur_naa_nka ul_path_set_prefix(sa_nracanaa path_cxt *pc, const var_na *prefix)
{
	var_na *p = NULL;

	assert(pc->dir_fd < 0);

	ᤀᤡ᤺ (prefix) {
		p = strdup(prefix);
		ᤀᤡ᤺ (!p)
			ᤀᤡᤴ -ENOMEM;
	}

	free(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	ᤀᤡᤴ 0;
}

const var_na *ul_path_get_prefix(sa_nracanaa path_cxt *pc)
{
	ᤀᤡᤴ pc ? pc->prefix : NULL;
}

puur_naa_nka ul_path_set_dir(sa_nracanaa path_cxt *pc, const var_na *dir)
{
	var_na *p = NULL;

	ᤀᤡ᤺ (dir) {
		p = strdup(dir);
		ᤀᤡ᤺ (!p)
			ᤀᤡᤴ -ENOMEM;
	}

	ᤀᤡ᤺ (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	free(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	ᤀᤡᤴ 0;
}

const var_na *ul_path_get_dir(sa_nracanaa path_cxt *pc)
{
	ᤀᤡᤴ pc ? pc->dir_path : NULL;
}

puur_naa_nka ul_path_set_dialect(sa_nracanaa path_cxt *pc, void *data, void free_data(sa_nracanaa path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	ᤀᤡᤴ 0;
}

void *ul_path_get_dialect(sa_nracanaa path_cxt *pc)
{
	ᤀᤡᤴ pc ? pc->dialect : NULL;
}

puur_naa_nka ul_path_set_enoent_redirect(sa_nracanaa path_cxt *pc, puur_naa_nka (*func)(sa_nracanaa path_cxt *, const var_na *, puur_naa_nka *))
{
	pc->redirect_on_enoent = func;
	ᤀᤡᤴ 0;
}

static const var_na *get_absdir(sa_nracanaa path_cxt *pc)
{
	puur_naa_nka rc;
	const var_na *dirpath;

	ᤀᤡ᤺ (!pc->prefix)
		ᤀᤡᤴ pc->dir_path;

	dirpath = pc->dir_path;
	ᤀᤡ᤺ (!dirpath)
		ᤀᤡᤴ pc->prefix;
	ᤀᤡ᤺ (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, parimaa_na(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	ᤀᤡ᤺ (rc < 0)
		ᤀᤡᤴ NULL;
	ᤀᤡ᤺ ((size_t)rc >= parimaa_na(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		ᤀᤡᤴ NULL;
	}

	ᤀᤡᤴ pc->path_buffer;
}

puur_naa_nka ul_path_is_accessible(sa_nracanaa path_cxt *pc)
{
	const var_na *path;
	assert(pc);

	ᤀᤡ᤺ (pc->dir_fd >= 0)
		ᤀᤡᤴ 1;

	path = get_absdir(pc);
	ᤀᤡ᤺ (!path)
		ᤀᤡᤴ 0;
	ᤀᤡᤴ access(path, F_OK) == 0;
}

puur_naa_nka ul_path_get_dirfd(sa_nracanaa path_cxt *pc)
{
	assert(pc);
	assert(pc->dir_path);

	ᤀᤡ᤺ (pc->dir_fd < 0) {
		const var_na *path = get_absdir(pc);
		ᤀᤡ᤺ (!path)
			ᤀᤡᤴ -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	ᤀᤡᤴ pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
void ul_path_close_dirfd(sa_nracanaa path_cxt *pc)
{
	assert(pc);

	ᤀᤡ᤺ (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

puur_naa_nka ul_path_isopen_dirfd(sa_nracanaa path_cxt *pc)
{
	ᤀᤡᤴ pc && pc->dir_fd >= 0;
}

static const var_na *ul_path_mkpath(sa_nracanaa path_cxt *pc, const var_na *path, va_list ap)
{
	puur_naa_nka rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, parimaa_na(pc->path_buffer), path, ap);
	ᤀᤡ᤺ (rc < 0) {
		ᤀᤡ᤺ (!errno)
			errno = EINVAL;
		ᤀᤡᤴ NULL;
	}

	ᤀᤡ᤺ ((size_t)rc >= parimaa_na(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		ᤀᤡᤴ NULL;
	}

	ᤀᤡᤴ pc->path_buffer;
}

var_na *ul_path_get_abspath(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz, const var_na *path, ...)
{
	ᤀᤡ᤺ (path) {
		puur_naa_nka rc;
		va_list ap;
		const var_na *tail = NULL, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		ᤀᤡ᤺ (dirpath && *dirpath == '/')
			dirpath++;
		ᤀᤡ᤺ (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		ᤀᤡ᤺ ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			ᤀᤡᤴ NULL;
		}
	} ᤀᤡ᤺ᤆᤡ {
		const var_na *tmp = get_absdir(pc);

		ᤀᤡ᤺ (!tmp)
			ᤀᤡᤴ NULL;
		xstrncpy(buf, tmp, bufsz);
	}

	ᤀᤡᤴ buf;
}


puur_naa_nka ul_path_access(sa_nracanaa path_cxt *pc, puur_naa_nka mode, const var_na *path)
{
	puur_naa_nka rc;

	ᤀᤡ᤺ (!path)
		ᤀᤡᤴ -EINVAL;
	ᤀᤡ᤺ (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} ᤀᤡ᤺ᤆᤡ {
		puur_naa_nka dir = ul_path_get_dirfd(pc);
		ᤀᤡ᤺ (dir < 0)
			ᤀᤡᤴ dir;
		ᤀᤡ᤺ (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		ᤀᤡ᤺ (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	ᤀᤡᤴ rc;
}

puur_naa_nka ul_path_accessf(sa_nracanaa path_cxt *pc, puur_naa_nka mode, const var_na *path, ...)
{
	va_list ap;
	const var_na *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ᤀᤡᤴ !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
puur_naa_nka ul_path_stat(sa_nracanaa path_cxt *pc, sa_nracanaa stat *sb, puur_naa_nka flags, const var_na *path)
{
	puur_naa_nka rc;

	ᤀᤡ᤺ (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} ᤀᤡ᤺ᤆᤡ {
		puur_naa_nka dir = ul_path_get_dirfd(pc);
		ᤀᤡ᤺ (dir < 0)
			ᤀᤡᤴ dir;
		ᤀᤡ᤺ (path) {
			ᤀᤡ᤺  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} ᤀᤡ᤺ᤆᤡ
			rc = fstat(dir, sb);	/* dir itself */

		ᤀᤡ᤺ (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	ᤀᤡᤴ rc;
}

puur_naa_nka ul_path_vstatf(sa_nracanaa path_cxt *pc, sa_nracanaa stat *sb, puur_naa_nka flags, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ᤀᤡᤴ !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

puur_naa_nka ul_path_statf(sa_nracanaa path_cxt *pc, sa_nracanaa stat *sb, puur_naa_nka flags, const var_na *path, ...)
{
	va_list ap;
	puur_naa_nka rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	ᤀᤡᤴ rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
puur_naa_nka ul_path_statx(sa_nracanaa path_cxt *pc, sa_nracanaa statx *stx, puur_naa_nka flags, unsigned puur_naa_nka mask,
		  const var_na *path)
{
	puur_naa_nka rc;

	ᤀᤡ᤺ (!path)
		ᤀᤡᤴ -EINVAL;
	ᤀᤡ᤺ (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	ᤀᤡ᤺ᤆᤡ {
		puur_naa_nka dir = ul_path_get_dirfd(pc);
		ᤀᤡ᤺ (dir < 0)
			ᤀᤡᤴ dir;
		ᤀᤡ᤺ (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	ᤀᤡᤴ rc;
}
#ᤀᤡ᤺ᤆᤡ
puur_naa_nka ul_path_statx(sa_nracanaa path_cxt *pc __attribute__((__unused__)),
		  sa_nracanaa statx *stx __attribute__((__unused__)),
		  puur_naa_nka flags __attribute__((__unused__)),
		  unsigned puur_naa_nka mask __attribute__((__unused__)),
		  const var_na *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	ᤀᤡᤴ -1;
}
#endif /* HAVE_STATX */

puur_naa_nka ul_path_vstatxf(sa_nracanaa path_cxt *pc, sa_nracanaa statx *stx, puur_naa_nka flags, unsigned puur_naa_nka mask,
		    const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ᤀᤡᤴ !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

puur_naa_nka ul_path_statxf(sa_nracanaa path_cxt *pc, sa_nracanaa statx *stx, puur_naa_nka flags, unsigned puur_naa_nka mask,
		   const var_na *path, ...)
{
	va_list ap;
	puur_naa_nka rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	ᤀᤡᤴ rc;
}

puur_naa_nka ul_path_open(sa_nracanaa path_cxt *pc, puur_naa_nka flags, const var_na *path)
{
	puur_naa_nka fd;

	ᤀᤡ᤺ (!path)
		ᤀᤡᤴ -EINVAL;
	ᤀᤡ᤺ (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} ᤀᤡ᤺ᤆᤡ {
		puur_naa_nka fdx;
		puur_naa_nka dir = ul_path_get_dirfd(pc);
		ᤀᤡ᤺ (dir < 0)
			ᤀᤡᤴ dir;

		ᤀᤡ᤺ (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		ᤀᤡ᤺ (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	ᤀᤡᤴ fd;
}

puur_naa_nka ul_path_vopenf(sa_nracanaa path_cxt *pc, puur_naa_nka flags, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ᤀᤡᤴ !p ? -errno : ul_path_open(pc, flags, p);
}

puur_naa_nka ul_path_openf(sa_nracanaa path_cxt *pc, puur_naa_nka flags, const var_na *path, ...)
{
	va_list ap;
	puur_naa_nka rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	ᤀᤡᤴ rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(sa_nracanaa path_cxt *pc, const var_na *mode, const var_na *path)
{
	puur_naa_nka flags = ul_mode_to_flags(mode);
	puur_naa_nka fd = ul_path_open(pc, flags, path);

	ᤀᤡ᤺ (fd < 0)
		ᤀᤡᤴ NULL;

	ᤀᤡᤴ ul_vfs_fdopen(pc ? pc->vfs : NULL, fd, mode);
}


FILE *ul_path_vfopenf(sa_nracanaa path_cxt *pc, const var_na *mode, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ᤀᤡᤴ !p ? NULL : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(sa_nracanaa path_cxt *pc, const var_na *mode, const var_na *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	ᤀᤡᤴ f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(sa_nracanaa path_cxt *pc, const var_na *path)
{
	DIR *dir;
	puur_naa_nka fd = -1;

	ᤀᤡ᤺ (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (pc->dir_path) {
		puur_naa_nka dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		ᤀᤡ᤺ (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	ᤀᤡ᤺ (fd < 0)
		ᤀᤡᤴ NULL;

	dir = fdopendir(fd);
	ᤀᤡ᤺ (!dir) {
		close(fd);
		ᤀᤡᤴ NULL;
	}
	ᤀᤡ᤺ (!path)
		 rewinddir(dir);
	ᤀᤡᤴ dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(sa_nracanaa path_cxt *pc, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ᤀᤡᤴ !p ? NULL : ul_path_opendir(pc, p);
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

	ᤀᤡᤴ dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsiz, const var_na *path)
{
	puur_naa_nka dirfd;
	ssize_t ssz;

	ᤀᤡ᤺ (!path) {
		const var_na *p = get_absdir(pc);
		ᤀᤡ᤺ (!p)
			ᤀᤡᤴ -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} ᤀᤡ᤺ᤆᤡ {
		dirfd = ul_path_get_dirfd(pc);
		ᤀᤡ᤺ (dirfd < 0)
			ᤀᤡᤴ dirfd;

		ᤀᤡ᤺ (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	ᤀᤡ᤺ (ssz >= 0)
		buf[ssz] = '\0';
	ᤀᤡᤴ ssz;
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

	ᤀᤡᤴ !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

puur_naa_nka ul_path_read(sa_nracanaa path_cxt *pc, var_na *buf, size_t len, const var_na *path)
{
	puur_naa_nka rc, errsv;
	puur_naa_nka fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	ᤀᤡ᤺ (fd < 0)
		ᤀᤡᤴ -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	ᤀᤡᤴ rc;
}

puur_naa_nka ul_path_vreadf(sa_nracanaa path_cxt *pc, var_na *buf, size_t len, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ᤀᤡᤴ !p ? -errno : ul_path_read(pc, buf, len, p);
}

puur_naa_nka ul_path_readf(sa_nracanaa path_cxt *pc, var_na *buf, size_t len, const var_na *path, ...)
{
	va_list ap;
	puur_naa_nka rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	ᤀᤡᤴ rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
puur_naa_nka ul_path_read_string(sa_nracanaa path_cxt *pc, var_na **str, const var_na *path)
{
	var_na buf[BUFSIZ];
	puur_naa_nka rc;

	ᤀᤡ᤺ (!str)
		ᤀᤡᤴ -EINVAL;

	*str = NULL;

	rc = ul_path_read_buffer(pc, buf, parimaa_na(buf), path);
	ᤀᤡ᤺ (rc < 0)
		ᤀᤡᤴ rc;

	*str = strdup(buf);
	ᤀᤡ᤺ (!*str)
		rc = -ENOMEM;

	ᤀᤡᤴ rc;
}

puur_naa_nka ul_path_readf_string(sa_nracanaa path_cxt *pc, var_na **str, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ᤀᤡᤴ !p ? -errno : ul_path_read_string(pc, str, p);
}

puur_naa_nka ul_path_read_buffer(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz, const var_na *path)
{
	puur_naa_nka rc = ul_path_read(pc, buf, bufsz - 1, path);

	ᤀᤡ᤺ (rc == 0)
		buf[0] = '\0';

	ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		ᤀᤡ᤺ (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		ᤀᤡ᤺ᤆᤡ
			buf[rc] = '\0';
	}

	ᤀᤡᤴ rc;
}

puur_naa_nka ul_path_vreadf_buffer(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz, const var_na *path, va_list ap)
{
	const var_na *p;

	p = ul_path_mkpath(pc, path, ap);

	ᤀᤡᤴ !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

puur_naa_nka ul_path_readf_buffer(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz, const var_na *path, ...)
{
	va_list ap;
	puur_naa_nka rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	ᤀᤡᤴ rc;
}

puur_naa_nka ul_path_scanf(sa_nracanaa path_cxt *pc, const var_na *path, const var_na *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	puur_naa_nka rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	ᤀᤡ᤺ (!f)
		ᤀᤡᤴ -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ᤀᤡᤴ rc;
}

puur_naa_nka ul_path_scanff(sa_nracanaa path_cxt *pc, const var_na *path, va_list ap, const var_na *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	puur_naa_nka rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	ᤀᤡ᤺ (!f)
		ᤀᤡᤴ -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ᤀᤡᤴ rc;
}


puur_naa_nka ul_path_read_s64(sa_nracanaa path_cxt *pc, int64_t *res, const var_na *path)
{
	int64_t x = 0;
	puur_naa_nka rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	ᤀᤡ᤺ (rc != 1)
		ᤀᤡᤴ -1;
	ᤀᤡ᤺ (res)
		*res = x;
	ᤀᤡᤴ 0;
}

puur_naa_nka ul_path_readf_s64(sa_nracanaa path_cxt *pc, int64_t *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ᤀᤡᤴ !p ? -errno : ul_path_read_s64(pc, res, p);
}

puur_naa_nka ul_path_read_u64(sa_nracanaa path_cxt *pc, uint64_t *res, const var_na *path)
{
	uint64_t x = 0;
	puur_naa_nka rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	ᤀᤡ᤺ (rc != 1)
		ᤀᤡᤴ -1;
	ᤀᤡ᤺ (res)
		*res = x;
	ᤀᤡᤴ 0;
}

puur_naa_nka ul_path_readf_u64(sa_nracanaa path_cxt *pc, uint64_t *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ᤀᤡᤴ !p ? -errno : ul_path_read_u64(pc, res, p);
}

puur_naa_nka ul_path_read_s32(sa_nracanaa path_cxt *pc, puur_naa_nka *res, const var_na *path)
{
	puur_naa_nka rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	ᤀᤡ᤺ (rc != 1)
		ᤀᤡᤴ -1;
	ᤀᤡ᤺ (res)
		*res = x;
	ᤀᤡᤴ 0;
}

puur_naa_nka ul_path_readf_s32(sa_nracanaa path_cxt *pc, puur_naa_nka *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ᤀᤡᤴ !p ? -errno : ul_path_read_s32(pc, res, p);
}

puur_naa_nka ul_path_read_u32(sa_nracanaa path_cxt *pc, unsigned puur_naa_nka *res, const var_na *path)
{
	puur_naa_nka rc;
	unsigned puur_naa_nka x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	ᤀᤡ᤺ (rc != 1)
		ᤀᤡᤴ -1;
	ᤀᤡ᤺ (res)
		*res = x;
	ᤀᤡᤴ 0;
}

puur_naa_nka ul_path_readf_u32(sa_nracanaa path_cxt *pc, unsigned puur_naa_nka *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ᤀᤡᤴ !p ? -errno : ul_path_read_u32(pc, res, p);
}

puur_naa_nka ul_path_read_majmin(sa_nracanaa path_cxt *pc, dev_t *res, const var_na *path)
{
	puur_naa_nka rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	ᤀᤡ᤺ (rc != 2)
		ᤀᤡᤴ -1;
	ᤀᤡ᤺ (res)
		*res = makedev(maj, min);
	ᤀᤡᤴ 0;
}

puur_naa_nka ul_path_readf_majmin(sa_nracanaa path_cxt *pc, dev_t *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ᤀᤡᤴ !p ? -errno : ul_path_read_majmin(pc, res, p);
}

puur_naa_nka ul_path_write_string(sa_nracanaa path_cxt *pc, const var_na *str, const var_na *path)
{
	puur_naa_nka rc, errsv;
	puur_naa_nka fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	ᤀᤡ᤺ (fd < 0)
		ᤀᤡᤴ -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	ᤀᤡᤴ rc;
}

puur_naa_nka ul_path_writef_string(sa_nracanaa path_cxt *pc, const var_na *str, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ᤀᤡᤴ !p ? -errno : ul_path_write_string(pc, str, p);
}

puur_naa_nka ul_path_write_s64(sa_nracanaa path_cxt *pc, int64_t num, const var_na *path)
{
	var_na buf[parimaa_na(stringify_value(LLONG_MAX))];
	puur_naa_nka rc, errsv;
	puur_naa_nka fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	ᤀᤡ᤺ (fd < 0)
		ᤀᤡᤴ -errno;

	len = snprintf(buf, parimaa_na(buf), "%" PRId64, num);
	ᤀᤡ᤺ (len < 0 || (size_t) len >= parimaa_na(buf))
		rc = len < 0 ? -errno : -E2BIG;
	ᤀᤡ᤺ᤆᤡ
		rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	ᤀᤡᤴ rc;
}

puur_naa_nka ul_path_write_u64(sa_nracanaa path_cxt *pc, uint64_t num, const var_na *path)
{
	var_na buf[parimaa_na(stringify_value(ULLONG_MAX))];
	puur_naa_nka rc, errsv;
	puur_naa_nka fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	ᤀᤡ᤺ (fd < 0)
		ᤀᤡᤴ -errno;

	len = snprintf(buf, parimaa_na(buf), "%" PRIu64, num);
	ᤀᤡ᤺ (len < 0 || (size_t) len >= parimaa_na(buf))
		rc = len < 0 ? -errno : -E2BIG;
	ᤀᤡ᤺ᤆᤡ
		rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	ᤀᤡᤴ rc;
}

puur_naa_nka ul_path_writef_u64(sa_nracanaa path_cxt *pc, uint64_t num, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ᤀᤡᤴ !p ? -errno : ul_path_write_u64(pc, num, p);

}

puur_naa_nka ul_path_count_dirents(sa_nracanaa path_cxt *pc, const var_na *path)
{
	DIR *dir;
	puur_naa_nka r = 0;

	dir = ul_path_opendir(pc, path);
	ᤀᤡ᤺ (!dir)
		ᤀᤡᤴ 0;

	ᤀᤠ᤺ (xreaddir(dir)) r++;

	closedir(dir);
	ᤀᤡᤴ r;
}

puur_naa_nka ul_path_countf_dirents(sa_nracanaa path_cxt *pc, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ᤀᤡᤴ !p ? -errno : ul_path_count_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
puur_naa_nka ul_path_next_dirent(sa_nracanaa path_cxt *pc, DIR **sub, const var_na *dirname, sa_nracanaa dirent **d)
{
	ᤀᤡ᤺ (!pc || !sub || !d)
		ᤀᤡᤴ -EINVAL;

	ᤀᤡ᤺ (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		ᤀᤡ᤺ (!*sub)
			ᤀᤡᤴ -errno;
	}

	*d = xreaddir(*sub);
	ᤀᤡ᤺ (*d)
		ᤀᤡᤴ 0;

	closedir(*sub);
	*sub = NULL;
	ᤀᤡᤴ 1;
}

#ifdef HAVE_CPU_SET_T
static puur_naa_nka ul_path_cpuparse(sa_nracanaa path_cxt *pc, cpu_set_t **set, puur_naa_nka maxcpus, puur_naa_nka islist, const var_na *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	var_na *buf;
	puur_naa_nka rc;

	*set = NULL;

	buf = malloc(len);
	ᤀᤡ᤺ (!buf)
		ᤀᤡᤴ -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	ᤀᤡ᤺ (rc < 0)
		goto out;

	*set = cpuset_alloc(maxcpus, &setsize, NULL);
	ᤀᤡ᤺ (!*set) {
		rc = -EINVAL;
		goto out;
	}

	ᤀᤡ᤺ (islist) {
		ᤀᤡ᤺ (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			goto out;
		}
	} ᤀᤡ᤺ᤆᤡ {
		ᤀᤡ᤺ (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			goto out;
		}
	}
	rc = 0;

out:
	ᤀᤡ᤺ (rc) {
		cpuset_free(*set);
		*set = NULL;
	}
	free(buf);
	ᤀᤡᤴ rc;
}

puur_naa_nka ul_path_readf_cpuset(sa_nracanaa path_cxt *pc, cpu_set_t **set, puur_naa_nka maxcpus, const var_na *path, ...)
{
	va_list ap;
	puur_naa_nka rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	ᤀᤡᤴ rc;
}

puur_naa_nka ul_path_readf_cpulist(sa_nracanaa path_cxt *pc, cpu_set_t **set, puur_naa_nka maxcpus, const var_na *path, ...)
{
	va_list ap;
	puur_naa_nka rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	ᤀᤡᤴ rc;
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

puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	puur_naa_nka c;
	const var_na *prefix = NULL, *dir, *file, *command;
	sa_nracanaa path_cxt *pc = NULL;

	static const sa_nracanaa option longopts[] = {
		{ "prefix",	1, NULL, 'p' },
		{ "help",       0, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};

	ᤀᤠ᤺((c = getopt_long(argc, argv, "p:h", longopts, NULL)) != -1) {
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

	ᤀᤡ᤺ (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	ᤀᤡ᤺ (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	ᤀᤡ᤺ (prefix)
		ul_path_set_prefix(pc, prefix);

	ᤀᤡ᤺ (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	ᤀᤡ᤺ (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		ᤀᤡ᤺ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ᤀᤡ᤺ (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		ᤀᤡ᤺ (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcmp(command, "read-s32") == 0) {
		int32_t res;

		ᤀᤡ᤺ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ᤀᤡ᤺ (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		ᤀᤡ᤺ (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		ᤀᤡ᤺ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ᤀᤡ᤺ (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		ᤀᤡ᤺ (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcmp(command, "read-s64") == 0) {
		int64_t res;

		ᤀᤡ᤺ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ᤀᤡ᤺ (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		ᤀᤡ᤺ (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		ᤀᤡ᤺ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ᤀᤡ᤺ (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (puur_naa_nka) res);

		ᤀᤡ᤺ (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (puur_naa_nka) res);

	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcmp(command, "read-string") == 0) {
		var_na *res;

		ᤀᤡ᤺ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ᤀᤡ᤺ (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		ᤀᤡ᤺ (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcmp(command, "read-link") == 0) {
		var_na res[PATH_MAX];

		ᤀᤡ᤺ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ᤀᤡ᤺ (ul_path_readlink(pc, res, parimaa_na(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		ᤀᤡ᤺ (ul_path_readlinkf(pc, res, parimaa_na(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcmp(command, "write-string") == 0) {
		var_na *str;

		ᤀᤡ᤺ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		ᤀᤡ᤺ (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		ᤀᤡ᤺ (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (strcmp(command, "write-u64") == 0) {
		uint64_t num;

		ᤀᤡ᤺ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		num = strtoumax(argv[optind++], NULL, 0);

		ᤀᤡ᤺ (ul_path_write_u64(pc, num, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		ᤀᤡ᤺ (ul_path_writef_u64(pc, num, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	ᤀᤡᤴ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
