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
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_تشه_MASKNAMES;

#تعریف_کړه ULPATH_DEBUG_INIT	(1 << 1)
#تعریف_کړه ULPATH_DEBUG_CXT	(1 << 2)

#تعریف_کړه DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#تعریف_کړه DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#تعریف_کړه ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

فراغ ul_لار_init_debug(فراغ)
{
	إذا (ulpath_debug_ماسک)
		أرجع;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

بنية لار_cxt *ul_new_لار(ثابت حرف *dir, ...)
{
	بنية لار_cxt *pc = حافظه_صفر_ورکړه(1, حجم(*pc));

	إذا (!pc)
		أرجع عدم;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	إذا (dir) {
		صحيح rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_لار, dir, ap);
		va_end(ap);

		إذا (rc < 0 || !pc->dir_لار)
			اذهب fail;
	}
	أرجع pc;
fail:
	ul_unref_لار(pc);
	أرجع عدم;
}

فراغ ul_ref_لار(بنية لار_cxt *pc)
{
	إذا (pc)
		pc->refcount++;
}

فراغ ul_unref_لار(بنية لار_cxt *pc)
{
	إذا (!pc)
		أرجع;

	pc->refcount--;

	إذا (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		إذا (pc->dialect)
			pc->free_dialect(pc);
		ul_لار_close_dirfd(pc);
		حرر(pc->dir_لار);
		حرر(pc->مختاړی);
		حرر(pc);
	}
}

فراغ ul_لار_refer_vfs(بنية لار_cxt *pc, ثابت بنية ul_vfs_ops *vfs)
{
	إذا (pc)
		pc->vfs = vfs;
}

صحيح ul_لار_سېټ_مختاړی(بنية لار_cxt *pc, ثابت حرف *مختاړی)
{
	حرف *p = عدم;

	أكد(pc->dir_fd < 0);

	إذا (مختاړی) {
		p = strdup(مختاړی);
		إذا (!p)
			أرجع -ENOMEM;
	}

	حرر(pc->مختاړی);
	pc->مختاړی = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	أرجع 0;
}

ثابت حرف *ul_لار_get_مختاړی(بنية لار_cxt *pc)
{
	أرجع pc ? pc->مختاړی : عدم;
}

صحيح ul_لار_سېټ_dir(بنية لار_cxt *pc, ثابت حرف *dir)
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

	حرر(pc->dir_لار);
	pc->dir_لار = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	أرجع 0;
}

ثابت حرف *ul_لار_get_dir(بنية لار_cxt *pc)
{
	أرجع pc ? pc->dir_لار : عدم;
}

صحيح ul_لار_سېټ_dialect(بنية لار_cxt *pc, فراغ *معلومات, فراغ free_معلومات(بنية لار_cxt *))
{
	pc->dialect = معلومات;
	pc->free_dialect = free_معلومات;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	أرجع 0;
}

فراغ *ul_لار_get_dialect(بنية لار_cxt *pc)
{
	أرجع pc ? pc->dialect : عدم;
}

صحيح ul_لار_سېټ_enoent_redirect(بنية لار_cxt *pc, صحيح (*func)(بنية لار_cxt *, ثابت حرف *, صحيح *))
{
	pc->redirect_on_enoent = func;
	أرجع 0;
}

ساكن ثابت حرف *get_absdir(بنية لار_cxt *pc)
{
	صحيح rc;
	ثابت حرف *dirpath;

	إذا (!pc->مختاړی)
		أرجع pc->dir_لار;

	dirpath = pc->dir_لار;
	إذا (!dirpath)
		أرجع pc->مختاړی;
	إذا (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->لار_بفر, حجم(pc->لار_بفر), "%s/%s", pc->مختاړی, dirpath);
	إذا (rc < 0)
		أرجع عدم;
	إذا ((size_t)rc >= حجم(pc->لار_بفر)) {
		errno = ENAMETOOLONG;
		أرجع عدم;
	}

	أرجع pc->لار_بفر;
}

صحيح ul_لار_is_accessible(بنية لار_cxt *pc)
{
	ثابت حرف *لار;
	أكد(pc);

	إذا (pc->dir_fd >= 0)
		أرجع 1;

	لار = get_absdir(pc);
	إذا (!لار)
		أرجع 0;
	أرجع access(لار, F_OK) == 0;
}

صحيح ul_لار_get_dirfd(بنية لار_cxt *pc)
{
	أكد(pc);
	أكد(pc->dir_لار);

	إذا (pc->dir_fd < 0) {
		ثابت حرف *لار = get_absdir(pc);
		إذا (!لار)
			أرجع -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", لار));
		pc->dir_fd = open(لار, O_RDONLY|O_CLOEXEC);
	}

	أرجع pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
فراغ ul_لار_close_dirfd(بنية لار_cxt *pc)
{
	أكد(pc);

	إذا (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

صحيح ul_لار_isopen_dirfd(بنية لار_cxt *pc)
{
	أرجع pc && pc->dir_fd >= 0;
}

ساكن ثابت حرف *ul_لار_mkpath(بنية لار_cxt *pc, ثابت حرف *لار, va_list ap)
{
	صحيح rc;

	errno = 0;

	rc = vsnprintf(pc->لار_بفر, حجم(pc->لار_بفر), لار, ap);
	إذا (rc < 0) {
		إذا (!errno)
			errno = EINVAL;
		أرجع عدم;
	}

	إذا ((size_t)rc >= حجم(pc->لار_بفر)) {
		errno = ENAMETOOLONG;
		أرجع عدم;
	}

	أرجع pc->لار_بفر;
}

حرف *ul_لار_get_abspath(بنية لار_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *لار, ...)
{
	إذا (لار) {
		صحيح rc;
		va_list ap;
		ثابت حرف *لکۍ = عدم, *dirpath = pc->dir_لار;

		va_start(ap, لار);
		لکۍ = ul_لار_mkpath(pc, لار, ap);
		va_end(ap);

		إذا (dirpath && *dirpath == '/')
			dirpath++;
		إذا (لکۍ && *لکۍ == '/')
			لکۍ++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->مختاړی ? pc->مختاړی : "",
				dirpath ? dirpath : "",
				لکۍ ? لکۍ : "");

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


صحيح ul_لار_access(بنية لار_cxt *pc, صحيح ډېرپېښ, ثابت حرف *لار)
{
	صحيح rc;

	إذا (!لار)
		أرجع -EINVAL;
	إذا (!pc) {
		rc = access(لار, ډېرپېښ);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", لار, rc));
	} وإلا {
		صحيح dir = ul_لار_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;
		إذا (*لار == '/')
			لار++;

		rc = faccessat(dir, لار, ډېرپېښ, 0);

		إذا (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, لار, &dir) == 0)
			rc = faccessat(dir, لار, ډېرپېښ, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", لار, rc));
	}
	أرجع rc;
}

صحيح ul_لار_accessf(بنية لار_cxt *pc, صحيح ډېرپېښ, ثابت حرف *لار, ...)
{
	va_list ap;
	ثابت حرف *p;

	va_start(ap, لار);
	p = ul_لار_mkpath(pc, لار, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_لار_access(pc, ډېرپېښ, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
صحيح ul_لار_stat(بنية لار_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *لار)
{
	صحيح rc;

	إذا (!pc) {
		rc = لار ? stat(لار, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", لار, rc));
	} وإلا {
		صحيح dir = ul_لار_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;
		إذا (لار) {
			إذا  (*لار == '/')
				لار++;
			rc = fstatat(dir, لار, sb, flags);

		} وإلا
			rc = fstat(dir, sb);	/* dir itself */

		إذا (rc && errno == ENOENT
		    && لار
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, لار, &dir) == 0)
			rc = fstatat(dir, لار, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", لار, rc));
	}
	أرجع rc;
}

صحيح ul_لار_vstatf(بنية لار_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *لار, va_list ap)
{
	ثابت حرف *p = ul_لار_mkpath(pc, لار, ap);

	أرجع !p ? -errno : ul_لار_stat(pc, sb, flags, p);
}

صحيح ul_لار_statf(بنية لار_cxt *pc, بنية stat *sb, صحيح flags, ثابت حرف *لار, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, لار);
	rc = ul_لار_vstatf(pc, sb, flags, لار, ap);
	va_end(ap);

	أرجع rc;
}
#که_تعریف_شوی HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
صحيح ul_لار_statx(بنية لار_cxt *pc, بنية statx *stx, صحيح flags, بینښه صحيح ماسک,
		  ثابت حرف *لار)
{
	صحيح rc;

	إذا (!لار)
		أرجع -EINVAL;
	إذا (!pc)
		rc = لار ? statx(AT_FDCWD, لار, flags, ماسک, stx) : - EINVAL;
	وإلا {
		صحيح dir = ul_لار_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;
		إذا (*لار == '/')
			لار++;

		rc = statx(dir, لار, flags, ماسک, stx);
	}

	أرجع rc;
}
#وإلا
صحيح ul_لار_statx(بنية لار_cxt *pc __attribute__((__unused__)),
		  بنية statx *stx __attribute__((__unused__)),
		  صحيح flags __attribute__((__unused__)),
		  بینښه صحيح ماسک __attribute__((__unused__)),
		  ثابت حرف *لار __attribute__((__unused__)))
{
	errno = ENOSYS;
	أرجع -1;
}
#د_که_پای /* HAVE_STATX */

صحيح ul_لار_vstatxf(بنية لار_cxt *pc, بنية statx *stx, صحيح flags, بینښه صحيح ماسک,
		    ثابت حرف *لار, va_list ap)
{
	ثابت حرف *p = ul_لار_mkpath(pc, لار, ap);

	أرجع !p ? -errno : ul_لار_statx(pc, stx, flags, ماسک, p);
}

صحيح ul_لار_statxf(بنية لار_cxt *pc, بنية statx *stx, صحيح flags, بینښه صحيح ماسک,
		   ثابت حرف *لار, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, لار);
	rc = ul_لار_vstatxf(pc, stx, flags, ماسک, لار, ap);
	va_end(ap);

	أرجع rc;
}

صحيح ul_لار_open(بنية لار_cxt *pc, صحيح flags, ثابت حرف *لار)
{
	صحيح fd;

	إذا (!لار)
		أرجع -EINVAL;
	إذا (!pc) {
		fd = open(لار, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", لار));
	} وإلا {
		صحيح fdx;
		صحيح dir = ul_لار_get_dirfd(pc);
		إذا (dir < 0)
			أرجع dir;

		إذا (*لار == '/')
			لار++;

		fdx = fd = openat(dir, لار, flags);

		إذا (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, لار, &dir) == 0)
			fd = openat(dir, لار, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", لار, fdx != fd ? " [redirected]" : ""));
	}
	أرجع fd;
}

صحيح ul_لار_vopenf(بنية لار_cxt *pc, صحيح flags, ثابت حرف *لار, va_list ap)
{
	ثابت حرف *p = ul_لار_mkpath(pc, لار, ap);

	أرجع !p ? -errno : ul_لار_open(pc, flags, p);
}

صحيح ul_لار_openf(بنية لار_cxt *pc, صحيح flags, ثابت حرف *لار, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, لار);
	rc = ul_لار_vopenf(pc, flags, لار, ap);
	va_end(ap);

	أرجع rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_لار_fopen(بنية لار_cxt *pc, ثابت حرف *ډېرپېښ, ثابت حرف *لار)
{
	صحيح flags = ul_ډېرپېښ_منزل_flags(ډېرپېښ);
	صحيح fd = ul_لار_open(pc, flags, لار);

	إذا (fd < 0)
		أرجع عدم;

	أرجع ul_vfs_fdopen(pc ? pc->vfs : عدم, fd, ډېرپېښ);
}


FILE *ul_لار_vfopenf(بنية لار_cxt *pc, ثابت حرف *ډېرپېښ, ثابت حرف *لار, va_list ap)
{
	ثابت حرف *p = ul_لار_mkpath(pc, لار, ap);

	أرجع !p ? عدم : ul_لار_fopen(pc, ډېرپېښ, p);
}

FILE *ul_لار_fopenf(بنية لار_cxt *pc, ثابت حرف *ډېرپېښ, ثابت حرف *لار, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, لار);
	f = ul_لار_vfopenf(pc, ډېرپېښ, لار, ap);
	va_end(ap);

	أرجع f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_لار_opendir(بنية لار_cxt *pc, ثابت حرف *لار)
{
	DIR *dir;
	صحيح fd = -1;

	إذا (لار)
		fd = ul_لار_open(pc, O_RDONLY|O_CLOEXEC, لار);
	وإلا إذا (pc->dir_لار) {
		صحيح dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_لار_get_dirfd(pc);
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
	إذا (!لار)
		 rewinddir(dir);
	أرجع dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_لار_vopendirf(بنية لار_cxt *pc, ثابت حرف *لار, va_list ap)
{
	ثابت حرف *p = ul_لار_mkpath(pc, لار, ap);

	أرجع !p ? عدم : ul_لار_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_لار_opendirf(بنية لار_cxt *pc, ثابت حرف *لار, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, لار);
	dir = ul_لار_vopendirf(pc, لار, ap);
	va_end(ap);

	أرجع dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_لار_readlink(بنية لار_cxt *pc, حرف *buf, size_t bufsiz, ثابت حرف *لار)
{
	صحيح dirfd;
	ssize_t ssz;

	إذا (!لار) {
		ثابت حرف *p = get_absdir(pc);
		إذا (!p)
			أرجع -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} وإلا {
		dirfd = ul_لار_get_dirfd(pc);
		إذا (dirfd < 0)
			أرجع dirfd;

		إذا (*لار == '/')
			لار++;

		ssz = readlinkat(dirfd, لار, buf, bufsiz - 1);
	}

	إذا (ssz >= 0)
		buf[ssz] = '\0';
	أرجع ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_لار_readlinkf(بنية لار_cxt *pc, حرف *buf, size_t bufsiz, ثابت حرف *لار, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, لار);
	p = ul_لار_mkpath(pc, لار, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_لار_readlink(pc, buf, bufsiz, p);
}

صحيح ul_لار_read(بنية لار_cxt *pc, حرف *buf, size_t len, ثابت حرف *لار)
{
	صحيح rc, errsv;
	صحيح fd;

	fd = ul_لار_open(pc, O_RDONLY|O_CLOEXEC, لار);
	إذا (fd < 0)
		أرجع -errno;

	DBG(CXT, ul_debug(" reading '%s'", لار));
	rc = ul_vfs_read_all(pc ? pc->vfs : عدم, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	أرجع rc;
}

صحيح ul_لار_vreadf(بنية لار_cxt *pc, حرف *buf, size_t len, ثابت حرف *لار, va_list ap)
{
	ثابت حرف *p = ul_لار_mkpath(pc, لار, ap);

	أرجع !p ? -errno : ul_لار_read(pc, buf, len, p);
}

صحيح ul_لار_readf(بنية لار_cxt *pc, حرف *buf, size_t len, ثابت حرف *لار, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, لار);
	rc = ul_لار_vreadf(pc, buf, len, لار, ap);
	va_end(ap);

	أرجع rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
صحيح ul_لار_read_string(بنية لار_cxt *pc, حرف **str, ثابت حرف *لار)
{
	حرف buf[BUFSIZ];
	صحيح rc;

	إذا (!str)
		أرجع -EINVAL;

	*str = عدم;

	rc = ul_لار_read_بفر(pc, buf, حجم(buf), لار);
	إذا (rc < 0)
		أرجع rc;

	*str = strdup(buf);
	إذا (!*str)
		rc = -ENOMEM;

	أرجع rc;
}

صحيح ul_لار_readf_string(بنية لار_cxt *pc, حرف **str, ثابت حرف *لار, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, لار);
	p = ul_لار_mkpath(pc, لار, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_لار_read_string(pc, str, p);
}

صحيح ul_لار_read_بفر(بنية لار_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *لار)
{
	صحيح rc = ul_لار_read(pc, buf, bufsz - 1, لار);

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

صحيح ul_لار_vreadf_بفر(بنية لار_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *لار, va_list ap)
{
	ثابت حرف *p;

	p = ul_لار_mkpath(pc, لار, ap);

	أرجع !p ? -errno : ul_لار_read_بفر(pc, buf, bufsz, p);
}

صحيح ul_لار_readf_بفر(بنية لار_cxt *pc, حرف *buf, size_t bufsz, ثابت حرف *لار, ...)
{
	va_list ap;
	صحيح rc;

	va_start(ap, لار);
	rc = ul_لار_vreadf_بفر(pc, buf, bufsz, لار, ap);
	va_end(ap);

	أرجع rc;
}

صحيح ul_لار_scanf(بنية لار_cxt *pc, ثابت حرف *لار, ثابت حرف *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	صحيح rc;

	f = ul_لار_fopen(pc, "r" UL_CLOEXECSTR, لار);
	إذا (!f)
		أرجع -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, لار));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	أرجع rc;
}

صحيح ul_لار_scanff(بنية لار_cxt *pc, ثابت حرف *لار, va_list ap, ثابت حرف *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	صحيح rc;

	f = ul_لار_vfopenf(pc, "r" UL_CLOEXECSTR, لار, ap);
	إذا (!f)
		أرجع -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	أرجع rc;
}


صحيح ul_لار_read_s64(بنية لار_cxt *pc, int64_t *res, ثابت حرف *لار)
{
	int64_t x = 0;
	صحيح rc;

	rc = ul_لار_scanf(pc, لار, "%"SCNd64, &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_لار_readf_s64(بنية لار_cxt *pc, int64_t *res, ثابت حرف *لار, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, لار);
	p = ul_لار_mkpath(pc, لار, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_لار_read_s64(pc, res, p);
}

صحيح ul_لار_read_u64(بنية لار_cxt *pc, uint64_t *res, ثابت حرف *لار)
{
	uint64_t x = 0;
	صحيح rc;

	rc = ul_لار_scanf(pc, لار, "%"SCNu64, &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_لار_readf_u64(بنية لار_cxt *pc, uint64_t *res, ثابت حرف *لار, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, لار);
	p = ul_لار_mkpath(pc, لار, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_لار_read_u64(pc, res, p);
}

صحيح ul_لار_read_s32(بنية لار_cxt *pc, صحيح *res, ثابت حرف *لار)
{
	صحيح rc, x = 0;

	rc = ul_لار_scanf(pc, لار, "%d", &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_لار_readf_s32(بنية لار_cxt *pc, صحيح *res, ثابت حرف *لار, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, لار);
	p = ul_لار_mkpath(pc, لار, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_لار_read_s32(pc, res, p);
}

صحيح ul_لار_read_u32(بنية لار_cxt *pc, بینښه صحيح *res, ثابت حرف *لار)
{
	صحيح rc;
	بینښه صحيح x = 0;

	rc = ul_لار_scanf(pc, لار, "%u", &x);
	إذا (rc != 1)
		أرجع -1;
	إذا (res)
		*res = x;
	أرجع 0;
}

صحيح ul_لار_readf_u32(بنية لار_cxt *pc, بینښه صحيح *res, ثابت حرف *لار, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, لار);
	p = ul_لار_mkpath(pc, لار, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_لار_read_u32(pc, res, p);
}

صحيح ul_لار_read_majmin(بنية لار_cxt *pc, dev_t *res, ثابت حرف *لار)
{
	صحيح rc, maj = 0, min = 0;

	rc = ul_لار_scanf(pc, لار, "%d:%d", &maj, &min);
	إذا (rc != 2)
		أرجع -1;
	إذا (res)
		*res = makedev(maj, min);
	أرجع 0;
}

صحيح ul_لار_readf_majmin(بنية لار_cxt *pc, dev_t *res, ثابت حرف *لار, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, لار);
	p = ul_لار_mkpath(pc, لار, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_لار_read_majmin(pc, res, p);
}

صحيح ul_لار_write_string(بنية لار_cxt *pc, ثابت حرف *str, ثابت حرف *لار)
{
	صحيح rc, errsv;
	صحيح fd;

	fd = ul_لار_open(pc, O_WRONLY|O_CLOEXEC, لار);
	إذا (fd < 0)
		أرجع -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : عدم, fd, str, مزي_اوږدوالی(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : عدم, fd);
	errno = errsv;
	أرجع rc;
}

صحيح ul_لار_writef_string(بنية لار_cxt *pc, ثابت حرف *str, ثابت حرف *لار, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, لار);
	p = ul_لار_mkpath(pc, لار, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_لار_write_string(pc, str, p);
}

صحيح ul_لار_write_s64(بنية لار_cxt *pc, int64_t عدد, ثابت حرف *لار)
{
	حرف buf[حجم(stringify_ارزښت(LLONG_MAX))];
	صحيح rc, errsv;
	صحيح fd, len;

	fd = ul_لار_open(pc, O_WRONLY|O_CLOEXEC, لار);
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

صحيح ul_لار_write_u64(بنية لار_cxt *pc, uint64_t عدد, ثابت حرف *لار)
{
	حرف buf[حجم(stringify_ارزښت(ULLONG_MAX))];
	صحيح rc, errsv;
	صحيح fd, len;

	fd = ul_لار_open(pc, O_WRONLY|O_CLOEXEC, لار);
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

صحيح ul_لار_writef_u64(بنية لار_cxt *pc, uint64_t عدد, ثابت حرف *لار, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, لار);
	p = ul_لار_mkpath(pc, لار, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_لار_write_u64(pc, عدد, p);

}

صحيح ul_لار_شمېر_dirents(بنية لار_cxt *pc, ثابت حرف *لار)
{
	DIR *dir;
	صحيح r = 0;

	dir = ul_لار_opendir(pc, لار);
	إذا (!dir)
		أرجع 0;

	طالما (xreaddir(dir)) r++;

	closedir(dir);
	أرجع r;
}

صحيح ul_لار_countf_dirents(بنية لار_cxt *pc, ثابت حرف *لار, ...)
{
	ثابت حرف *p;
	va_list ap;

	va_start(ap, لار);
	p = ul_لار_mkpath(pc, لار, ap);
	va_end(ap);

	أرجع !p ? -errno : ul_لار_شمېر_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
صحيح ul_لار_بل_dirent(بنية لار_cxt *pc, DIR **sub, ثابت حرف *dirname, بنية dirent **d)
{
	إذا (!pc || !sub || !d)
		أرجع -EINVAL;

	إذا (!*sub) {
		*sub = ul_لار_opendir(pc, dirname);
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

#که_تعریف_شوی HAVE_CPU_سېټ_T
ساكن صحيح ul_لار_cpuparse(بنية لار_cxt *pc, cpu_set_t **سېټ, صحيح maxcpus, صحيح islist, ثابت حرف *لار, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	حرف *buf;
	صحيح rc;

	*سېټ = عدم;

	buf = حافظه_ورکړه(len);
	إذا (!buf)
		أرجع -ENOMEM;

	rc = ul_لار_vreadf_بفر(pc, buf, len, لار, ap);
	إذا (rc < 0)
		اذهب بهر;

	*سېټ = cpuset_alloc(maxcpus, &setsize, عدم);
	إذا (!*سېټ) {
		rc = -EINVAL;
		اذهب بهر;
	}

	إذا (islist) {
		إذا (cpulist_parse(buf, *سېټ, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			اذهب بهر;
		}
	} وإلا {
		إذا (cpumask_parse(buf, *سېټ, setsize)) {
			errno = EINVAL;
			rc = -errno;
			اذهب بهر;
		}
	}
	rc = 0;

بهر:
	إذا (rc) {
		cpuset_free(*سېټ);
		*سېټ = عدم;
	}
	حرر(buf);
	أرجع rc;
}

صحيح ul_لار_readf_cpuset(بنية لار_cxt *pc, cpu_set_t **سېټ, صحيح maxcpus, ثابت حرف *لار, ...)
{
	va_list ap;
	صحيح rc = 0;

	va_start(ap, لار);
	rc = ul_لار_cpuparse(pc, سېټ, maxcpus, 0, لار, ap);
	va_end(ap);

	أرجع rc;
}

صحيح ul_لار_readf_cpulist(بنية لار_cxt *pc, cpu_set_t **سېټ, صحيح maxcpus, ثابت حرف *لار, ...)
{
	va_list ap;
	صحيح rc = 0;

	va_start(ap, لار);
	rc = ul_لار_cpuparse(pc, سېټ, maxcpus, 1, لار, ap);
	va_end(ap);

	أرجع rc;
}

#د_که_پای /* HAVE_CPU_SET_T */


#که_تعریف_شوی TEST_پروګرام_لار
#ضمن <getopt.h>

ساكن فراغ __attribute__((__noreturn__)) usage(فراغ)
{
	fprintf(stdout, " %s [options] <dir> <command>\n\n", پروګرام_invocation_short_name);
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

	وتل(EXIT_SUCCESS);
}

صحيح اصلي(صحيح argc, حرف *argv[])
{
	صحيح c;
	ثابت حرف *مختاړی = عدم, *dir, *file, *command;
	بنية لار_cxt *pc = عدم;

	ساكن ثابت بنية option longopts[] = {
		{ "prefix",	1, عدم, 'p' },
		{ "help",       0, عدم, 'h' },
		{ عدم, 0, عدم, 0 },
	};

	طالما((c = getopt_long(argc, argv, "p:h", longopts, عدم)) != -1) {
		بدل(c) {
		حالة 'p':
			مختاړی = optarg;
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

	ul_لار_init_debug();

	pc = ul_new_لار("%s", dir);
	إذا (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	إذا (مختاړی)
		ul_لار_سېټ_مختاړی(pc, مختاړی);

	إذا (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	إذا (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_لار_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		چاپ_کړه("read:  %s: %u\n", file, res);

		إذا (ul_لار_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		چاپ_کړه("readf: %s: %u\n", file, res);

	} وإلا إذا (strcmp(command, "read-s32") == 0) {
		int32_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_لار_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		چاپ_کړه("read:  %s: %d\n", file, res);

		إذا (ul_لار_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		چاپ_کړه("readf: %s: %d\n", file, res);

	} وإلا إذا (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_لار_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		چاپ_کړه("read:  %s: %" PRIu64 "\n", file, res);

		إذا (ul_لار_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		چاپ_کړه("readf: %s: %" PRIu64 "\n", file, res);

	} وإلا إذا (strcmp(command, "read-s64") == 0) {
		int64_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_لار_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		چاپ_کړه("read:  %s: %" PRId64 "\n", file, res);

		إذا (ul_لار_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		چاپ_کړه("readf: %s: %" PRId64 "\n", file, res);

	} وإلا إذا (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_لار_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		چاپ_کړه("read:  %s: %d\n", file, (صحيح) res);

		إذا (ul_لار_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		چاپ_کړه("readf: %s: %d\n", file, (صحيح) res);

	} وإلا إذا (strcmp(command, "read-string") == 0) {
		حرف *res;

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_لار_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		چاپ_کړه("read:  %s: %s\n", file, res);

		إذا (ul_لار_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		چاپ_کړه("readf: %s: %s\n", file, res);

	} وإلا إذا (strcmp(command, "read-link") == 0) {
		حرف res[PATH_MAX];

		إذا (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		إذا (ul_لار_readlink(pc, res, حجم(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		چاپ_کړه("read:  %s: %s\n", file, res);

		إذا (ul_لار_readlinkf(pc, res, حجم(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		چاپ_کړه("readf: %s: %s\n", file, res);

	} وإلا إذا (strcmp(command, "write-string") == 0) {
		حرف *str;

		إذا (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		إذا (ul_لار_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		إذا (ul_لار_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} وإلا إذا (strcmp(command, "write-u64") == 0) {
		uint64_t عدد;

		إذا (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		عدد = strtoumax(argv[optind++], عدم, 0);

		إذا (ul_لار_write_u64(pc, عدد, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		إذا (ul_لار_writef_u64(pc, عدد, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_لار(pc);
	أرجع EXIT_SUCCESS;
}
#د_که_پای /* TEST_PROGRAM_PATH */
