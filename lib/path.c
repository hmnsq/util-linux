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
#شامل <stdarg.h>
#شامل <string.h>
#شامل <unistd.h>
#شامل <stdio.h>
#شامل <inttypes.h>
#شامل <errno.h>

#شامل "c.h"
#شامل "fileutils.h"
#شامل "all-io.h"
#شامل "vfs.h"
#شامل "path.h"
#شامل "debug.h"
#شامل "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
ایستا UL_DEBUG_DEFINE_MASK(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPATH_DEBUG_INIT	(1 << 1)
#define ULPATH_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

تهی ul_path_init_debug(تهی)
{
	اگر (ulpath_debug_mask)
		بازگشت;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

ساختار path_cxt *ul_new_path(ثابت نویسه *dir, ...)
{
	ساختار path_cxt *pc = calloc(1, اندازه(*pc));

	اگر (!pc)
		بازگشت پوچ;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	اگر (dir) {
		صحیح rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		va_end(ap);

		اگر (rc < 0 || !pc->dir_path)
			برو fail;
	}
	بازگشت pc;
fail:
	ul_unref_path(pc);
	بازگشت پوچ;
}

تهی ul_ref_path(ساختار path_cxt *pc)
{
	اگر (pc)
		pc->refcount++;
}

تهی ul_unref_path(ساختار path_cxt *pc)
{
	اگر (!pc)
		بازگشت;

	pc->refcount--;

	اگر (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		اگر (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		آزاد(pc->dir_path);
		آزاد(pc->prefix);
		آزاد(pc);
	}
}

تهی ul_path_refer_vfs(ساختار path_cxt *pc, ثابت ساختار ul_vfs_ops *vfs)
{
	اگر (pc)
		pc->vfs = vfs;
}

صحیح ul_path_set_prefix(ساختار path_cxt *pc, ثابت نویسه *prefix)
{
	نویسه *p = پوچ;

	أكد(pc->dir_fd < 0);

	اگر (prefix) {
		p = strdup(prefix);
		اگر (!p)
			بازگشت -ENOMEM;
	}

	آزاد(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	بازگشت 0;
}

ثابت نویسه *ul_path_get_prefix(ساختار path_cxt *pc)
{
	بازگشت pc ? pc->prefix : پوچ;
}

صحیح ul_path_set_dir(ساختار path_cxt *pc, ثابت نویسه *dir)
{
	نویسه *p = پوچ;

	اگر (dir) {
		p = strdup(dir);
		اگر (!p)
			بازگشت -ENOMEM;
	}

	اگر (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	آزاد(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	بازگشت 0;
}

ثابت نویسه *ul_path_get_dir(ساختار path_cxt *pc)
{
	بازگشت pc ? pc->dir_path : پوچ;
}

صحیح ul_path_set_dialect(ساختار path_cxt *pc, تهی *data, تهی free_data(ساختار path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	بازگشت 0;
}

تهی *ul_path_get_dialect(ساختار path_cxt *pc)
{
	بازگشت pc ? pc->dialect : پوچ;
}

صحیح ul_path_set_enoent_redirect(ساختار path_cxt *pc, صحیح (*func)(ساختار path_cxt *, ثابت نویسه *, صحیح *))
{
	pc->redirect_on_enoent = func;
	بازگشت 0;
}

ایستا ثابت نویسه *get_absdir(ساختار path_cxt *pc)
{
	صحیح rc;
	ثابت نویسه *dirpath;

	اگر (!pc->prefix)
		بازگشت pc->dir_path;

	dirpath = pc->dir_path;
	اگر (!dirpath)
		بازگشت pc->prefix;
	اگر (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, اندازه(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	اگر (rc < 0)
		بازگشت پوچ;
	اگر ((size_t)rc >= اندازه(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		بازگشت پوچ;
	}

	بازگشت pc->path_buffer;
}

صحیح ul_path_is_accessible(ساختار path_cxt *pc)
{
	ثابت نویسه *path;
	أكد(pc);

	اگر (pc->dir_fd >= 0)
		بازگشت 1;

	path = get_absdir(pc);
	اگر (!path)
		بازگشت 0;
	بازگشت access(path, F_OK) == 0;
}

صحیح ul_path_get_dirfd(ساختار path_cxt *pc)
{
	أكد(pc);
	أكد(pc->dir_path);

	اگر (pc->dir_fd < 0) {
		ثابت نویسه *path = get_absdir(pc);
		اگر (!path)
			بازگشت -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = open(path, O_RDONLY|O_CLOEXEC);
	}

	بازگشت pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
تهی ul_path_close_dirfd(ساختار path_cxt *pc)
{
	أكد(pc);

	اگر (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

صحیح ul_path_isopen_dirfd(ساختار path_cxt *pc)
{
	بازگشت pc && pc->dir_fd >= 0;
}

ایستا ثابت نویسه *ul_path_mkpath(ساختار path_cxt *pc, ثابت نویسه *path, va_list ap)
{
	صحیح rc;

	errno = 0;

	rc = vsnprintf(pc->path_buffer, اندازه(pc->path_buffer), path, ap);
	اگر (rc < 0) {
		اگر (!errno)
			errno = EINVAL;
		بازگشت پوچ;
	}

	اگر ((size_t)rc >= اندازه(pc->path_buffer)) {
		errno = ENAMETOOLONG;
		بازگشت پوچ;
	}

	بازگشت pc->path_buffer;
}

نویسه *ul_path_get_abspath(ساختار path_cxt *pc, نویسه *buf, size_t bufsz, ثابت نویسه *path, ...)
{
	اگر (path) {
		صحیح rc;
		va_list ap;
		ثابت نویسه *tail = پوچ, *dirpath = pc->dir_path;

		va_start(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		va_end(ap);

		اگر (dirpath && *dirpath == '/')
			dirpath++;
		اگر (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		اگر ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			بازگشت پوچ;
		}
	} وگرنه {
		ثابت نویسه *tmp = get_absdir(pc);

		اگر (!tmp)
			بازگشت پوچ;
		xstrncpy(buf, tmp, bufsz);
	}

	بازگشت buf;
}


صحیح ul_path_access(ساختار path_cxt *pc, صحیح mode, ثابت نویسه *path)
{
	صحیح rc;

	اگر (!path)
		بازگشت -EINVAL;
	اگر (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} وگرنه {
		صحیح dir = ul_path_get_dirfd(pc);
		اگر (dir < 0)
			بازگشت dir;
		اگر (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		اگر (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	بازگشت rc;
}

صحیح ul_path_accessf(ساختار path_cxt *pc, صحیح mode, ثابت نویسه *path, ...)
{
	va_list ap;
	ثابت نویسه *p;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	بازگشت !p ? -errno : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
صحیح ul_path_stat(ساختار path_cxt *pc, ساختار stat *sb, صحیح flags, ثابت نویسه *path)
{
	صحیح rc;

	اگر (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} وگرنه {
		صحیح dir = ul_path_get_dirfd(pc);
		اگر (dir < 0)
			بازگشت dir;
		اگر (path) {
			اگر  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} وگرنه
			rc = fstat(dir, sb);	/* dir itself */

		اگر (rc && errno == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	بازگشت rc;
}

صحیح ul_path_vstatf(ساختار path_cxt *pc, ساختار stat *sb, صحیح flags, ثابت نویسه *path, va_list ap)
{
	ثابت نویسه *p = ul_path_mkpath(pc, path, ap);

	بازگشت !p ? -errno : ul_path_stat(pc, sb, flags, p);
}

صحیح ul_path_statf(ساختار path_cxt *pc, ساختار stat *sb, صحیح flags, ثابت نویسه *path, ...)
{
	va_list ap;
	صحیح rc;

	va_start(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	va_end(ap);

	بازگشت rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
صحیح ul_path_statx(ساختار path_cxt *pc, ساختار statx *stx, صحیح flags, unsigned صحیح mask,
		  ثابت نویسه *path)
{
	صحیح rc;

	اگر (!path)
		بازگشت -EINVAL;
	اگر (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	وگرنه {
		صحیح dir = ul_path_get_dirfd(pc);
		اگر (dir < 0)
			بازگشت dir;
		اگر (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	بازگشت rc;
}
#وگرنه
صحیح ul_path_statx(ساختار path_cxt *pc __attribute__((__unused__)),
		  ساختار statx *stx __attribute__((__unused__)),
		  صحیح flags __attribute__((__unused__)),
		  unsigned صحیح mask __attribute__((__unused__)),
		  ثابت نویسه *path __attribute__((__unused__)))
{
	errno = ENOSYS;
	بازگشت -1;
}
#endif /* HAVE_STATX */

صحیح ul_path_vstatxf(ساختار path_cxt *pc, ساختار statx *stx, صحیح flags, unsigned صحیح mask,
		    ثابت نویسه *path, va_list ap)
{
	ثابت نویسه *p = ul_path_mkpath(pc, path, ap);

	بازگشت !p ? -errno : ul_path_statx(pc, stx, flags, mask, p);
}

صحیح ul_path_statxf(ساختار path_cxt *pc, ساختار statx *stx, صحیح flags, unsigned صحیح mask,
		   ثابت نویسه *path, ...)
{
	va_list ap;
	صحیح rc;

	va_start(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	va_end(ap);

	بازگشت rc;
}

صحیح ul_path_open(ساختار path_cxt *pc, صحیح flags, ثابت نویسه *path)
{
	صحیح fd;

	اگر (!path)
		بازگشت -EINVAL;
	اگر (!pc) {
		fd = open(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} وگرنه {
		صحیح fdx;
		صحیح dir = ul_path_get_dirfd(pc);
		اگر (dir < 0)
			بازگشت dir;

		اگر (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		اگر (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	بازگشت fd;
}

صحیح ul_path_vopenf(ساختار path_cxt *pc, صحیح flags, ثابت نویسه *path, va_list ap)
{
	ثابت نویسه *p = ul_path_mkpath(pc, path, ap);

	بازگشت !p ? -errno : ul_path_open(pc, flags, p);
}

صحیح ul_path_openf(ساختار path_cxt *pc, صحیح flags, ثابت نویسه *path, ...)
{
	va_list ap;
	صحیح rc;

	va_start(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	va_end(ap);

	بازگشت rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_path_fopen(ساختار path_cxt *pc, ثابت نویسه *mode, ثابت نویسه *path)
{
	صحیح flags = ul_mode_to_flags(mode);
	صحیح fd = ul_path_open(pc, flags, path);

	اگر (fd < 0)
		بازگشت پوچ;

	بازگشت ul_vfs_fdopen(pc ? pc->vfs : پوچ, fd, mode);
}


FILE *ul_path_vfopenf(ساختار path_cxt *pc, ثابت نویسه *mode, ثابت نویسه *path, va_list ap)
{
	ثابت نویسه *p = ul_path_mkpath(pc, path, ap);

	بازگشت !p ? پوچ : ul_path_fopen(pc, mode, p);
}

FILE *ul_path_fopenf(ساختار path_cxt *pc, ثابت نویسه *mode, ثابت نویسه *path, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	va_end(ap);

	بازگشت f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendir(ساختار path_cxt *pc, ثابت نویسه *path)
{
	DIR *dir;
	صحیح fd = -1;

	اگر (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	وگرنه اگر (pc->dir_path) {
		صحیح dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		اگر (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	اگر (fd < 0)
		بازگشت پوچ;

	dir = fdopendir(fd);
	اگر (!dir) {
		close(fd);
		بازگشت پوچ;
	}
	اگر (!path)
		 rewinddir(dir);
	بازگشت dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_vopendirf(ساختار path_cxt *pc, ثابت نویسه *path, va_list ap)
{
	ثابت نویسه *p = ul_path_mkpath(pc, path, ap);

	بازگشت !p ? پوچ : ul_path_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_path_opendirf(ساختار path_cxt *pc, ثابت نویسه *path, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, path);
	dir = ul_path_vopendirf(pc, path, ap);
	va_end(ap);

	بازگشت dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(ساختار path_cxt *pc, نویسه *buf, size_t bufsiz, ثابت نویسه *path)
{
	صحیح dirfd;
	ssize_t ssz;

	اگر (!path) {
		ثابت نویسه *p = get_absdir(pc);
		اگر (!p)
			بازگشت -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} وگرنه {
		dirfd = ul_path_get_dirfd(pc);
		اگر (dirfd < 0)
			بازگشت dirfd;

		اگر (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	اگر (ssz >= 0)
		buf[ssz] = '\0';
	بازگشت ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlinkf(ساختار path_cxt *pc, نویسه *buf, size_t bufsiz, ثابت نویسه *path, ...)
{
	ثابت نویسه *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	بازگشت !p ? -errno : ul_path_readlink(pc, buf, bufsiz, p);
}

صحیح ul_path_read(ساختار path_cxt *pc, نویسه *buf, size_t len, ثابت نویسه *path)
{
	صحیح rc, errsv;
	صحیح fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	اگر (fd < 0)
		بازگشت -errno;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : پوچ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : پوچ, fd);
	errno = errsv;
	بازگشت rc;
}

صحیح ul_path_vreadf(ساختار path_cxt *pc, نویسه *buf, size_t len, ثابت نویسه *path, va_list ap)
{
	ثابت نویسه *p = ul_path_mkpath(pc, path, ap);

	بازگشت !p ? -errno : ul_path_read(pc, buf, len, p);
}

صحیح ul_path_readf(ساختار path_cxt *pc, نویسه *buf, size_t len, ثابت نویسه *path, ...)
{
	va_list ap;
	صحیح rc;

	va_start(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	va_end(ap);

	بازگشت rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
صحیح ul_path_read_string(ساختار path_cxt *pc, نویسه **str, ثابت نویسه *path)
{
	نویسه buf[BUFSIZ];
	صحیح rc;

	اگر (!str)
		بازگشت -EINVAL;

	*str = پوچ;

	rc = ul_path_read_buffer(pc, buf, اندازه(buf), path);
	اگر (rc < 0)
		بازگشت rc;

	*str = strdup(buf);
	اگر (!*str)
		rc = -ENOMEM;

	بازگشت rc;
}

صحیح ul_path_readf_string(ساختار path_cxt *pc, نویسه **str, ثابت نویسه *path, ...)
{
	ثابت نویسه *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	بازگشت !p ? -errno : ul_path_read_string(pc, str, p);
}

صحیح ul_path_read_buffer(ساختار path_cxt *pc, نویسه *buf, size_t bufsz, ثابت نویسه *path)
{
	صحیح rc = ul_path_read(pc, buf, bufsz - 1, path);

	اگر (rc == 0)
		buf[0] = '\0';

	وگرنه اگر (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		اگر (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		وگرنه
			buf[rc] = '\0';
	}

	بازگشت rc;
}

صحیح ul_path_vreadf_buffer(ساختار path_cxt *pc, نویسه *buf, size_t bufsz, ثابت نویسه *path, va_list ap)
{
	ثابت نویسه *p;

	p = ul_path_mkpath(pc, path, ap);

	بازگشت !p ? -errno : ul_path_read_buffer(pc, buf, bufsz, p);
}

صحیح ul_path_readf_buffer(ساختار path_cxt *pc, نویسه *buf, size_t bufsz, ثابت نویسه *path, ...)
{
	va_list ap;
	صحیح rc;

	va_start(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	va_end(ap);

	بازگشت rc;
}

صحیح ul_path_scanf(ساختار path_cxt *pc, ثابت نویسه *path, ثابت نویسه *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	صحیح rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	اگر (!f)
		بازگشت -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	بازگشت rc;
}

صحیح ul_path_scanff(ساختار path_cxt *pc, ثابت نویسه *path, va_list ap, ثابت نویسه *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	صحیح rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	اگر (!f)
		بازگشت -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	بازگشت rc;
}


صحیح ul_path_read_s64(ساختار path_cxt *pc, int64_t *res, ثابت نویسه *path)
{
	int64_t x = 0;
	صحیح rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	اگر (rc != 1)
		بازگشت -1;
	اگر (res)
		*res = x;
	بازگشت 0;
}

صحیح ul_path_readf_s64(ساختار path_cxt *pc, int64_t *res, ثابت نویسه *path, ...)
{
	ثابت نویسه *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	بازگشت !p ? -errno : ul_path_read_s64(pc, res, p);
}

صحیح ul_path_read_u64(ساختار path_cxt *pc, uint64_t *res, ثابت نویسه *path)
{
	uint64_t x = 0;
	صحیح rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	اگر (rc != 1)
		بازگشت -1;
	اگر (res)
		*res = x;
	بازگشت 0;
}

صحیح ul_path_readf_u64(ساختار path_cxt *pc, uint64_t *res, ثابت نویسه *path, ...)
{
	ثابت نویسه *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	بازگشت !p ? -errno : ul_path_read_u64(pc, res, p);
}

صحیح ul_path_read_s32(ساختار path_cxt *pc, صحیح *res, ثابت نویسه *path)
{
	صحیح rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	اگر (rc != 1)
		بازگشت -1;
	اگر (res)
		*res = x;
	بازگشت 0;
}

صحیح ul_path_readf_s32(ساختار path_cxt *pc, صحیح *res, ثابت نویسه *path, ...)
{
	ثابت نویسه *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	بازگشت !p ? -errno : ul_path_read_s32(pc, res, p);
}

صحیح ul_path_read_u32(ساختار path_cxt *pc, unsigned صحیح *res, ثابت نویسه *path)
{
	صحیح rc;
	unsigned صحیح x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	اگر (rc != 1)
		بازگشت -1;
	اگر (res)
		*res = x;
	بازگشت 0;
}

صحیح ul_path_readf_u32(ساختار path_cxt *pc, unsigned صحیح *res, ثابت نویسه *path, ...)
{
	ثابت نویسه *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	بازگشت !p ? -errno : ul_path_read_u32(pc, res, p);
}

صحیح ul_path_read_majmin(ساختار path_cxt *pc, dev_t *res, ثابت نویسه *path)
{
	صحیح rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	اگر (rc != 2)
		بازگشت -1;
	اگر (res)
		*res = makedev(maj, min);
	بازگشت 0;
}

صحیح ul_path_readf_majmin(ساختار path_cxt *pc, dev_t *res, ثابت نویسه *path, ...)
{
	ثابت نویسه *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	بازگشت !p ? -errno : ul_path_read_majmin(pc, res, p);
}

صحیح ul_path_write_string(ساختار path_cxt *pc, ثابت نویسه *str, ثابت نویسه *path)
{
	صحیح rc, errsv;
	صحیح fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	اگر (fd < 0)
		بازگشت -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : پوچ, fd, str, strlen(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : پوچ, fd);
	errno = errsv;
	بازگشت rc;
}

صحیح ul_path_writef_string(ساختار path_cxt *pc, ثابت نویسه *str, ثابت نویسه *path, ...)
{
	ثابت نویسه *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	بازگشت !p ? -errno : ul_path_write_string(pc, str, p);
}

صحیح ul_path_write_s64(ساختار path_cxt *pc, int64_t رقم, ثابت نویسه *path)
{
	نویسه buf[اندازه(stringify_مقدار(LLONG_MAX))];
	صحیح rc, errsv;
	صحیح fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	اگر (fd < 0)
		بازگشت -errno;

	len = snprintf(buf, اندازه(buf), "%" PRId64, رقم);
	اگر (len < 0 || (size_t) len >= اندازه(buf))
		rc = len < 0 ? -errno : -E2BIG;
	وگرنه
		rc = ul_vfs_write_all(pc ? pc->vfs : پوچ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : پوچ, fd);
	errno = errsv;
	بازگشت rc;
}

صحیح ul_path_write_u64(ساختار path_cxt *pc, uint64_t رقم, ثابت نویسه *path)
{
	نویسه buf[اندازه(stringify_مقدار(ULLONG_MAX))];
	صحیح rc, errsv;
	صحیح fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	اگر (fd < 0)
		بازگشت -errno;

	len = snprintf(buf, اندازه(buf), "%" PRIu64, رقم);
	اگر (len < 0 || (size_t) len >= اندازه(buf))
		rc = len < 0 ? -errno : -E2BIG;
	وگرنه
		rc = ul_vfs_write_all(pc ? pc->vfs : پوچ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : پوچ, fd);
	errno = errsv;
	بازگشت rc;
}

صحیح ul_path_writef_u64(ساختار path_cxt *pc, uint64_t رقم, ثابت نویسه *path, ...)
{
	ثابت نویسه *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	بازگشت !p ? -errno : ul_path_write_u64(pc, رقم, p);

}

صحیح ul_path_شمار_dirents(ساختار path_cxt *pc, ثابت نویسه *path)
{
	DIR *dir;
	صحیح r = 0;

	dir = ul_path_opendir(pc, path);
	اگر (!dir)
		بازگشت 0;

	وقتی (xreaddir(dir)) r++;

	closedir(dir);
	بازگشت r;
}

صحیح ul_path_countf_dirents(ساختار path_cxt *pc, ثابت نویسه *path, ...)
{
	ثابت نویسه *p;
	va_list ap;

	va_start(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	va_end(ap);

	بازگشت !p ? -errno : ul_path_شمار_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
صحیح ul_path_next_dirent(ساختار path_cxt *pc, DIR **sub, ثابت نویسه *dirname, ساختار dirent **d)
{
	اگر (!pc || !sub || !d)
		بازگشت -EINVAL;

	اگر (!*sub) {
		*sub = ul_path_opendir(pc, dirname);
		اگر (!*sub)
			بازگشت -errno;
	}

	*d = xreaddir(*sub);
	اگر (*d)
		بازگشت 0;

	closedir(*sub);
	*sub = پوچ;
	بازگشت 1;
}

#ifdef HAVE_CPU_SET_T
ایستا صحیح ul_path_cpuparse(ساختار path_cxt *pc, cpu_set_t **set, صحیح maxcpus, صحیح islist, ثابت نویسه *path, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	نویسه *buf;
	صحیح rc;

	*set = پوچ;

	buf = malloc(len);
	اگر (!buf)
		بازگشت -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	اگر (rc < 0)
		برو out;

	*set = cpuset_alloc(maxcpus, &setsize, پوچ);
	اگر (!*set) {
		rc = -EINVAL;
		برو out;
	}

	اگر (islist) {
		اگر (cpulist_parse(buf, *set, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			برو out;
		}
	} وگرنه {
		اگر (cpumask_parse(buf, *set, setsize)) {
			errno = EINVAL;
			rc = -errno;
			برو out;
		}
	}
	rc = 0;

out:
	اگر (rc) {
		cpuset_free(*set);
		*set = پوچ;
	}
	آزاد(buf);
	بازگشت rc;
}

صحیح ul_path_readf_cpuset(ساختار path_cxt *pc, cpu_set_t **set, صحیح maxcpus, ثابت نویسه *path, ...)
{
	va_list ap;
	صحیح rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	va_end(ap);

	بازگشت rc;
}

صحیح ul_path_readf_cpulist(ساختار path_cxt *pc, cpu_set_t **set, صحیح maxcpus, ثابت نویسه *path, ...)
{
	va_list ap;
	صحیح rc = 0;

	va_start(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	va_end(ap);

	بازگشت rc;
}

#endif /* HAVE_CPU_SET_T */


#ifdef TEST_PROGRAM_PATH
#شامل <getopt.h>

ایستا تهی __attribute__((__noreturn__)) usage(تهی)
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

صحیح اصلی(صحیح argc, نویسه *argv[])
{
	صحیح c;
	ثابت نویسه *prefix = پوچ, *dir, *file, *command;
	ساختار path_cxt *pc = پوچ;

	ایستا ثابت ساختار option longopts[] = {
		{ "prefix",	1, پوچ, 'p' },
		{ "help",       0, پوچ, 'h' },
		{ پوچ, 0, پوچ, 0 },
	};

	وقتی((c = getopt_long(argc, argv, "p:h", longopts, پوچ)) != -1) {
		انتخاب(c) {
		حالت 'p':
			prefix = optarg;
			بشکن;
		حالت 'h':
			usage();
			بشکن;
		پیشفرض:
			err(EXIT_FAILURE, "try --help");
		}
	}

	اگر (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	اگر (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	اگر (prefix)
		ul_path_set_prefix(pc, prefix);

	اگر (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	اگر (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		اگر (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		اگر (ul_path_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %u\n", file, res);

		اگر (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %u\n", file, res);

	} وگرنه اگر (strcmp(command, "read-s32") == 0) {
		int32_t res;

		اگر (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		اگر (ul_path_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %d\n", file, res);

		اگر (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %d\n", file, res);

	} وگرنه اگر (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		اگر (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		اگر (ul_path_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRIu64 "\n", file, res);

		اگر (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRIu64 "\n", file, res);

	} وگرنه اگر (strcmp(command, "read-s64") == 0) {
		int64_t res;

		اگر (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		اگر (ul_path_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		printf("read:  %s: %" PRId64 "\n", file, res);

		اگر (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		printf("readf: %s: %" PRId64 "\n", file, res);

	} وگرنه اگر (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		اگر (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		اگر (ul_path_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		printf("read:  %s: %d\n", file, (صحیح) res);

		اگر (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		printf("readf: %s: %d\n", file, (صحیح) res);

	} وگرنه اگر (strcmp(command, "read-string") == 0) {
		نویسه *res;

		اگر (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		اگر (ul_path_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		printf("read:  %s: %s\n", file, res);

		اگر (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		printf("readf: %s: %s\n", file, res);

	} وگرنه اگر (strcmp(command, "read-link") == 0) {
		نویسه res[PATH_MAX];

		اگر (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		اگر (ul_path_readlink(pc, res, اندازه(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		printf("read:  %s: %s\n", file, res);

		اگر (ul_path_readlinkf(pc, res, اندازه(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		printf("readf: %s: %s\n", file, res);

	} وگرنه اگر (strcmp(command, "write-string") == 0) {
		نویسه *str;

		اگر (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		اگر (ul_path_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		اگر (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} وگرنه اگر (strcmp(command, "write-u64") == 0) {
		uint64_t رقم;

		اگر (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		رقم = strtoumax(argv[optind++], پوچ, 0);

		اگر (ul_path_write_u64(pc, رقم, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		اگر (ul_path_writef_u64(pc, رقم, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_path(pc);
	بازگشت EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PATH */
