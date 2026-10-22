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
	如果 (ulpath_debug_mask)
		返回;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

struct path_cxt *ul_new_path(const char *dir, ...)
{
	struct path_cxt *pc = calloc(1, sizeof(*pc));

	如果 (!pc)
		返回 NULL;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	如果 (dir) {
		整数 rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		如果 (rc < 0 || !pc->dir_path)
			goto fail;
	}
	返回 pc;
fail:
	ul_unref_path(pc);
	返回 NULL;
}

void ul_ref_path(struct path_cxt *pc)
{
	如果 (pc)
		pc->refcount++;
}

void ul_unref_path(struct path_cxt *pc)
{
	如果 (!pc)
		返回;

	pc->refcount--;

	如果 (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		如果 (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		free(pc->dir_path);
		free(pc->prefix);
		free(pc);
	}
}

void ul_path_refer_vfs(struct path_cxt *pc, const struct ul_vfs_ops *vfs)
{
	如果 (pc)
		pc->vfs = vfs;
}

整数 ul_path_set_prefix(struct path_cxt *pc, const char *prefix)
{
	char *p = NULL;

	assert(pc->dir_fd < 0);

	如果 (prefix) {
		p = strdup(prefix);
		如果 (!p)
			返回 -ENOMEM;
	}

	free(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	返回 0;
}

const char *ul_path_get_prefix(struct path_cxt *pc)
{
	返回 pc ? pc->prefix : NULL;
}

整数 ul_path_set_dir(struct path_cxt *pc, const char *dir)
{
	char *p = NULL;

	如果 (dir) {
		p = strdup(dir);
		如果 (!p)
			返回 -ENOMEM;
	}

	如果 (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	free(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	返回 0;
}

const char *ul_path_get_dir(struct path_cxt *pc)
{
	返回 pc ? pc->dir_path : NULL;
}

整数 ul_path_set_dialect(struct path_cxt *pc, void *data, void free_data(struct path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	返回 0;
}

void *ul_path_get_dialect(struct path_cxt *pc)
{
	返回 pc ? pc->dialect : NULL;
}

整数 ul_path_set_enoent_redirect(struct path_cxt *pc, 整数 (*func)(struct path_cxt *, const char *, 整数 *))
{
	pc->redirect_on_enoent = func;
	返回 0;
}

static const char *get_absdir(struct path_cxt *pc)
{
	整数 rc;
	const char *dirpath;

	如果 (!pc->prefix)
		返回 pc->dir_path;

	dirpath = pc->dir_path;
	如果 (!dirpath)
		返回 pc->prefix;
	如果 (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, sizeof(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	如果 (rc < 0)
		返回 NULL;
	如果 ((size_t)rc >= sizeof(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		返回 NULL;
	}

	返回 pc->path_buffer;
}

整数 ul_path_is_accessible(struct path_cxt *pc)
{
	const char *path;
	assert(pc);

	如果 (pc->dir_fd >= 0)
		返回 1;

	path = get_absdir(pc);
	如果 (!path)
		返回 0;
	返回 access(path, F_OK) == 0;
}

整数 ul_path_get_dirfd(struct path_cxt *pc)
{
	assert(pc);
	assert(pc->dir_path);

	如果 (pc->dir_fd < 0) {
		const char *path = get_absdir(pc);
		如果 (!path)
			返回 -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	返回 pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
void ul_path_close_dirfd(struct path_cxt *pc)
{
	assert(pc);

	如果 (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

整数 ul_path_isopen_dirfd(struct path_cxt *pc)
{
	返回 pc && pc->dir_fd >= 0;
}

static const char *ul_path_mkpath(struct path_cxt *pc, const char *path, va_list ap)
{
	整数 rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, sizeof(pc->path_buffer), path, ap);
	如果 (rc < 0) {
		如果 (!errno)
			errno = EINVAL;
		返回 NULL;
	}

	如果 ((size_t)rc >= sizeof(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		返回 NULL;
	}

	返回 pc->path_buffer;
}

char *ul_path_get_abspath(struct path_cxt *pc, char *buf, size_t bufsz, const char *path, ...)
{
	如果 (path) {
		整数 rc;
		va_list ap;
		const char *tail = NULL, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		如果 (dirpath && *dirpath == '/')
			dirpath++;
		如果 (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		如果 ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			返回 NULL;
		}
	} 否则 {
		const char *tmp = get_absdir(pc);

		如果 (!tmp)
			返回 NULL;
		xstrncpy(buf, tmp, bufsz);
	}

	返回 buf;
}


整数 ul_path_access(struct path_cxt *pc, 整数 mode, const char *path)
{
	整数 rc;

	如果 (!path)
		返回 -EINVAL;
	如果 (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} 否则 {
		整数 dir = ul_path_get_dirfd(pc);
		如果 (dir < 0)
			返回 dir;
		如果 (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		如果 (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	返回 rc;
}

整数 ul_path_accessf(struct path_cxt *pc, 整数 mode, const char *path, ...)
{
	va_list ap;
	const char *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	返回 !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
整数 ul_path_stat(struct path_cxt *pc, struct stat *sb, 整数 flags, const char *path)
{
	整数 rc;

	如果 (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} 否则 {
		整数 dir = ul_path_get_dirfd(pc);
		如果 (dir < 0)
			返回 dir;
		如果 (path) {
			如果  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} 否则
			rc = fstat(dir, sb);	/* dir itself */

		如果 (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	返回 rc;
}

整数 ul_path_vstatf(struct path_cxt *pc, struct stat *sb, 整数 flags, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	返回 !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

整数 ul_path_statf(struct path_cxt *pc, struct stat *sb, 整数 flags, const char *path, ...)
{
	va_list ap;
	整数 rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	返回 rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
整数 ul_path_statx(struct path_cxt *pc, struct statx *stx, 整数 flags, unsigned 整数 mask,
		  const char *path)
{
	整数 rc;

	如果 (!path)
		返回 -EINVAL;
	如果 (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	否则 {
		整数 dir = ul_path_get_dirfd(pc);
		如果 (dir < 0)
			返回 dir;
		如果 (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	返回 rc;
}
#否则
整数 ul_path_statx(struct path_cxt *pc __attribute__((__unused__)),
		  struct statx *stx __attribute__((__unused__)),
		  整数 flags __attribute__((__unused__)),
		  unsigned 整数 mask __attribute__((__unused__)),
		  const char *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	返回 -1;
}
#endif /* HAVE_STATX */

整数 ul_path_vstatxf(struct path_cxt *pc, struct statx *stx, 整数 flags, unsigned 整数 mask,
		    const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	返回 !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

整数 ul_path_statxf(struct path_cxt *pc, struct statx *stx, 整数 flags, unsigned 整数 mask,
		   const char *path, ...)
{
	va_list ap;
	整数 rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	返回 rc;
}

整数 ul_path_open(struct path_cxt *pc, 整数 flags, const char *path)
{
	整数 fd;

	如果 (!path)
		返回 -EINVAL;
	如果 (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} 否则 {
		整数 fdx;
		整数 dir = ul_path_get_dirfd(pc);
		如果 (dir < 0)
			返回 dir;

		如果 (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		如果 (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	返回 fd;
}

整数 ul_path_vopenf(struct path_cxt *pc, 整数 flags, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	返回 !p ? -errno : ul_path_open(pc, flags, p);
}

整数 ul_path_openf(struct path_cxt *pc, 整数 flags, const char *path, ...)
{
	va_list ap;
	整数 rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	返回 rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(struct path_cxt *pc, const char *mode, const char *path)
{
	整数 flags = ul_mode_to_flags(mode);
	整数 fd = ul_path_open(pc, flags, path);

	如果 (fd < 0)
		返回 NULL;

	返回 ul_vfs_fdopen(pc ? pc->vfs : NULL, fd, mode);
}


FILE *ul_path_vfopenf(struct path_cxt *pc, const char *mode, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	返回 !p ? NULL : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(struct path_cxt *pc, const char *mode, const char *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	返回 f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(struct path_cxt *pc, const char *path)
{
	DIR *dir;
	整数 fd = -1;

	如果 (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	否则 如果 (pc->dir_path) {
		整数 dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		如果 (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	如果 (fd < 0)
		返回 NULL;

	dir = fdopendir(fd);
	如果 (!dir) {
		close(fd);
		返回 NULL;
	}
	如果 (!path)
		 rewinddir(dir);
	返回 dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(struct path_cxt *pc, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	返回 !p ? NULL : ul_path_opendir(pc, p);
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

	返回 dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(struct path_cxt *pc, char *buf, size_t bufsiz, const char *path)
{
	整数 dirfd;
	ssize_t ssz;

	如果 (!path) {
		const char *p = get_absdir(pc);
		如果 (!p)
			返回 -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} 否则 {
		dirfd = ul_path_get_dirfd(pc);
		如果 (dirfd < 0)
			返回 dirfd;

		如果 (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	如果 (ssz >= 0)
		buf[ssz] = '\0';
	返回 ssz;
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

	返回 !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

整数 ul_path_read(struct path_cxt *pc, char *buf, size_t len, const char *path)
{
	整数 rc, errsv;
	整数 fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	如果 (fd < 0)
		返回 -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	返回 rc;
}

整数 ul_path_vreadf(struct path_cxt *pc, char *buf, size_t len, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	返回 !p ? -errno : ul_path_read(pc, buf, len, p);
}

整数 ul_path_readf(struct path_cxt *pc, char *buf, size_t len, const char *path, ...)
{
	va_list ap;
	整数 rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	返回 rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
整数 ul_path_read_string(struct path_cxt *pc, char **str, const char *path)
{
	char buf[BUFSIZ];
	整数 rc;

	如果 (!str)
		返回 -EINVAL;

	*str = NULL;

	rc = ul_path_read_buffer(pc, buf, sizeof(buf), path);
	如果 (rc < 0)
		返回 rc;

	*str = strdup(buf);
	如果 (!*str)
		rc = -ENOMEM;

	返回 rc;
}

整数 ul_path_readf_string(struct path_cxt *pc, char **str, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	返回 !p ? -errno : ul_path_read_string(pc, str, p);
}

整数 ul_path_read_buffer(struct path_cxt *pc, char *buf, size_t bufsz, const char *path)
{
	整数 rc = ul_path_read(pc, buf, bufsz - 1, path);

	如果 (rc == 0)
		buf[0] = '\0';

	否则 如果 (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		如果 (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		否则
			buf[rc] = '\0';
	}

	返回 rc;
}

整数 ul_path_vreadf_buffer(struct path_cxt *pc, char *buf, size_t bufsz, const char *path, va_list ap)
{
	const char *p;

	p = ul_path_mkpath(pc, path, ap);

	返回 !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

整数 ul_path_readf_buffer(struct path_cxt *pc, char *buf, size_t bufsz, const char *path, ...)
{
	va_list ap;
	整数 rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	返回 rc;
}

整数 ul_path_scanf(struct path_cxt *pc, const char *path, const char *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	整数 rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	如果 (!f)
		返回 -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	返回 rc;
}

整数 ul_path_scanff(struct path_cxt *pc, const char *path, va_list ap, const char *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	整数 rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	如果 (!f)
		返回 -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	返回 rc;
}


整数 ul_path_read_s64(struct path_cxt *pc, int64_t *res, const char *path)
{
	int64_t x = 0;
	整数 rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	如果 (rc != 1)
		返回 -1;
	如果 (res)
		*res = x;
	返回 0;
}

整数 ul_path_readf_s64(struct path_cxt *pc, int64_t *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	返回 !p ? -errno : ul_path_read_s64(pc, res, p);
}

整数 ul_path_read_u64(struct path_cxt *pc, uint64_t *res, const char *path)
{
	uint64_t x = 0;
	整数 rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	如果 (rc != 1)
		返回 -1;
	如果 (res)
		*res = x;
	返回 0;
}

整数 ul_path_readf_u64(struct path_cxt *pc, uint64_t *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	返回 !p ? -errno : ul_path_read_u64(pc, res, p);
}

整数 ul_path_read_s32(struct path_cxt *pc, 整数 *res, const char *path)
{
	整数 rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	如果 (rc != 1)
		返回 -1;
	如果 (res)
		*res = x;
	返回 0;
}

整数 ul_path_readf_s32(struct path_cxt *pc, 整数 *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	返回 !p ? -errno : ul_path_read_s32(pc, res, p);
}

整数 ul_path_read_u32(struct path_cxt *pc, unsigned 整数 *res, const char *path)
{
	整数 rc;
	unsigned 整数 x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	如果 (rc != 1)
		返回 -1;
	如果 (res)
		*res = x;
	返回 0;
}

整数 ul_path_readf_u32(struct path_cxt *pc, unsigned 整数 *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	返回 !p ? -errno : ul_path_read_u32(pc, res, p);
}

整数 ul_path_read_majmin(struct path_cxt *pc, dev_t *res, const char *path)
{
	整数 rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	如果 (rc != 2)
		返回 -1;
	如果 (res)
		*res = makedev(maj, min);
	返回 0;
}

整数 ul_path_readf_majmin(struct path_cxt *pc, dev_t *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	返回 !p ? -errno : ul_path_read_majmin(pc, res, p);
}

整数 ul_path_write_string(struct path_cxt *pc, const char *str, const char *path)
{
	整数 rc, errsv;
	整数 fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	如果 (fd < 0)
		返回 -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	返回 rc;
}

整数 ul_path_writef_string(struct path_cxt *pc, const char *str, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	返回 !p ? -errno : ul_path_write_string(pc, str, p);
}

整数 ul_path_write_s64(struct path_cxt *pc, int64_t num, const char *path)
{
	char buf[sizeof(stringify_value(LLONG_MAX))];
	整数 rc, errsv;
	整数 fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	如果 (fd < 0)
		返回 -errno;

	len = snprintf(buf, sizeof(buf), "%" PRId64, num);
	如果 (len < 0 || (size_t) len >= sizeof(buf))
		rc = len < 0 ? -errno : -E2BIG;
	否则
		rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	返回 rc;
}

整数 ul_path_write_u64(struct path_cxt *pc, uint64_t num, const char *path)
{
	char buf[sizeof(stringify_value(ULLONG_MAX))];
	整数 rc, errsv;
	整数 fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	如果 (fd < 0)
		返回 -errno;

	len = snprintf(buf, sizeof(buf), "%" PRIu64, num);
	如果 (len < 0 || (size_t) len >= sizeof(buf))
		rc = len < 0 ? -errno : -E2BIG;
	否则
		rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	返回 rc;
}

整数 ul_path_writef_u64(struct path_cxt *pc, uint64_t num, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	返回 !p ? -errno : ul_path_write_u64(pc, num, p);

}

整数 ul_path_count_dirents(struct path_cxt *pc, const char *path)
{
	DIR *dir;
	整数 r = 0;

	dir = ul_path_opendir(pc, path);
	如果 (!dir)
		返回 0;

	当 (xreaddir(dir)) r++;

	closedir(dir);
	返回 r;
}

整数 ul_path_countf_dirents(struct path_cxt *pc, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	返回 !p ? -errno : ul_path_count_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
整数 ul_path_next_dirent(struct path_cxt *pc, DIR **sub, const char *dirname, struct dirent **d)
{
	如果 (!pc || !sub || !d)
		返回 -EINVAL;

	如果 (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		如果 (!*sub)
			返回 -errno;
	}

	*d = xreaddir(*sub);
	如果 (*d)
		返回 0;

	closedir(*sub);
	*sub = NULL;
	返回 1;
}

#ifdef HAVE_CPU_SET_T
static 整数 ul_path_cpuparse(struct path_cxt *pc, cpu_set_t **set, 整数 maxcpus, 整数 islist, const char *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	char *buf;
	整数 rc;

	*set = NULL;

	buf = malloc(len);
	如果 (!buf)
		返回 -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	如果 (rc < 0)
		goto out;

	*set = cpuset_alloc(maxcpus, &setsize, NULL);
	如果 (!*set) {
		rc = -EINVAL;
		goto out;
	}

	如果 (islist) {
		如果 (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			goto out;
		}
	} 否则 {
		如果 (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			goto out;
		}
	}
	rc = 0;

out:
	如果 (rc) {
		cpuset_free(*set);
		*set = NULL;
	}
	free(buf);
	返回 rc;
}

整数 ul_path_readf_cpuset(struct path_cxt *pc, cpu_set_t **set, 整数 maxcpus, const char *path, ...)
{
	va_list ap;
	整数 rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	返回 rc;
}

整数 ul_path_readf_cpulist(struct path_cxt *pc, cpu_set_t **set, 整数 maxcpus, const char *path, ...)
{
	va_list ap;
	整数 rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	返回 rc;
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

整数 主要(整数 argc, char *argv[])
{
	整数 c;
	const char *prefix = NULL, *dir, *file, *command;
	struct path_cxt *pc = NULL;

	static const struct option longopts[] = {
		{ "prefix",	1, NULL, 'p' },
		{ "help",       0, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};

	当((c = getopt_long(argc, argv, "p:h", longopts, NULL)) != -1) {
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

	如果 (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	如果 (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	如果 (prefix)
		ul_path_set_prefix(pc, prefix);

	如果 (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	如果 (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		如果 (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		如果 (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		如果 (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} 否则 如果 (strcmp(command, "read-s32") == 0) {
		int32_t res;

		如果 (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		如果 (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		如果 (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} 否则 如果 (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		如果 (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		如果 (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		如果 (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} 否则 如果 (strcmp(command, "read-s64") == 0) {
		int64_t res;

		如果 (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		如果 (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		如果 (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} 否则 如果 (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		如果 (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		如果 (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (整数) res);

		如果 (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (整数) res);

	} 否则 如果 (strcmp(command, "read-string") == 0) {
		char *res;

		如果 (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		如果 (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		如果 (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} 否则 如果 (strcmp(command, "read-link") == 0) {
		char res[PATH_MAX];

		如果 (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		如果 (ul_path_readlink(pc, res, sizeof(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		如果 (ul_path_readlinkf(pc, res, sizeof(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} 否则 如果 (strcmp(command, "write-string") == 0) {
		char *str;

		如果 (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		如果 (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		如果 (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} 否则 如果 (strcmp(command, "write-u64") == 0) {
		uint64_t num;

		如果 (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		num = strtoumax(argv[optind++], NULL, 0);

		如果 (ul_path_write_u64(pc, num, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		如果 (ul_path_writef_u64(pc, num, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	返回 EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
