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
ساكن UL_DEBUG_DEFINE_نقاب(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_خالی_تھاواں_MASKNAMES;

#تعریف_کرو ULPATH_DEBUG_INIT	(1 << 1)
#تعریف_کرو ULPATH_DEBUG_CXT	(1 << 2)

#تعریف_کرو DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#تعریف_کرو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#تعریف_کرو ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

فراغ ul_رستہ_init_debug(فراغ)
{
	إذا (ulpath_debug_نقاب)
		أرجع;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

بنية رستہ_cxt *ul_new_رستہ(ثابت حرف *dir, ...)
{
	بنية رستہ_cxt *pc = صفر_حافظہ_دیو(1, حجم(*pc));

	إذا (!pc)
		أرجع عدم;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	إذا (dir) {
		صحيح rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_رستہ, dir, ap);
		va_end(ap);

		إذا (rc < 0 || !pc->dir_رستہ)
			اذهب fail;
	}
	أرجع pc;
fail:
	ul_unref_رستہ(pc);
	أرجع عدم;
}

فراغ ul_ref_رستہ(بنية رستہ_cxt *pc)
{
	إذا (pc)
		pc->refcount++;
}

فراغ ul_unref_رستہ(بنية رستہ_cxt *pc)
{
	إذا (!pc)
		أرجع;

	pc->refcount--;

	إذا (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		إذا (pc->dialect)
			pc->free_dialect(pc);
		ul_رستہ_close_dirfd(pc);
		حرر(pc->dir_رستہ);
		حرر(pc->اگیتر);
		حرر(pc);
	}
}

فراغ ul_رستہ_refer_vfs(بنية رستہ_cxt *pc, ثابت بنية ul_vfs_ops *vfs)
{
	إذا (pc)
		pc->vfs = vfs;
}

صحيح ul_رستہ_سیٹ_اگیتر(بنية رستہ_cxt *pc, ثابت حرف *اگیتر)
{
	حرف *p = عدم;

	أكد(pc->dir_fd < 0);

	إذا (اگیتر) {
		p = strdup(اگیتر);
		إذا (!p)
			أرجع -ENOMEM;
	}

	حرر(pc->اگیتر);
	pc->اگیتر = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	أرجع 0;
}

ثابت حرف *ul_رستہ_get_اگیتر(بنية رستہ_cxt *pc)
{
	أرجع pc ? pc->اگیتر : عدم;
}

صحيح ul_رستہ_سیٹ_dir(بنية رستہ_cxt *pc, ثابت حرف *dir)
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

	حرر(pc->dir_رستہ);
	pc->dir_رستہ = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	أرجع 0;
}

ثابت حرف *ul_رستہ_get_dir(بنية رستہ_cxt *pc)
{
	أرجع pc ? pc->dir_رستہ : عدم;
}

صحيح ul_رستہ_سیٹ_dialect(بنية رستہ_cxt *pc, فراغ *انکڑے, فراغ free_انکڑے(بنية رستہ_cxt *))
{
	pc->dialect = انکڑے;
	pc->free_dialect = free_انکڑے;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	أرجع 0;
}

فراغ *ul_رستہ_get_dialect(بنية رستہ_cxt *pc)
{
	أرجع pc ? pc->dialect : عدم;
}

صحيح ul_رستہ_سیٹ_enoent_redirect(بنية رستہ_cxt *pc, صحيح (*func)(بنية رستہ_cxt *, ثابت حرف *, صحيح *))
{
	pc->redirect_on_enoent = func;
	أرجع 0;
}

ساكن ثابت حرف *get_absdir(بنية رستہ_cxt *pc)
{
	صحيح rc;
	ثابت حرف *dirpath;

	إذا (!pc->اگیتر)
		أرجع pc->dir_رستہ;

	dirpath = pc->dir_رستہ;
	إذا (!dirpath)
		أرجع pc->اگیتر;
	إذا (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->رستہ_بفر, حجم(pc->رستہ_بفر), "%s/%s", pc->اگیتر, dirpath);
	إذا (rc < 0)
		أرجع عدم;
	إذا ((size_t)rc >= حجم(pc->رستہ_بفر)) {
		errno = ENAMETOOLONG;
		أرجع عدم;
	}

	أرجع pc->رستہ_بفر;
}

صحيح ul_رستہ_is_accessible(بنية رستہ_cxt *pc)
{
	ثابت حرف *رستہ;
	أكد(pc);

	إذا (pc->dir_fd >= 0)
		أرجع 1;

	رستہ = get_absdir(pc);
	إذا (!رستہ)
		أرجع 0;
	أرجع access(رستہ, F_OK) == 0;
}

صحيح ul_رستہ_get_dirfd(بنية رستہ_cxt *pc)
{
	أكد(pc);
	أكد(pc->dir_رستہ);

	إذا (pc->dir_fd < 0) {
		ثابت حرف *رستہ = get_absdir(pc);
		إذا (!رستہ)
			أرجع -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", رستہ));
		pc->dir_fd = open(رستہ, O_RDONLY|O_CLOEXEC);
	}

	أرجع pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
فراغ ul_رستہ_close_dirfd(بنية رستہ_cxt *pc)
{
	أكد(pc);

	إذا (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

صحيح ul_رستہ_isopen_dirfd(بنية رستہ_cxt *pc)
{
	أرجع pc && pc->dir_fd >= 0;
}

ساكن ثابت حرف *ul_رستہ_mkpath(بنية رستہ_cxt *pc, ثابت حرف *رستہ, va_list ap)
{
	صحيح rc;

	errno = 0;

	rc = vsnprintf(pc->رستہ_بفر, حجم(pc->رستہ_بفر), رستہ, ap);
	إذا (rc < 0) {
		إذا (!errno)
			errno = EINVAL;
		أرجع عدم;
	}

	إذا ((size_t)rc >= حجم(pc->رستہ_بفر)) {
		errno = ENAMETOOLONG;
		أرجع عدم;
	}

	أرجع pc->رستہ_بفر;
}

حرف *ul_رستہ_get_abspath(بنية رستہ_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *رستہ, ...)
{
	إذا (رستہ) {
		صحيح rc;
		va_list ap;
		ثابت حرف *پوچھ = عدم, *dirpath = pc->dir_رستہ;

		va_start(ap, رستہ);
		پوچھ = ul_رستہ_mkpath(pc, رستہ, ap);
		va_end(ap);

		إذا (dirpath && *dirpath == '/')
			dirpath++;
		إذا (پوچھ && *پوچھ == '/')
			پوچھ++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->اگیتر ? pc->اگیتر : "",
				dirpath ? dirpath : "",
				پوچھ ? پوچھ : "");

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


صحيح ul_رستہ_access(بنية رستہ_cxt *pc, صحيح منوال, ثابت حرف *رستہ)
{
	صحيح rc;

	إذا (!رستہ)
		أرجع -EINVAL;
	إذا (!pc) {
		rc = access(رستہ, منوال);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", رستہ, rc));
	} وإلا {
		صحيح dir = ul_رستہ_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;
		إذا (*رستہ == '/')
			رستہ++;

		rc = faccessat(dir, رستہ, منوال, 0);

		إذا (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, رستہ, &dir) == 0)
			rc = faccessat(dir, رستہ, منوال, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", رستہ, rc));
	}
	أرجع rc;
}

صحيح ul_رستہ_accessf(بنية رستہ_cxt *pc, صحيح منوال, ثابت حرف *رستہ, ...)
{
	va_list ap;
	ثابت حرف *p;

	va_start(ap, رستہ);
	p = ul_رستہ_mkpath(pc, رستہ, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستہ_access(pc, منوال, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
صحيح ul_رستہ_stat(بنية رستہ_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *رستہ)
{
	صحيح rc;

	إذا (!pc) {
		rc = رستہ ? stat(رستہ, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", رستہ, rc));
	} وإلا {
		صحيح dir = ul_رستہ_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;
		إذا (رستہ) {
			إذا  (*رستہ == '/')
				رستہ++;
			rc = fstatat(dir, رستہ, sb, flags);

		} وإلا
			rc = fstat(dir, sb);	/* dir itself */

		إذا (rc && errno == ENOENT
		    && رستہ
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, رستہ, &dir) == 0)
			rc = fstatat(dir, رستہ, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", رستہ, rc));
	}
	أرجع rc;
}

صحيح ul_رستہ_vstatf(بنية رستہ_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *رستہ, va_list ap)
{
	ثابت حرف *p = ul_رستہ_mkpath(pc, رستہ, ap);

	أرجع !p ? -errno : ul_رستہ_stat(pc, sb, flags, p);
}

صحيح ul_رستہ_statf(بنية رستہ_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *رستہ, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, رستہ);
	rc = ul_رستہ_vstatf(pc, sb, flags, رستہ, ap);
	va_end(ap);

	أرجع rc;
}
#جے_تعریف_شدہ HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
صحيح ul_رستہ_statx(بنية رستہ_cxt *pc, بنية statx *stx, صحيح flags, بنانشانی صحيح نقاب,
		  ثابت حرف *رستہ)
{
	صحيح rc;

	إذا (!رستہ)
		أرجع -EINVAL;
	إذا (!pc)
		rc = رستہ ? statx(AT_FDCWD, رستہ, flags, نقاب, stx) : - EINVAL;
	وإلا {
		صحيح dir = ul_رستہ_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;
		إذا (*رستہ == '/')
			رستہ++;

		rc = statx(dir, رستہ, flags, نقاب, stx);
	}

	أرجع rc;
}
#وإلا
صحيح ul_رستہ_statx(بنية رستہ_cxt *pc __attribute__((__unused__)),
		  بنية statx *stx __attribute__((__unused__)),
		  صحيح flags __attribute__((__unused__)),
		  بنانشانی صحيح نقاب __attribute__((__unused__)),
		  ثابت حرف *رستہ __attribute__((__unused__)))
{
	errno = ENOSYS;
	أرجع -1;
}
#جے_انت /* HAVE_STATX */

صحيح ul_رستہ_vstatxf(بنية رستہ_cxt *pc, بنية statx *stx, صحيح flags, بنانشانی صحيح نقاب,
		    ثابت حرف *رستہ, va_list ap)
{
	ثابت حرف *p = ul_رستہ_mkpath(pc, رستہ, ap);

	أرجع !p ? -errno : ul_رستہ_statx(pc, stx, flags, نقاب, p);
}

صحيح ul_رستہ_statxf(بنية رستہ_cxt *pc, بنية statx *stx, صحيح flags, بنانشانی صحيح نقاب,
		   ثابت حرف *رستہ, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, رستہ);
	rc = ul_رستہ_vstatxf(pc, stx, flags, نقاب, رستہ, ap);
	va_end(ap);

	أرجع rc;
}

صحيح ul_رستہ_open(بنية رستہ_cxt *pc, صحيح flags, ثابت حرف *رستہ)
{
	صحيح fd;

	إذا (!رستہ)
		أرجع -EINVAL;
	إذا (!pc) {
		fd = open(رستہ, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", رستہ));
	} وإلا {
		صحيح fdx;
		صحيح dir = ul_رستہ_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;

		إذا (*رستہ == '/')
			رستہ++;

		fdx = fd = openat(dir, رستہ, flags);

		إذا (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, رستہ, &dir) == 0)
			fd = openat(dir, رستہ, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", رستہ, fdx != fd ? " [redirected]" : ""));
	}
	أرجع fd;
}

صحيح ul_رستہ_vopenf(بنية رستہ_cxt *pc, صحيح flags, ثابت حرف *رستہ, va_list ap)
{
	ثابت حرف *p = ul_رستہ_mkpath(pc, رستہ, ap);

	أرجع !p ? -errno : ul_رستہ_open(pc, flags, p);
}

صحيح ul_رستہ_openf(بنية رستہ_cxt *pc, صحيح flags, ثابت حرف *رستہ, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, رستہ);
	rc = ul_رستہ_vopenf(pc, flags, رستہ, ap);
	va_end(ap);

	أرجع rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_رستہ_fopen(بنية رستہ_cxt *pc, ثابت حرف *منوال, ثابت حرف *رستہ)
{
	صحيح flags = ul_منوال_منزل_flags(منوال);
	صحيح fd = ul_رستہ_open(pc, flags, رستہ);

	إذا (fd < 0)
		أرجع عدم;

	أرجع ul_vfs_fdopen(pc ? pc->vfs : عدم, fd, منوال);
}


FILE *ul_رستہ_vfopenf(بنية رستہ_cxt *pc, ثابت حرف *منوال, ثابت حرف *رستہ, va_list ap)
{
	ثابت حرف *p = ul_رستہ_mkpath(pc, رستہ, ap);

	أرجع !p ? عدم : ul_رستہ_fopen(pc, منوال, p);
}

FILE *ul_رستہ_fopenf(بنية رستہ_cxt *pc, ثابت حرف *منوال, ثابت حرف *رستہ, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, رستہ);
	f = ul_رستہ_vfopenf(pc, منوال, رستہ, ap);
	va_end(ap);

	أرجع f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_رستہ_opendir(بنية رستہ_cxt *pc, ثابت حرف *رستہ)
{
	DIR *dir;
	صحيح fd = -1;

	إذا (رستہ)
		fd = ul_رستہ_open(pc, O_RDONLY|O_CLOEXEC, رستہ);
	وإلا إذا (pc->dir_رستہ) {
		صحيح dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_رستہ_get_dirfd(pc);
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
	إذا (!رستہ)
		 rewinddir(dir);
	أرجع dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_رستہ_vopendirf(بنية رستہ_cxt *pc, ثابت حرف *رستہ, va_list ap)
{
	ثابت حرف *p = ul_رستہ_mkpath(pc, رستہ, ap);

	أرجع !p ? عدم : ul_رستہ_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_رستہ_opendirf(بنية رستہ_cxt *pc, ثابت حرف *رستہ, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, رستہ);
	dir = ul_رستہ_vopendirf(pc, رستہ, ap);
	va_end(ap);

	أرجع dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_رستہ_readlink(بنية رستہ_cxt *pc, حرف *buf, size_t bufsiz, ثابت حرف *رستہ)
{
	صحيح dirfd;
	ssize_t ssz;

	إذا (!رستہ) {
		ثابت حرف *p = get_absdir(pc);
		إذا (!p)
			أرجع -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} وإلا {
		dirfd = ul_رستہ_get_dirfd(pc);
		إذا (dirfd < 0)
			أرجع dirfd;

		إذا (*رستہ == '/')
			رستہ++;

		ssz = readlinkat(dirfd, رستہ, buf, bufsiz - 1);
	}

	إذا (ssz >= 0)
		buf[ssz] = '\0';
	أرجع ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_رستہ_readlinkf(بنية رستہ_cxt *pc, حرف *buf, size_t bufsiz, ثابت حرف *رستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستہ);
	p = ul_رستہ_mkpath(pc, رستہ, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستہ_readlink(pc, buf, bufsiz, p);
}

صحيح ul_رستہ_read(بنية رستہ_cxt *pc, حرف *buf, size_t len, ثابت حرف *رستہ)
{
	صحيح rc, errsv;
	صحيح fd;

	fd = ul_رستہ_open(pc, O_RDONLY|O_CLOEXEC, رستہ);
	إذا (fd < 0)
		أرجع -errno;

	DBG(CXT, ul_debug(" reading '%s'", رستہ));
	rc = ul_vfs_read_all(pc ? pc->vfs : عدم, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	أرجع rc;
}

صحيح ul_رستہ_vreadf(بنية رستہ_cxt *pc, حرف *buf, size_t len, ثابت حرف *رستہ, va_list ap)
{
	ثابت حرف *p = ul_رستہ_mkpath(pc, رستہ, ap);

	أرجع !p ? -errno : ul_رستہ_read(pc, buf, len, p);
}

صحيح ul_رستہ_readf(بنية رستہ_cxt *pc, حرف *buf, size_t len, ثابت حرف *رستہ, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, رستہ);
	rc = ul_رستہ_vreadf(pc, buf, len, رستہ, ap);
	va_end(ap);

	أرجع rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
صحيح ul_رستہ_read_string(بنية رستہ_cxt *pc, حرف **str, ثابت حرف *رستہ)
{
	حرف buf[BUFSIZ];
	صحيح rc;

	إذا (!str)
		أرجع -EINVAL;

	*str = عدم;

	rc = ul_رستہ_read_بفر(pc, buf, حجم(buf), رستہ);
	إذا (rc < 0)
		أرجع rc;

	*str = strdup(buf);
	إذا (!*str)
		rc = -ENOMEM;

	أرجع rc;
}

صحيح ul_رستہ_readf_string(بنية رستہ_cxt *pc, حرف **str, ثابت حرف *رستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستہ);
	p = ul_رستہ_mkpath(pc, رستہ, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستہ_read_string(pc, str, p);
}

صحيح ul_رستہ_read_بفر(بنية رستہ_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *رستہ)
{
	صحيح rc = ul_رستہ_read(pc, buf, bufsz - 1, رستہ);

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

صحيح ul_رستہ_vreadf_بفر(بنية رستہ_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *رستہ, va_list ap)
{
	ثابت حرف *p;

	p = ul_رستہ_mkpath(pc, رستہ, ap);

	أرجع !p ? -errno : ul_رستہ_read_بفر(pc, buf, bufsz, p);
}

صحيح ul_رستہ_readf_بفر(بنية رستہ_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *رستہ, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, رستہ);
	rc = ul_رستہ_vreadf_بفر(pc, buf, bufsz, رستہ, ap);
	va_end(ap);

	أرجع rc;
}

صحيح ul_رستہ_scanf(بنية رستہ_cxt *pc, ثابت حرف *رستہ, ثابت حرف *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	صحيح rc;

	f = ul_رستہ_fopen(pc, "r" UL_CLOEXECSTR, رستہ);
	إذا (!f)
		أرجع -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, رستہ));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	أرجع rc;
}

صحيح ul_رستہ_scanff(بنية رستہ_cxt *pc, ثابت حرف *رستہ, va_list ap, ثابت حرف *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	صحيح rc;

	f = ul_رستہ_vfopenf(pc, "r" UL_CLOEXECSTR, رستہ, ap);
	إذا (!f)
		أرجع -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	أرجع rc;
}


صحيح ul_رستہ_read_s64(بنية رستہ_cxt *pc, int64_t *res, ثابت حرف *رستہ)
{
	int64_t x = 0;
	صحيح rc;

	rc = ul_رستہ_scanf(pc, رستہ, "%"SCNd64, &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_رستہ_readf_s64(بنية رستہ_cxt *pc, int64_t *res, ثابت حرف *رستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستہ);
	p = ul_رستہ_mkpath(pc, رستہ, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستہ_read_s64(pc, res, p);
}

صحيح ul_رستہ_read_u64(بنية رستہ_cxt *pc, uint64_t *res, ثابت حرف *رستہ)
{
	uint64_t x = 0;
	صحيح rc;

	rc = ul_رستہ_scanf(pc, رستہ, "%"SCNu64, &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_رستہ_readf_u64(بنية رستہ_cxt *pc, uint64_t *res, ثابت حرف *رستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستہ);
	p = ul_رستہ_mkpath(pc, رستہ, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستہ_read_u64(pc, res, p);
}

صحيح ul_رستہ_read_s32(بنية رستہ_cxt *pc, صحيح *res, ثابت حرف *رستہ)
{
	صحيح rc, x = 0;

	rc = ul_رستہ_scanf(pc, رستہ, "%d", &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_رستہ_readf_s32(بنية رستہ_cxt *pc, صحيح *res, ثابت حرف *رستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستہ);
	p = ul_رستہ_mkpath(pc, رستہ, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستہ_read_s32(pc, res, p);
}

صحيح ul_رستہ_read_u32(بنية رستہ_cxt *pc, بنانشانی صحيح *res, ثابت حرف *رستہ)
{
	صحيح rc;
	بنانشانی صحيح x = 0;

	rc = ul_رستہ_scanf(pc, رستہ, "%u", &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_رستہ_readf_u32(بنية رستہ_cxt *pc, بنانشانی صحيح *res, ثابت حرف *رستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستہ);
	p = ul_رستہ_mkpath(pc, رستہ, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستہ_read_u32(pc, res, p);
}

صحيح ul_رستہ_read_majmin(بنية رستہ_cxt *pc, dev_t *res, ثابت حرف *رستہ)
{
	صحيح rc, maj = 0, min = 0;

	rc = ul_رستہ_scanf(pc, رستہ, "%d:%d", &maj, &min);
	إذا (rc != 2)
		أرجع -1;
	إذا (res)
		*res = makedev(maj, min);
	أرجع 0;
}

صحيح ul_رستہ_readf_majmin(بنية رستہ_cxt *pc, dev_t *res, ثابت حرف *رستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستہ);
	p = ul_رستہ_mkpath(pc, رستہ, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستہ_read_majmin(pc, res, p);
}

صحيح ul_رستہ_write_string(بنية رستہ_cxt *pc, ثابت حرف *str, ثابت حرف *رستہ)
{
	صحيح rc, errsv;
	صحيح fd;

	fd = ul_رستہ_open(pc, O_WRONLY|O_CLOEXEC, رستہ);
	إذا (fd < 0)
		أرجع -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : عدم, fd, str, لڑی_لمبائی(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	أرجع rc;
}

صحيح ul_رستہ_writef_string(بنية رستہ_cxt *pc, ثابت حرف *str, ثابت حرف *رستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستہ);
	p = ul_رستہ_mkpath(pc, رستہ, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستہ_write_string(pc, str, p);
}

صحيح ul_رستہ_write_s64(بنية رستہ_cxt *pc, int64_t عدد, ثابت حرف *رستہ)
{
	حرف buf[حجم(stringify_قدر(LLONG_MAX))];
	صحيح rc, errsv;
	صحيح fd, len;

	fd = ul_رستہ_open(pc, O_WRONLY|O_CLOEXEC, رستہ);
	إذا (fd < 0)
		أرجع -errno;

	len = snprintf(buf, حجم(buf), "%" PRId64, عدد);
	إذا (len < 0 || (size_t) len >= حجم(buf))
		rc = len < 0 ? -errno : -E2BIG;
	وإلا
		rc = ul_vfs_write_all(pc ? pc->vfs : عدم, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	أرجع rc;
}

صحيح ul_رستہ_write_u64(بنية رستہ_cxt *pc, uint64_t عدد, ثابت حرف *رستہ)
{
	حرف buf[حجم(stringify_قدر(ULLONG_MAX))];
	صحيح rc, errsv;
	صحيح fd, len;

	fd = ul_رستہ_open(pc, O_WRONLY|O_CLOEXEC, رستہ);
	إذا (fd < 0)
		أرجع -errno;

	len = snprintf(buf, حجم(buf), "%" PRIu64, عدد);
	إذا (len < 0 || (size_t) len >= حجم(buf))
		rc = len < 0 ? -errno : -E2BIG;
	وإلا
		rc = ul_vfs_write_all(pc ? pc->vfs : عدم, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	أرجع rc;
}

صحيح ul_رستہ_writef_u64(بنية رستہ_cxt *pc, uint64_t عدد, ثابت حرف *رستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستہ);
	p = ul_رستہ_mkpath(pc, رستہ, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستہ_write_u64(pc, عدد, p);

}

صحيح ul_رستہ_گنتی_dirents(بنية رستہ_cxt *pc, ثابت حرف *رستہ)
{
	DIR *dir;
	صحيح r = 0;

	dir = ul_رستہ_opendir(pc, رستہ);
	إذا (!dir)
		أرجع 0;

	طالما (xreaddir(dir)) r++;

	closedir(dir);
	أرجع r;
}

صحيح ul_رستہ_countf_dirents(بنية رستہ_cxt *pc, ثابت حرف *رستہ, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستہ);
	p = ul_رستہ_mkpath(pc, رستہ, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستہ_گنتی_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
صحيح ul_رستہ_اگلا_dirent(بنية رستہ_cxt *pc, DIR **sub, ثابت حرف *dirname, بنية dirent **d)
{
	إذا (!pc || !sub || !d)
		أرجع -EINVAL;

	إذا (!*sub) {
		*sub = ul_رستہ_opendir(pc, dirname);
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

#جے_تعریف_شدہ HAVE_CPU_سیٹ_T
ساكن صحيح ul_رستہ_cpuparse(بنية رستہ_cxt *pc, cpu_set_t **سیٹ, صحيح maxcpus, صحيح islist, ثابت حرف *رستہ, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	حرف *buf;
	صحيح rc;

	*سیٹ = عدم;

	buf = حافظہ_دیو(len);
	إذا (!buf)
		أرجع -ENOMEM;

	rc = ul_رستہ_vreadf_بفر(pc, buf, len, رستہ, ap);
	إذا (rc < 0)
		اذهب باہرلا;

	*سیٹ = cpuset_alloc(maxcpus, &setsize, عدم);
	إذا (!*سیٹ) {
		rc = -EINVAL;
		اذهب باہرلا;
	}

	إذا (islist) {
		إذا (cpulist_parse(buf, *سیٹ, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			اذهب باہرلا;
		}
	} وإلا {
		إذا (cpumask_parse(buf, *سیٹ, setsize)) {
			errno = EINVAL;
			rc = -errno;
			اذهب باہرلا;
		}
	}
	rc = 0;

باہرلا:
	إذا (rc) {
		cpuset_free(*سیٹ);
		*سیٹ = عدم;
	}
	حرر(buf);
	أرجع rc;
}

صحيح ul_رستہ_readf_cpuset(بنية رستہ_cxt *pc, cpu_set_t **سیٹ, صحيح maxcpus, ثابت حرف *رستہ, ...)
{
	va_list ap;
	صحيح rc = 0;

	va_start(ap, رستہ);
	rc = ul_رستہ_cpuparse(pc, سیٹ, maxcpus, 0, رستہ, ap);
	va_end(ap);

	أرجع rc;
}

صحيح ul_رستہ_readf_cpulist(بنية رستہ_cxt *pc, cpu_set_t **سیٹ, صحيح maxcpus, ثابت حرف *رستہ, ...)
{
	va_list ap;
	صحيح rc = 0;

	va_start(ap, رستہ);
	rc = ul_رستہ_cpuparse(pc, سیٹ, maxcpus, 1, رستہ, ap);
	va_end(ap);

	أرجع rc;
}

#جے_انت /* HAVE_CPU_SET_T */


#جے_تعریف_شدہ TEST_پروگرام_رستہ
#ضمن <getopt.h>

ساكن فراغ __attribute__((__noreturn__)) usage(فراغ)
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

صحيح مکھ(صحيح argc, حرف *argv[])
{
	صحيح c;
	ثابت حرف *اگیتر = عدم, *dir, *file, *command;
	بنية رستہ_cxt *pc = عدم;

	ساكن ثابت بنية option longopts[] = {
		{ "prefix",	1, عدم, 'p' },
		{ "help",       0, عدم, 'h' },
		{ عدم, 0, عدم, 0 },
	};

	طالما((c = getopt_long(argc, argv, "p:h", longopts, عدم)) != -1) {
		بدل(c) {
		حالة 'p':
			اگیتر = optarg;
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

	ul_رستہ_init_debug();

	pc = ul_new_رستہ("%s", dir);
	إذا (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	إذا (اگیتر)
		ul_رستہ_سیٹ_اگیتر(pc, اگیتر);

	إذا (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	إذا (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_رستہ_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		لکھو("read:  %s: %u\n", file, res);

		إذا (ul_رستہ_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		لکھو("readf: %s: %u\n", file, res);

	} وإلا إذا (strcmp(command, "read-s32") == 0) {
		int32_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_رستہ_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		لکھو("read:  %s: %d\n", file, res);

		إذا (ul_رستہ_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		لکھو("readf: %s: %d\n", file, res);

	} وإلا إذا (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_رستہ_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		لکھو("read:  %s: %" PRIu64 "\n", file, res);

		إذا (ul_رستہ_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		لکھو("readf: %s: %" PRIu64 "\n", file, res);

	} وإلا إذا (strcmp(command, "read-s64") == 0) {
		int64_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_رستہ_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		لکھو("read:  %s: %" PRId64 "\n", file, res);

		إذا (ul_رستہ_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		لکھو("readf: %s: %" PRId64 "\n", file, res);

	} وإلا إذا (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_رستہ_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		لکھو("read:  %s: %d\n", file, (صحيح) res);

		إذا (ul_رستہ_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		لکھو("readf: %s: %d\n", file, (صحيح) res);

	} وإلا إذا (strcmp(command, "read-string") == 0) {
		حرف *res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_رستہ_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		لکھو("read:  %s: %s\n", file, res);

		إذا (ul_رستہ_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		لکھو("readf: %s: %s\n", file, res);

	} وإلا إذا (strcmp(command, "read-link") == 0) {
		حرف res[PATH_MAX];

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_رستہ_readlink(pc, res, حجم(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		لکھو("read:  %s: %s\n", file, res);

		إذا (ul_رستہ_readlinkf(pc, res, حجم(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		لکھو("readf: %s: %s\n", file, res);

	} وإلا إذا (strcmp(command, "write-string") == 0) {
		حرف *str;

		إذا (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		إذا (ul_رستہ_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		إذا (ul_رستہ_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} وإلا إذا (strcmp(command, "write-u64") == 0) {
		uint64_t عدد;

		إذا (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		عدد = strtoumax(argv[optind++], عدم, 0);

		إذا (ul_رستہ_write_u64(pc, عدد, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		إذا (ul_رستہ_writef_u64(pc, عدد, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_رستہ(pc);
	أرجع EXIT_SUCCESS;
}
#جے_انت /* TEST_PROGRAM_PATH */
