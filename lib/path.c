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
	nếu (ulpath_debug_mask)
		trả_về;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

struct path_cxt *ul_new_path(const char *dir, ...)
{
	struct path_cxt *pc = calloc(1, sizeof(*pc));

	nếu (!pc)
		trả_về NULL;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	nếu (dir) {
		số_nguyên rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		nếu (rc < 0 || !pc->dir_path)
			goto fail;
	}
	trả_về pc;
fail:
	ul_unref_path(pc);
	trả_về NULL;
}

void ul_ref_path(struct path_cxt *pc)
{
	nếu (pc)
		pc->refcount++;
}

void ul_unref_path(struct path_cxt *pc)
{
	nếu (!pc)
		trả_về;

	pc->refcount--;

	nếu (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		nếu (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		free(pc->dir_path);
		free(pc->prefix);
		free(pc);
	}
}

void ul_path_refer_vfs(struct path_cxt *pc, const struct ul_vfs_ops *vfs)
{
	nếu (pc)
		pc->vfs = vfs;
}

số_nguyên ul_path_set_prefix(struct path_cxt *pc, const char *prefix)
{
	char *p = NULL;

	assert(pc->dir_fd < 0);

	nếu (prefix) {
		p = strdup(prefix);
		nếu (!p)
			trả_về -ENOMEM;
	}

	free(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	trả_về 0;
}

const char *ul_path_get_prefix(struct path_cxt *pc)
{
	trả_về pc ? pc->prefix : NULL;
}

số_nguyên ul_path_set_dir(struct path_cxt *pc, const char *dir)
{
	char *p = NULL;

	nếu (dir) {
		p = strdup(dir);
		nếu (!p)
			trả_về -ENOMEM;
	}

	nếu (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	free(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	trả_về 0;
}

const char *ul_path_get_dir(struct path_cxt *pc)
{
	trả_về pc ? pc->dir_path : NULL;
}

số_nguyên ul_path_set_dialect(struct path_cxt *pc, void *data, void free_data(struct path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	trả_về 0;
}

void *ul_path_get_dialect(struct path_cxt *pc)
{
	trả_về pc ? pc->dialect : NULL;
}

số_nguyên ul_path_set_enoent_redirect(struct path_cxt *pc, số_nguyên (*func)(struct path_cxt *, const char *, số_nguyên *))
{
	pc->redirect_on_enoent = func;
	trả_về 0;
}

static const char *get_absdir(struct path_cxt *pc)
{
	số_nguyên rc;
	const char *dirpath;

	nếu (!pc->prefix)
		trả_về pc->dir_path;

	dirpath = pc->dir_path;
	nếu (!dirpath)
		trả_về pc->prefix;
	nếu (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, sizeof(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	nếu (rc < 0)
		trả_về NULL;
	nếu ((size_t)rc >= sizeof(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		trả_về NULL;
	}

	trả_về pc->path_buffer;
}

số_nguyên ul_path_is_accessible(struct path_cxt *pc)
{
	const char *path;
	assert(pc);

	nếu (pc->dir_fd >= 0)
		trả_về 1;

	path = get_absdir(pc);
	nếu (!path)
		trả_về 0;
	trả_về access(path, F_OK) == 0;
}

số_nguyên ul_path_get_dirfd(struct path_cxt *pc)
{
	assert(pc);
	assert(pc->dir_path);

	nếu (pc->dir_fd < 0) {
		const char *path = get_absdir(pc);
		nếu (!path)
			trả_về -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	trả_về pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
void ul_path_close_dirfd(struct path_cxt *pc)
{
	assert(pc);

	nếu (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

số_nguyên ul_path_isopen_dirfd(struct path_cxt *pc)
{
	trả_về pc && pc->dir_fd >= 0;
}

static const char *ul_path_mkpath(struct path_cxt *pc, const char *path, va_list ap)
{
	số_nguyên rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, sizeof(pc->path_buffer), path, ap);
	nếu (rc < 0) {
		nếu (!errno)
			errno = EINVAL;
		trả_về NULL;
	}

	nếu ((size_t)rc >= sizeof(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		trả_về NULL;
	}

	trả_về pc->path_buffer;
}

char *ul_path_get_abspath(struct path_cxt *pc, char *buf, size_t bufsz, const char *path, ...)
{
	nếu (path) {
		số_nguyên rc;
		va_list ap;
		const char *tail = NULL, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		nếu (dirpath && *dirpath == '/')
			dirpath++;
		nếu (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		nếu ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			trả_về NULL;
		}
	} không_thì {
		const char *tmp = get_absdir(pc);

		nếu (!tmp)
			trả_về NULL;
		xstrncpy(buf, tmp, bufsz);
	}

	trả_về buf;
}


số_nguyên ul_path_access(struct path_cxt *pc, số_nguyên mode, const char *path)
{
	số_nguyên rc;

	nếu (!path)
		trả_về -EINVAL;
	nếu (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} không_thì {
		số_nguyên dir = ul_path_get_dirfd(pc);
		nếu (dir < 0)
			trả_về dir;
		nếu (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		nếu (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	trả_về rc;
}

số_nguyên ul_path_accessf(struct path_cxt *pc, số_nguyên mode, const char *path, ...)
{
	va_list ap;
	const char *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	trả_về !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
số_nguyên ul_path_stat(struct path_cxt *pc, struct stat *sb, số_nguyên flags, const char *path)
{
	số_nguyên rc;

	nếu (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} không_thì {
		số_nguyên dir = ul_path_get_dirfd(pc);
		nếu (dir < 0)
			trả_về dir;
		nếu (path) {
			nếu  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} không_thì
			rc = fstat(dir, sb);	/* dir itself */

		nếu (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	trả_về rc;
}

số_nguyên ul_path_vstatf(struct path_cxt *pc, struct stat *sb, số_nguyên flags, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	trả_về !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

số_nguyên ul_path_statf(struct path_cxt *pc, struct stat *sb, số_nguyên flags, const char *path, ...)
{
	va_list ap;
	số_nguyên rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	trả_về rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
số_nguyên ul_path_statx(struct path_cxt *pc, struct statx *stx, số_nguyên flags, unsigned số_nguyên mask,
		  const char *path)
{
	số_nguyên rc;

	nếu (!path)
		trả_về -EINVAL;
	nếu (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	không_thì {
		số_nguyên dir = ul_path_get_dirfd(pc);
		nếu (dir < 0)
			trả_về dir;
		nếu (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	trả_về rc;
}
#không_thì
số_nguyên ul_path_statx(struct path_cxt *pc __attribute__((__unused__)),
		  struct statx *stx __attribute__((__unused__)),
		  số_nguyên flags __attribute__((__unused__)),
		  unsigned số_nguyên mask __attribute__((__unused__)),
		  const char *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	trả_về -1;
}
#endif /* HAVE_STATX */

số_nguyên ul_path_vstatxf(struct path_cxt *pc, struct statx *stx, số_nguyên flags, unsigned số_nguyên mask,
		    const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	trả_về !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

số_nguyên ul_path_statxf(struct path_cxt *pc, struct statx *stx, số_nguyên flags, unsigned số_nguyên mask,
		   const char *path, ...)
{
	va_list ap;
	số_nguyên rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	trả_về rc;
}

số_nguyên ul_path_open(struct path_cxt *pc, số_nguyên flags, const char *path)
{
	số_nguyên fd;

	nếu (!path)
		trả_về -EINVAL;
	nếu (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} không_thì {
		số_nguyên fdx;
		số_nguyên dir = ul_path_get_dirfd(pc);
		nếu (dir < 0)
			trả_về dir;

		nếu (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		nếu (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	trả_về fd;
}

số_nguyên ul_path_vopenf(struct path_cxt *pc, số_nguyên flags, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	trả_về !p ? -errno : ul_path_open(pc, flags, p);
}

số_nguyên ul_path_openf(struct path_cxt *pc, số_nguyên flags, const char *path, ...)
{
	va_list ap;
	số_nguyên rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	trả_về rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(struct path_cxt *pc, const char *mode, const char *path)
{
	số_nguyên flags = ul_mode_to_flags(mode);
	số_nguyên fd = ul_path_open(pc, flags, path);

	nếu (fd < 0)
		trả_về NULL;

	trả_về ul_vfs_fdopen(pc ? pc->vfs : NULL, fd, mode);
}


FILE *ul_path_vfopenf(struct path_cxt *pc, const char *mode, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	trả_về !p ? NULL : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(struct path_cxt *pc, const char *mode, const char *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	trả_về f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(struct path_cxt *pc, const char *path)
{
	DIR *dir;
	số_nguyên fd = -1;

	nếu (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	không_thì nếu (pc->dir_path) {
		số_nguyên dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		nếu (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	nếu (fd < 0)
		trả_về NULL;

	dir = fdopendir(fd);
	nếu (!dir) {
		close(fd);
		trả_về NULL;
	}
	nếu (!path)
		 rewinddir(dir);
	trả_về dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(struct path_cxt *pc, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	trả_về !p ? NULL : ul_path_opendir(pc, p);
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

	trả_về dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(struct path_cxt *pc, char *buf, size_t bufsiz, const char *path)
{
	số_nguyên dirfd;
	ssize_t ssz;

	nếu (!path) {
		const char *p = get_absdir(pc);
		nếu (!p)
			trả_về -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} không_thì {
		dirfd = ul_path_get_dirfd(pc);
		nếu (dirfd < 0)
			trả_về dirfd;

		nếu (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	nếu (ssz >= 0)
		buf[ssz] = '\0';
	trả_về ssz;
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

	trả_về !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

số_nguyên ul_path_read(struct path_cxt *pc, char *buf, size_t len, const char *path)
{
	số_nguyên rc, errsv;
	số_nguyên fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	nếu (fd < 0)
		trả_về -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	trả_về rc;
}

số_nguyên ul_path_vreadf(struct path_cxt *pc, char *buf, size_t len, const char *path, va_list ap)
{
	const char *p = ul_path_mkpath(pc, path, ap);

	trả_về !p ? -errno : ul_path_read(pc, buf, len, p);
}

số_nguyên ul_path_readf(struct path_cxt *pc, char *buf, size_t len, const char *path, ...)
{
	va_list ap;
	số_nguyên rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	trả_về rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
số_nguyên ul_path_read_string(struct path_cxt *pc, char **str, const char *path)
{
	char buf[BUFSIZ];
	số_nguyên rc;

	nếu (!str)
		trả_về -EINVAL;

	*str = NULL;

	rc = ul_path_read_buffer(pc, buf, sizeof(buf), path);
	nếu (rc < 0)
		trả_về rc;

	*str = strdup(buf);
	nếu (!*str)
		rc = -ENOMEM;

	trả_về rc;
}

số_nguyên ul_path_readf_string(struct path_cxt *pc, char **str, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	trả_về !p ? -errno : ul_path_read_string(pc, str, p);
}

số_nguyên ul_path_read_buffer(struct path_cxt *pc, char *buf, size_t bufsz, const char *path)
{
	số_nguyên rc = ul_path_read(pc, buf, bufsz - 1, path);

	nếu (rc == 0)
		buf[0] = '\0';

	không_thì nếu (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		nếu (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		không_thì
			buf[rc] = '\0';
	}

	trả_về rc;
}

số_nguyên ul_path_vreadf_buffer(struct path_cxt *pc, char *buf, size_t bufsz, const char *path, va_list ap)
{
	const char *p;

	p = ul_path_mkpath(pc, path, ap);

	trả_về !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

số_nguyên ul_path_readf_buffer(struct path_cxt *pc, char *buf, size_t bufsz, const char *path, ...)
{
	va_list ap;
	số_nguyên rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	trả_về rc;
}

số_nguyên ul_path_scanf(struct path_cxt *pc, const char *path, const char *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	số_nguyên rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	nếu (!f)
		trả_về -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	trả_về rc;
}

số_nguyên ul_path_scanff(struct path_cxt *pc, const char *path, va_list ap, const char *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	số_nguyên rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	nếu (!f)
		trả_về -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	trả_về rc;
}


số_nguyên ul_path_read_s64(struct path_cxt *pc, int64_t *res, const char *path)
{
	int64_t x = 0;
	số_nguyên rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	nếu (rc != 1)
		trả_về -1;
	nếu (res)
		*res = x;
	trả_về 0;
}

số_nguyên ul_path_readf_s64(struct path_cxt *pc, int64_t *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	trả_về !p ? -errno : ul_path_read_s64(pc, res, p);
}

số_nguyên ul_path_read_u64(struct path_cxt *pc, uint64_t *res, const char *path)
{
	uint64_t x = 0;
	số_nguyên rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	nếu (rc != 1)
		trả_về -1;
	nếu (res)
		*res = x;
	trả_về 0;
}

số_nguyên ul_path_readf_u64(struct path_cxt *pc, uint64_t *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	trả_về !p ? -errno : ul_path_read_u64(pc, res, p);
}

số_nguyên ul_path_read_s32(struct path_cxt *pc, số_nguyên *res, const char *path)
{
	số_nguyên rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	nếu (rc != 1)
		trả_về -1;
	nếu (res)
		*res = x;
	trả_về 0;
}

số_nguyên ul_path_readf_s32(struct path_cxt *pc, số_nguyên *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	trả_về !p ? -errno : ul_path_read_s32(pc, res, p);
}

số_nguyên ul_path_read_u32(struct path_cxt *pc, unsigned số_nguyên *res, const char *path)
{
	số_nguyên rc;
	unsigned số_nguyên x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	nếu (rc != 1)
		trả_về -1;
	nếu (res)
		*res = x;
	trả_về 0;
}

số_nguyên ul_path_readf_u32(struct path_cxt *pc, unsigned số_nguyên *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	trả_về !p ? -errno : ul_path_read_u32(pc, res, p);
}

số_nguyên ul_path_read_majmin(struct path_cxt *pc, dev_t *res, const char *path)
{
	số_nguyên rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	nếu (rc != 2)
		trả_về -1;
	nếu (res)
		*res = makedev(maj, min);
	trả_về 0;
}

số_nguyên ul_path_readf_majmin(struct path_cxt *pc, dev_t *res, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	trả_về !p ? -errno : ul_path_read_majmin(pc, res, p);
}

số_nguyên ul_path_write_string(struct path_cxt *pc, const char *str, const char *path)
{
	số_nguyên rc, errsv;
	số_nguyên fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	nếu (fd < 0)
		trả_về -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	trả_về rc;
}

số_nguyên ul_path_writef_string(struct path_cxt *pc, const char *str, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	trả_về !p ? -errno : ul_path_write_string(pc, str, p);
}

số_nguyên ul_path_write_s64(struct path_cxt *pc, int64_t num, const char *path)
{
	char buf[sizeof(stringify_value(LLONG_MAX))];
	số_nguyên rc, errsv;
	số_nguyên fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	nếu (fd < 0)
		trả_về -errno;

	len = snprintf(buf, sizeof(buf), "%" PRId64, num);
	nếu (len < 0 || (size_t) len >= sizeof(buf))
		rc = len < 0 ? -errno : -E2BIG;
	không_thì
		rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	trả_về rc;
}

số_nguyên ul_path_write_u64(struct path_cxt *pc, uint64_t num, const char *path)
{
	char buf[sizeof(stringify_value(ULLONG_MAX))];
	số_nguyên rc, errsv;
	số_nguyên fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	nếu (fd < 0)
		trả_về -errno;

	len = snprintf(buf, sizeof(buf), "%" PRIu64, num);
	nếu (len < 0 || (size_t) len >= sizeof(buf))
		rc = len < 0 ? -errno : -E2BIG;
	không_thì
		rc = ul_vfs_write_all(pc ? pc->vfs : NULL, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : NULL, fd);
	errno = errsv;
	trả_về rc;
}

số_nguyên ul_path_writef_u64(struct path_cxt *pc, uint64_t num, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	trả_về !p ? -errno : ul_path_write_u64(pc, num, p);

}

số_nguyên ul_path_count_dirents(struct path_cxt *pc, const char *path)
{
	DIR *dir;
	số_nguyên r = 0;

	dir = ul_path_opendir(pc, path);
	nếu (!dir)
		trả_về 0;

	trong_khi (xreaddir(dir)) r++;

	closedir(dir);
	trả_về r;
}

số_nguyên ul_path_countf_dirents(struct path_cxt *pc, const char *path, ...)
{
	const char *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	trả_về !p ? -errno : ul_path_count_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
số_nguyên ul_path_next_dirent(struct path_cxt *pc, DIR **sub, const char *dirname, struct dirent **d)
{
	nếu (!pc || !sub || !d)
		trả_về -EINVAL;

	nếu (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		nếu (!*sub)
			trả_về -errno;
	}

	*d = xreaddir(*sub);
	nếu (*d)
		trả_về 0;

	closedir(*sub);
	*sub = NULL;
	trả_về 1;
}

#ifdef HAVE_CPU_SET_T
static số_nguyên ul_path_cpuparse(struct path_cxt *pc, cpu_set_t **set, số_nguyên maxcpus, số_nguyên islist, const char *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	char *buf;
	số_nguyên rc;

	*set = NULL;

	buf = malloc(len);
	nếu (!buf)
		trả_về -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	nếu (rc < 0)
		goto out;

	*set = cpuset_alloc(maxcpus, &setsize, NULL);
	nếu (!*set) {
		rc = -EINVAL;
		goto out;
	}

	nếu (islist) {
		nếu (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			goto out;
		}
	} không_thì {
		nếu (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			goto out;
		}
	}
	rc = 0;

out:
	nếu (rc) {
		cpuset_free(*set);
		*set = NULL;
	}
	free(buf);
	trả_về rc;
}

số_nguyên ul_path_readf_cpuset(struct path_cxt *pc, cpu_set_t **set, số_nguyên maxcpus, const char *path, ...)
{
	va_list ap;
	số_nguyên rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	trả_về rc;
}

số_nguyên ul_path_readf_cpulist(struct path_cxt *pc, cpu_set_t **set, số_nguyên maxcpus, const char *path, ...)
{
	va_list ap;
	số_nguyên rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	trả_về rc;
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

số_nguyên main(số_nguyên argc, char *argv[])
{
	số_nguyên c;
	const char *prefix = NULL, *dir, *file, *command;
	struct path_cxt *pc = NULL;

	static const struct option longopts[] = {
		{ "prefix",	1, NULL, 'p' },
		{ "help",       0, NULL, 'h' },
		{ NULL, 0, NULL, 0 },
	};

	trong_khi((c = getopt_long(argc, argv, "p:h", longopts, NULL)) != -1) {
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

	nếu (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	nếu (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	nếu (prefix)
		ul_path_set_prefix(pc, prefix);

	nếu (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	nếu (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		nếu (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		nếu (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		nếu (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} không_thì nếu (strcmp(command, "read-s32") == 0) {
		int32_t res;

		nếu (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		nếu (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		nếu (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} không_thì nếu (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		nếu (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		nếu (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		nếu (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} không_thì nếu (strcmp(command, "read-s64") == 0) {
		int64_t res;

		nếu (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		nếu (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		nếu (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} không_thì nếu (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		nếu (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		nếu (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (số_nguyên) res);

		nếu (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (số_nguyên) res);

	} không_thì nếu (strcmp(command, "read-string") == 0) {
		char *res;

		nếu (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		nếu (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		nếu (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} không_thì nếu (strcmp(command, "read-link") == 0) {
		char res[PATH_MAX];

		nếu (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		nếu (ul_path_readlink(pc, res, sizeof(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		nếu (ul_path_readlinkf(pc, res, sizeof(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} không_thì nếu (strcmp(command, "write-string") == 0) {
		char *str;

		nếu (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		nếu (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		nếu (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} không_thì nếu (strcmp(command, "write-u64") == 0) {
		uint64_t num;

		nếu (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		num = strtoumax(argv[optind++], NULL, 0);

		nếu (ul_path_write_u64(pc, num, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		nếu (ul_path_writef_u64(pc, num, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	trả_về EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
