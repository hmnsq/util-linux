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
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_خالي_جايون_MASKNAMES;

#وصف_ڪريو ULPATH_DEBUG_INIT	(1 << 1)
#وصف_ڪريو ULPATH_DEBUG_CXT	(1 << 2)

#وصف_ڪريو DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#وصف_ڪريو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#وصف_ڪريو ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

فراغ ul_رستو_init_debug(فراغ)
{
	إذا (ulpath_debug_نقاب)
		أرجع;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

بنية رستو_cxt *ul_new_رستو(ثابت حرف *dir, ...)
{
	بنية رستو_cxt *pc = صفر_يادگيري_ڏيو(1, حجم(*pc));

	إذا (!pc)
		أرجع عدم;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	إذا (dir) {
		صحيح rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_رستو, dir, ap);
		va_end(ap);

		إذا (rc < 0 || !pc->dir_رستو)
			اذهب fail;
	}
	أرجع pc;
fail:
	ul_unref_رستو(pc);
	أرجع عدم;
}

فراغ ul_ref_رستو(بنية رستو_cxt *pc)
{
	إذا (pc)
		pc->refcount++;
}

فراغ ul_unref_رستو(بنية رستو_cxt *pc)
{
	إذا (!pc)
		أرجع;

	pc->refcount--;

	إذا (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		إذا (pc->dialect)
			pc->free_dialect(pc);
		ul_رستو_close_dirfd(pc);
		حرر(pc->dir_رستو);
		حرر(pc->اڳياڙي);
		حرر(pc);
	}
}

فراغ ul_رستو_refer_vfs(بنية رستو_cxt *pc, ثابت بنية ul_vfs_ops *vfs)
{
	إذا (pc)
		pc->vfs = vfs;
}

صحيح ul_رستو_سيٽ_اڳياڙي(بنية رستو_cxt *pc, ثابت حرف *اڳياڙي)
{
	حرف *p = عدم;

	أكد(pc->dir_fd < 0);

	إذا (اڳياڙي) {
		p = strdup(اڳياڙي);
		إذا (!p)
			أرجع -ENOMEM;
	}

	حرر(pc->اڳياڙي);
	pc->اڳياڙي = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	أرجع 0;
}

ثابت حرف *ul_رستو_get_اڳياڙي(بنية رستو_cxt *pc)
{
	أرجع pc ? pc->اڳياڙي : عدم;
}

صحيح ul_رستو_سيٽ_dir(بنية رستو_cxt *pc, ثابت حرف *dir)
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

	حرر(pc->dir_رستو);
	pc->dir_رستو = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	أرجع 0;
}

ثابت حرف *ul_رستو_get_dir(بنية رستو_cxt *pc)
{
	أرجع pc ? pc->dir_رستو : عدم;
}

صحيح ul_رستو_سيٽ_dialect(بنية رستو_cxt *pc, فراغ *معلومات, فراغ free_معلومات(بنية رستو_cxt *))
{
	pc->dialect = معلومات;
	pc->free_dialect = free_معلومات;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	أرجع 0;
}

فراغ *ul_رستو_get_dialect(بنية رستو_cxt *pc)
{
	أرجع pc ? pc->dialect : عدم;
}

صحيح ul_رستو_سيٽ_enoent_redirect(بنية رستو_cxt *pc, صحيح (*func)(بنية رستو_cxt *, ثابت حرف *, صحيح *))
{
	pc->redirect_on_enoent = func;
	أرجع 0;
}

ساكن ثابت حرف *get_absdir(بنية رستو_cxt *pc)
{
	صحيح rc;
	ثابت حرف *dirpath;

	إذا (!pc->اڳياڙي)
		أرجع pc->dir_رستو;

	dirpath = pc->dir_رستو;
	إذا (!dirpath)
		أرجع pc->اڳياڙي;
	إذا (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->رستو_بفر, حجم(pc->رستو_بفر), "%s/%s", pc->اڳياڙي, dirpath);
	إذا (rc < 0)
		أرجع عدم;
	إذا ((size_t)rc >= حجم(pc->رستو_بفر)) {
		errno = ENAMETOOLONG;
		أرجع عدم;
	}

	أرجع pc->رستو_بفر;
}

صحيح ul_رستو_is_accessible(بنية رستو_cxt *pc)
{
	ثابت حرف *رستو;
	أكد(pc);

	إذا (pc->dir_fd >= 0)
		أرجع 1;

	رستو = get_absdir(pc);
	إذا (!رستو)
		أرجع 0;
	أرجع access(رستو, F_OK) == 0;
}

صحيح ul_رستو_get_dirfd(بنية رستو_cxt *pc)
{
	أكد(pc);
	أكد(pc->dir_رستو);

	إذا (pc->dir_fd < 0) {
		ثابت حرف *رستو = get_absdir(pc);
		إذا (!رستو)
			أرجع -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", رستو));
		pc->dir_fd = open(رستو, O_RDONLY|O_CLOEXEC);
	}

	أرجع pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
فراغ ul_رستو_close_dirfd(بنية رستو_cxt *pc)
{
	أكد(pc);

	إذا (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

صحيح ul_رستو_isopen_dirfd(بنية رستو_cxt *pc)
{
	أرجع pc && pc->dir_fd >= 0;
}

ساكن ثابت حرف *ul_رستو_mkpath(بنية رستو_cxt *pc, ثابت حرف *رستو, va_list ap)
{
	صحيح rc;

	errno = 0;

	rc = vsnprintf(pc->رستو_بفر, حجم(pc->رستو_بفر), رستو, ap);
	إذا (rc < 0) {
		إذا (!errno)
			errno = EINVAL;
		أرجع عدم;
	}

	إذا ((size_t)rc >= حجم(pc->رستو_بفر)) {
		errno = ENAMETOOLONG;
		أرجع عدم;
	}

	أرجع pc->رستو_بفر;
}

حرف *ul_رستو_get_abspath(بنية رستو_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *رستو, ...)
{
	إذا (رستو) {
		صحيح rc;
		va_list ap;
		ثابت حرف *پڇ = عدم, *dirpath = pc->dir_رستو;

		va_start(ap, رستو);
		پڇ = ul_رستو_mkpath(pc, رستو, ap);
		va_end(ap);

		إذا (dirpath && *dirpath == '/')
			dirpath++;
		إذا (پڇ && *پڇ == '/')
			پڇ++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->اڳياڙي ? pc->اڳياڙي : "",
				dirpath ? dirpath : "",
				پڇ ? پڇ : "");

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


صحيح ul_رستو_access(بنية رستو_cxt *pc, صحيح گهڻائي, ثابت حرف *رستو)
{
	صحيح rc;

	إذا (!رستو)
		أرجع -EINVAL;
	إذا (!pc) {
		rc = access(رستو, گهڻائي);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", رستو, rc));
	} وإلا {
		صحيح dir = ul_رستو_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;
		إذا (*رستو == '/')
			رستو++;

		rc = faccessat(dir, رستو, گهڻائي, 0);

		إذا (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, رستو, &dir) == 0)
			rc = faccessat(dir, رستو, گهڻائي, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", رستو, rc));
	}
	أرجع rc;
}

صحيح ul_رستو_accessf(بنية رستو_cxt *pc, صحيح گهڻائي, ثابت حرف *رستو, ...)
{
	va_list ap;
	ثابت حرف *p;

	va_start(ap, رستو);
	p = ul_رستو_mkpath(pc, رستو, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستو_access(pc, گهڻائي, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
صحيح ul_رستو_stat(بنية رستو_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *رستو)
{
	صحيح rc;

	إذا (!pc) {
		rc = رستو ? stat(رستو, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", رستو, rc));
	} وإلا {
		صحيح dir = ul_رستو_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;
		إذا (رستو) {
			إذا  (*رستو == '/')
				رستو++;
			rc = fstatat(dir, رستو, sb, flags);

		} وإلا
			rc = fstat(dir, sb);	/* dir itself */

		إذا (rc && errno == ENOENT
		    && رستو
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, رستو, &dir) == 0)
			rc = fstatat(dir, رستو, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", رستو, rc));
	}
	أرجع rc;
}

صحيح ul_رستو_vstatf(بنية رستو_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *رستو, va_list ap)
{
	ثابت حرف *p = ul_رستو_mkpath(pc, رستو, ap);

	أرجع !p ? -errno : ul_رستو_stat(pc, sb, flags, p);
}

صحيح ul_رستو_statf(بنية رستو_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *رستو, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, رستو);
	rc = ul_رستو_vstatf(pc, sb, flags, رستو, ap);
	va_end(ap);

	أرجع rc;
}
#جيڪڏهن_وصف_ٿيل HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
صحيح ul_رستو_statx(بنية رستو_cxt *pc, بنية statx *stx, صحيح flags, بنانشان صحيح نقاب,
		  ثابت حرف *رستو)
{
	صحيح rc;

	إذا (!رستو)
		أرجع -EINVAL;
	إذا (!pc)
		rc = رستو ? statx(AT_FDCWD, رستو, flags, نقاب, stx) : - EINVAL;
	وإلا {
		صحيح dir = ul_رستو_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;
		إذا (*رستو == '/')
			رستو++;

		rc = statx(dir, رستو, flags, نقاب, stx);
	}

	أرجع rc;
}
#وإلا
صحيح ul_رستو_statx(بنية رستو_cxt *pc __attribute__((__unused__)),
		  بنية statx *stx __attribute__((__unused__)),
		  صحيح flags __attribute__((__unused__)),
		  بنانشان صحيح نقاب __attribute__((__unused__)),
		  ثابت حرف *رستو __attribute__((__unused__)))
{
	errno = ENOSYS;
	أرجع -1;
}
#جيڪڏهن_پورو /* HAVE_STATX */

صحيح ul_رستو_vstatxf(بنية رستو_cxt *pc, بنية statx *stx, صحيح flags, بنانشان صحيح نقاب,
		    ثابت حرف *رستو, va_list ap)
{
	ثابت حرف *p = ul_رستو_mkpath(pc, رستو, ap);

	أرجع !p ? -errno : ul_رستو_statx(pc, stx, flags, نقاب, p);
}

صحيح ul_رستو_statxf(بنية رستو_cxt *pc, بنية statx *stx, صحيح flags, بنانشان صحيح نقاب,
		   ثابت حرف *رستو, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, رستو);
	rc = ul_رستو_vstatxf(pc, stx, flags, نقاب, رستو, ap);
	va_end(ap);

	أرجع rc;
}

صحيح ul_رستو_open(بنية رستو_cxt *pc, صحيح flags, ثابت حرف *رستو)
{
	صحيح fd;

	إذا (!رستو)
		أرجع -EINVAL;
	إذا (!pc) {
		fd = open(رستو, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", رستو));
	} وإلا {
		صحيح fdx;
		صحيح dir = ul_رستو_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;

		إذا (*رستو == '/')
			رستو++;

		fdx = fd = openat(dir, رستو, flags);

		إذا (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, رستو, &dir) == 0)
			fd = openat(dir, رستو, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", رستو, fdx != fd ? " [redirected]" : ""));
	}
	أرجع fd;
}

صحيح ul_رستو_vopenf(بنية رستو_cxt *pc, صحيح flags, ثابت حرف *رستو, va_list ap)
{
	ثابت حرف *p = ul_رستو_mkpath(pc, رستو, ap);

	أرجع !p ? -errno : ul_رستو_open(pc, flags, p);
}

صحيح ul_رستو_openf(بنية رستو_cxt *pc, صحيح flags, ثابت حرف *رستو, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, رستو);
	rc = ul_رستو_vopenf(pc, flags, رستو, ap);
	va_end(ap);

	أرجع rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_رستو_fopen(بنية رستو_cxt *pc, ثابت حرف *گهڻائي, ثابت حرف *رستو)
{
	صحيح flags = ul_گهڻائي_منزل_flags(گهڻائي);
	صحيح fd = ul_رستو_open(pc, flags, رستو);

	إذا (fd < 0)
		أرجع عدم;

	أرجع ul_vfs_fdopen(pc ? pc->vfs : عدم, fd, گهڻائي);
}


FILE *ul_رستو_vfopenf(بنية رستو_cxt *pc, ثابت حرف *گهڻائي, ثابت حرف *رستو, va_list ap)
{
	ثابت حرف *p = ul_رستو_mkpath(pc, رستو, ap);

	أرجع !p ? عدم : ul_رستو_fopen(pc, گهڻائي, p);
}

FILE *ul_رستو_fopenf(بنية رستو_cxt *pc, ثابت حرف *گهڻائي, ثابت حرف *رستو, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, رستو);
	f = ul_رستو_vfopenf(pc, گهڻائي, رستو, ap);
	va_end(ap);

	أرجع f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_رستو_opendir(بنية رستو_cxt *pc, ثابت حرف *رستو)
{
	DIR *dir;
	صحيح fd = -1;

	إذا (رستو)
		fd = ul_رستو_open(pc, O_RDONLY|O_CLOEXEC, رستو);
	وإلا إذا (pc->dir_رستو) {
		صحيح dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_رستو_get_dirfd(pc);
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
	إذا (!رستو)
		 rewinddir(dir);
	أرجع dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_رستو_vopendirf(بنية رستو_cxt *pc, ثابت حرف *رستو, va_list ap)
{
	ثابت حرف *p = ul_رستو_mkpath(pc, رستو, ap);

	أرجع !p ? عدم : ul_رستو_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_رستو_opendirf(بنية رستو_cxt *pc, ثابت حرف *رستو, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, رستو);
	dir = ul_رستو_vopendirf(pc, رستو, ap);
	va_end(ap);

	أرجع dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_رستو_readlink(بنية رستو_cxt *pc, حرف *buf, size_t bufsiz, ثابت حرف *رستو)
{
	صحيح dirfd;
	ssize_t ssz;

	إذا (!رستو) {
		ثابت حرف *p = get_absdir(pc);
		إذا (!p)
			أرجع -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} وإلا {
		dirfd = ul_رستو_get_dirfd(pc);
		إذا (dirfd < 0)
			أرجع dirfd;

		إذا (*رستو == '/')
			رستو++;

		ssz = readlinkat(dirfd, رستو, buf, bufsiz - 1);
	}

	إذا (ssz >= 0)
		buf[ssz] = '\0';
	أرجع ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_رستو_readlinkf(بنية رستو_cxt *pc, حرف *buf, size_t bufsiz, ثابت حرف *رستو, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستو);
	p = ul_رستو_mkpath(pc, رستو, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستو_readlink(pc, buf, bufsiz, p);
}

صحيح ul_رستو_read(بنية رستو_cxt *pc, حرف *buf, size_t len, ثابت حرف *رستو)
{
	صحيح rc, errsv;
	صحيح fd;

	fd = ul_رستو_open(pc, O_RDONLY|O_CLOEXEC, رستو);
	إذا (fd < 0)
		أرجع -errno;

	DBG(CXT, ul_debug(" reading '%s'", رستو));
	rc = ul_vfs_read_all(pc ? pc->vfs : عدم, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	أرجع rc;
}

صحيح ul_رستو_vreadf(بنية رستو_cxt *pc, حرف *buf, size_t len, ثابت حرف *رستو, va_list ap)
{
	ثابت حرف *p = ul_رستو_mkpath(pc, رستو, ap);

	أرجع !p ? -errno : ul_رستو_read(pc, buf, len, p);
}

صحيح ul_رستو_readf(بنية رستو_cxt *pc, حرف *buf, size_t len, ثابت حرف *رستو, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, رستو);
	rc = ul_رستو_vreadf(pc, buf, len, رستو, ap);
	va_end(ap);

	أرجع rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
صحيح ul_رستو_read_string(بنية رستو_cxt *pc, حرف **str, ثابت حرف *رستو)
{
	حرف buf[BUFSIZ];
	صحيح rc;

	إذا (!str)
		أرجع -EINVAL;

	*str = عدم;

	rc = ul_رستو_read_بفر(pc, buf, حجم(buf), رستو);
	إذا (rc < 0)
		أرجع rc;

	*str = strdup(buf);
	إذا (!*str)
		rc = -ENOMEM;

	أرجع rc;
}

صحيح ul_رستو_readf_string(بنية رستو_cxt *pc, حرف **str, ثابت حرف *رستو, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستو);
	p = ul_رستو_mkpath(pc, رستو, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستو_read_string(pc, str, p);
}

صحيح ul_رستو_read_بفر(بنية رستو_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *رستو)
{
	صحيح rc = ul_رستو_read(pc, buf, bufsz - 1, رستو);

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

صحيح ul_رستو_vreadf_بفر(بنية رستو_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *رستو, va_list ap)
{
	ثابت حرف *p;

	p = ul_رستو_mkpath(pc, رستو, ap);

	أرجع !p ? -errno : ul_رستو_read_بفر(pc, buf, bufsz, p);
}

صحيح ul_رستو_readf_بفر(بنية رستو_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *رستو, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, رستو);
	rc = ul_رستو_vreadf_بفر(pc, buf, bufsz, رستو, ap);
	va_end(ap);

	أرجع rc;
}

صحيح ul_رستو_scanf(بنية رستو_cxt *pc, ثابت حرف *رستو, ثابت حرف *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	صحيح rc;

	f = ul_رستو_fopen(pc, "r" UL_CLOEXECSTR, رستو);
	إذا (!f)
		أرجع -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, رستو));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	أرجع rc;
}

صحيح ul_رستو_scanff(بنية رستو_cxt *pc, ثابت حرف *رستو, va_list ap, ثابت حرف *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	صحيح rc;

	f = ul_رستو_vfopenf(pc, "r" UL_CLOEXECSTR, رستو, ap);
	إذا (!f)
		أرجع -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	أرجع rc;
}


صحيح ul_رستو_read_s64(بنية رستو_cxt *pc, int64_t *res, ثابت حرف *رستو)
{
	int64_t x = 0;
	صحيح rc;

	rc = ul_رستو_scanf(pc, رستو, "%"SCNd64, &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_رستو_readf_s64(بنية رستو_cxt *pc, int64_t *res, ثابت حرف *رستو, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستو);
	p = ul_رستو_mkpath(pc, رستو, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستو_read_s64(pc, res, p);
}

صحيح ul_رستو_read_u64(بنية رستو_cxt *pc, uint64_t *res, ثابت حرف *رستو)
{
	uint64_t x = 0;
	صحيح rc;

	rc = ul_رستو_scanf(pc, رستو, "%"SCNu64, &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_رستو_readf_u64(بنية رستو_cxt *pc, uint64_t *res, ثابت حرف *رستو, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستو);
	p = ul_رستو_mkpath(pc, رستو, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستو_read_u64(pc, res, p);
}

صحيح ul_رستو_read_s32(بنية رستو_cxt *pc, صحيح *res, ثابت حرف *رستو)
{
	صحيح rc, x = 0;

	rc = ul_رستو_scanf(pc, رستو, "%d", &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_رستو_readf_s32(بنية رستو_cxt *pc, صحيح *res, ثابت حرف *رستو, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستو);
	p = ul_رستو_mkpath(pc, رستو, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستو_read_s32(pc, res, p);
}

صحيح ul_رستو_read_u32(بنية رستو_cxt *pc, بنانشان صحيح *res, ثابت حرف *رستو)
{
	صحيح rc;
	بنانشان صحيح x = 0;

	rc = ul_رستو_scanf(pc, رستو, "%u", &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_رستو_readf_u32(بنية رستو_cxt *pc, بنانشان صحيح *res, ثابت حرف *رستو, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستو);
	p = ul_رستو_mkpath(pc, رستو, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستو_read_u32(pc, res, p);
}

صحيح ul_رستو_read_majmin(بنية رستو_cxt *pc, dev_t *res, ثابت حرف *رستو)
{
	صحيح rc, maj = 0, min = 0;

	rc = ul_رستو_scanf(pc, رستو, "%d:%d", &maj, &min);
	إذا (rc != 2)
		أرجع -1;
	إذا (res)
		*res = makedev(maj, min);
	أرجع 0;
}

صحيح ul_رستو_readf_majmin(بنية رستو_cxt *pc, dev_t *res, ثابت حرف *رستو, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستو);
	p = ul_رستو_mkpath(pc, رستو, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستو_read_majmin(pc, res, p);
}

صحيح ul_رستو_write_string(بنية رستو_cxt *pc, ثابت حرف *str, ثابت حرف *رستو)
{
	صحيح rc, errsv;
	صحيح fd;

	fd = ul_رستو_open(pc, O_WRONLY|O_CLOEXEC, رستو);
	إذا (fd < 0)
		أرجع -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : عدم, fd, str, لڙي_ڊيگهه(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	أرجع rc;
}

صحيح ul_رستو_writef_string(بنية رستو_cxt *pc, ثابت حرف *str, ثابت حرف *رستو, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستو);
	p = ul_رستو_mkpath(pc, رستو, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستو_write_string(pc, str, p);
}

صحيح ul_رستو_write_s64(بنية رستو_cxt *pc, int64_t انگ, ثابت حرف *رستو)
{
	حرف buf[حجم(stringify_قدر(LLONG_MAX))];
	صحيح rc, errsv;
	صحيح fd, len;

	fd = ul_رستو_open(pc, O_WRONLY|O_CLOEXEC, رستو);
	إذا (fd < 0)
		أرجع -errno;

	len = snprintf(buf, حجم(buf), "%" PRId64, انگ);
	إذا (len < 0 || (size_t) len >= حجم(buf))
		rc = len < 0 ? -errno : -E2BIG;
	وإلا
		rc = ul_vfs_write_all(pc ? pc->vfs : عدم, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	أرجع rc;
}

صحيح ul_رستو_write_u64(بنية رستو_cxt *pc, uint64_t انگ, ثابت حرف *رستو)
{
	حرف buf[حجم(stringify_قدر(ULLONG_MAX))];
	صحيح rc, errsv;
	صحيح fd, len;

	fd = ul_رستو_open(pc, O_WRONLY|O_CLOEXEC, رستو);
	إذا (fd < 0)
		أرجع -errno;

	len = snprintf(buf, حجم(buf), "%" PRIu64, انگ);
	إذا (len < 0 || (size_t) len >= حجم(buf))
		rc = len < 0 ? -errno : -E2BIG;
	وإلا
		rc = ul_vfs_write_all(pc ? pc->vfs : عدم, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	أرجع rc;
}

صحيح ul_رستو_writef_u64(بنية رستو_cxt *pc, uint64_t انگ, ثابت حرف *رستو, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستو);
	p = ul_رستو_mkpath(pc, رستو, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستو_write_u64(pc, انگ, p);

}

صحيح ul_رستو_ڳڻپ_dirents(بنية رستو_cxt *pc, ثابت حرف *رستو)
{
	DIR *dir;
	صحيح r = 0;

	dir = ul_رستو_opendir(pc, رستو);
	إذا (!dir)
		أرجع 0;

	طالما (xreaddir(dir)) r++;

	closedir(dir);
	أرجع r;
}

صحيح ul_رستو_countf_dirents(بنية رستو_cxt *pc, ثابت حرف *رستو, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, رستو);
	p = ul_رستو_mkpath(pc, رستو, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_رستو_ڳڻپ_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
صحيح ul_رستو_ايندڙ_dirent(بنية رستو_cxt *pc, DIR **sub, ثابت حرف *dirname, بنية dirent **d)
{
	إذا (!pc || !sub || !d)
		أرجع -EINVAL;

	إذا (!*sub) {
		*sub = ul_رستو_opendir(pc, dirname);
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

#جيڪڏهن_وصف_ٿيل HAVE_CPU_سيٽ_T
ساكن صحيح ul_رستو_cpuparse(بنية رستو_cxt *pc, cpu_set_t **سيٽ, صحيح maxcpus, صحيح islist, ثابت حرف *رستو, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	حرف *buf;
	صحيح rc;

	*سيٽ = عدم;

	buf = يادگيري_ڏيو(len);
	إذا (!buf)
		أرجع -ENOMEM;

	rc = ul_رستو_vreadf_بفر(pc, buf, len, رستو, ap);
	إذا (rc < 0)
		اذهب ٻاهريون;

	*سيٽ = cpuset_alloc(maxcpus, &setsize, عدم);
	إذا (!*سيٽ) {
		rc = -EINVAL;
		اذهب ٻاهريون;
	}

	إذا (islist) {
		إذا (cpulist_parse(buf, *سيٽ, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			اذهب ٻاهريون;
		}
	} وإلا {
		إذا (cpumask_parse(buf, *سيٽ, setsize)) {
			errno = EINVAL;
			rc = -errno;
			اذهب ٻاهريون;
		}
	}
	rc = 0;

ٻاهريون:
	إذا (rc) {
		cpuset_free(*سيٽ);
		*سيٽ = عدم;
	}
	حرر(buf);
	أرجع rc;
}

صحيح ul_رستو_readf_cpuset(بنية رستو_cxt *pc, cpu_set_t **سيٽ, صحيح maxcpus, ثابت حرف *رستو, ...)
{
	va_list ap;
	صحيح rc = 0;

	va_start(ap, رستو);
	rc = ul_رستو_cpuparse(pc, سيٽ, maxcpus, 0, رستو, ap);
	va_end(ap);

	أرجع rc;
}

صحيح ul_رستو_readf_cpulist(بنية رستو_cxt *pc, cpu_set_t **سيٽ, صحيح maxcpus, ثابت حرف *رستو, ...)
{
	va_list ap;
	صحيح rc = 0;

	va_start(ap, رستو);
	rc = ul_رستو_cpuparse(pc, سيٽ, maxcpus, 1, رستو, ap);
	va_end(ap);

	أرجع rc;
}

#جيڪڏهن_پورو /* HAVE_CPU_SET_T */


#جيڪڏهن_وصف_ٿيل TEST_پروگرام_رستو
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

	ٻاهر_نڪرو(EXIT_SUCCESS);
}

صحيح مکيه(صحيح argc, حرف *argv[])
{
	صحيح c;
	ثابت حرف *اڳياڙي = عدم, *dir, *file, *command;
	بنية رستو_cxt *pc = عدم;

	ساكن ثابت بنية option longopts[] = {
		{ "prefix",	1, عدم, 'p' },
		{ "help",       0, عدم, 'h' },
		{ عدم, 0, عدم, 0 },
	};

	طالما((c = getopt_long(argc, argv, "p:h", longopts, عدم)) != -1) {
		بدل(c) {
		حالة 'p':
			اڳياڙي = optarg;
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

	ul_رستو_init_debug();

	pc = ul_new_رستو("%s", dir);
	إذا (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	إذا (اڳياڙي)
		ul_رستو_سيٽ_اڳياڙي(pc, اڳياڙي);

	إذا (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	إذا (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_رستو_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		لکو("read:  %s: %u\n", file, res);

		إذا (ul_رستو_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		لکو("readf: %s: %u\n", file, res);

	} وإلا إذا (strcmp(command, "read-s32") == 0) {
		int32_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_رستو_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		لکو("read:  %s: %d\n", file, res);

		إذا (ul_رستو_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		لکو("readf: %s: %d\n", file, res);

	} وإلا إذا (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_رستو_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		لکو("read:  %s: %" PRIu64 "\n", file, res);

		إذا (ul_رستو_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		لکو("readf: %s: %" PRIu64 "\n", file, res);

	} وإلا إذا (strcmp(command, "read-s64") == 0) {
		int64_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_رستو_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		لکو("read:  %s: %" PRId64 "\n", file, res);

		إذا (ul_رستو_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		لکو("readf: %s: %" PRId64 "\n", file, res);

	} وإلا إذا (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_رستو_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		لکو("read:  %s: %d\n", file, (صحيح) res);

		إذا (ul_رستو_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		لکو("readf: %s: %d\n", file, (صحيح) res);

	} وإلا إذا (strcmp(command, "read-string") == 0) {
		حرف *res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_رستو_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		لکو("read:  %s: %s\n", file, res);

		إذا (ul_رستو_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		لکو("readf: %s: %s\n", file, res);

	} وإلا إذا (strcmp(command, "read-link") == 0) {
		حرف res[PATH_MAX];

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_رستو_readlink(pc, res, حجم(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		لکو("read:  %s: %s\n", file, res);

		إذا (ul_رستو_readlinkf(pc, res, حجم(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		لکو("readf: %s: %s\n", file, res);

	} وإلا إذا (strcmp(command, "write-string") == 0) {
		حرف *str;

		إذا (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		إذا (ul_رستو_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		إذا (ul_رستو_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} وإلا إذا (strcmp(command, "write-u64") == 0) {
		uint64_t انگ;

		إذا (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		انگ = strtoumax(argv[optind++], عدم, 0);

		إذا (ul_رستو_write_u64(pc, انگ, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		إذا (ul_رستو_writef_u64(pc, انگ, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_رستو(pc);
	أرجع EXIT_SUCCESS;
}
#جيڪڏهن_پورو /* TEST_PROGRAM_PATH */
