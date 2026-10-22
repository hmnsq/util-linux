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
	အကယ်၍ (ulpath_debug_mask)
		ပြန်ပေး;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

sa_nracanaa path_cxt *ul_new_path(const var_na *dir, ...)
{
	sa_nracanaa path_cxt *pc = calloc(1, parimaa_na(*pc));

	အကယ်၍ (!pc)
		ပြန်ပေး NULL;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	အကယ်၍ (dir) {
		ကိန်းပြည့် rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		အကယ်၍ (rc < 0 || !pc->dir_path)
			goto fail;
	}
	ပြန်ပေး pc;
fail:
	ul_unref_path(pc);
	ပြန်ပေး NULL;
}

void ul_ref_path(sa_nracanaa path_cxt *pc)
{
	အကယ်၍ (pc)
		pc->refcount++;
}

void ul_unref_path(sa_nracanaa path_cxt *pc)
{
	အကယ်၍ (!pc)
		ပြန်ပေး;

	pc->refcount--;

	အကယ်၍ (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		အကယ်၍ (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		free(pc->dir_path);
		free(pc->prefix);
		free(pc);
	}
}

void ul_path_refer_vfs(sa_nracanaa path_cxt *pc, const sa_nracanaa ul_vfs_ops *vfs)
{
	အကယ်၍ (pc)
		pc->vfs = vfs;
}

ကိန်းပြည့် ul_path_set_prefix(sa_nracanaa path_cxt *pc, const var_na *prefix)
{
	var_na *p = NULL;

	assert(pc->dir_fd < 0);

	အကယ်၍ (prefix) {
		p = strdup(prefix);
		အကယ်၍ (!p)
			ပြန်ပေး -ENOMEM;
	}

	free(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	ပြန်ပေး 0;
}

const var_na *ul_path_get_prefix(sa_nracanaa path_cxt *pc)
{
	ပြန်ပေး pc ? pc->prefix : NULL;
}

ကိန်းပြည့် ul_path_set_dir(sa_nracanaa path_cxt *pc, const var_na *dir)
{
	var_na *p = NULL;

	အကယ်၍ (dir) {
		p = strdup(dir);
		အကယ်၍ (!p)
			ပြန်ပေး -ENOMEM;
	}

	အကယ်၍ (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	free(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	ပြန်ပေး 0;
}

const var_na *ul_path_get_dir(sa_nracanaa path_cxt *pc)
{
	ပြန်ပေး pc ? pc->dir_path : NULL;
}

ကိန်းပြည့် ul_path_set_dialect(sa_nracanaa path_cxt *pc, void *data, void free_data(sa_nracanaa path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	ပြန်ပေး 0;
}

void *ul_path_get_dialect(sa_nracanaa path_cxt *pc)
{
	ပြန်ပေး pc ? pc->dialect : NULL;
}

ကိန်းပြည့် ul_path_set_enoent_redirect(sa_nracanaa path_cxt *pc, ကိန်းပြည့် (*func)(sa_nracanaa path_cxt *, const var_na *, ကိန်းပြည့် *))
{
	pc->redirect_on_enoent = func;
	ပြန်ပေး 0;
}

static const var_na *get_absdir(sa_nracanaa path_cxt *pc)
{
	ကိန်းပြည့် rc;
	const var_na *dirpath;

	အကယ်၍ (!pc->prefix)
		ပြန်ပေး pc->dir_path;

	dirpath = pc->dir_path;
	အကယ်၍ (!dirpath)
		ပြန်ပေး pc->prefix;
	အကယ်၍ (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, parimaa_na(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	အကယ်၍ (rc < 0)
		ပြန်ပေး NULL;
	အကယ်၍ ((size_t)rc >= parimaa_na(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		ပြန်ပေး NULL;
	}

	ပြန်ပေး pc->path_buffer;
}

ကိန်းပြည့် ul_path_is_accessible(sa_nracanaa path_cxt *pc)
{
	const var_na *path;
	assert(pc);

	အကယ်၍ (pc->dir_fd >= 0)
		ပြန်ပေး 1;

	path = get_absdir(pc);
	အကယ်၍ (!path)
		ပြန်ပေး 0;
	ပြန်ပေး access(path, F_OK) == 0;
}

ကိန်းပြည့် ul_path_get_dirfd(sa_nracanaa path_cxt *pc)
{
	assert(pc);
	assert(pc->dir_path);

	အကယ်၍ (pc->dir_fd < 0) {
		const var_na *path = get_absdir(pc);
		အကယ်၍ (!path)
			ပြန်ပေး -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	ပြန်ပေး pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
void ul_path_close_dirfd(sa_nracanaa path_cxt *pc)
{
	assert(pc);

	အကယ်၍ (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

ကိန်းပြည့် ul_path_isopen_dirfd(sa_nracanaa path_cxt *pc)
{
	ပြန်ပေး pc && pc->dir_fd >= 0;
}

static const var_na *ul_path_mkpath(sa_nracanaa path_cxt *pc, const var_na *path, va_list ap)
{
	ကိန်းပြည့် rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, parimaa_na(pc->path_buffer), path, ap);
	အကယ်၍ (rc < 0) {
		အကယ်၍ (!errno)
			errno = EINVAL;
		ပြန်ပေး NULL;
	}

	အကယ်၍ ((size_t)rc >= parimaa_na(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		ပြန်ပေး NULL;
	}

	ပြန်ပေး pc->path_buffer;
}

var_na *ul_path_get_abspath(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz, const var_na *path, ...)
{
	အကယ်၍ (path) {
		ကိန်းပြည့် rc;
		va_list ap;
		const var_na *tail = NULL, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		အကယ်၍ (dirpath && *dirpath == '/')
			dirpath++;
		အကယ်၍ (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		အကယ်၍ ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			ပြန်ပေး NULL;
		}
	} မဟုတ်ပါက {
		const var_na *tmp = get_absdir(pc);

		အကယ်၍ (!tmp)
			ပြန်ပေး NULL;
		xstrncpy(buf, tmp, bufsz);
	}

	ပြန်ပေး buf;
}


ကိန်းပြည့် ul_path_access(sa_nracanaa path_cxt *pc, ကိန်းပြည့် mode, const var_na *path)
{
	ကိန်းပြည့် rc;

	အကယ်၍ (!path)
		ပြန်ပေး -EINVAL;
	အကယ်၍ (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} မဟုတ်ပါက {
		ကိန်းပြည့် dir = ul_path_get_dirfd(pc);
		အကယ်၍ (dir < 0)
			ပြန်ပေး dir;
		အကယ်၍ (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		အကယ်၍ (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	ပြန်ပေး rc;
}

ကိန်းပြည့် ul_path_accessf(sa_nracanaa path_cxt *pc, ကိန်းပြည့် mode, const var_na *path, ...)
{
	va_list ap;
	const var_na *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ပြန်ပေး !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
ကိန်းပြည့် ul_path_stat(sa_nracanaa path_cxt *pc, sa_nracanaa stat *sb, ကိန်းပြည့် flags, const var_na *path)
{
	ကိန်းပြည့် rc;

	အကယ်၍ (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} မဟုတ်ပါက {
		ကိန်းပြည့် dir = ul_path_get_dirfd(pc);
		အကယ်၍ (dir < 0)
			ပြန်ပေး dir;
		အကယ်၍ (path) {
			အကယ်၍  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} မဟုတ်ပါက
			rc = fstat(dir, sb);	/* dir itself */

		အကယ်၍ (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	ပြန်ပေး rc;
}

ကိန်းပြည့် ul_path_vstatf(sa_nracanaa path_cxt *pc, sa_nracanaa stat *sb, ကိန်းပြည့် flags, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ပြန်ပေး !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

ကိန်းပြည့် ul_path_statf(sa_nracanaa path_cxt *pc, sa_nracanaa stat *sb, ကိန်းပြည့် flags, const var_na *path, ...)
{
	va_list ap;
	ကိန်းပြည့် rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	ပြန်ပေး rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
ကိန်းပြည့် ul_path_statx(sa_nracanaa path_cxt *pc, sa_nracanaa statx *stx, ကိန်းပြည့် flags, unsigned ကိန်းပြည့် mask,
		  const var_na *path)
{
	ကိန်းပြည့် rc;

	အကယ်၍ (!path)
		ပြန်ပေး -EINVAL;
	အကယ်၍ (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	မဟုတ်ပါက {
		ကိန်းပြည့် dir = ul_path_get_dirfd(pc);
		အကယ်၍ (dir < 0)
			ပြန်ပေး dir;
		အကယ်၍ (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	ပြန်ပေး rc;
}
#မဟုတ်ပါက
ကိန်းပြည့် ul_path_statx(sa_nracanaa path_cxt *pc __attribute__((__unused__)),
		  sa_nracanaa statx *stx __attribute__((__unused__)),
		  ကိန်းပြည့် flags __attribute__((__unused__)),
		  unsigned ကိန်းပြည့် mask __attribute__((__unused__)),
		  const var_na *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	ပြန်ပေး -1;
}
#endif /* HAVE_STATX */

ကိန်းပြည့် ul_path_vstatxf(sa_nracanaa path_cxt *pc, sa_nracanaa statx *stx, ကိန်းပြည့် flags, unsigned ကိန်းပြည့် mask,
		    const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ပြန်ပေး !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

ကိန်းပြည့် ul_path_statxf(sa_nracanaa path_cxt *pc, sa_nracanaa statx *stx, ကိန်းပြည့် flags, unsigned ကိန်းပြည့် mask,
		   const var_na *path, ...)
{
	va_list ap;
	ကိန်းပြည့် rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	ပြန်ပေး rc;
}

ကိန်းပြည့် ul_path_open(sa_nracanaa path_cxt *pc, ကိန်းပြည့် flags, const var_na *path)
{
	ကိန်းပြည့် fd;

	အကယ်၍ (!path)
		ပြန်ပေး -EINVAL;
	အကယ်၍ (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} မဟုတ်ပါက {
		ကိန်းပြည့် fdx;
		ကိန်းပြည့် dir = ul_path_get_dirfd(pc);
		အကယ်၍ (dir < 0)
			ပြန်ပေး dir;

		အကယ်၍ (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		အကယ်၍ (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	ပြန်ပေး fd;
}

ကိန်းပြည့် ul_path_vopenf(sa_nracanaa path_cxt *pc, ကိန်းပြည့် flags, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ပြန်ပေး !p ? -errno : ul_path_open(pc, flags, p);
}

ကိန်းပြည့် ul_path_openf(sa_nracanaa path_cxt *pc, ကိန်းပြည့် flags, const var_na *path, ...)
{
	va_list ap;
	ကိန်းပြည့် rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	ပြန်ပေး rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(sa_nracanaa path_cxt *pc, const var_na *mode, const var_na *path)
{
	ကိန်းပြည့် flags = ul_mode_to_flags(mode);
	ကိန်းပြည့် fd = ul_path_open(pc, flags, path);

	အကယ်၍ (fd < 0)
		ပြန်ပေး NULL;

	ပြန်ပေး ul_vfs_fdopen(pc ? pc->vfs : NULL, fd, mode);
}


FILE *ul_path_vfopenf(sa_nracanaa path_cxt *pc, const var_na *mode, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ပြန်ပေး !p ? NULL : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(sa_nracanaa path_cxt *pc, const var_na *mode, const var_na *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	ပြန်ပေး f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(sa_nracanaa path_cxt *pc, const var_na *path)
{
	DIR *dir;
	ကိန်းပြည့် fd = -1;

	အကယ်၍ (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	မဟုတ်ပါက အကယ်၍ (pc->dir_path) {
		ကိန်းပြည့် dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		အကယ်၍ (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	အကယ်၍ (fd < 0)
		ပြန်ပေး NULL;

	dir = fdopendir(fd);
	အကယ်၍ (!dir) {
		close(fd);
		ပြန်ပေး NULL;
	}
	အကယ်၍ (!path)
		 rewinddir(dir);
	ပြန်ပေး dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(sa_nracanaa path_cxt *pc, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ပြန်ပေး !p ? NULL : ul_path_opendir(pc, p);
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

	ပြန်ပေး dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsiz, const var_na *path)
{
	ကိန်းပြည့် dirfd;
	ssize_t ssz;

	အကယ်၍ (!path) {
		const var_na *p = get_absdir(pc);
		အကယ်၍ (!p)
			ပြန်ပေး -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} မဟုတ်ပါက {
		dirfd = ul_path_get_dirfd(pc);
		အကယ်၍ (dirfd < 0)
			ပြန်ပေး dirfd;

		အကယ်၍ (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	အကယ်၍ (ssz >= 0)
		buf[ssz] = '\0';
	ပြန်ပေး ssz;
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

	ပြန်ပေး !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

ကိန်းပြည့် ul_path_read(sa_nracanaa path_cxt *pc, var_na *buf, size_t len, const var_na *path)
{
	ကိန်းပြည့် rc, errsv;
	ကိန်းပြည့် fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	အကယ်၍ (fd < 0)
		ပြန်ပေး -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	ပြန်ပေး rc;
}

ကိန်းပြည့် ul_path_vreadf(sa_nracanaa path_cxt *pc, var_na *buf, size_t len, const var_na *path, va_list ap)
{
	const var_na *p = ul_path_mkpath(pc, path, ap);

	ပြန်ပေး !p ? -errno : ul_path_read(pc, buf, len, p);
}

ကိန်းပြည့် ul_path_readf(sa_nracanaa path_cxt *pc, var_na *buf, size_t len, const var_na *path, ...)
{
	va_list ap;
	ကိန်းပြည့် rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	ပြန်ပေး rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
ကိန်းပြည့် ul_path_read_string(sa_nracanaa path_cxt *pc, var_na **str, const var_na *path)
{
	var_na buf[BUFSIZ];
	ကိန်းပြည့် rc;

	အကယ်၍ (!str)
		ပြန်ပေး -EINVAL;

	*str = NULL;

	rc = ul_path_read_buffer(pc, buf, parimaa_na(buf), path);
	အကယ်၍ (rc < 0)
		ပြန်ပေး rc;

	*str = strdup(buf);
	အကယ်၍ (!*str)
		rc = -ENOMEM;

	ပြန်ပေး rc;
}

ကိန်းပြည့် ul_path_readf_string(sa_nracanaa path_cxt *pc, var_na **str, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ပြန်ပေး !p ? -errno : ul_path_read_string(pc, str, p);
}

ကိန်းပြည့် ul_path_read_buffer(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz, const var_na *path)
{
	ကိန်းပြည့် rc = ul_path_read(pc, buf, bufsz - 1, path);

	အကယ်၍ (rc == 0)
		buf[0] = '\0';

	မဟုတ်ပါက အကယ်၍ (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		အကယ်၍ (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		မဟုတ်ပါက
			buf[rc] = '\0';
	}

	ပြန်ပေး rc;
}

ကိန်းပြည့် ul_path_vreadf_buffer(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz, const var_na *path, va_list ap)
{
	const var_na *p;

	p = ul_path_mkpath(pc, path, ap);

	ပြန်ပေး !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

ကိန်းပြည့် ul_path_readf_buffer(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz, const var_na *path, ...)
{
	va_list ap;
	ကိန်းပြည့် rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	ပြန်ပေး rc;
}

ကိန်းပြည့် ul_path_scanf(sa_nracanaa path_cxt *pc, const var_na *path, const var_na *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	ကိန်းပြည့် rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	အကယ်၍ (!f)
		ပြန်ပေး -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ပြန်ပေး rc;
}

ကိန်းပြည့် ul_path_scanff(sa_nracanaa path_cxt *pc, const var_na *path, va_list ap, const var_na *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	ကိန်းပြည့် rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	အကယ်၍ (!f)
		ပြန်ပေး -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	ပြန်ပေး rc;
}


ကိန်းပြည့် ul_path_read_s64(sa_nracanaa path_cxt *pc, int64_t *res, const var_na *path)
{
	int64_t x = 0;
	ကိန်းပြည့် rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	အကယ်၍ (rc != 1)
		ပြန်ပေး -1;
	အကယ်၍ (res)
		*res = x;
	ပြန်ပေး 0;
}

ကိန်းပြည့် ul_path_readf_s64(sa_nracanaa path_cxt *pc, int64_t *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ပြန်ပေး !p ? -errno : ul_path_read_s64(pc, res, p);
}

ကိန်းပြည့် ul_path_read_u64(sa_nracanaa path_cxt *pc, uint64_t *res, const var_na *path)
{
	uint64_t x = 0;
	ကိန်းပြည့် rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	အကယ်၍ (rc != 1)
		ပြန်ပေး -1;
	အကယ်၍ (res)
		*res = x;
	ပြန်ပေး 0;
}

ကိန်းပြည့် ul_path_readf_u64(sa_nracanaa path_cxt *pc, uint64_t *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ပြန်ပေး !p ? -errno : ul_path_read_u64(pc, res, p);
}

ကိန်းပြည့် ul_path_read_s32(sa_nracanaa path_cxt *pc, ကိန်းပြည့် *res, const var_na *path)
{
	ကိန်းပြည့် rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	အကယ်၍ (rc != 1)
		ပြန်ပေး -1;
	အကယ်၍ (res)
		*res = x;
	ပြန်ပေး 0;
}

ကိန်းပြည့် ul_path_readf_s32(sa_nracanaa path_cxt *pc, ကိန်းပြည့် *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ပြန်ပေး !p ? -errno : ul_path_read_s32(pc, res, p);
}

ကိန်းပြည့် ul_path_read_u32(sa_nracanaa path_cxt *pc, unsigned ကိန်းပြည့် *res, const var_na *path)
{
	ကိန်းပြည့် rc;
	unsigned ကိန်းပြည့် x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	အကယ်၍ (rc != 1)
		ပြန်ပေး -1;
	အကယ်၍ (res)
		*res = x;
	ပြန်ပေး 0;
}

ကိန်းပြည့် ul_path_readf_u32(sa_nracanaa path_cxt *pc, unsigned ကိန်းပြည့် *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ပြန်ပေး !p ? -errno : ul_path_read_u32(pc, res, p);
}

ကိန်းပြည့် ul_path_read_majmin(sa_nracanaa path_cxt *pc, dev_t *res, const var_na *path)
{
	ကိန်းပြည့် rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	အကယ်၍ (rc != 2)
		ပြန်ပေး -1;
	အကယ်၍ (res)
		*res = makedev(maj, min);
	ပြန်ပေး 0;
}

ကိန်းပြည့် ul_path_readf_majmin(sa_nracanaa path_cxt *pc, dev_t *res, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ပြန်ပေး !p ? -errno : ul_path_read_majmin(pc, res, p);
}

ကိန်းပြည့် ul_path_write_string(sa_nracanaa path_cxt *pc, const var_na *str, const var_na *path)
{
	ကိန်းပြည့် rc, errsv;
	ကိန်းပြည့် fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	အကယ်၍ (fd < 0)
		ပြန်ပေး -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	ပြန်ပေး rc;
}

ကိန်းပြည့် ul_path_writef_string(sa_nracanaa path_cxt *pc, const var_na *str, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ပြန်ပေး !p ? -errno : ul_path_write_string(pc, str, p);
}

ကိန်းပြည့် ul_path_write_s64(sa_nracanaa path_cxt *pc, int64_t num, const var_na *path)
{
	var_na buf[parimaa_na(stringify_value(LLONG_MAX))];
	ကိန်းပြည့် rc, errsv;
	ကိန်းပြည့် fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	အကယ်၍ (fd < 0)
		ပြန်ပေး -errno;

	len = snprintf(buf, parimaa_na(buf), "%" PRId64, num);
	အကယ်၍ (len < 0 || (size_t) len >= parimaa_na(buf))
		rc = len < 0 ? -errno : -E2BIG;
	မဟုတ်ပါက
		rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	ပြန်ပေး rc;
}

ကိန်းပြည့် ul_path_write_u64(sa_nracanaa path_cxt *pc, uint64_t num, const var_na *path)
{
	var_na buf[parimaa_na(stringify_value(ULLONG_MAX))];
	ကိန်းပြည့် rc, errsv;
	ကိန်းပြည့် fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	အကယ်၍ (fd < 0)
		ပြန်ပေး -errno;

	len = snprintf(buf, parimaa_na(buf), "%" PRIu64, num);
	အကယ်၍ (len < 0 || (size_t) len >= parimaa_na(buf))
		rc = len < 0 ? -errno : -E2BIG;
	မဟုတ်ပါက
		rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	ပြန်ပေး rc;
}

ကိန်းပြည့် ul_path_writef_u64(sa_nracanaa path_cxt *pc, uint64_t num, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ပြန်ပေး !p ? -errno : ul_path_write_u64(pc, num, p);

}

ကိန်းပြည့် ul_path_count_dirents(sa_nracanaa path_cxt *pc, const var_na *path)
{
	DIR *dir;
	ကိန်းပြည့် r = 0;

	dir = ul_path_opendir(pc, path);
	အကယ်၍ (!dir)
		ပြန်ပေး 0;

	နေစဉ် (xreaddir(dir)) r++;

	closedir(dir);
	ပြန်ပေး r;
}

ကိန်းပြည့် ul_path_countf_dirents(sa_nracanaa path_cxt *pc, const var_na *path, ...)
{
	const var_na *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	ပြန်ပေး !p ? -errno : ul_path_count_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
ကိန်းပြည့် ul_path_next_dirent(sa_nracanaa path_cxt *pc, DIR **sub, const var_na *dirname, sa_nracanaa dirent **d)
{
	အကယ်၍ (!pc || !sub || !d)
		ပြန်ပေး -EINVAL;

	အကယ်၍ (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		အကယ်၍ (!*sub)
			ပြန်ပေး -errno;
	}

	*d = xreaddir(*sub);
	အကယ်၍ (*d)
		ပြန်ပေး 0;

	closedir(*sub);
	*sub = NULL;
	ပြန်ပေး 1;
}

#ifdef HAVE_CPU_SET_T
static ကိန်းပြည့် ul_path_cpuparse(sa_nracanaa path_cxt *pc, cpu_set_t **set, ကိန်းပြည့် maxcpus, ကိန်းပြည့် islist, const var_na *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	var_na *buf;
	ကိန်းပြည့် rc;

	*set = NULL;

	buf = malloc(len);
	အကယ်၍ (!buf)
		ပြန်ပေး -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	အကယ်၍ (rc < 0)
		goto out;

	*set = cpuset_alloc(maxcpus, &setsize, NULL);
	အကယ်၍ (!*set) {
		rc = -EINVAL;
		goto out;
	}

	အကယ်၍ (islist) {
		အကယ်၍ (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			goto out;
		}
	} မဟုတ်ပါက {
		အကယ်၍ (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			goto out;
		}
	}
	rc = 0;

out:
	အကယ်၍ (rc) {
		cpuset_free(*set);
		*set = NULL;
	}
	free(buf);
	ပြန်ပေး rc;
}

ကိန်းပြည့် ul_path_readf_cpuset(sa_nracanaa path_cxt *pc, cpu_set_t **set, ကိန်းပြည့် maxcpus, const var_na *path, ...)
{
	va_list ap;
	ကိန်းပြည့် rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	ပြန်ပေး rc;
}

ကိန်းပြည့် ul_path_readf_cpulist(sa_nracanaa path_cxt *pc, cpu_set_t **set, ကိန်းပြည့် maxcpus, const var_na *path, ...)
{
	va_list ap;
	ကိန်းပြည့် rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	ပြန်ပေး rc;
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

ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na *argv[])
{
	ကိန်းပြည့် c;
	const var_na *prefix = NULL, *dir, *file, *command;
	sa_nracanaa path_cxt *pc = NULL;

	static const sa_nracanaa option longopts[] = {
		{ "prefix",	1, NULL, 'p' },
		{ "help",       0, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};

	နေစဉ်((c = getopt_long(argc, argv, "p:h", longopts, NULL)) != -1) {
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

	အကယ်၍ (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	အကယ်၍ (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	အကယ်၍ (prefix)
		ul_path_set_prefix(pc, prefix);

	အကယ်၍ (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	အကယ်၍ (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		အကယ်၍ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		အကယ်၍ (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		အကယ်၍ (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} မဟုတ်ပါက အကယ်၍ (strcmp(command, "read-s32") == 0) {
		int32_t res;

		အကယ်၍ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		အကယ်၍ (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		အကယ်၍ (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} မဟုတ်ပါက အကယ်၍ (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		အကယ်၍ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		အကယ်၍ (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		အကယ်၍ (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} မဟုတ်ပါက အကယ်၍ (strcmp(command, "read-s64") == 0) {
		int64_t res;

		အကယ်၍ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		အကယ်၍ (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		အကယ်၍ (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} မဟုတ်ပါက အကယ်၍ (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		အကယ်၍ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		အကယ်၍ (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (ကိန်းပြည့်) res);

		အကယ်၍ (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (ကိန်းပြည့်) res);

	} မဟုတ်ပါက အကယ်၍ (strcmp(command, "read-string") == 0) {
		var_na *res;

		အကယ်၍ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		အကယ်၍ (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		အကယ်၍ (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} မဟုတ်ပါက အကယ်၍ (strcmp(command, "read-link") == 0) {
		var_na res[PATH_MAX];

		အကယ်၍ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		အကယ်၍ (ul_path_readlink(pc, res, parimaa_na(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		အကယ်၍ (ul_path_readlinkf(pc, res, parimaa_na(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} မဟုတ်ပါက အကယ်၍ (strcmp(command, "write-string") == 0) {
		var_na *str;

		အကယ်၍ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		အကယ်၍ (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		အကယ်၍ (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} မဟုတ်ပါက အကယ်၍ (strcmp(command, "write-u64") == 0) {
		uint64_t num;

		အကယ်၍ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		num = strtoumax(argv[optind++], NULL, 0);

		အကယ်၍ (ul_path_write_u64(pc, num, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		အကယ်၍ (ul_path_writef_u64(pc, num, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	ပြန်ပေး EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
