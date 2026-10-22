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
ساکن UL_DEBUG_DEFINE_نقاب(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_خالی_خانے_MASKNAMES;

#تعریف_کرو ULPATH_DEBUG_INIT	(1 << 1)
#تعریف_کرو ULPATH_DEBUG_CXT	(1 << 2)

#تعریف_کرو DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#تعریف_کرو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#تعریف_کرو ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

خالی ul_راستہ_init_debug(خالی)
{
	اگر (ulpath_debug_نقاب)
		واپس;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

ساخت راستہ_cxt *ul_new_راستہ(ثابت حرف *dir, ...)
{
	ساخت راستہ_cxt *pc = صفر_حافظہ_دو(1, حجم(*pc));

	اگر (!pc)
		واپس تہی;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	اگر (dir) {
		صحیح rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_راستہ, dir, ap);
		va_end(ap);

		اگر (rc < 0 || !pc->dir_راستہ)
			جاؤ fail;
	}
	واپس pc;
fail:
	ul_unref_راستہ(pc);
	واپس تہی;
}

خالی ul_ref_راستہ(ساخت راستہ_cxt *pc)
{
	اگر (pc)
		pc->refcount++;
}

خالی ul_unref_راستہ(ساخت راستہ_cxt *pc)
{
	اگر (!pc)
		واپس;

	pc->refcount--;

	اگر (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		اگر (pc->dialect)
			pc->free_dialect(pc);
		ul_راستہ_close_dirfd(pc);
		آزاد(pc->dir_راستہ);
		آزاد(pc->سابقہ);
		آزاد(pc);
	}
}

خالی ul_راستہ_refer_vfs(ساخت راستہ_cxt *pc, ثابت ساخت ul_vfs_ops *vfs)
{
	اگر (pc)
		pc->vfs = vfs;
}

صحیح ul_راستہ_سیٹ_سابقہ(ساخت راستہ_cxt *pc, ثابت حرف *سابقہ)
{
	حرف *p = تہی;

	أكد(pc->dir_fd < 0);

	اگر (سابقہ) {
		p = strdup(سابقہ);
		اگر (!p)
			واپس -ENOMEM;
	}

	آزاد(pc->سابقہ);
	pc->سابقہ = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	واپس 0;
}

ثابت حرف *ul_راستہ_get_سابقہ(ساخت راستہ_cxt *pc)
{
	واپس pc ? pc->سابقہ : تہی;
}

صحیح ul_راستہ_سیٹ_dir(ساخت راستہ_cxt *pc, ثابت حرف *dir)
{
	حرف *p = تہی;

	اگر (dir) {
		p = strdup(dir);
		اگر (!p)
			واپس -ENOMEM;
	}

	اگر (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	آزاد(pc->dir_راستہ);
	pc->dir_راستہ = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	واپس 0;
}

ثابت حرف *ul_راستہ_get_dir(ساخت راستہ_cxt *pc)
{
	واپس pc ? pc->dir_راستہ : تہی;
}

صحیح ul_راستہ_سیٹ_dialect(ساخت راستہ_cxt *pc, خالی *ڈیٹا, خالی free_ڈیٹا(ساخت راستہ_cxt *))
{
	pc->dialect = ڈیٹا;
	pc->free_dialect = free_ڈیٹا;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	واپس 0;
}

خالی *ul_راستہ_get_dialect(ساخت راستہ_cxt *pc)
{
	واپس pc ? pc->dialect : تہی;
}

صحیح ul_راستہ_سیٹ_enoent_redirect(ساخت راستہ_cxt *pc, صحیح (*func)(ساخت راستہ_cxt *, ثابت حرف *, صحیح *))
{
	pc->redirect_on_enoent = func;
	واپس 0;
}

ساکن ثابت حرف *get_absdir(ساخت راستہ_cxt *pc)
{
	صحیح rc;
	ثابت حرف *dirpath;

	اگر (!pc->سابقہ)
		واپس pc->dir_راستہ;

	dirpath = pc->dir_راستہ;
	اگر (!dirpath)
		واپس pc->سابقہ;
	اگر (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->راستہ_بفر, حجم(pc->راستہ_بفر), "%s/%s", pc->سابقہ, dirpath);
	اگر (rc < 0)
		واپس تہی;
	اگر ((size_t)rc >= حجم(pc->راستہ_بفر)) {
		errno = ENAMETOOLONG;
		واپس تہی;
	}

	واپس pc->راستہ_بفر;
}

صحیح ul_راستہ_is_accessible(ساخت راستہ_cxt *pc)
{
	ثابت حرف *راستہ;
	أكد(pc);

	اگر (pc->dir_fd >= 0)
		واپس 1;

	راستہ = get_absdir(pc);
	اگر (!راستہ)
		واپس 0;
	واپس access(راستہ, F_OK) == 0;
}

صحیح ul_راستہ_get_dirfd(ساخت راستہ_cxt *pc)
{
	أكد(pc);
	أكد(pc->dir_راستہ);

	اگر (pc->dir_fd < 0) {
		ثابت حرف *راستہ = get_absdir(pc);
		اگر (!راستہ)
			واپس -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", راستہ));
		pc->dir_fd = open(راستہ, O_RDONLY|O_CLOEXEC);
	}

	واپس pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
خالی ul_راستہ_close_dirfd(ساخت راستہ_cxt *pc)
{
	أكد(pc);

	اگر (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

صحیح ul_راستہ_isopen_dirfd(ساخت راستہ_cxt *pc)
{
	واپس pc && pc->dir_fd >= 0;
}

ساکن ثابت حرف *ul_راستہ_mkpath(ساخت راستہ_cxt *pc, ثابت حرف *راستہ, va_list ap)
{
	صحیح rc;

	errno = 0;

	rc = vsnprintf(pc->راستہ_بفر, حجم(pc->راستہ_بفر), راستہ, ap);
	اگر (rc < 0) {
		اگر (!errno)
			errno = EINVAL;
		واپس تہی;
	}

	اگر ((size_t)rc >= حجم(pc->راستہ_بفر)) {
		errno = ENAMETOOLONG;
		واپس تہی;
	}

	واپس pc->راستہ_بفر;
}

حرف *ul_راستہ_get_abspath(ساخت راستہ_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *راستہ, ...)
{
	اگر (راستہ) {
		صحیح rc;
		va_list ap;
		ثابت حرف *دم = تہی, *dirpath = pc->dir_راستہ;

		va_start(ap, راستہ);
		دم = ul_راستہ_mkpath(pc, راستہ, ap);
		va_end(ap);

		اگر (dirpath && *dirpath == '/')
			dirpath++;
		اگر (دم && *دم == '/')
			دم++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->سابقہ ? pc->سابقہ : "",
				dirpath ? dirpath : "",
				دم ? دم : "");

		اگر ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			واپس تہی;
		}
	} ورنہ {
		ثابت حرف *tmp = get_absdir(pc);

		اگر (!tmp)
			واپس تہی;
		xstrncpy(buf, tmp, bufsz);
	}

	واپس buf;
}


صحیح ul_راستہ_access(ساخت راستہ_cxt *pc, صحیح منوال, ثابت حرف *راستہ)
{
	صحیح rc;

	اگر (!راستہ)
		واپس -EINVAL;
	اگر (!pc) {
		rc = access(راستہ, منوال);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", راستہ, rc));
	} ورنہ {
		صحیح dir = ul_راستہ_get_dirfd(pc);
		اگر (dir < 0)
			واپس dir;
		اگر (*راستہ == '/')
			راستہ++;

		rc = faccessat(dir, راستہ, منوال, 0);

		اگر (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, راستہ, &dir) == 0)
			rc = faccessat(dir, راستہ, منوال, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", راستہ, rc));
	}
	واپس rc;
}

صحیح ul_راستہ_accessf(ساخت راستہ_cxt *pc, صحیح منوال, ثابت حرف *راستہ, ...)
{
	va_list ap;
	ثابت حرف *p;

	va_start(ap, راستہ);
	p = ul_راستہ_mkpath(pc, راستہ, ap);
	va_end(ap);

	واپس !p ? -errno : ul_راستہ_access(pc, منوال, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
صحیح ul_راستہ_stat(ساخت راستہ_cxt *pc, ساخت stat *sb, صحیح flags, ثابت حرف *راستہ)
{
	صحیح rc;

	اگر (!pc) {
		rc = راستہ ? stat(راستہ, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", راستہ, rc));
	} ورنہ {
		صحیح dir = ul_راستہ_get_dirfd(pc);
		اگر (dir < 0)
			واپس dir;
		اگر (راستہ) {
			اگر  (*راستہ == '/')
				راستہ++;
			rc = fstatat(dir, راستہ, sb, flags);

		} ورنہ
			rc = fstat(dir, sb);	/* dir itself */

		اگر (rc && errno == ENOENT
		    && راستہ
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, راستہ, &dir) == 0)
			rc = fstatat(dir, راستہ, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", راستہ, rc));
	}
	واپس rc;
}

صحیح ul_راستہ_vstatf(ساخت راستہ_cxt *pc, ساخت stat *sb, صحیح flags, ثابت حرف *راستہ, va_list ap)
{
	ثابت حرف *p = ul_راستہ_mkpath(pc, راستہ, ap);

	واپس !p ? -errno : ul_راستہ_stat(pc, sb, flags, p);
}

صحیح ul_راستہ_statf(ساخت راستہ_cxt *pc, ساخت stat *sb, صحیح flags, ثابت حرف *راستہ, ...)
{
	va_list ap;
	صحیح rc;

	va_start(ap, راستہ);
	rc = ul_راستہ_vstatf(pc, sb, flags, راستہ, ap);
	va_end(ap);

	واپس rc;
}
#اگر_تعریف_شدہ HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
صحیح ul_راستہ_statx(ساخت راستہ_cxt *pc, ساخت statx *stx, صحیح flags, غیرعلامتی صحیح نقاب,
		  ثابت حرف *راستہ)
{
	صحیح rc;

	اگر (!راستہ)
		واپس -EINVAL;
	اگر (!pc)
		rc = راستہ ? statx(AT_FDCWD, راستہ, flags, نقاب, stx) : - EINVAL;
	ورنہ {
		صحیح dir = ul_راستہ_get_dirfd(pc);
		اگر (dir < 0)
			واپس dir;
		اگر (*راستہ == '/')
			راستہ++;

		rc = statx(dir, راستہ, flags, نقاب, stx);
	}

	واپس rc;
}
#ورنہ
صحیح ul_راستہ_statx(ساخت راستہ_cxt *pc __attribute__((__unused__)),
		  ساخت statx *stx __attribute__((__unused__)),
		  صحیح flags __attribute__((__unused__)),
		  غیرعلامتی صحیح نقاب __attribute__((__unused__)),
		  ثابت حرف *راستہ __attribute__((__unused__)))
{
	errno = ENOSYS;
	واپس -1;
}
#اگر_ختم /* HAVE_STATX */

صحیح ul_راستہ_vstatxf(ساخت راستہ_cxt *pc, ساخت statx *stx, صحیح flags, غیرعلامتی صحیح نقاب,
		    ثابت حرف *راستہ, va_list ap)
{
	ثابت حرف *p = ul_راستہ_mkpath(pc, راستہ, ap);

	واپس !p ? -errno : ul_راستہ_statx(pc, stx, flags, نقاب, p);
}

صحیح ul_راستہ_statxf(ساخت راستہ_cxt *pc, ساخت statx *stx, صحیح flags, غیرعلامتی صحیح نقاب,
		   ثابت حرف *راستہ, ...)
{
	va_list ap;
	صحیح rc;

	va_start(ap, راستہ);
	rc = ul_راستہ_vstatxf(pc, stx, flags, نقاب, راستہ, ap);
	va_end(ap);

	واپس rc;
}

صحیح ul_راستہ_open(ساخت راستہ_cxt *pc, صحیح flags, ثابت حرف *راستہ)
{
	صحیح fd;

	اگر (!راستہ)
		واپس -EINVAL;
	اگر (!pc) {
		fd = open(راستہ, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", راستہ));
	} ورنہ {
		صحیح fdx;
		صحیح dir = ul_راستہ_get_dirfd(pc);
		اگر (dir < 0)
			واپس dir;

		اگر (*راستہ == '/')
			راستہ++;

		fdx = fd = openat(dir, راستہ, flags);

		اگر (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, راستہ, &dir) == 0)
			fd = openat(dir, راستہ, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", راستہ, fdx != fd ? " [redirected]" : ""));
	}
	واپس fd;
}

صحیح ul_راستہ_vopenf(ساخت راستہ_cxt *pc, صحیح flags, ثابت حرف *راستہ, va_list ap)
{
	ثابت حرف *p = ul_راستہ_mkpath(pc, راستہ, ap);

	واپس !p ? -errno : ul_راستہ_open(pc, flags, p);
}

صحیح ul_راستہ_openf(ساخت راستہ_cxt *pc, صحیح flags, ثابت حرف *راستہ, ...)
{
	va_list ap;
	صحیح rc;

	va_start(ap, راستہ);
	rc = ul_راستہ_vopenf(pc, flags, راستہ, ap);
	va_end(ap);

	واپس rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_راستہ_fopen(ساخت راستہ_cxt *pc, ثابت حرف *منوال, ثابت حرف *راستہ)
{
	صحیح flags = ul_منوال_مقصد_flags(منوال);
	صحیح fd = ul_راستہ_open(pc, flags, راستہ);

	اگر (fd < 0)
		واپس تہی;

	واپس ul_vfs_fdopen(pc ? pc->vfs : تہی, fd, منوال);
}


FILE *ul_راستہ_vfopenf(ساخت راستہ_cxt *pc, ثابت حرف *منوال, ثابت حرف *راستہ, va_list ap)
{
	ثابت حرف *p = ul_راستہ_mkpath(pc, راستہ, ap);

	واپس !p ? تہی : ul_راستہ_fopen(pc, منوال, p);
}

FILE *ul_راستہ_fopenf(ساخت راستہ_cxt *pc, ثابت حرف *منوال, ثابت حرف *راستہ, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, راستہ);
	f = ul_راستہ_vfopenf(pc, منوال, راستہ, ap);
	va_end(ap);

	واپس f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_راستہ_opendir(ساخت راستہ_cxt *pc, ثابت حرف *راستہ)
{
	DIR *dir;
	صحیح fd = -1;

	اگر (راستہ)
		fd = ul_راستہ_open(pc, O_RDONLY|O_CLOEXEC, راستہ);
	ورنہ اگر (pc->dir_راستہ) {
		صحیح dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_راستہ_get_dirfd(pc);
		اگر (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	اگر (fd < 0)
		واپس تہی;

	dir = fdopendir(fd);
	اگر (!dir) {
		close(fd);
		واپس تہی;
	}
	اگر (!راستہ)
		 rewinddir(dir);
	واپس dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_راستہ_vopendirf(ساخت راستہ_cxt *pc, ثابت حرف *راستہ, va_list ap)
{
	ثابت حرف *p = ul_راستہ_mkpath(pc, راستہ, ap);

	واپس !p ? تہی : ul_راستہ_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_راستہ_opendirf(ساخت راستہ_cxt *pc, ثابت حرف *راستہ, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, راستہ);
	dir = ul_راستہ_vopendirf(pc, راستہ, ap);
	va_end(ap);

	واپس dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_راستہ_readlink(ساخت راستہ_cxt *pc, حرف *buf, size_t bufsiz, ثابت حرف *راستہ)
{
	صحیح dirfd;
	ssize_t ssz;

	اگر (!راستہ) {
		ثابت حرف *p = get_absdir(pc);
		اگر (!p)
			واپس -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} ورنہ {
		dirfd = ul_راستہ_get_dirfd(pc);
		اگر (dirfd < 0)
			واپس dirfd;

		اگر (*راستہ == '/')
			راستہ++;

		ssz = readlinkat(dirfd, راستہ, buf, bufsiz - 1);
	}

	اگر (ssz >= 0)
		buf[ssz] = '\0';
	واپس ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_راستہ_readlinkf(ساخت راستہ_cxt *pc, حرف *buf, size_t bufsiz, ثابت حرف *راستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, راستہ);
	p = ul_راستہ_mkpath(pc, راستہ, ap);
	va_end(ap);

	واپس !p ? -errno : ul_راستہ_readlink(pc, buf, bufsiz, p);
}

صحیح ul_راستہ_read(ساخت راستہ_cxt *pc, حرف *buf, size_t len, ثابت حرف *راستہ)
{
	صحیح rc, errsv;
	صحیح fd;

	fd = ul_راستہ_open(pc, O_RDONLY|O_CLOEXEC, راستہ);
	اگر (fd < 0)
		واپس -errno;

	DBG(CXT, ul_debug(" reading '%s'", راستہ));
	rc = ul_vfs_read_all(pc ? pc->vfs : تہی, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : تہی, fd);
	errno = errsv;
	واپس rc;
}

صحیح ul_راستہ_vreadf(ساخت راستہ_cxt *pc, حرف *buf, size_t len, ثابت حرف *راستہ, va_list ap)
{
	ثابت حرف *p = ul_راستہ_mkpath(pc, راستہ, ap);

	واپس !p ? -errno : ul_راستہ_read(pc, buf, len, p);
}

صحیح ul_راستہ_readf(ساخت راستہ_cxt *pc, حرف *buf, size_t len, ثابت حرف *راستہ, ...)
{
	va_list ap;
	صحیح rc;

	va_start(ap, راستہ);
	rc = ul_راستہ_vreadf(pc, buf, len, راستہ, ap);
	va_end(ap);

	واپس rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
صحیح ul_راستہ_read_string(ساخت راستہ_cxt *pc, حرف **str, ثابت حرف *راستہ)
{
	حرف buf[BUFSIZ];
	صحیح rc;

	اگر (!str)
		واپس -EINVAL;

	*str = تہی;

	rc = ul_راستہ_read_بفر(pc, buf, حجم(buf), راستہ);
	اگر (rc < 0)
		واپس rc;

	*str = strdup(buf);
	اگر (!*str)
		rc = -ENOMEM;

	واپس rc;
}

صحیح ul_راستہ_readf_string(ساخت راستہ_cxt *pc, حرف **str, ثابت حرف *راستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, راستہ);
	p = ul_راستہ_mkpath(pc, راستہ, ap);
	va_end(ap);

	واپس !p ? -errno : ul_راستہ_read_string(pc, str, p);
}

صحیح ul_راستہ_read_بفر(ساخت راستہ_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *راستہ)
{
	صحیح rc = ul_راستہ_read(pc, buf, bufsz - 1, راستہ);

	اگر (rc == 0)
		buf[0] = '\0';

	ورنہ اگر (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		اگر (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		ورنہ
			buf[rc] = '\0';
	}

	واپس rc;
}

صحیح ul_راستہ_vreadf_بفر(ساخت راستہ_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *راستہ, va_list ap)
{
	ثابت حرف *p;

	p = ul_راستہ_mkpath(pc, راستہ, ap);

	واپس !p ? -errno : ul_راستہ_read_بفر(pc, buf, bufsz, p);
}

صحیح ul_راستہ_readf_بفر(ساخت راستہ_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *راستہ, ...)
{
	va_list ap;
	صحیح rc;

	va_start(ap, راستہ);
	rc = ul_راستہ_vreadf_بفر(pc, buf, bufsz, راستہ, ap);
	va_end(ap);

	واپس rc;
}

صحیح ul_راستہ_scanf(ساخت راستہ_cxt *pc, ثابت حرف *راستہ, ثابت حرف *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	صحیح rc;

	f = ul_راستہ_fopen(pc, "r" UL_CLOEXECSTR, راستہ);
	اگر (!f)
		واپس -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, راستہ));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	واپس rc;
}

صحیح ul_راستہ_scanff(ساخت راستہ_cxt *pc, ثابت حرف *راستہ, va_list ap, ثابت حرف *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	صحیح rc;

	f = ul_راستہ_vfopenf(pc, "r" UL_CLOEXECSTR, راستہ, ap);
	اگر (!f)
		واپس -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	واپس rc;
}


صحیح ul_راستہ_read_s64(ساخت راستہ_cxt *pc, int64_t *res, ثابت حرف *راستہ)
{
	int64_t x = 0;
	صحیح rc;

	rc = ul_راستہ_scanf(pc, راستہ, "%"SCNd64, &x);
	اگر (rc != 1)
		واپس -1;
	اگر (res)
		*res = x;
	واپس 0;
}

صحیح ul_راستہ_readf_s64(ساخت راستہ_cxt *pc, int64_t *res, ثابت حرف *راستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, راستہ);
	p = ul_راستہ_mkpath(pc, راستہ, ap);
	va_end(ap);

	واپس !p ? -errno : ul_راستہ_read_s64(pc, res, p);
}

صحیح ul_راستہ_read_u64(ساخت راستہ_cxt *pc, uint64_t *res, ثابت حرف *راستہ)
{
	uint64_t x = 0;
	صحیح rc;

	rc = ul_راستہ_scanf(pc, راستہ, "%"SCNu64, &x);
	اگر (rc != 1)
		واپس -1;
	اگر (res)
		*res = x;
	واپس 0;
}

صحیح ul_راستہ_readf_u64(ساخت راستہ_cxt *pc, uint64_t *res, ثابت حرف *راستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, راستہ);
	p = ul_راستہ_mkpath(pc, راستہ, ap);
	va_end(ap);

	واپس !p ? -errno : ul_راستہ_read_u64(pc, res, p);
}

صحیح ul_راستہ_read_s32(ساخت راستہ_cxt *pc, صحیح *res, ثابت حرف *راستہ)
{
	صحیح rc, x = 0;

	rc = ul_راستہ_scanf(pc, راستہ, "%d", &x);
	اگر (rc != 1)
		واپس -1;
	اگر (res)
		*res = x;
	واپس 0;
}

صحیح ul_راستہ_readf_s32(ساخت راستہ_cxt *pc, صحیح *res, ثابت حرف *راستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, راستہ);
	p = ul_راستہ_mkpath(pc, راستہ, ap);
	va_end(ap);

	واپس !p ? -errno : ul_راستہ_read_s32(pc, res, p);
}

صحیح ul_راستہ_read_u32(ساخت راستہ_cxt *pc, غیرعلامتی صحیح *res, ثابت حرف *راستہ)
{
	صحیح rc;
	غیرعلامتی صحیح x = 0;

	rc = ul_راستہ_scanf(pc, راستہ, "%u", &x);
	اگر (rc != 1)
		واپس -1;
	اگر (res)
		*res = x;
	واپس 0;
}

صحیح ul_راستہ_readf_u32(ساخت راستہ_cxt *pc, غیرعلامتی صحیح *res, ثابت حرف *راستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, راستہ);
	p = ul_راستہ_mkpath(pc, راستہ, ap);
	va_end(ap);

	واپس !p ? -errno : ul_راستہ_read_u32(pc, res, p);
}

صحیح ul_راستہ_read_majmin(ساخت راستہ_cxt *pc, dev_t *res, ثابت حرف *راستہ)
{
	صحیح rc, maj = 0, min = 0;

	rc = ul_راستہ_scanf(pc, راستہ, "%d:%d", &maj, &min);
	اگر (rc != 2)
		واپس -1;
	اگر (res)
		*res = makedev(maj, min);
	واپس 0;
}

صحیح ul_راستہ_readf_majmin(ساخت راستہ_cxt *pc, dev_t *res, ثابت حرف *راستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, راستہ);
	p = ul_راستہ_mkpath(pc, راستہ, ap);
	va_end(ap);

	واپس !p ? -errno : ul_راستہ_read_majmin(pc, res, p);
}

صحیح ul_راستہ_write_string(ساخت راستہ_cxt *pc, ثابت حرف *str, ثابت حرف *راستہ)
{
	صحیح rc, errsv;
	صحیح fd;

	fd = ul_راستہ_open(pc, O_WRONLY|O_CLOEXEC, راستہ);
	اگر (fd < 0)
		واپس -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : تہی, fd, str, لڑی_لمبائی(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : تہی, fd);
	errno = errsv;
	واپس rc;
}

صحیح ul_راستہ_writef_string(ساخت راستہ_cxt *pc, ثابت حرف *str, ثابت حرف *راستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, راستہ);
	p = ul_راستہ_mkpath(pc, راستہ, ap);
	va_end(ap);

	واپس !p ? -errno : ul_راستہ_write_string(pc, str, p);
}

صحیح ul_راستہ_write_s64(ساخت راستہ_cxt *pc, int64_t عدد, ثابت حرف *راستہ)
{
	حرف buf[حجم(stringify_قدر(LLONG_MAX))];
	صحیح rc, errsv;
	صحیح fd, len;

	fd = ul_راستہ_open(pc, O_WRONLY|O_CLOEXEC, راستہ);
	اگر (fd < 0)
		واپس -errno;

	len = snprintf(buf, حجم(buf), "%" PRId64, عدد);
	اگر (len < 0 || (size_t) len >= حجم(buf))
		rc = len < 0 ? -errno : -E2BIG;
	ورنہ
		rc = ul_vfs_write_all(pc ? pc->vfs : تہی, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : تہی, fd);
	errno = errsv;
	واپس rc;
}

صحیح ul_راستہ_write_u64(ساخت راستہ_cxt *pc, uint64_t عدد, ثابت حرف *راستہ)
{
	حرف buf[حجم(stringify_قدر(ULLONG_MAX))];
	صحیح rc, errsv;
	صحیح fd, len;

	fd = ul_راستہ_open(pc, O_WRONLY|O_CLOEXEC, راستہ);
	اگر (fd < 0)
		واپس -errno;

	len = snprintf(buf, حجم(buf), "%" PRIu64, عدد);
	اگر (len < 0 || (size_t) len >= حجم(buf))
		rc = len < 0 ? -errno : -E2BIG;
	ورنہ
		rc = ul_vfs_write_all(pc ? pc->vfs : تہی, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : تہی, fd);
	errno = errsv;
	واپس rc;
}

صحیح ul_راستہ_writef_u64(ساخت راستہ_cxt *pc, uint64_t عدد, ثابت حرف *راستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, راستہ);
	p = ul_راستہ_mkpath(pc, راستہ, ap);
	va_end(ap);

	واپس !p ? -errno : ul_راستہ_write_u64(pc, عدد, p);

}

صحیح ul_راستہ_گنتی_dirents(ساخت راستہ_cxt *pc, ثابت حرف *راستہ)
{
	DIR *dir;
	صحیح r = 0;

	dir = ul_راستہ_opendir(pc, راستہ);
	اگر (!dir)
		واپس 0;

	جب (xreaddir(dir)) r++;

	closedir(dir);
	واپس r;
}

صحیح ul_راستہ_countf_dirents(ساخت راستہ_cxt *pc, ثابت حرف *راستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, راستہ);
	p = ul_راستہ_mkpath(pc, راستہ, ap);
	va_end(ap);

	واپس !p ? -errno : ul_راستہ_گنتی_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
صحیح ul_راستہ_اگلا_dirent(ساخت راستہ_cxt *pc, DIR **sub, ثابت حرف *dirname, ساخت dirent **d)
{
	اگر (!pc || !sub || !d)
		واپس -EINVAL;

	اگر (!*sub) {
		*sub = ul_راستہ_opendir(pc, dirname);
		اگر (!*sub)
			واپس -errno;
	}

	*d = xreaddir(*sub);
	اگر (*d)
		واپس 0;

	closedir(*sub);
	*sub = تہی;
	واپس 1;
}

#اگر_تعریف_شدہ HAVE_CPU_سیٹ_T
ساکن صحیح ul_راستہ_cpuparse(ساخت راستہ_cxt *pc, cpu_set_t **سیٹ, صحیح maxcpus, صحیح islist, ثابت حرف *راستہ, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	حرف *buf;
	صحیح rc;

	*سیٹ = تہی;

	buf = حافظہ_دو(len);
	اگر (!buf)
		واپس -ENOMEM;

	rc = ul_راستہ_vreadf_بفر(pc, buf, len, راستہ, ap);
	اگر (rc < 0)
		جاؤ برآمد;

	*سیٹ = cpuset_alloc(maxcpus, &setsize, تہی);
	اگر (!*سیٹ) {
		rc = -EINVAL;
		جاؤ برآمد;
	}

	اگر (islist) {
		اگر (cpulist_parse(buf, *سیٹ, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			جاؤ برآمد;
		}
	} ورنہ {
		اگر (cpumask_parse(buf, *سیٹ, setsize)) {
			errno = EINVAL;
			rc = -errno;
			جاؤ برآمد;
		}
	}
	rc = 0;

برآمد:
	اگر (rc) {
		cpuset_free(*سیٹ);
		*سیٹ = تہی;
	}
	آزاد(buf);
	واپس rc;
}

صحیح ul_راستہ_readf_cpuset(ساخت راستہ_cxt *pc, cpu_set_t **سیٹ, صحیح maxcpus, ثابت حرف *راستہ, ...)
{
	va_list ap;
	صحیح rc = 0;

	va_start(ap, راستہ);
	rc = ul_راستہ_cpuparse(pc, سیٹ, maxcpus, 0, راستہ, ap);
	va_end(ap);

	واپس rc;
}

صحیح ul_راستہ_readf_cpulist(ساخت راستہ_cxt *pc, cpu_set_t **سیٹ, صحیح maxcpus, ثابت حرف *راستہ, ...)
{
	va_list ap;
	صحیح rc = 0;

	va_start(ap, راستہ);
	rc = ul_راستہ_cpuparse(pc, سیٹ, maxcpus, 1, راستہ, ap);
	va_end(ap);

	واپس rc;
}

#اگر_ختم /* HAVE_CPU_SET_T */


#اگر_تعریف_شدہ TEST_پروگرام_راستہ
#شامل <getopt.h>

ساکن خالی __attribute__((__noreturn__)) usage(خالی)
{
	fprintf(stdout, " %s [options] <dir> <command>\n\n", پروگرام_invocation_short_name);
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

	باہر_نکلو(EXIT_SUCCESS);
}

صحیح مین(صحیح argc, حرف *argv[])
{
	صحیح c;
	ثابت حرف *سابقہ = تہی, *dir, *file, *command;
	ساخت راستہ_cxt *pc = تہی;

	ساکن ثابت ساخت option longopts[] = {
		{ "prefix",	1, تہی, 'p' },
		{ "help",       0, تہی, 'h' },
		{ تہی, 0, تہی, 0 },
	};

	جب((c = getopt_long(argc, argv, "p:h", longopts, تہی)) != -1) {
		انتخاب(c) {
		حالت 'p':
			سابقہ = optarg;
			توڑو;
		حالت 'h':
			usage();
			توڑو;
		طےشدہ:
			err(EXIT_FAILURE, "try --help");
		}
	}

	اگر (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_راستہ_init_debug();

	pc = ul_new_راستہ("%s", dir);
	اگر (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	اگر (سابقہ)
		ul_راستہ_سیٹ_سابقہ(pc, سابقہ);

	اگر (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	اگر (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		اگر (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		اگر (ul_راستہ_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		لکھو("read:  %s: %u\n", file, res);

		اگر (ul_راستہ_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		لکھو("readf: %s: %u\n", file, res);

	} ورنہ اگر (strcmp(command, "read-s32") == 0) {
		int32_t res;

		اگر (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		اگر (ul_راستہ_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		لکھو("read:  %s: %d\n", file, res);

		اگر (ul_راستہ_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		لکھو("readf: %s: %d\n", file, res);

	} ورنہ اگر (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		اگر (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		اگر (ul_راستہ_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		لکھو("read:  %s: %" PRIu64 "\n", file, res);

		اگر (ul_راستہ_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		لکھو("readf: %s: %" PRIu64 "\n", file, res);

	} ورنہ اگر (strcmp(command, "read-s64") == 0) {
		int64_t res;

		اگر (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		اگر (ul_راستہ_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		لکھو("read:  %s: %" PRId64 "\n", file, res);

		اگر (ul_راستہ_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		لکھو("readf: %s: %" PRId64 "\n", file, res);

	} ورنہ اگر (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		اگر (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		اگر (ul_راستہ_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		لکھو("read:  %s: %d\n", file, (صحیح) res);

		اگر (ul_راستہ_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		لکھو("readf: %s: %d\n", file, (صحیح) res);

	} ورنہ اگر (strcmp(command, "read-string") == 0) {
		حرف *res;

		اگر (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		اگر (ul_راستہ_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		لکھو("read:  %s: %s\n", file, res);

		اگر (ul_راستہ_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		لکھو("readf: %s: %s\n", file, res);

	} ورنہ اگر (strcmp(command, "read-link") == 0) {
		حرف res[PATH_MAX];

		اگر (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		اگر (ul_راستہ_readlink(pc, res, حجم(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		لکھو("read:  %s: %s\n", file, res);

		اگر (ul_راستہ_readlinkf(pc, res, حجم(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		لکھو("readf: %s: %s\n", file, res);

	} ورنہ اگر (strcmp(command, "write-string") == 0) {
		حرف *str;

		اگر (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		اگر (ul_راستہ_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		اگر (ul_راستہ_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} ورنہ اگر (strcmp(command, "write-u64") == 0) {
		uint64_t عدد;

		اگر (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		عدد = strtoumax(argv[optind++], تہی, 0);

		اگر (ul_راستہ_write_u64(pc, عدد, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		اگر (ul_راستہ_writef_u64(pc, عدد, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_راستہ(pc);
	واپس EXIT_SUCCESS;
}
#اگر_ختم /* TEST_PROGRAM_PATH */
