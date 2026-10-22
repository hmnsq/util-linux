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
#ضمن <stdarg.h>
#ضمن <string.h>
#ضمن <unistd.h>
#ضمن <stdio.h>
#ضمن <inttypes.h>
#ضمن <errno.h>

#ضمن "c.h"
#ضمن "fileutils.h"
#ضمن "all-io.h"
#ضمن "vfs.h"
#ضمن "path.h"
#ضمن "debug.h"
#ضمن "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
ساكن UL_DEBUG_DEFINE_MASK(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPATH_DEBUG_INIT	(1 << 1)
#define ULPATH_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

فراغ ul_path_init_debug(فراغ)
{
	אם (ulpath_debug_mask)
		החזר;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

بنية path_cxt *ul_new_path(ثابت حرف *dir, ...)
{
	بنية path_cxt *pc = calloc(1, حجم(*pc));

	אם (!pc)
		החזר عدم;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	אם (dir) {
		صحيح rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		אם (rc < 0 || !pc->dir_path)
			اذهب fail;
	}
	החזר pc;
fail:
	ul_unref_path(pc);
	החזר عدم;
}

فراغ ul_ref_path(بنية path_cxt *pc)
{
	אם (pc)
		pc->refcount++;
}

فراغ ul_unref_path(بنية path_cxt *pc)
{
	אם (!pc)
		החזר;

	pc->refcount--;

	אם (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		אם (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		حرر(pc->dir_path);
		حرر(pc->prefix);
		حرر(pc);
	}
}

فراغ ul_path_refer_vfs(بنية path_cxt *pc, ثابت بنية ul_vfs_ops *vfs)
{
	אם (pc)
		pc->vfs = vfs;
}

صحيح ul_path_set_prefix(بنية path_cxt *pc, ثابت حرف *prefix)
{
	حرف *p = عدم;

	أكد(pc->dir_fd < 0);

	אם (prefix) {
		p = strdup(prefix);
		אם (!p)
			החזר -ENOMEM;
	}

	حرر(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	החזר 0;
}

ثابت حرف *ul_path_get_prefix(بنية path_cxt *pc)
{
	החזר pc ? pc->prefix : عدم;
}

صحيح ul_path_set_dir(بنية path_cxt *pc, ثابت حرف *dir)
{
	حرف *p = عدم;

	אם (dir) {
		p = strdup(dir);
		אם (!p)
			החזר -ENOMEM;
	}

	אם (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	حرر(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	החזר 0;
}

ثابت حرف *ul_path_get_dir(بنية path_cxt *pc)
{
	החזר pc ? pc->dir_path : عدم;
}

صحيح ul_path_set_dialect(بنية path_cxt *pc, فراغ *data, فراغ free_data(بنية path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	החזר 0;
}

فراغ *ul_path_get_dialect(بنية path_cxt *pc)
{
	החזר pc ? pc->dialect : عدم;
}

صحيح ul_path_set_enoent_redirect(بنية path_cxt *pc, صحيح (*func)(بنية path_cxt *, ثابت حرف *, صحيح *))
{
	pc->redirect_on_enoent = func;
	החזר 0;
}

ساكن ثابت حرف *get_absdir(بنية path_cxt *pc)
{
	صحيح rc;
	ثابت حرف *dirpath;

	אם (!pc->prefix)
		החזר pc->dir_path;

	dirpath = pc->dir_path;
	אם (!dirpath)
		החזר pc->prefix;
	אם (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, حجم(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	אם (rc < 0)
		החזר عدم;
	אם ((size_t)rc >= حجم(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		החזר عدم;
	}

	החזר pc->path_buffer;
}

صحيح ul_path_is_accessible(بنية path_cxt *pc)
{
	ثابت حرف *path;
	أكد(pc);

	אם (pc->dir_fd >= 0)
		החזר 1;

	path = get_absdir(pc);
	אם (!path)
		החזר 0;
	החזר access(path, F_OK) == 0;
}

صحيح ul_path_get_dirfd(بنية path_cxt *pc)
{
	أكد(pc);
	أكد(pc->dir_path);

	אם (pc->dir_fd < 0) {
		ثابت حرف *path = get_absdir(pc);
		אם (!path)
			החזר -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	החזר pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
فراغ ul_path_close_dirfd(بنية path_cxt *pc)
{
	أكد(pc);

	אם (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

صحيح ul_path_isopen_dirfd(بنية path_cxt *pc)
{
	החזר pc && pc->dir_fd >= 0;
}

ساكن ثابت حرف *ul_path_mkpath(بنية path_cxt *pc, ثابت حرف *path, va_list ap)
{
	صحيح rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, حجم(pc->path_buffer), path, ap);
	אם (rc < 0) {
		אם (!errno)
			errno = EINVAL;
		החזר عدم;
	}

	אם ((size_t)rc >= حجم(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		החזר عدم;
	}

	החזר pc->path_buffer;
}

حرف *ul_path_get_abspath(بنية path_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *path, ...)
{
	אם (path) {
		صحيح rc;
		va_list ap;
		ثابت حرف *tail = عدم, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		אם (dirpath && *dirpath == '/')
			dirpath++;
		אם (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		אם ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			החזר عدم;
		}
	} אחרת {
		ثابت حرف *tmp = get_absdir(pc);

		אם (!tmp)
			החזר عدم;
		xstrncpy(buf, tmp, bufsz);
	}

	החזר buf;
}


صحيح ul_path_access(بنية path_cxt *pc, صحيح mode, ثابت حرف *path)
{
	صحيح rc;

	אם (!path)
		החזר -EINVAL;
	אם (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} אחרת {
		صحيح dir = ul_path_get_dirfd(pc);
		אם (dir < 0)
			החזר dir;
		אם (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		אם (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	החזר rc;
}

صحيح ul_path_accessf(بنية path_cxt *pc, صحيح mode, ثابت حرف *path, ...)
{
	va_list ap;
	ثابت حرف *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	החזר !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
صحيح ul_path_stat(بنية path_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *path)
{
	صحيح rc;

	אם (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} אחרת {
		صحيح dir = ul_path_get_dirfd(pc);
		אם (dir < 0)
			החזר dir;
		אם (path) {
			אם  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} אחרת
			rc = fstat(dir, sb);	/* dir itself */

		אם (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	החזר rc;
}

صحيح ul_path_vstatf(بنية path_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *path, va_list ap)
{
	ثابت حرف *p = ul_path_mkpath(pc, path, ap);

	החזר !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

صحيح ul_path_statf(بنية path_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *path, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	החזר rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
صحيح ul_path_statx(بنية path_cxt *pc, بنية statx *stx, صحيح flags, unsigned صحيح mask,
		  ثابت حرف *path)
{
	صحيح rc;

	אם (!path)
		החזר -EINVAL;
	אם (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	אחרת {
		صحيح dir = ul_path_get_dirfd(pc);
		אם (dir < 0)
			החזר dir;
		אם (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	החזר rc;
}
#אחרת
صحيح ul_path_statx(بنية path_cxt *pc __attribute__((__unused__)),
		  بنية statx *stx __attribute__((__unused__)),
		  صحيح flags __attribute__((__unused__)),
		  unsigned صحيح mask __attribute__((__unused__)),
		  ثابت حرف *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	החזר -1;
}
#endif /* HAVE_STATX */

صحيح ul_path_vstatxf(بنية path_cxt *pc, بنية statx *stx, صحيح flags, unsigned صحيح mask,
		    ثابت حرف *path, va_list ap)
{
	ثابت حرف *p = ul_path_mkpath(pc, path, ap);

	החזר !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

صحيح ul_path_statxf(بنية path_cxt *pc, بنية statx *stx, صحيح flags, unsigned صحيح mask,
		   ثابت حرف *path, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	החזר rc;
}

صحيح ul_path_open(بنية path_cxt *pc, صحيح flags, ثابت حرف *path)
{
	صحيح fd;

	אם (!path)
		החזר -EINVAL;
	אם (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} אחרת {
		صحيح fdx;
		صحيح dir = ul_path_get_dirfd(pc);
		אם (dir < 0)
			החזר dir;

		אם (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		אם (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	החזר fd;
}

صحيح ul_path_vopenf(بنية path_cxt *pc, صحيح flags, ثابت حرف *path, va_list ap)
{
	ثابت حرف *p = ul_path_mkpath(pc, path, ap);

	החזר !p ? -errno : ul_path_open(pc, flags, p);
}

صحيح ul_path_openf(بنية path_cxt *pc, صحيح flags, ثابت حرف *path, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	החזר rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(بنية path_cxt *pc, ثابت حرف *mode, ثابت حرف *path)
{
	صحيح flags = ul_mode_to_flags(mode);
	صحيح fd = ul_path_open(pc, flags, path);

	אם (fd < 0)
		החזר عدم;

	החזר ul_vfs_fdopen(pc ? pc->vfs : عدم, fd, mode);
}


FILE *ul_path_vfopenf(بنية path_cxt *pc, ثابت حرف *mode, ثابت حرف *path, va_list ap)
{
	ثابت حرف *p = ul_path_mkpath(pc, path, ap);

	החזר !p ? عدم : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(بنية path_cxt *pc, ثابت حرف *mode, ثابت حرف *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	החזר f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(بنية path_cxt *pc, ثابت حرف *path)
{
	DIR *dir;
	صحيح fd = -1;

	אם (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	אחרת אם (pc->dir_path) {
		صحيح dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		אם (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	אם (fd < 0)
		החזר عدم;

	dir = fdopendir(fd);
	אם (!dir) {
		close(fd);
		החזר عدم;
	}
	אם (!path)
		 rewinddir(dir);
	החזר dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(بنية path_cxt *pc, ثابت حرف *path, va_list ap)
{
	ثابت حرف *p = ul_path_mkpath(pc, path, ap);

	החזר !p ? عدم : ul_path_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendirf(بنية path_cxt *pc, ثابت حرف *path, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, path);
	dir = ul_path_vopendirf(pc, path, ap);
	va_end(ap);

	החזר dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(بنية path_cxt *pc, حرف *buf, size_t bufsiz, ثابت حرف *path)
{
	صحيح dirfd;
	ssize_t ssz;

	אם (!path) {
		ثابت حرف *p = get_absdir(pc);
		אם (!p)
			החזר -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} אחרת {
		dirfd = ul_path_get_dirfd(pc);
		אם (dirfd < 0)
			החזר dirfd;

		אם (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	אם (ssz >= 0)
		buf[ssz] = '\0';
	החזר ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlinkf(بنية path_cxt *pc, حرف *buf, size_t bufsiz, ثابت حرف *path, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	החזר !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

صحيح ul_path_read(بنية path_cxt *pc, حرف *buf, size_t len, ثابت حرف *path)
{
	صحيح rc, errsv;
	صحيح fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	אם (fd < 0)
		החזר -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : عدم, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	החזר rc;
}

صحيح ul_path_vreadf(بنية path_cxt *pc, حرف *buf, size_t len, ثابت حرف *path, va_list ap)
{
	ثابت حرف *p = ul_path_mkpath(pc, path, ap);

	החזר !p ? -errno : ul_path_read(pc, buf, len, p);
}

صحيح ul_path_readf(بنية path_cxt *pc, حرف *buf, size_t len, ثابت حرف *path, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	החזר rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
صحيح ul_path_read_string(بنية path_cxt *pc, حرف **str, ثابت حرف *path)
{
	حرف buf[BUFSIZ];
	صحيح rc;

	אם (!str)
		החזר -EINVAL;

	*str = عدم;

	rc = ul_path_read_buffer(pc, buf, حجم(buf), path);
	אם (rc < 0)
		החזר rc;

	*str = strdup(buf);
	אם (!*str)
		rc = -ENOMEM;

	החזר rc;
}

صحيح ul_path_readf_string(بنية path_cxt *pc, حرف **str, ثابت حرف *path, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	החזר !p ? -errno : ul_path_read_string(pc, str, p);
}

صحيح ul_path_read_buffer(بنية path_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *path)
{
	صحيح rc = ul_path_read(pc, buf, bufsz - 1, path);

	אם (rc == 0)
		buf[0] = '\0';

	אחרת אם (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		אם (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		אחרת
			buf[rc] = '\0';
	}

	החזר rc;
}

صحيح ul_path_vreadf_buffer(بنية path_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *path, va_list ap)
{
	ثابت حرف *p;

	p = ul_path_mkpath(pc, path, ap);

	החזר !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

صحيح ul_path_readf_buffer(بنية path_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *path, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	החזר rc;
}

صحيح ul_path_scanf(بنية path_cxt *pc, ثابت حرف *path, ثابت حرف *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	صحيح rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	אם (!f)
		החזר -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	החזר rc;
}

صحيح ul_path_scanff(بنية path_cxt *pc, ثابت حرف *path, va_list ap, ثابت حرف *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	صحيح rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	אם (!f)
		החזר -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	החזר rc;
}


صحيح ul_path_read_s64(بنية path_cxt *pc, int64_t *res, ثابت حرف *path)
{
	int64_t x = 0;
	صحيح rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	אם (rc != 1)
		החזר -1;
	אם (res)
		*res = x;
	החזר 0;
}

صحيح ul_path_readf_s64(بنية path_cxt *pc, int64_t *res, ثابت حرف *path, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	החזר !p ? -errno : ul_path_read_s64(pc, res, p);
}

صحيح ul_path_read_u64(بنية path_cxt *pc, uint64_t *res, ثابت حرف *path)
{
	uint64_t x = 0;
	صحيح rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	אם (rc != 1)
		החזר -1;
	אם (res)
		*res = x;
	החזר 0;
}

صحيح ul_path_readf_u64(بنية path_cxt *pc, uint64_t *res, ثابت حرف *path, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	החזר !p ? -errno : ul_path_read_u64(pc, res, p);
}

صحيح ul_path_read_s32(بنية path_cxt *pc, صحيح *res, ثابت حرف *path)
{
	صحيح rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	אם (rc != 1)
		החזר -1;
	אם (res)
		*res = x;
	החזר 0;
}

صحيح ul_path_readf_s32(بنية path_cxt *pc, صحيح *res, ثابت حرف *path, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	החזר !p ? -errno : ul_path_read_s32(pc, res, p);
}

صحيح ul_path_read_u32(بنية path_cxt *pc, unsigned صحيح *res, ثابت حرف *path)
{
	صحيح rc;
	unsigned صحيح x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	אם (rc != 1)
		החזר -1;
	אם (res)
		*res = x;
	החזר 0;
}

صحيح ul_path_readf_u32(بنية path_cxt *pc, unsigned صحيح *res, ثابت حرف *path, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	החזר !p ? -errno : ul_path_read_u32(pc, res, p);
}

صحيح ul_path_read_majmin(بنية path_cxt *pc, dev_t *res, ثابت حرف *path)
{
	صحيح rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	אם (rc != 2)
		החזר -1;
	אם (res)
		*res = makedev(maj, min);
	החזר 0;
}

صحيح ul_path_readf_majmin(بنية path_cxt *pc, dev_t *res, ثابت حرف *path, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	החזר !p ? -errno : ul_path_read_majmin(pc, res, p);
}

صحيح ul_path_write_string(بنية path_cxt *pc, ثابت حرف *str, ثابت حرف *path)
{
	صحيح rc, errsv;
	صحيح fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	אם (fd < 0)
		החזר -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : عدم, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	החזר rc;
}

صحيح ul_path_writef_string(بنية path_cxt *pc, ثابت حرف *str, ثابت حرف *path, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	החזר !p ? -errno : ul_path_write_string(pc, str, p);
}

صحيح ul_path_write_s64(بنية path_cxt *pc, int64_t num, ثابت حرف *path)
{
	حرف buf[حجم(stringify_value(LLONG_MAX))];
	صحيح rc, errsv;
	صحيح fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	אם (fd < 0)
		החזר -errno;

	len = snprintf(buf, حجم(buf), "%" PRId64, num);
	אם (len < 0 || (size_t) len >= حجم(buf))
		rc = len < 0 ? -errno : -E2BIG;
	אחרת
		rc = ul_vfs_write_all(pc ? pc->vfs : عدم, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	החזר rc;
}

صحيح ul_path_write_u64(بنية path_cxt *pc, uint64_t num, ثابت حرف *path)
{
	حرف buf[حجم(stringify_value(ULLONG_MAX))];
	صحيح rc, errsv;
	صحيح fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	אם (fd < 0)
		החזר -errno;

	len = snprintf(buf, حجم(buf), "%" PRIu64, num);
	אם (len < 0 || (size_t) len >= حجم(buf))
		rc = len < 0 ? -errno : -E2BIG;
	אחרת
		rc = ul_vfs_write_all(pc ? pc->vfs : عدم, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	החזר rc;
}

صحيح ul_path_writef_u64(بنية path_cxt *pc, uint64_t num, ثابت حرف *path, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	החזר !p ? -errno : ul_path_write_u64(pc, num, p);

}

صحيح ul_path_count_dirents(بنية path_cxt *pc, ثابت حرف *path)
{
	DIR *dir;
	صحيح r = 0;

	dir = ul_path_opendir(pc, path);
	אם (!dir)
		החזר 0;

	כלעוד (xreaddir(dir)) r++;

	closedir(dir);
	החזר r;
}

صحيح ul_path_countf_dirents(بنية path_cxt *pc, ثابت حرف *path, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	החזר !p ? -errno : ul_path_count_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
صحيح ul_path_next_dirent(بنية path_cxt *pc, DIR **sub, ثابت حرف *dirname, بنية dirent **d)
{
	אם (!pc || !sub || !d)
		החזר -EINVAL;

	אם (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		אם (!*sub)
			החזר -errno;
	}

	*d = xreaddir(*sub);
	אם (*d)
		החזר 0;

	closedir(*sub);
	*sub = عدم;
	החזר 1;
}

#ifdef HAVE_CPU_SET_T
ساكن صحيح ul_path_cpuparse(بنية path_cxt *pc, cpu_set_t **set, صحيح maxcpus, صحيح islist, ثابت حرف *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	حرف *buf;
	صحيح rc;

	*set = عدم;

	buf = malloc(len);
	אם (!buf)
		החזר -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	אם (rc < 0)
		اذهب out;

	*set = cpuset_alloc(maxcpus, &setsize, عدم);
	אם (!*set) {
		rc = -EINVAL;
		اذهب out;
	}

	אם (islist) {
		אם (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			اذهب out;
		}
	} אחרת {
		אם (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			اذهب out;
		}
	}
	rc = 0;

out:
	אם (rc) {
		cpuset_free(*set);
		*set = عدم;
	}
	حرر(buf);
	החזר rc;
}

صحيح ul_path_readf_cpuset(بنية path_cxt *pc, cpu_set_t **set, صحيح maxcpus, ثابت حرف *path, ...)
{
	va_list ap;
	صحيح rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	החזר rc;
}

صحيح ul_path_readf_cpulist(بنية path_cxt *pc, cpu_set_t **set, صحيح maxcpus, ثابت حرف *path, ...)
{
	va_list ap;
	صحيح rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	החזר rc;
}

#endif /* HAVE_CPU_SET_T */


#ifdef TEST_PROGRAM_PATH
#ضمن <getopt.h>

ساكن فراغ __attribute__((__noreturn__)) usage(فراغ)
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

صحيح main(صحيح argc, حرف *argv[])
{
	صحيح c;
	ثابت حرف *prefix = عدم, *dir, *file, *command;
	بنية path_cxt *pc = عدم;

	ساكن ثابت بنية option longopts[] = {
		{ "prefix",	1, عدم, 'p' },
		{ "help",       0, عدم, 'h' },
		{ عدم, 0, عدم, 0 },
	};

	כלעוד((c = getopt_long(argc, argv, "p:h", longopts, عدم)) != -1) {
		بدل(c) {
		حالة 'p':
			prefix = optarg;
			اكسر;
		حالة 'h':
			usage();
			اكسر;
		افتراضي:
			err(EXIT_FAILURE, "try --help");
		}
	}

	אם (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	אם (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	אם (prefix)
		ul_path_set_prefix(pc, prefix);

	אם (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	אם (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		אם (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		אם (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		אם (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} אחרת אם (strcmp(command, "read-s32") == 0) {
		int32_t res;

		אם (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		אם (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		אם (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} אחרת אם (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		אם (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		אם (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		אם (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} אחרת אם (strcmp(command, "read-s64") == 0) {
		int64_t res;

		אם (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		אם (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		אם (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} אחרת אם (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		אם (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		אם (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (صحيح) res);

		אם (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (صحيح) res);

	} אחרת אם (strcmp(command, "read-string") == 0) {
		حرف *res;

		אם (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		אם (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		אם (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} אחרת אם (strcmp(command, "read-link") == 0) {
		حرف res[PATH_MAX];

		אם (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		אם (ul_path_readlink(pc, res, حجم(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		אם (ul_path_readlinkf(pc, res, حجم(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} אחרת אם (strcmp(command, "write-string") == 0) {
		حرف *str;

		אם (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		אם (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		אם (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} אחרת אם (strcmp(command, "write-u64") == 0) {
		uint64_t num;

		אם (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		num = strtoumax(argv[optind++], عدم, 0);

		אם (ul_path_write_u64(pc, num, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		אם (ul_path_writef_u64(pc, num, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	החזר EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
