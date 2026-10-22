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
	если (ulpath_debug_mask)
		вернуть;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

struct path_cxt *ul_new_path(const char *dir, ...)
{
	struct path_cxt *pc = calloc(1, sizeof(*pc));

	если (!pc)
		вернуть NULL;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	если (dir) {
		целое rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		если (rc < 0 || !pc->dir_path)
			goto fail;
	}
	вернуть pc;
fail:
	ul_unref_path(pc);
	вернуть NULL;
}

void ul_ref_path(struct path_cxt *pc)
{
	если (pc)
		pc->refcount++;
}

void ul_unref_path(struct path_cxt *pc)
{
	если (!pc)
		вернуть;

	pc->refcount--;

	если (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		если (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		free(pc->dir_path);
		free(pc->prefix);
		free(pc);
	}
}

void ul_path_refer_vfs(struct path_cxt *pc, const struct ul_vfs_ops *vfs)
{
	если (pc)
		pc->vfs = vfs;
}

целое ul_path_set_prefix(struct path_cxt *pc, const char *prefix)
{
	char *p = NULL;

	assert(pc->dir_fd < 0);

	если (prefix) {
		p = strdup(prefix);
		если (!p)
			вернуть -ENOMEM;
	}

	free(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	вернуть 0;
}

const char *ul_path_get_prefix(struct path_cxt *pc)
{
	вернуть pc ? pc->prefix : NULL;
}

целое ul_path_set_dir(struct path_cxt *pc, const char *dir)
{
	char *p = NULL;

	если (dir) {
		p = strdup(dir);
		если (!p)
			вернуть -ENOMEM;
	}

	если (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	free(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	вернуть 0;
}

const char *ul_path_get_dir(struct path_cxt *pc)
{
	вернуть pc ? pc->dir_path : NULL;
}

целое ul_path_set_dialect(struct path_cxt *pc, void *data, void free_data(struct path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	вернуть 0;
}

void *ul_path_get_dialect(struct path_cxt *pc)
{
	вернуть pc ? pc->dialect : NULL;
}

целое ul_path_set_enoent_redirect(struct path_cxt *pc, целое (*func)(struct path_cxt *, const char *, целое *))
{
	pc->redirect_on_enoent = func;
	вернуть 0;
}

static const char *get_absdir(struct path_cxt *pc)
{
	целое rc;
	const char *dirpath;

	если (!pc->prefix)
		вернуть pc->dir_path;

	dirpath = pc->dir_path;
	если (!dirpath)
		вернуть pc->prefix;
	если (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, sizeof(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	если (rc < 0)
		вернуть NULL;
	если ((size_t)rc >= sizeof(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		вернуть NULL;
	}

	вернуть pc->path_buffer;
}

целое ul_path_is_accessible(struct path_cxt *pc)
{
	const char *path;
	assert(pc);

	если (pc->dir_fd >= 0)
		вернуть 1;

	path = get_absdir(pc);
	если (!path)
		вернуть 0;
	вернуть access(path, F_OK) == 0;
}

целое ul_path_get_dirfd(struct path_cxt *pc)
{
	assert(pc);
	assert(pc->dir_path);

	если (pc->dir_fd < 0) {
		const char *path = get_absdir(pc);
		если (!path)
			вернуть -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	вернуть pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
void ul_path_close_dirfd(struct path_cxt *pc)
{
	assert(pc);

	если (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

целое ul_path_isopen_dirfd(struct path_cxt *pc)
{
	вернуть pc && pc->dir_fd >= 0;
}

static const char *ul_path_mkpath(struct path_cxt *pc, const char *path, va_list ap)
{
	целое rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, sizeof(pc->path_buffer), path, ap);
	если (rc < 0) {
		если (!errno)
			errno = EINVAL;
		вернуть NULL;
	}

	если ((size_t)rc >= sizeof(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		вернуть NULL;
	}

	вернуть pc->path_buffer;
}

char *ul_path_get_abspath(struct path_cxt *pc, char *buf, size_t bufsz, const char *path, ...)
{
	если (path) {
		целое rc;
		va_list ap;
		const char *tail = NULL, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		если (dirpath && *dirpath == '/')
			dirpath++;
		если (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		если ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			вернуть NULL;
		}
	} иначе {
		const char *tmp = get_absdir(pc);

		если (!tmp)
			вернуть NULL;
		xstrncpy(buf, tmp, bufsz);
	}

	вернуть buf;
}


целое ul_path_access(struct path_cxt *pc, целое mode, const char *path)
{
	целое rc;

	если (!path)
		вернуть -EINVAL;
	если (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} иначе {
		целое dir = ul_path_get_dirfd(pc);
		если (dir < 0)
			вернуть dir;
		если (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		если (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	вернуть rc;
}

целое ul_path_accessf(struct path_cxt *pc, целое mode, const char *path, ...)
{
	va_list ap;
	const char *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	вернуть !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
целое ul_path_stat(struct path_cxt *pc, struct stat *sb, целое flags, const char *path)
{
	целое rc;

	если (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} иначе {
		целое dir = ul_path_get_dirfd(pc);
		если (dir < 0)
			вернуть dir;
		если (path) {
			если  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} иначе
			rc = fstat(dir, sb);	/* dir itself */

		если (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	вернуть rc;
}

целое ul_path_vstatf(struct path_cxt *pc, struct stat *sb, целое flags, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	вернуть !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

целое ul_path_statf(struct path_cxt *pc, struct stat *sb, целое flags, const char *path, ...)
{
	va_list ap;
	целое rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	вернуть rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
целое ul_path_statx(struct path_cxt *pc, struct statx *stx, целое flags, unsigned целое mask,
		  const char *path)
{
	целое rc;

	если (!path)
		вернуть -EINVAL;
	если (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	иначе {
		целое dir = ul_path_get_dirfd(pc);
		если (dir < 0)
			вернуть dir;
		если (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	вернуть rc;
}
#иначе
целое ul_path_statx(struct path_cxt *pc __attribute__((__unused__)),
		  struct statx *stx __attribute__((__unused__)),
		  целое flags __attribute__((__unused__)),
		  unsigned целое mask __attribute__((__unused__)),
		  const char *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	вернуть -1;
}
#endif /* HAVE_STATX */

целое ul_path_vstatxf(struct path_cxt *pc, struct statx *stx, целое flags, unsigned целое mask,
		    const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	вернуть !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

целое ul_path_statxf(struct path_cxt *pc, struct statx *stx, целое flags, unsigned целое mask,
		   const char *path, ...)
{
	va_list ap;
	целое rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	вернуть rc;
}

целое ul_path_open(struct path_cxt *pc, целое flags, const char *path)
{
	целое fd;

	если (!path)
		вернуть -EINVAL;
	если (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} иначе {
		целое fdx;
		целое dir = ul_path_get_dirfd(pc);
		если (dir < 0)
			вернуть dir;

		если (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		если (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	вернуть fd;
}

целое ul_path_vopenf(struct path_cxt *pc, целое flags, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	вернуть !p ? -errno : ul_path_open(pc, flags, p);
}

целое ul_path_openf(struct path_cxt *pc, целое flags, const char *path, ...)
{
	va_list ap;
	целое rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	вернуть rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(struct path_cxt *pc, const char *mode, const char *path)
{
	целое flags = ul_mode_to_flags(mode);
	целое fd = ul_path_open(pc, flags, path);

	если (fd < 0)
		вернуть NULL;

	вернуть ul_vfs_fdopen(pc ? pc->vfs : NULL, fd, mode);
}


FILE *ul_path_vfopenf(struct path_cxt *pc, const char *mode, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	вернуть !p ? NULL : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(struct path_cxt *pc, const char *mode, const char *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	вернуть f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(struct path_cxt *pc, const char *path)
{
	DIR *dir;
	целое fd = -1;

	если (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	иначе если (pc->dir_path) {
		целое dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		если (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	если (fd < 0)
		вернуть NULL;

	dir = fdopendir(fd);
	если (!dir) {
		close(fd);
		вернуть NULL;
	}
	если (!path)
		 rewinddir(dir);
	вернуть dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(struct path_cxt *pc, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	вернуть !p ? NULL : ul_path_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendirf(struct path_cxt *pc, const char *path, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, path);
	dir = ul_path_vopendirf(pc, path, ap);
	va_end(ap);

	вернуть dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(struct path_cxt *pc, char *buf, size_t bufsiz, const char *path)
{
	целое dirfd;
	ssize_t ssz;

	если (!path) {
		const char *p = get_absdir(pc);
		если (!p)
			вернуть -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} иначе {
		dirfd = ul_path_get_dirfd(pc);
		если (dirfd < 0)
			вернуть dirfd;

		если (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	если (ssz >= 0)
		buf[ssz] = '\0';
	вернуть ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlinkf(struct path_cxt *pc, char *buf, size_t bufsiz, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	вернуть !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

целое ul_path_read(struct path_cxt *pc, char *buf, size_t len, const char *path)
{
	целое rc, errsv;
	целое fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	если (fd < 0)
		вернуть -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	вернуть rc;
}

целое ul_path_vreadf(struct path_cxt *pc, char *buf, size_t len, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	вернуть !p ? -errno : ul_path_read(pc, buf, len, p);
}

целое ul_path_readf(struct path_cxt *pc, char *buf, size_t len, const char *path, ...)
{
	va_list ap;
	целое rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	вернуть rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
целое ul_path_read_string(struct path_cxt *pc, char **str, const char *path)
{
	char buf[BUFSIZ];
	целое rc;

	если (!str)
		вернуть -EINVAL;

	*str = NULL;

	rc = ul_path_read_buffer(pc, buf, sizeof(buf), path);
	если (rc < 0)
		вернуть rc;

	*str = strdup(buf);
	если (!*str)
		rc = -ENOMEM;

	вернуть rc;
}

целое ul_path_readf_string(struct path_cxt *pc, char **str, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	вернуть !p ? -errno : ul_path_read_string(pc, str, p);
}

целое ul_path_read_buffer(struct path_cxt *pc, char *buf, size_t bufsz, const char *path)
{
	целое rc = ul_path_read(pc, buf, bufsz - 1, path);

	если (rc == 0)
		buf[0] = '\0';

	иначе если (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		если (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		иначе
			buf[rc] = '\0';
	}

	вернуть rc;
}

целое ul_path_vreadf_buffer(struct path_cxt *pc, char *buf, size_t bufsz, const char *path, va_list ap)
{
	const char *p;

	p = ul_path_mkpath(pc, path, ap);

	вернуть !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

целое ul_path_readf_buffer(struct path_cxt *pc, char *buf, size_t bufsz, const char *path, ...)
{
	va_list ap;
	целое rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	вернуть rc;
}

целое ul_path_scanf(struct path_cxt *pc, const char *path, const char *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	целое rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	если (!f)
		вернуть -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	вернуть rc;
}

целое ul_path_scanff(struct path_cxt *pc, const char *path, va_list ap, const char *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	целое rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	если (!f)
		вернуть -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	вернуть rc;
}


целое ul_path_read_s64(struct path_cxt *pc, int64_t *res, const char *path)
{
	int64_t x = 0;
	целое rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	если (rc != 1)
		вернуть -1;
	если (res)
		*res = x;
	вернуть 0;
}

целое ul_path_readf_s64(struct path_cxt *pc, int64_t *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	вернуть !p ? -errno : ul_path_read_s64(pc, res, p);
}

целое ul_path_read_u64(struct path_cxt *pc, uint64_t *res, const char *path)
{
	uint64_t x = 0;
	целое rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	если (rc != 1)
		вернуть -1;
	если (res)
		*res = x;
	вернуть 0;
}

целое ul_path_readf_u64(struct path_cxt *pc, uint64_t *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	вернуть !p ? -errno : ul_path_read_u64(pc, res, p);
}

целое ul_path_read_s32(struct path_cxt *pc, целое *res, const char *path)
{
	целое rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	если (rc != 1)
		вернуть -1;
	если (res)
		*res = x;
	вернуть 0;
}

целое ul_path_readf_s32(struct path_cxt *pc, целое *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	вернуть !p ? -errno : ul_path_read_s32(pc, res, p);
}

целое ul_path_read_u32(struct path_cxt *pc, unsigned целое *res, const char *path)
{
	целое rc;
	unsigned целое x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	если (rc != 1)
		вернуть -1;
	если (res)
		*res = x;
	вернуть 0;
}

целое ul_path_readf_u32(struct path_cxt *pc, unsigned целое *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	вернуть !p ? -errno : ul_path_read_u32(pc, res, p);
}

целое ul_path_read_majmin(struct path_cxt *pc, dev_t *res, const char *path)
{
	целое rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	если (rc != 2)
		вернуть -1;
	если (res)
		*res = makedev(maj, min);
	вернуть 0;
}

целое ul_path_readf_majmin(struct path_cxt *pc, dev_t *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	вернуть !p ? -errno : ul_path_read_majmin(pc, res, p);
}

целое ul_path_write_string(struct path_cxt *pc, const char *str, const char *path)
{
	целое rc, errsv;
	целое fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	если (fd < 0)
		вернуть -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	вернуть rc;
}

целое ul_path_writef_string(struct path_cxt *pc, const char *str, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	вернуть !p ? -errno : ul_path_write_string(pc, str, p);
}

целое ul_path_write_s64(struct path_cxt *pc, int64_t num, const char *path)
{
	char buf[sizeof(stringify_value(LLONG_MAX))];
	целое rc, errsv;
	целое fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	если (fd < 0)
		вернуть -errno;

	len = snprintf(buf, sizeof(buf), "%" PRId64, num);
	если (len < 0 || (size_t) len >= sizeof(buf))
		rc = len < 0 ? -errno : -E2BIG;
	иначе
		rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	вернуть rc;
}

целое ul_path_write_u64(struct path_cxt *pc, uint64_t num, const char *path)
{
	char buf[sizeof(stringify_value(ULLONG_MAX))];
	целое rc, errsv;
	целое fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	если (fd < 0)
		вернуть -errno;

	len = snprintf(buf, sizeof(buf), "%" PRIu64, num);
	если (len < 0 || (size_t) len >= sizeof(buf))
		rc = len < 0 ? -errno : -E2BIG;
	иначе
		rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	вернуть rc;
}

целое ul_path_writef_u64(struct path_cxt *pc, uint64_t num, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	вернуть !p ? -errno : ul_path_write_u64(pc, num, p);

}

целое ul_path_count_dirents(struct path_cxt *pc, const char *path)
{
	DIR *dir;
	целое r = 0;

	dir = ul_path_opendir(pc, path);
	если (!dir)
		вернуть 0;

	пока (xreaddir(dir)) r++;

	closedir(dir);
	вернуть r;
}

целое ul_path_countf_dirents(struct path_cxt *pc, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	вернуть !p ? -errno : ul_path_count_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
целое ul_path_next_dirent(struct path_cxt *pc, DIR **sub, const char *dirname, struct dirent **d)
{
	если (!pc || !sub || !d)
		вернуть -EINVAL;

	если (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		если (!*sub)
			вернуть -errno;
	}

	*d = xreaddir(*sub);
	если (*d)
		вернуть 0;

	closedir(*sub);
	*sub = NULL;
	вернуть 1;
}

#ifdef HAVE_CPU_SET_T
static целое ul_path_cpuparse(struct path_cxt *pc, cpu_set_t **set, целое maxcpus, целое islist, const char *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	char *buf;
	целое rc;

	*set = NULL;

	buf = malloc(len);
	если (!buf)
		вернуть -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	если (rc < 0)
		goto out;

	*set = cpuset_alloc(maxcpus, &setsize, NULL);
	если (!*set) {
		rc = -EINVAL;
		goto out;
	}

	если (islist) {
		если (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			goto out;
		}
	} иначе {
		если (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			goto out;
		}
	}
	rc = 0;

out:
	если (rc) {
		cpuset_free(*set);
		*set = NULL;
	}
	free(buf);
	вернуть rc;
}

целое ul_path_readf_cpuset(struct path_cxt *pc, cpu_set_t **set, целое maxcpus, const char *path, ...)
{
	va_list ap;
	целое rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	вернуть rc;
}

целое ul_path_readf_cpulist(struct path_cxt *pc, cpu_set_t **set, целое maxcpus, const char *path, ...)
{
	va_list ap;
	целое rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	вернуть rc;
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

целое основной(целое argc, char *argv[])
{
	целое c;
	const char *prefix = NULL, *dir, *file, *command;
	struct path_cxt *pc = NULL;

	static const struct option longopts[] = {
		{ "prefix",	1, NULL, 'p' },
		{ "help",       0, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};

	пока((c = getopt_long(argc, argv, "p:h", longopts, NULL)) != -1) {
		switch(c) {
		case 'p':
			prefix = optarg;
			break;
		case 'h':
			usage();
			break;
		default:
			err(EXIT_FAILURE, "try --help");
		}
	}

	если (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	если (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	если (prefix)
		ul_path_set_prefix(pc, prefix);

	если (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	если (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		если (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		если (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		если (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} иначе если (strcmp(command, "read-s32") == 0) {
		int32_t res;

		если (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		если (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		если (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} иначе если (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		если (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		если (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		если (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} иначе если (strcmp(command, "read-s64") == 0) {
		int64_t res;

		если (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		если (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		если (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} иначе если (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		если (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		если (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (целое) res);

		если (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (целое) res);

	} иначе если (strcmp(command, "read-string") == 0) {
		char *res;

		если (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		если (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		если (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} иначе если (strcmp(command, "read-link") == 0) {
		char res[PATH_MAX];

		если (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		если (ul_path_readlink(pc, res, sizeof(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		если (ul_path_readlinkf(pc, res, sizeof(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} иначе если (strcmp(command, "write-string") == 0) {
		char *str;

		если (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		если (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		если (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} иначе если (strcmp(command, "write-u64") == 0) {
		uint64_t num;

		если (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		num = strtoumax(argv[optind++], NULL, 0);

		если (ul_path_write_u64(pc, num, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		если (ul_path_writef_u64(pc, num, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	вернуть EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
