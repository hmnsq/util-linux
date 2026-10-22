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
ساكن UL_DEBUG_DEFINE_ماسک(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_خالی_MASKNAMES;

#تعریف_کن ULPATH_DEBUG_INIT	(1 << 1)
#تعریف_کن ULPATH_DEBUG_CXT	(1 << 2)

#تعریف_کن DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#تعریف_کن DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#تعریف_کن ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

فراغ ul_مسیر_init_debug(فراغ)
{
	إذا (ulpath_debug_ماسک)
		أرجع;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

بنية مسیر_cxt *ul_new_مسیر(ثابت حرف *dir, ...)
{
	بنية مسیر_cxt *pc = حافظه_صفر_بده(1, حجم(*pc));

	إذا (!pc)
		أرجع عدم;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	إذا (dir) {
		صحيح rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_مسیر, dir, ap);
		va_end(ap);

		إذا (rc < 0 || !pc->dir_مسیر)
			اذهب fail;
	}
	أرجع pc;
fail:
	ul_unref_مسیر(pc);
	أرجع عدم;
}

فراغ ul_ref_مسیر(بنية مسیر_cxt *pc)
{
	إذا (pc)
		pc->refcount++;
}

فراغ ul_unref_مسیر(بنية مسیر_cxt *pc)
{
	إذا (!pc)
		أرجع;

	pc->refcount--;

	إذا (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		إذا (pc->dialect)
			pc->free_dialect(pc);
		ul_مسیر_close_dirfd(pc);
		حرر(pc->dir_مسیر);
		حرر(pc->پیشوند);
		حرر(pc);
	}
}

فراغ ul_مسیر_refer_vfs(بنية مسیر_cxt *pc, ثابت بنية ul_vfs_ops *vfs)
{
	إذا (pc)
		pc->vfs = vfs;
}

صحيح ul_مسیر_مجموعه_پیشوند(بنية مسیر_cxt *pc, ثابت حرف *پیشوند)
{
	حرف *p = عدم;

	أكد(pc->dir_fd < 0);

	إذا (پیشوند) {
		p = strdup(پیشوند);
		إذا (!p)
			أرجع -ENOMEM;
	}

	حرر(pc->پیشوند);
	pc->پیشوند = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	أرجع 0;
}

ثابت حرف *ul_مسیر_get_پیشوند(بنية مسیر_cxt *pc)
{
	أرجع pc ? pc->پیشوند : عدم;
}

صحيح ul_مسیر_مجموعه_dir(بنية مسیر_cxt *pc, ثابت حرف *dir)
{
	حرف *p = عدم;

	إذا (dir) {
		p = strdup(dir);
		إذا (!p)
			أرجع -ENOMEM;
	}

	إذا (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	حرر(pc->dir_مسیر);
	pc->dir_مسیر = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	أرجع 0;
}

ثابت حرف *ul_مسیر_get_dir(بنية مسیر_cxt *pc)
{
	أرجع pc ? pc->dir_مسیر : عدم;
}

صحيح ul_مسیر_مجموعه_dialect(بنية مسیر_cxt *pc, فراغ *داده, فراغ free_داده(بنية مسیر_cxt *))
{
	pc->dialect = داده;
	pc->free_dialect = free_داده;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	أرجع 0;
}

فراغ *ul_مسیر_get_dialect(بنية مسیر_cxt *pc)
{
	أرجع pc ? pc->dialect : عدم;
}

صحيح ul_مسیر_مجموعه_enoent_redirect(بنية مسیر_cxt *pc, صحيح (*func)(بنية مسیر_cxt *, ثابت حرف *, صحيح *))
{
	pc->redirect_on_enoent = func;
	أرجع 0;
}

ساكن ثابت حرف *get_absdir(بنية مسیر_cxt *pc)
{
	صحيح rc;
	ثابت حرف *dirpath;

	إذا (!pc->پیشوند)
		أرجع pc->dir_مسیر;

	dirpath = pc->dir_مسیر;
	إذا (!dirpath)
		أرجع pc->پیشوند;
	إذا (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->مسیر_میانگیر, حجم(pc->مسیر_میانگیر), "%s/%s", pc->پیشوند, dirpath);
	إذا (rc < 0)
		أرجع عدم;
	إذا ((size_t)rc >= حجم(pc->مسیر_میانگیر)) {
		errno = ENAMETOOLONG;
		أرجع عدم;
	}

	أرجع pc->مسیر_میانگیر;
}

صحيح ul_مسیر_is_accessible(بنية مسیر_cxt *pc)
{
	ثابت حرف *مسیر;
	أكد(pc);

	إذا (pc->dir_fd >= 0)
		أرجع 1;

	مسیر = get_absdir(pc);
	إذا (!مسیر)
		أرجع 0;
	أرجع access(مسیر, F_OK) == 0;
}

صحيح ul_مسیر_get_dirfd(بنية مسیر_cxt *pc)
{
	أكد(pc);
	أكد(pc->dir_مسیر);

	إذا (pc->dir_fd < 0) {
		ثابت حرف *مسیر = get_absdir(pc);
		إذا (!مسیر)
			أرجع -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", مسیر));
		pc->dir_fd = open(مسیر, O_RDONLY|O_CLOEXEC);
	}

	أرجع pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
فراغ ul_مسیر_close_dirfd(بنية مسیر_cxt *pc)
{
	أكد(pc);

	إذا (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

صحيح ul_مسیر_isopen_dirfd(بنية مسیر_cxt *pc)
{
	أرجع pc && pc->dir_fd >= 0;
}

ساكن ثابت حرف *ul_مسیر_mkpath(بنية مسیر_cxt *pc, ثابت حرف *مسیر, va_list ap)
{
	صحيح rc;

	errno = 0;

	rc = vsnprintf(pc->مسیر_میانگیر, حجم(pc->مسیر_میانگیر), مسیر, ap);
	إذا (rc < 0) {
		إذا (!errno)
			errno = EINVAL;
		أرجع عدم;
	}

	إذا ((size_t)rc >= حجم(pc->مسیر_میانگیر)) {
		errno = ENAMETOOLONG;
		أرجع عدم;
	}

	أرجع pc->مسیر_میانگیر;
}

حرف *ul_مسیر_get_abspath(بنية مسیر_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *مسیر, ...)
{
	إذا (مسیر) {
		صحيح rc;
		va_list ap;
		ثابت حرف *دم = عدم, *dirpath = pc->dir_مسیر;

		va_start(ap, مسیر);
		دم = ul_مسیر_mkpath(pc, مسیر, ap);
		va_end(ap);

		إذا (dirpath && *dirpath == '/')
			dirpath++;
		إذا (دم && *دم == '/')
			دم++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->پیشوند ? pc->پیشوند : "",
				dirpath ? dirpath : "",
				دم ? دم : "");

		إذا ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			أرجع عدم;
		}
	} وإلا {
		ثابت حرف *tmp = get_absdir(pc);

		إذا (!tmp)
			أرجع عدم;
		xstrncpy(buf, tmp, bufsz);
	}

	أرجع buf;
}


صحيح ul_مسیر_access(بنية مسیر_cxt *pc, صحيح نما, ثابت حرف *مسیر)
{
	صحيح rc;

	إذا (!مسیر)
		أرجع -EINVAL;
	إذا (!pc) {
		rc = access(مسیر, نما);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", مسیر, rc));
	} وإلا {
		صحيح dir = ul_مسیر_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;
		إذا (*مسیر == '/')
			مسیر++;

		rc = faccessat(dir, مسیر, نما, 0);

		إذا (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, مسیر, &dir) == 0)
			rc = faccessat(dir, مسیر, نما, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", مسیر, rc));
	}
	أرجع rc;
}

صحيح ul_مسیر_accessf(بنية مسیر_cxt *pc, صحيح نما, ثابت حرف *مسیر, ...)
{
	va_list ap;
	ثابت حرف *p;

	va_start(ap, مسیر);
	p = ul_مسیر_mkpath(pc, مسیر, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_مسیر_access(pc, نما, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
صحيح ul_مسیر_stat(بنية مسیر_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *مسیر)
{
	صحيح rc;

	إذا (!pc) {
		rc = مسیر ? stat(مسیر, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", مسیر, rc));
	} وإلا {
		صحيح dir = ul_مسیر_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;
		إذا (مسیر) {
			إذا  (*مسیر == '/')
				مسیر++;
			rc = fstatat(dir, مسیر, sb, flags);

		} وإلا
			rc = fstat(dir, sb);	/* dir itself */

		إذا (rc && errno == ENOENT
		    && مسیر
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, مسیر, &dir) == 0)
			rc = fstatat(dir, مسیر, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", مسیر, rc));
	}
	أرجع rc;
}

صحيح ul_مسیر_vstatf(بنية مسیر_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *مسیر, va_list ap)
{
	ثابت حرف *p = ul_مسیر_mkpath(pc, مسیر, ap);

	أرجع !p ? -errno : ul_مسیر_stat(pc, sb, flags, p);
}

صحيح ul_مسیر_statf(بنية مسیر_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *مسیر, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, مسیر);
	rc = ul_مسیر_vstatf(pc, sb, flags, مسیر, ap);
	va_end(ap);

	أرجع rc;
}
#اگر_تعریف_شده HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
صحيح ul_مسیر_statx(بنية مسیر_cxt *pc, بنية statx *stx, صحيح flags, بیعلامت صحيح ماسک,
		  ثابت حرف *مسیر)
{
	صحيح rc;

	إذا (!مسیر)
		أرجع -EINVAL;
	إذا (!pc)
		rc = مسیر ? statx(AT_FDCWD, مسیر, flags, ماسک, stx) : - EINVAL;
	وإلا {
		صحيح dir = ul_مسیر_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;
		إذا (*مسیر == '/')
			مسیر++;

		rc = statx(dir, مسیر, flags, ماسک, stx);
	}

	أرجع rc;
}
#وإلا
صحيح ul_مسیر_statx(بنية مسیر_cxt *pc __attribute__((__unused__)),
		  بنية statx *stx __attribute__((__unused__)),
		  صحيح flags __attribute__((__unused__)),
		  بیعلامت صحيح ماسک __attribute__((__unused__)),
		  ثابت حرف *مسیر __attribute__((__unused__)))
{
	errno = ENOSYS;
	أرجع -1;
}
#پایان_اگر /* HAVE_STATX */

صحيح ul_مسیر_vstatxf(بنية مسیر_cxt *pc, بنية statx *stx, صحيح flags, بیعلامت صحيح ماسک,
		    ثابت حرف *مسیر, va_list ap)
{
	ثابت حرف *p = ul_مسیر_mkpath(pc, مسیر, ap);

	أرجع !p ? -errno : ul_مسیر_statx(pc, stx, flags, ماسک, p);
}

صحيح ul_مسیر_statxf(بنية مسیر_cxt *pc, بنية statx *stx, صحيح flags, بیعلامت صحيح ماسک,
		   ثابت حرف *مسیر, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, مسیر);
	rc = ul_مسیر_vstatxf(pc, stx, flags, ماسک, مسیر, ap);
	va_end(ap);

	أرجع rc;
}

صحيح ul_مسیر_open(بنية مسیر_cxt *pc, صحيح flags, ثابت حرف *مسیر)
{
	صحيح fd;

	إذا (!مسیر)
		أرجع -EINVAL;
	إذا (!pc) {
		fd = open(مسیر, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", مسیر));
	} وإلا {
		صحيح fdx;
		صحيح dir = ul_مسیر_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;

		إذا (*مسیر == '/')
			مسیر++;

		fdx = fd = openat(dir, مسیر, flags);

		إذا (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, مسیر, &dir) == 0)
			fd = openat(dir, مسیر, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", مسیر, fdx != fd ? " [redirected]" : ""));
	}
	أرجع fd;
}

صحيح ul_مسیر_vopenf(بنية مسیر_cxt *pc, صحيح flags, ثابت حرف *مسیر, va_list ap)
{
	ثابت حرف *p = ul_مسیر_mkpath(pc, مسیر, ap);

	أرجع !p ? -errno : ul_مسیر_open(pc, flags, p);
}

صحيح ul_مسیر_openf(بنية مسیر_cxt *pc, صحيح flags, ثابت حرف *مسیر, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, مسیر);
	rc = ul_مسیر_vopenf(pc, flags, مسیر, ap);
	va_end(ap);

	أرجع rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_مسیر_fopen(بنية مسیر_cxt *pc, ثابت حرف *نما, ثابت حرف *مسیر)
{
	صحيح flags = ul_نما_مقصد_flags(نما);
	صحيح fd = ul_مسیر_open(pc, flags, مسیر);

	إذا (fd < 0)
		أرجع عدم;

	أرجع ul_vfs_fdopen(pc ? pc->vfs : عدم, fd, نما);
}


FILE *ul_مسیر_vfopenf(بنية مسیر_cxt *pc, ثابت حرف *نما, ثابت حرف *مسیر, va_list ap)
{
	ثابت حرف *p = ul_مسیر_mkpath(pc, مسیر, ap);

	أرجع !p ? عدم : ul_مسیر_fopen(pc, نما, p);
}

FILE *ul_مسیر_fopenf(بنية مسیر_cxt *pc, ثابت حرف *نما, ثابت حرف *مسیر, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, مسیر);
	f = ul_مسیر_vfopenf(pc, نما, مسیر, ap);
	va_end(ap);

	أرجع f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_مسیر_opendir(بنية مسیر_cxt *pc, ثابت حرف *مسیر)
{
	DIR *dir;
	صحيح fd = -1;

	إذا (مسیر)
		fd = ul_مسیر_open(pc, O_RDONLY|O_CLOEXEC, مسیر);
	وإلا إذا (pc->dir_مسیر) {
		صحيح dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_مسیر_get_dirfd(pc);
		إذا (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	إذا (fd < 0)
		أرجع عدم;

	dir = fdopendir(fd);
	إذا (!dir) {
		close(fd);
		أرجع عدم;
	}
	إذا (!مسیر)
		 rewinddir(dir);
	أرجع dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_مسیر_vopendirf(بنية مسیر_cxt *pc, ثابت حرف *مسیر, va_list ap)
{
	ثابت حرف *p = ul_مسیر_mkpath(pc, مسیر, ap);

	أرجع !p ? عدم : ul_مسیر_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_مسیر_opendirf(بنية مسیر_cxt *pc, ثابت حرف *مسیر, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, مسیر);
	dir = ul_مسیر_vopendirf(pc, مسیر, ap);
	va_end(ap);

	أرجع dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_مسیر_readlink(بنية مسیر_cxt *pc, حرف *buf, size_t bufsiz, ثابت حرف *مسیر)
{
	صحيح dirfd;
	ssize_t ssz;

	إذا (!مسیر) {
		ثابت حرف *p = get_absdir(pc);
		إذا (!p)
			أرجع -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} وإلا {
		dirfd = ul_مسیر_get_dirfd(pc);
		إذا (dirfd < 0)
			أرجع dirfd;

		إذا (*مسیر == '/')
			مسیر++;

		ssz = readlinkat(dirfd, مسیر, buf, bufsiz - 1);
	}

	إذا (ssz >= 0)
		buf[ssz] = '\0';
	أرجع ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_مسیر_readlinkf(بنية مسیر_cxt *pc, حرف *buf, size_t bufsiz, ثابت حرف *مسیر, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, مسیر);
	p = ul_مسیر_mkpath(pc, مسیر, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_مسیر_readlink(pc, buf, bufsiz, p);
}

صحيح ul_مسیر_read(بنية مسیر_cxt *pc, حرف *buf, size_t len, ثابت حرف *مسیر)
{
	صحيح rc, errsv;
	صحيح fd;

	fd = ul_مسیر_open(pc, O_RDONLY|O_CLOEXEC, مسیر);
	إذا (fd < 0)
		أرجع -errno;

	DBG(CXT, ul_debug(" reading '%s'", مسیر));
	rc = ul_vfs_read_all(pc ? pc->vfs : عدم, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	أرجع rc;
}

صحيح ul_مسیر_vreadf(بنية مسیر_cxt *pc, حرف *buf, size_t len, ثابت حرف *مسیر, va_list ap)
{
	ثابت حرف *p = ul_مسیر_mkpath(pc, مسیر, ap);

	أرجع !p ? -errno : ul_مسیر_read(pc, buf, len, p);
}

صحيح ul_مسیر_readf(بنية مسیر_cxt *pc, حرف *buf, size_t len, ثابت حرف *مسیر, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, مسیر);
	rc = ul_مسیر_vreadf(pc, buf, len, مسیر, ap);
	va_end(ap);

	أرجع rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
صحيح ul_مسیر_read_string(بنية مسیر_cxt *pc, حرف **str, ثابت حرف *مسیر)
{
	حرف buf[BUFSIZ];
	صحيح rc;

	إذا (!str)
		أرجع -EINVAL;

	*str = عدم;

	rc = ul_مسیر_read_میانگیر(pc, buf, حجم(buf), مسیر);
	إذا (rc < 0)
		أرجع rc;

	*str = strdup(buf);
	إذا (!*str)
		rc = -ENOMEM;

	أرجع rc;
}

صحيح ul_مسیر_readf_string(بنية مسیر_cxt *pc, حرف **str, ثابت حرف *مسیر, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, مسیر);
	p = ul_مسیر_mkpath(pc, مسیر, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_مسیر_read_string(pc, str, p);
}

صحيح ul_مسیر_read_میانگیر(بنية مسیر_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *مسیر)
{
	صحيح rc = ul_مسیر_read(pc, buf, bufsz - 1, مسیر);

	إذا (rc == 0)
		buf[0] = '\0';

	وإلا إذا (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		إذا (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		وإلا
			buf[rc] = '\0';
	}

	أرجع rc;
}

صحيح ul_مسیر_vreadf_میانگیر(بنية مسیر_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *مسیر, va_list ap)
{
	ثابت حرف *p;

	p = ul_مسیر_mkpath(pc, مسیر, ap);

	أرجع !p ? -errno : ul_مسیر_read_میانگیر(pc, buf, bufsz, p);
}

صحيح ul_مسیر_readf_میانگیر(بنية مسیر_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *مسیر, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, مسیر);
	rc = ul_مسیر_vreadf_میانگیر(pc, buf, bufsz, مسیر, ap);
	va_end(ap);

	أرجع rc;
}

صحيح ul_مسیر_scanf(بنية مسیر_cxt *pc, ثابت حرف *مسیر, ثابت حرف *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	صحيح rc;

	f = ul_مسیر_fopen(pc, "r" UL_CLOEXECSTR, مسیر);
	إذا (!f)
		أرجع -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, مسیر));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	أرجع rc;
}

صحيح ul_مسیر_scanff(بنية مسیر_cxt *pc, ثابت حرف *مسیر, va_list ap, ثابت حرف *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	صحيح rc;

	f = ul_مسیر_vfopenf(pc, "r" UL_CLOEXECSTR, مسیر, ap);
	إذا (!f)
		أرجع -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	أرجع rc;
}


صحيح ul_مسیر_read_s64(بنية مسیر_cxt *pc, int64_t *res, ثابت حرف *مسیر)
{
	int64_t x = 0;
	صحيح rc;

	rc = ul_مسیر_scanf(pc, مسیر, "%"SCNd64, &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_مسیر_readf_s64(بنية مسیر_cxt *pc, int64_t *res, ثابت حرف *مسیر, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, مسیر);
	p = ul_مسیر_mkpath(pc, مسیر, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_مسیر_read_s64(pc, res, p);
}

صحيح ul_مسیر_read_u64(بنية مسیر_cxt *pc, uint64_t *res, ثابت حرف *مسیر)
{
	uint64_t x = 0;
	صحيح rc;

	rc = ul_مسیر_scanf(pc, مسیر, "%"SCNu64, &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_مسیر_readf_u64(بنية مسیر_cxt *pc, uint64_t *res, ثابت حرف *مسیر, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, مسیر);
	p = ul_مسیر_mkpath(pc, مسیر, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_مسیر_read_u64(pc, res, p);
}

صحيح ul_مسیر_read_s32(بنية مسیر_cxt *pc, صحيح *res, ثابت حرف *مسیر)
{
	صحيح rc, x = 0;

	rc = ul_مسیر_scanf(pc, مسیر, "%d", &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_مسیر_readf_s32(بنية مسیر_cxt *pc, صحيح *res, ثابت حرف *مسیر, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, مسیر);
	p = ul_مسیر_mkpath(pc, مسیر, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_مسیر_read_s32(pc, res, p);
}

صحيح ul_مسیر_read_u32(بنية مسیر_cxt *pc, بیعلامت صحيح *res, ثابت حرف *مسیر)
{
	صحيح rc;
	بیعلامت صحيح x = 0;

	rc = ul_مسیر_scanf(pc, مسیر, "%u", &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_مسیر_readf_u32(بنية مسیر_cxt *pc, بیعلامت صحيح *res, ثابت حرف *مسیر, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, مسیر);
	p = ul_مسیر_mkpath(pc, مسیر, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_مسیر_read_u32(pc, res, p);
}

صحيح ul_مسیر_read_majmin(بنية مسیر_cxt *pc, dev_t *res, ثابت حرف *مسیر)
{
	صحيح rc, maj = 0, min = 0;

	rc = ul_مسیر_scanf(pc, مسیر, "%d:%d", &maj, &min);
	إذا (rc != 2)
		أرجع -1;
	إذا (res)
		*res = makedev(maj, min);
	أرجع 0;
}

صحيح ul_مسیر_readf_majmin(بنية مسیر_cxt *pc, dev_t *res, ثابت حرف *مسیر, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, مسیر);
	p = ul_مسیر_mkpath(pc, مسیر, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_مسیر_read_majmin(pc, res, p);
}

صحيح ul_مسیر_write_string(بنية مسیر_cxt *pc, ثابت حرف *str, ثابت حرف *مسیر)
{
	صحيح rc, errsv;
	صحيح fd;

	fd = ul_مسیر_open(pc, O_WRONLY|O_CLOEXEC, مسیر);
	إذا (fd < 0)
		أرجع -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : عدم, fd, str, طول_رشته(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	أرجع rc;
}

صحيح ul_مسیر_writef_string(بنية مسیر_cxt *pc, ثابت حرف *str, ثابت حرف *مسیر, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, مسیر);
	p = ul_مسیر_mkpath(pc, مسیر, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_مسیر_write_string(pc, str, p);
}

صحيح ul_مسیر_write_s64(بنية مسیر_cxt *pc, int64_t رقم, ثابت حرف *مسیر)
{
	حرف buf[حجم(stringify_مقدار(LLONG_MAX))];
	صحيح rc, errsv;
	صحيح fd, len;

	fd = ul_مسیر_open(pc, O_WRONLY|O_CLOEXEC, مسیر);
	إذا (fd < 0)
		أرجع -errno;

	len = snprintf(buf, حجم(buf), "%" PRId64, رقم);
	إذا (len < 0 || (size_t) len >= حجم(buf))
		rc = len < 0 ? -errno : -E2BIG;
	وإلا
		rc = ul_vfs_write_all(pc ? pc->vfs : عدم, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	أرجع rc;
}

صحيح ul_مسیر_write_u64(بنية مسیر_cxt *pc, uint64_t رقم, ثابت حرف *مسیر)
{
	حرف buf[حجم(stringify_مقدار(ULLONG_MAX))];
	صحيح rc, errsv;
	صحيح fd, len;

	fd = ul_مسیر_open(pc, O_WRONLY|O_CLOEXEC, مسیر);
	إذا (fd < 0)
		أرجع -errno;

	len = snprintf(buf, حجم(buf), "%" PRIu64, رقم);
	إذا (len < 0 || (size_t) len >= حجم(buf))
		rc = len < 0 ? -errno : -E2BIG;
	وإلا
		rc = ul_vfs_write_all(pc ? pc->vfs : عدم, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	أرجع rc;
}

صحيح ul_مسیر_writef_u64(بنية مسیر_cxt *pc, uint64_t رقم, ثابت حرف *مسیر, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, مسیر);
	p = ul_مسیر_mkpath(pc, مسیر, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_مسیر_write_u64(pc, رقم, p);

}

صحيح ul_مسیر_شمار_dirents(بنية مسیر_cxt *pc, ثابت حرف *مسیر)
{
	DIR *dir;
	صحيح r = 0;

	dir = ul_مسیر_opendir(pc, مسیر);
	إذا (!dir)
		أرجع 0;

	طالما (xreaddir(dir)) r++;

	closedir(dir);
	أرجع r;
}

صحيح ul_مسیر_countf_dirents(بنية مسیر_cxt *pc, ثابت حرف *مسیر, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, مسیر);
	p = ul_مسیر_mkpath(pc, مسیر, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_مسیر_شمار_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
صحيح ul_مسیر_بعدی_dirent(بنية مسیر_cxt *pc, DIR **sub, ثابت حرف *dirname, بنية dirent **d)
{
	إذا (!pc || !sub || !d)
		أرجع -EINVAL;

	إذا (!*sub) {
		*sub = ul_مسیر_opendir(pc, dirname);
		إذا (!*sub)
			أرجع -errno;
	}

	*d = xreaddir(*sub);
	إذا (*d)
		أرجع 0;

	closedir(*sub);
	*sub = عدم;
	أرجع 1;
}

#اگر_تعریف_شده HAVE_CPU_مجموعه_T
ساكن صحيح ul_مسیر_cpuparse(بنية مسیر_cxt *pc, cpu_set_t **مجموعه, صحيح maxcpus, صحيح islist, ثابت حرف *مسیر, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	حرف *buf;
	صحيح rc;

	*مجموعه = عدم;

	buf = حافظه_بده(len);
	إذا (!buf)
		أرجع -ENOMEM;

	rc = ul_مسیر_vreadf_میانگیر(pc, buf, len, مسیر, ap);
	إذا (rc < 0)
		اذهب بیرون;

	*مجموعه = cpuset_alloc(maxcpus, &setsize, عدم);
	إذا (!*مجموعه) {
		rc = -EINVAL;
		اذهب بیرون;
	}

	إذا (islist) {
		إذا (cpulist_parse(buf, *مجموعه, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			اذهب بیرون;
		}
	} وإلا {
		إذا (cpumask_parse(buf, *مجموعه, setsize)) {
			errno = EINVAL;
			rc = -errno;
			اذهب بیرون;
		}
	}
	rc = 0;

بیرون:
	إذا (rc) {
		cpuset_free(*مجموعه);
		*مجموعه = عدم;
	}
	حرر(buf);
	أرجع rc;
}

صحيح ul_مسیر_readf_cpuset(بنية مسیر_cxt *pc, cpu_set_t **مجموعه, صحيح maxcpus, ثابت حرف *مسیر, ...)
{
	va_list ap;
	صحيح rc = 0;

	va_start(ap, مسیر);
	rc = ul_مسیر_cpuparse(pc, مجموعه, maxcpus, 0, مسیر, ap);
	va_end(ap);

	أرجع rc;
}

صحيح ul_مسیر_readf_cpulist(بنية مسیر_cxt *pc, cpu_set_t **مجموعه, صحيح maxcpus, ثابت حرف *مسیر, ...)
{
	va_list ap;
	صحيح rc = 0;

	va_start(ap, مسیر);
	rc = ul_مسیر_cpuparse(pc, مجموعه, maxcpus, 1, مسیر, ap);
	va_end(ap);

	أرجع rc;
}

#پایان_اگر /* HAVE_CPU_SET_T */


#اگر_تعریف_شده TEST_برنامه_مسیر
#ضمن <getopt.h>

ساكن فراغ __attribute__((__noreturn__)) usage(فراغ)
{
	fprintf(stdout, " %s [options] <dir> <command>\n\n", برنامه_invocation_short_name);
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

	خروج(EXIT_SUCCESS);
}

صحيح اصلی(صحيح argc, حرف *argv[])
{
	صحيح c;
	ثابت حرف *پیشوند = عدم, *dir, *file, *command;
	بنية مسیر_cxt *pc = عدم;

	ساكن ثابت بنية option longopts[] = {
		{ "prefix",	1, عدم, 'p' },
		{ "help",       0, عدم, 'h' },
		{ عدم, 0, عدم, 0 },
	};

	طالما((c = getopt_long(argc, argv, "p:h", longopts, عدم)) != -1) {
		بدل(c) {
		حالة 'p':
			پیشوند = optarg;
			اكسر;
		حالة 'h':
			usage();
			اكسر;
		افتراضي:
			err(EXIT_FAILURE, "try --help");
		}
	}

	إذا (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_مسیر_init_debug();

	pc = ul_new_مسیر("%s", dir);
	إذا (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	إذا (پیشوند)
		ul_مسیر_مجموعه_پیشوند(pc, پیشوند);

	إذا (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	إذا (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_مسیر_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		چاپ_کن("read:  %s: %u\n", file, res);

		إذا (ul_مسیر_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		چاپ_کن("readf: %s: %u\n", file, res);

	} وإلا إذا (strcmp(command, "read-s32") == 0) {
		int32_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_مسیر_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		چاپ_کن("read:  %s: %d\n", file, res);

		إذا (ul_مسیر_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		چاپ_کن("readf: %s: %d\n", file, res);

	} وإلا إذا (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_مسیر_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		چاپ_کن("read:  %s: %" PRIu64 "\n", file, res);

		إذا (ul_مسیر_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		چاپ_کن("readf: %s: %" PRIu64 "\n", file, res);

	} وإلا إذا (strcmp(command, "read-s64") == 0) {
		int64_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_مسیر_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		چاپ_کن("read:  %s: %" PRId64 "\n", file, res);

		إذا (ul_مسیر_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		چاپ_کن("readf: %s: %" PRId64 "\n", file, res);

	} وإلا إذا (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_مسیر_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		چاپ_کن("read:  %s: %d\n", file, (صحيح) res);

		إذا (ul_مسیر_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		چاپ_کن("readf: %s: %d\n", file, (صحيح) res);

	} وإلا إذا (strcmp(command, "read-string") == 0) {
		حرف *res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_مسیر_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		چاپ_کن("read:  %s: %s\n", file, res);

		إذا (ul_مسیر_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		چاپ_کن("readf: %s: %s\n", file, res);

	} وإلا إذا (strcmp(command, "read-link") == 0) {
		حرف res[PATH_MAX];

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_مسیر_readlink(pc, res, حجم(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		چاپ_کن("read:  %s: %s\n", file, res);

		إذا (ul_مسیر_readlinkf(pc, res, حجم(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		چاپ_کن("readf: %s: %s\n", file, res);

	} وإلا إذا (strcmp(command, "write-string") == 0) {
		حرف *str;

		إذا (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		إذا (ul_مسیر_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		إذا (ul_مسیر_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} وإلا إذا (strcmp(command, "write-u64") == 0) {
		uint64_t رقم;

		إذا (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		رقم = strtoumax(argv[optind++], عدم, 0);

		إذا (ul_مسیر_write_u64(pc, رقم, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		إذا (ul_مسیر_writef_u64(pc, رقم, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_مسیر(pc);
	أرجع EXIT_SUCCESS;
}
#پایان_اگر /* TEST_PROGRAM_PATH */
