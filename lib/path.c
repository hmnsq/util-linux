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
	ако (ulpath_debug_mask)
		врати;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

struct path_cxt *ul_new_path(const char *dir, ...)
{
	struct path_cxt *pc = calloc(1, sizeof(*pc));

	ако (!pc)
		врати NULL;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	ако (dir) {
		цео rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		ако (rc < 0 || !pc->dir_path)
			goto fail;
	}
	врати pc;
fail:
	ul_unref_path(pc);
	врати NULL;
}

void ul_ref_path(struct path_cxt *pc)
{
	ако (pc)
		pc->refcount++;
}

void ul_unref_path(struct path_cxt *pc)
{
	ако (!pc)
		врати;

	pc->refcount--;

	ако (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		ако (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		free(pc->dir_path);
		free(pc->prefix);
		free(pc);
	}
}

void ul_path_refer_vfs(struct path_cxt *pc, const struct ul_vfs_ops *vfs)
{
	ако (pc)
		pc->vfs = vfs;
}

цео ul_path_set_prefix(struct path_cxt *pc, const char *prefix)
{
	char *p = NULL;

	assert(pc->dir_fd < 0);

	ако (prefix) {
		p = strdup(prefix);
		ако (!p)
			врати -ENOMEM;
	}

	free(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	врати 0;
}

const char *ul_path_get_prefix(struct path_cxt *pc)
{
	врати pc ? pc->prefix : NULL;
}

цео ul_path_set_dir(struct path_cxt *pc, const char *dir)
{
	char *p = NULL;

	ако (dir) {
		p = strdup(dir);
		ако (!p)
			врати -ENOMEM;
	}

	ако (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	free(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	врати 0;
}

const char *ul_path_get_dir(struct path_cxt *pc)
{
	врати pc ? pc->dir_path : NULL;
}

цео ul_path_set_dialect(struct path_cxt *pc, void *data, void free_data(struct path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	врати 0;
}

void *ul_path_get_dialect(struct path_cxt *pc)
{
	врати pc ? pc->dialect : NULL;
}

цео ul_path_set_enoent_redirect(struct path_cxt *pc, цео (*func)(struct path_cxt *, const char *, цео *))
{
	pc->redirect_on_enoent = func;
	врати 0;
}

static const char *get_absdir(struct path_cxt *pc)
{
	цео rc;
	const char *dirpath;

	ако (!pc->prefix)
		врати pc->dir_path;

	dirpath = pc->dir_path;
	ако (!dirpath)
		врати pc->prefix;
	ако (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, sizeof(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	ако (rc < 0)
		врати NULL;
	ако ((size_t)rc >= sizeof(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		врати NULL;
	}

	врати pc->path_buffer;
}

цео ul_path_is_accessible(struct path_cxt *pc)
{
	const char *path;
	assert(pc);

	ако (pc->dir_fd >= 0)
		врати 1;

	path = get_absdir(pc);
	ако (!path)
		врати 0;
	врати access(path, F_OK) == 0;
}

цео ul_path_get_dirfd(struct path_cxt *pc)
{
	assert(pc);
	assert(pc->dir_path);

	ако (pc->dir_fd < 0) {
		const char *path = get_absdir(pc);
		ако (!path)
			врати -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	врати pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
void ul_path_close_dirfd(struct path_cxt *pc)
{
	assert(pc);

	ако (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

цео ul_path_isopen_dirfd(struct path_cxt *pc)
{
	врати pc && pc->dir_fd >= 0;
}

static const char *ul_path_mkpath(struct path_cxt *pc, const char *path, va_list ap)
{
	цео rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, sizeof(pc->path_buffer), path, ap);
	ако (rc < 0) {
		ако (!errno)
			errno = EINVAL;
		врати NULL;
	}

	ако ((size_t)rc >= sizeof(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		врати NULL;
	}

	врати pc->path_buffer;
}

char *ul_path_get_abspath(struct path_cxt *pc, char *buf, size_t bufsz, const char *path, ...)
{
	ако (path) {
		цео rc;
		va_list ap;
		const char *tail = NULL, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		ако (dirpath && *dirpath == '/')
			dirpath++;
		ако (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		ако ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			врати NULL;
		}
	} инако {
		const char *tmp = get_absdir(pc);

		ако (!tmp)
			врати NULL;
		xstrncpy(buf, tmp, bufsz);
	}

	врати buf;
}


цео ul_path_access(struct path_cxt *pc, цео mode, const char *path)
{
	цео rc;

	ако (!path)
		врати -EINVAL;
	ако (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} инако {
		цео dir = ul_path_get_dirfd(pc);
		ако (dir < 0)
			врати dir;
		ако (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		ако (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	врати rc;
}

цео ul_path_accessf(struct path_cxt *pc, цео mode, const char *path, ...)
{
	va_list ap;
	const char *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	врати !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
цео ul_path_stat(struct path_cxt *pc, struct stat *sb, цео flags, const char *path)
{
	цео rc;

	ако (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} инако {
		цео dir = ul_path_get_dirfd(pc);
		ако (dir < 0)
			врати dir;
		ако (path) {
			ако  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} инако
			rc = fstat(dir, sb);	/* dir itself */

		ако (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	врати rc;
}

цео ul_path_vstatf(struct path_cxt *pc, struct stat *sb, цео flags, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	врати !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

цео ul_path_statf(struct path_cxt *pc, struct stat *sb, цео flags, const char *path, ...)
{
	va_list ap;
	цео rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	врати rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
цео ul_path_statx(struct path_cxt *pc, struct statx *stx, цео flags, unsigned цео mask,
		  const char *path)
{
	цео rc;

	ако (!path)
		врати -EINVAL;
	ако (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	инако {
		цео dir = ul_path_get_dirfd(pc);
		ако (dir < 0)
			врати dir;
		ако (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	врати rc;
}
#инако
цео ul_path_statx(struct path_cxt *pc __attribute__((__unused__)),
		  struct statx *stx __attribute__((__unused__)),
		  цео flags __attribute__((__unused__)),
		  unsigned цео mask __attribute__((__unused__)),
		  const char *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	врати -1;
}
#endif /* HAVE_STATX */

цео ul_path_vstatxf(struct path_cxt *pc, struct statx *stx, цео flags, unsigned цео mask,
		    const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	врати !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

цео ul_path_statxf(struct path_cxt *pc, struct statx *stx, цео flags, unsigned цео mask,
		   const char *path, ...)
{
	va_list ap;
	цео rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	врати rc;
}

цео ul_path_open(struct path_cxt *pc, цео flags, const char *path)
{
	цео fd;

	ако (!path)
		врати -EINVAL;
	ако (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} инако {
		цео fdx;
		цео dir = ul_path_get_dirfd(pc);
		ако (dir < 0)
			врати dir;

		ако (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		ако (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	врати fd;
}

цео ul_path_vopenf(struct path_cxt *pc, цео flags, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	врати !p ? -errno : ul_path_open(pc, flags, p);
}

цео ul_path_openf(struct path_cxt *pc, цео flags, const char *path, ...)
{
	va_list ap;
	цео rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	врати rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(struct path_cxt *pc, const char *mode, const char *path)
{
	цео flags = ul_mode_to_flags(mode);
	цео fd = ul_path_open(pc, flags, path);

	ако (fd < 0)
		врати NULL;

	врати ul_vfs_fdopen(pc ? pc->vfs : NULL, fd, mode);
}


FILE *ul_path_vfopenf(struct path_cxt *pc, const char *mode, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	врати !p ? NULL : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(struct path_cxt *pc, const char *mode, const char *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	врати f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(struct path_cxt *pc, const char *path)
{
	DIR *dir;
	цео fd = -1;

	ако (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	инако ако (pc->dir_path) {
		цео dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		ако (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	ако (fd < 0)
		врати NULL;

	dir = fdopendir(fd);
	ако (!dir) {
		close(fd);
		врати NULL;
	}
	ако (!path)
		 rewinddir(dir);
	врати dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(struct path_cxt *pc, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	врати !p ? NULL : ul_path_opendir(pc, p);
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

	врати dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(struct path_cxt *pc, char *buf, size_t bufsiz, const char *path)
{
	цео dirfd;
	ssize_t ssz;

	ако (!path) {
		const char *p = get_absdir(pc);
		ако (!p)
			врати -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} инако {
		dirfd = ul_path_get_dirfd(pc);
		ако (dirfd < 0)
			врати dirfd;

		ако (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	ако (ssz >= 0)
		buf[ssz] = '\0';
	врати ssz;
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

	врати !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

цео ul_path_read(struct path_cxt *pc, char *buf, size_t len, const char *path)
{
	цео rc, errsv;
	цео fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	ако (fd < 0)
		врати -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	врати rc;
}

цео ul_path_vreadf(struct path_cxt *pc, char *buf, size_t len, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	врати !p ? -errno : ul_path_read(pc, buf, len, p);
}

цео ul_path_readf(struct path_cxt *pc, char *buf, size_t len, const char *path, ...)
{
	va_list ap;
	цео rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	врати rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
цео ul_path_read_string(struct path_cxt *pc, char **str, const char *path)
{
	char buf[BUFSIZ];
	цео rc;

	ако (!str)
		врати -EINVAL;

	*str = NULL;

	rc = ul_path_read_buffer(pc, buf, sizeof(buf), path);
	ако (rc < 0)
		врати rc;

	*str = strdup(buf);
	ако (!*str)
		rc = -ENOMEM;

	врати rc;
}

цео ul_path_readf_string(struct path_cxt *pc, char **str, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	врати !p ? -errno : ul_path_read_string(pc, str, p);
}

цео ul_path_read_buffer(struct path_cxt *pc, char *buf, size_t bufsz, const char *path)
{
	цео rc = ul_path_read(pc, buf, bufsz - 1, path);

	ако (rc == 0)
		buf[0] = '\0';

	инако ако (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		ако (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		инако
			buf[rc] = '\0';
	}

	врати rc;
}

цео ul_path_vreadf_buffer(struct path_cxt *pc, char *buf, size_t bufsz, const char *path, va_list ap)
{
	const char *p;

	p = ul_path_mkpath(pc, path, ap);

	врати !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

цео ul_path_readf_buffer(struct path_cxt *pc, char *buf, size_t bufsz, const char *path, ...)
{
	va_list ap;
	цео rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	врати rc;
}

цео ul_path_scanf(struct path_cxt *pc, const char *path, const char *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	цео rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	ако (!f)
		врати -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	врати rc;
}

цео ul_path_scanff(struct path_cxt *pc, const char *path, va_list ap, const char *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	цео rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	ако (!f)
		врати -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	врати rc;
}


цео ul_path_read_s64(struct path_cxt *pc, int64_t *res, const char *path)
{
	int64_t x = 0;
	цео rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	ако (rc != 1)
		врати -1;
	ако (res)
		*res = x;
	врати 0;
}

цео ul_path_readf_s64(struct path_cxt *pc, int64_t *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	врати !p ? -errno : ul_path_read_s64(pc, res, p);
}

цео ul_path_read_u64(struct path_cxt *pc, uint64_t *res, const char *path)
{
	uint64_t x = 0;
	цео rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	ако (rc != 1)
		врати -1;
	ако (res)
		*res = x;
	врати 0;
}

цео ul_path_readf_u64(struct path_cxt *pc, uint64_t *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	врати !p ? -errno : ul_path_read_u64(pc, res, p);
}

цео ul_path_read_s32(struct path_cxt *pc, цео *res, const char *path)
{
	цео rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	ако (rc != 1)
		врати -1;
	ако (res)
		*res = x;
	врати 0;
}

цео ul_path_readf_s32(struct path_cxt *pc, цео *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	врати !p ? -errno : ul_path_read_s32(pc, res, p);
}

цео ul_path_read_u32(struct path_cxt *pc, unsigned цео *res, const char *path)
{
	цео rc;
	unsigned цео x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	ако (rc != 1)
		врати -1;
	ако (res)
		*res = x;
	врати 0;
}

цео ul_path_readf_u32(struct path_cxt *pc, unsigned цео *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	врати !p ? -errno : ul_path_read_u32(pc, res, p);
}

цео ul_path_read_majmin(struct path_cxt *pc, dev_t *res, const char *path)
{
	цео rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	ако (rc != 2)
		врати -1;
	ако (res)
		*res = makedev(maj, min);
	врати 0;
}

цео ul_path_readf_majmin(struct path_cxt *pc, dev_t *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	врати !p ? -errno : ul_path_read_majmin(pc, res, p);
}

цео ul_path_write_string(struct path_cxt *pc, const char *str, const char *path)
{
	цео rc, errsv;
	цео fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	ако (fd < 0)
		врати -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	врати rc;
}

цео ul_path_writef_string(struct path_cxt *pc, const char *str, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	врати !p ? -errno : ul_path_write_string(pc, str, p);
}

цео ul_path_write_s64(struct path_cxt *pc, int64_t num, const char *path)
{
	char buf[sizeof(stringify_value(LLONG_MAX))];
	цео rc, errsv;
	цео fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	ако (fd < 0)
		врати -errno;

	len = snprintf(buf, sizeof(buf), "%" PRId64, num);
	ако (len < 0 || (size_t) len >= sizeof(buf))
		rc = len < 0 ? -errno : -E2BIG;
	инако
		rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	врати rc;
}

цео ul_path_write_u64(struct path_cxt *pc, uint64_t num, const char *path)
{
	char buf[sizeof(stringify_value(ULLONG_MAX))];
	цео rc, errsv;
	цео fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	ако (fd < 0)
		врати -errno;

	len = snprintf(buf, sizeof(buf), "%" PRIu64, num);
	ако (len < 0 || (size_t) len >= sizeof(buf))
		rc = len < 0 ? -errno : -E2BIG;
	инако
		rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	врати rc;
}

цео ul_path_writef_u64(struct path_cxt *pc, uint64_t num, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	врати !p ? -errno : ul_path_write_u64(pc, num, p);

}

цео ul_path_count_dirents(struct path_cxt *pc, const char *path)
{
	DIR *dir;
	цео r = 0;

	dir = ul_path_opendir(pc, path);
	ако (!dir)
		врати 0;

	док (xreaddir(dir)) r++;

	closedir(dir);
	врати r;
}

цео ul_path_countf_dirents(struct path_cxt *pc, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	врати !p ? -errno : ul_path_count_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
цео ul_path_next_dirent(struct path_cxt *pc, DIR **sub, const char *dirname, struct dirent **d)
{
	ако (!pc || !sub || !d)
		врати -EINVAL;

	ако (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		ако (!*sub)
			врати -errno;
	}

	*d = xreaddir(*sub);
	ако (*d)
		врати 0;

	closedir(*sub);
	*sub = NULL;
	врати 1;
}

#ifdef HAVE_CPU_SET_T
static цео ul_path_cpuparse(struct path_cxt *pc, cpu_set_t **set, цео maxcpus, цео islist, const char *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	char *buf;
	цео rc;

	*set = NULL;

	buf = malloc(len);
	ако (!buf)
		врати -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	ако (rc < 0)
		goto out;

	*set = cpuset_alloc(maxcpus, &setsize, NULL);
	ако (!*set) {
		rc = -EINVAL;
		goto out;
	}

	ако (islist) {
		ако (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			goto out;
		}
	} инако {
		ако (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			goto out;
		}
	}
	rc = 0;

out:
	ако (rc) {
		cpuset_free(*set);
		*set = NULL;
	}
	free(buf);
	врати rc;
}

цео ul_path_readf_cpuset(struct path_cxt *pc, cpu_set_t **set, цео maxcpus, const char *path, ...)
{
	va_list ap;
	цео rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	врати rc;
}

цео ul_path_readf_cpulist(struct path_cxt *pc, cpu_set_t **set, цео maxcpus, const char *path, ...)
{
	va_list ap;
	цео rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	врати rc;
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

цео main(цео argc, char *argv[])
{
	цео c;
	const char *prefix = NULL, *dir, *file, *command;
	struct path_cxt *pc = NULL;

	static const struct option longopts[] = {
		{ "prefix",	1, NULL, 'p' },
		{ "help",       0, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};

	док((c = getopt_long(argc, argv, "p:h", longopts, NULL)) != -1) {
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

	ако (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	ако (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	ако (prefix)
		ul_path_set_prefix(pc, prefix);

	ако (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	ако (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		ако (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ако (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		ако (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} инако ако (strcmp(command, "read-s32") == 0) {
		int32_t res;

		ако (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ако (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		ако (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} инако ако (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		ако (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ако (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		ако (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} инако ако (strcmp(command, "read-s64") == 0) {
		int64_t res;

		ако (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ако (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		ако (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} инако ако (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		ако (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ако (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (цео) res);

		ако (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (цео) res);

	} инако ако (strcmp(command, "read-string") == 0) {
		char *res;

		ако (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ако (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		ако (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} инако ако (strcmp(command, "read-link") == 0) {
		char res[PATH_MAX];

		ако (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ако (ul_path_readlink(pc, res, sizeof(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		ако (ul_path_readlinkf(pc, res, sizeof(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} инако ако (strcmp(command, "write-string") == 0) {
		char *str;

		ако (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		ако (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		ако (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} инако ако (strcmp(command, "write-u64") == 0) {
		uint64_t num;

		ако (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		num = strtoumax(argv[optind++], NULL, 0);

		ако (ul_path_write_u64(pc, num, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		ако (ul_path_writef_u64(pc, num, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	врати EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
