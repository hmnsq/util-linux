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
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <inttypes.h>
#అన్తర్భావ <errno.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "fileutils.h"
#అన్తర్భావ "all-io.h"
#అన్తర్భావ "vfs.h"
#అన్తర్భావ "path.h"
#అన్తర్భావ "debug.h"
#అన్తర్భావ "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
స్థిర UL_DEBUG_DEFINE_ముసుగు(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_ఖాళీలు_MASKNAMES;

#నిర్వచించు ULPATH_DEBUG_INIT	(1 << 1)
#నిర్వచించు ULPATH_DEBUG_CXT	(1 << 2)

#నిర్వచించు DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#నిర్వచించు DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#నిర్వచించు ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

శూన్య ul_మార్గం_init_debug(శూన్య)
{
	ఒకవేళ (ulpath_debug_ముసుగు)
		తిరిగి;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

నిర్మాణం మార్గం_cxt *ul_new_మార్గం(స్థిరరాశి అక్షరం *dir, ...)
{
	నిర్మాణం మార్గం_cxt *pc = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*pc));

	ఒకవేళ (!pc)
		తిరిగి ఖాళీ;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	ఒకవేళ (dir) {
		పూర్ణాంకం rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_మార్గం, dir, ap);
		va_end(ap);

		ఒకవేళ (rc < 0 || !pc->dir_మార్గం)
			వెళ్ళు fail;
	}
	తిరిగి pc;
fail:
	ul_unref_మార్గం(pc);
	తిరిగి ఖాళీ;
}

శూన్య ul_ref_మార్గం(నిర్మాణం మార్గం_cxt *pc)
{
	ఒకవేళ (pc)
		pc->refcount++;
}

శూన్య ul_unref_మార్గం(నిర్మాణం మార్గం_cxt *pc)
{
	ఒకవేళ (!pc)
		తిరిగి;

	pc->refcount--;

	ఒకవేళ (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		ఒకవేళ (pc->dialect)
			pc->free_dialect(pc);
		ul_మార్గం_close_dirfd(pc);
		విడుదల(pc->dir_మార్గం);
		విడుదల(pc->ఉపసర్గ);
		విడుదల(pc);
	}
}

శూన్య ul_మార్గం_refer_vfs(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి నిర్మాణం ul_vfs_ops *vfs)
{
	ఒకవేళ (pc)
		pc->vfs = vfs;
}

పూర్ణాంకం ul_మార్గం_సమితి_ఉపసర్గ(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *ఉపసర్గ)
{
	అక్షరం *p = ఖాళీ;

	ప్రతిజ్ఞా(pc->dir_fd < 0);

	ఒకవేళ (ఉపసర్గ) {
		p = strdup(ఉపసర్గ);
		ఒకవేళ (!p)
			తిరిగి -ENOMEM;
	}

	విడుదల(pc->ఉపసర్గ);
	pc->ఉపసర్గ = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	తిరిగి 0;
}

స్థిరరాశి అక్షరం *ul_మార్గం_get_ఉపసర్గ(నిర్మాణం మార్గం_cxt *pc)
{
	తిరిగి pc ? pc->ఉపసర్గ : ఖాళీ;
}

పూర్ణాంకం ul_మార్గం_సమితి_dir(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *dir)
{
	అక్షరం *p = ఖాళీ;

	ఒకవేళ (dir) {
		p = strdup(dir);
		ఒకవేళ (!p)
			తిరిగి -ENOMEM;
	}

	ఒకవేళ (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	విడుదల(pc->dir_మార్గం);
	pc->dir_మార్గం = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	తిరిగి 0;
}

స్థిరరాశి అక్షరం *ul_మార్గం_get_dir(నిర్మాణం మార్గం_cxt *pc)
{
	తిరిగి pc ? pc->dir_మార్గం : ఖాళీ;
}

పూర్ణాంకం ul_మార్గం_సమితి_dialect(నిర్మాణం మార్గం_cxt *pc, శూన్య *దత్తాంశం, శూన్య free_దత్తాంశం(నిర్మాణం మార్గం_cxt *))
{
	pc->dialect = దత్తాంశం;
	pc->free_dialect = free_దత్తాంశం;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	తిరిగి 0;
}

శూన్య *ul_మార్గం_get_dialect(నిర్మాణం మార్గం_cxt *pc)
{
	తిరిగి pc ? pc->dialect : ఖాళీ;
}

పూర్ణాంకం ul_మార్గం_సమితి_enoent_redirect(నిర్మాణం మార్గం_cxt *pc, పూర్ణాంకం (*func)(నిర్మాణం మార్గం_cxt *, స్థిరరాశి అక్షరం *, పూర్ణాంకం *))
{
	pc->redirect_on_enoent = func;
	తిరిగి 0;
}

స్థిర స్థిరరాశి అక్షరం *get_absdir(నిర్మాణం మార్గం_cxt *pc)
{
	పూర్ణాంకం rc;
	స్థిరరాశి అక్షరం *dirpath;

	ఒకవేళ (!pc->ఉపసర్గ)
		తిరిగి pc->dir_మార్గం;

	dirpath = pc->dir_మార్గం;
	ఒకవేళ (!dirpath)
		తిరిగి pc->ఉపసర్గ;
	ఒకవేళ (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->మార్గం_బఫరు, పరిమాణం(pc->మార్గం_బఫరు), "%s/%s", pc->ఉపసర్గ, dirpath);
	ఒకవేళ (rc < 0)
		తిరిగి ఖాళీ;
	ఒకవేళ ((size_t)rc >= పరిమాణం(pc->మార్గం_బఫరు)) {
		errno = ENAMETOOLONG;
		తిరిగి ఖాళీ;
	}

	తిరిగి pc->మార్గం_బఫరు;
}

పూర్ణాంకం ul_మార్గం_is_accessible(నిర్మాణం మార్గం_cxt *pc)
{
	స్థిరరాశి అక్షరం *మార్గం;
	ప్రతిజ్ఞా(pc);

	ఒకవేళ (pc->dir_fd >= 0)
		తిరిగి 1;

	మార్గం = get_absdir(pc);
	ఒకవేళ (!మార్గం)
		తిరిగి 0;
	తిరిగి access(మార్గం, F_OK) == 0;
}

పూర్ణాంకం ul_మార్గం_get_dirfd(నిర్మాణం మార్గం_cxt *pc)
{
	ప్రతిజ్ఞా(pc);
	ప్రతిజ్ఞా(pc->dir_మార్గం);

	ఒకవేళ (pc->dir_fd < 0) {
		స్థిరరాశి అక్షరం *మార్గం = get_absdir(pc);
		ఒకవేళ (!మార్గం)
			తిరిగి -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", మార్గం));
		pc->dir_fd = open(మార్గం, O_RDONLY|O_CLOEXEC);
	}

	తిరిగి pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
శూన్య ul_మార్గం_close_dirfd(నిర్మాణం మార్గం_cxt *pc)
{
	ప్రతిజ్ఞా(pc);

	ఒకవేళ (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

పూర్ణాంకం ul_మార్గం_isopen_dirfd(నిర్మాణం మార్గం_cxt *pc)
{
	తిరిగి pc && pc->dir_fd >= 0;
}

స్థిర స్థిరరాశి అక్షరం *ul_మార్గం_mkpath(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *మార్గం, va_list ap)
{
	పూర్ణాంకం rc;

	errno = 0;

	rc = vsnprintf(pc->మార్గం_బఫరు, పరిమాణం(pc->మార్గం_బఫరు), మార్గం, ap);
	ఒకవేళ (rc < 0) {
		ఒకవేళ (!errno)
			errno = EINVAL;
		తిరిగి ఖాళీ;
	}

	ఒకవేళ ((size_t)rc >= పరిమాణం(pc->మార్గం_బఫరు)) {
		errno = ENAMETOOLONG;
		తిరిగి ఖాళీ;
	}

	తిరిగి pc->మార్గం_బఫరు;
}

అక్షరం *ul_మార్గం_get_abspath(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t bufsz, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	ఒకవేళ (మార్గం) {
		పూర్ణాంకం rc;
		va_list ap;
		స్థిరరాశి అక్షరం *తోక = ఖాళీ, *dirpath = pc->dir_మార్గం;

		va_start(ap, మార్గం);
		తోక = ul_మార్గం_mkpath(pc, మార్గం, ap);
		va_end(ap);

		ఒకవేళ (dirpath && *dirpath == '/')
			dirpath++;
		ఒకవేళ (తోక && *తోక == '/')
			తోక++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->ఉపసర్గ ? pc->ఉపసర్గ : "",
				dirpath ? dirpath : "",
				తోక ? తోక : "");

		ఒకవేళ ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			తిరిగి ఖాళీ;
		}
	} లేకపోతే {
		స్థిరరాశి అక్షరం *tmp = get_absdir(pc);

		ఒకవేళ (!tmp)
			తిరిగి ఖాళీ;
		xstrncpy(buf, tmp, bufsz);
	}

	తిరిగి buf;
}


పూర్ణాంకం ul_మార్గం_access(నిర్మాణం మార్గం_cxt *pc, పూర్ణాంకం బహుళకం, స్థిరరాశి అక్షరం *మార్గం)
{
	పూర్ణాంకం rc;

	ఒకవేళ (!మార్గం)
		తిరిగి -EINVAL;
	ఒకవేళ (!pc) {
		rc = access(మార్గం, బహుళకం);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", మార్గం, rc));
	} లేకపోతే {
		పూర్ణాంకం dir = ul_మార్గం_get_dirfd(pc);
		ఒకవేళ (dir < 0)
			తిరిగి dir;
		ఒకవేళ (*మార్గం == '/')
			మార్గం++;

		rc = faccessat(dir, మార్గం, బహుళకం, 0);

		ఒకవేళ (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, మార్గం, &dir) == 0)
			rc = faccessat(dir, మార్గం, బహుళకం, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", మార్గం, rc));
	}
	తిరిగి rc;
}

పూర్ణాంకం ul_మార్గం_accessf(నిర్మాణం మార్గం_cxt *pc, పూర్ణాంకం బహుళకం, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	va_list ap;
	స్థిరరాశి అక్షరం *p;

	va_start(ap, మార్గం);
	p = ul_మార్గం_mkpath(pc, మార్గం, ap);
	va_end(ap);

	తిరిగి !p ? -errno : ul_మార్గం_access(pc, బహుళకం, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
పూర్ణాంకం ul_మార్గం_stat(నిర్మాణం మార్గం_cxt *pc, నిర్మాణం stat *sb, పూర్ణాంకం flags, స్థిరరాశి అక్షరం *మార్గం)
{
	పూర్ణాంకం rc;

	ఒకవేళ (!pc) {
		rc = మార్గం ? stat(మార్గం, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", మార్గం, rc));
	} లేకపోతే {
		పూర్ణాంకం dir = ul_మార్గం_get_dirfd(pc);
		ఒకవేళ (dir < 0)
			తిరిగి dir;
		ఒకవేళ (మార్గం) {
			ఒకవేళ  (*మార్గం == '/')
				మార్గం++;
			rc = fstatat(dir, మార్గం, sb, flags);

		} లేకపోతే
			rc = fstat(dir, sb);	/* dir itself */

		ఒకవేళ (rc && errno == ENOENT
		    && మార్గం
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, మార్గం, &dir) == 0)
			rc = fstatat(dir, మార్గం, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", మార్గం, rc));
	}
	తిరిగి rc;
}

పూర్ణాంకం ul_మార్గం_vstatf(నిర్మాణం మార్గం_cxt *pc, నిర్మాణం stat *sb, పూర్ణాంకం flags, స్థిరరాశి అక్షరం *మార్గం, va_list ap)
{
	స్థిరరాశి అక్షరం *p = ul_మార్గం_mkpath(pc, మార్గం, ap);

	తిరిగి !p ? -errno : ul_మార్గం_stat(pc, sb, flags, p);
}

పూర్ణాంకం ul_మార్గం_statf(నిర్మాణం మార్గం_cxt *pc, నిర్మాణం stat *sb, పూర్ణాంకం flags, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	va_list ap;
	పూర్ణాంకం rc;

	va_start(ap, మార్గం);
	rc = ul_మార్గం_vstatf(pc, sb, flags, మార్గం, ap);
	va_end(ap);

	తిరిగి rc;
}
#నిర్వచితమైతే HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
పూర్ణాంకం ul_మార్గం_statx(నిర్మాణం మార్గం_cxt *pc, నిర్మాణం statx *stx, పూర్ణాంకం flags, గుర్తులేని పూర్ణాంకం ముసుగు,
		  స్థిరరాశి అక్షరం *మార్గం)
{
	పూర్ణాంకం rc;

	ఒకవేళ (!మార్గం)
		తిరిగి -EINVAL;
	ఒకవేళ (!pc)
		rc = మార్గం ? statx(AT_FDCWD, మార్గం, flags, ముసుగు, stx) : - EINVAL;
	లేకపోతే {
		పూర్ణాంకం dir = ul_మార్గం_get_dirfd(pc);
		ఒకవేళ (dir < 0)
			తిరిగి dir;
		ఒకవేళ (*మార్గం == '/')
			మార్గం++;

		rc = statx(dir, మార్గం, flags, ముసుగు, stx);
	}

	తిరిగి rc;
}
#లేకపోతే
పూర్ణాంకం ul_మార్గం_statx(నిర్మాణం మార్గం_cxt *pc __attribute__((__unused__)),
		  నిర్మాణం statx *stx __attribute__((__unused__)),
		  పూర్ణాంకం flags __attribute__((__unused__)),
		  గుర్తులేని పూర్ణాంకం ముసుగు __attribute__((__unused__)),
		  స్థిరరాశి అక్షరం *మార్గం __attribute__((__unused__)))
{
	errno = ENOSYS;
	తిరిగి -1;
}
#అయితే_ముగింపు /* HAVE_STATX */

పూర్ణాంకం ul_మార్గం_vstatxf(నిర్మాణం మార్గం_cxt *pc, నిర్మాణం statx *stx, పూర్ణాంకం flags, గుర్తులేని పూర్ణాంకం ముసుగు,
		    స్థిరరాశి అక్షరం *మార్గం, va_list ap)
{
	స్థిరరాశి అక్షరం *p = ul_మార్గం_mkpath(pc, మార్గం, ap);

	తిరిగి !p ? -errno : ul_మార్గం_statx(pc, stx, flags, ముసుగు, p);
}

పూర్ణాంకం ul_మార్గం_statxf(నిర్మాణం మార్గం_cxt *pc, నిర్మాణం statx *stx, పూర్ణాంకం flags, గుర్తులేని పూర్ణాంకం ముసుగు,
		   స్థిరరాశి అక్షరం *మార్గం, ...)
{
	va_list ap;
	పూర్ణాంకం rc;

	va_start(ap, మార్గం);
	rc = ul_మార్గం_vstatxf(pc, stx, flags, ముసుగు, మార్గం, ap);
	va_end(ap);

	తిరిగి rc;
}

పూర్ణాంకం ul_మార్గం_open(నిర్మాణం మార్గం_cxt *pc, పూర్ణాంకం flags, స్థిరరాశి అక్షరం *మార్గం)
{
	పూర్ణాంకం fd;

	ఒకవేళ (!మార్గం)
		తిరిగి -EINVAL;
	ఒకవేళ (!pc) {
		fd = open(మార్గం, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", మార్గం));
	} లేకపోతే {
		పూర్ణాంకం fdx;
		పూర్ణాంకం dir = ul_మార్గం_get_dirfd(pc);
		ఒకవేళ (dir < 0)
			తిరిగి dir;

		ఒకవేళ (*మార్గం == '/')
			మార్గం++;

		fdx = fd = openat(dir, మార్గం, flags);

		ఒకవేళ (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, మార్గం, &dir) == 0)
			fd = openat(dir, మార్గం, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", మార్గం, fdx != fd ? " [redirected]" : ""));
	}
	తిరిగి fd;
}

పూర్ణాంకం ul_మార్గం_vopenf(నిర్మాణం మార్గం_cxt *pc, పూర్ణాంకం flags, స్థిరరాశి అక్షరం *మార్గం, va_list ap)
{
	స్థిరరాశి అక్షరం *p = ul_మార్గం_mkpath(pc, మార్గం, ap);

	తిరిగి !p ? -errno : ul_మార్గం_open(pc, flags, p);
}

పూర్ణాంకం ul_మార్గం_openf(నిర్మాణం మార్గం_cxt *pc, పూర్ణాంకం flags, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	va_list ap;
	పూర్ణాంకం rc;

	va_start(ap, మార్గం);
	rc = ul_మార్గం_vopenf(pc, flags, మార్గం, ap);
	va_end(ap);

	తిరిగి rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_మార్గం_fopen(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *బహుళకం, స్థిరరాశి అక్షరం *మార్గం)
{
	పూర్ణాంకం flags = ul_బహుళకం_గమ్యం_flags(బహుళకం);
	పూర్ణాంకం fd = ul_మార్గం_open(pc, flags, మార్గం);

	ఒకవేళ (fd < 0)
		తిరిగి ఖాళీ;

	తిరిగి ul_vfs_fdopen(pc ? pc->vfs : ఖాళీ, fd, బహుళకం);
}


FILE *ul_మార్గం_vfopenf(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *బహుళకం, స్థిరరాశి అక్షరం *మార్గం, va_list ap)
{
	స్థిరరాశి అక్షరం *p = ul_మార్గం_mkpath(pc, మార్గం, ap);

	తిరిగి !p ? ఖాళీ : ul_మార్గం_fopen(pc, బహుళకం, p);
}

FILE *ul_మార్గం_fopenf(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *బహుళకం, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, మార్గం);
	f = ul_మార్గం_vfopenf(pc, బహుళకం, మార్గం, ap);
	va_end(ap);

	తిరిగి f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_మార్గం_opendir(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *మార్గం)
{
	DIR *dir;
	పూర్ణాంకం fd = -1;

	ఒకవేళ (మార్గం)
		fd = ul_మార్గం_open(pc, O_RDONLY|O_CLOEXEC, మార్గం);
	లేకపోతే ఒకవేళ (pc->dir_మార్గం) {
		పూర్ణాంకం dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_మార్గం_get_dirfd(pc);
		ఒకవేళ (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	ఒకవేళ (fd < 0)
		తిరిగి ఖాళీ;

	dir = fdopendir(fd);
	ఒకవేళ (!dir) {
		close(fd);
		తిరిగి ఖాళీ;
	}
	ఒకవేళ (!మార్గం)
		 rewinddir(dir);
	తిరిగి dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_మార్గం_vopendirf(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *మార్గం, va_list ap)
{
	స్థిరరాశి అక్షరం *p = ul_మార్గం_mkpath(pc, మార్గం, ap);

	తిరిగి !p ? ఖాళీ : ul_మార్గం_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_మార్గం_opendirf(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, మార్గం);
	dir = ul_మార్గం_vopendirf(pc, మార్గం, ap);
	va_end(ap);

	తిరిగి dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_మార్గం_readlink(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t bufsiz, స్థిరరాశి అక్షరం *మార్గం)
{
	పూర్ణాంకం dirfd;
	ssize_t ssz;

	ఒకవేళ (!మార్గం) {
		స్థిరరాశి అక్షరం *p = get_absdir(pc);
		ఒకవేళ (!p)
			తిరిగి -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} లేకపోతే {
		dirfd = ul_మార్గం_get_dirfd(pc);
		ఒకవేళ (dirfd < 0)
			తిరిగి dirfd;

		ఒకవేళ (*మార్గం == '/')
			మార్గం++;

		ssz = readlinkat(dirfd, మార్గం, buf, bufsiz - 1);
	}

	ఒకవేళ (ssz >= 0)
		buf[ssz] = '\0';
	తిరిగి ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_మార్గం_readlinkf(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t bufsiz, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	స్థిరరాశి అక్షరం *p;
	va_list ap;

	va_start(ap, మార్గం);
	p = ul_మార్గం_mkpath(pc, మార్గం, ap);
	va_end(ap);

	తిరిగి !p ? -errno : ul_మార్గం_readlink(pc, buf, bufsiz, p);
}

పూర్ణాంకం ul_మార్గం_read(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t len, స్థిరరాశి అక్షరం *మార్గం)
{
	పూర్ణాంకం rc, errsv;
	పూర్ణాంకం fd;

	fd = ul_మార్గం_open(pc, O_RDONLY|O_CLOEXEC, మార్గం);
	ఒకవేళ (fd < 0)
		తిరిగి -errno;

	DBG(CXT, ul_debug(" reading '%s'", మార్గం));
	rc = ul_vfs_read_all(pc ? pc->vfs : ఖాళీ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ఖాళీ, fd);
	errno = errsv;
	తిరిగి rc;
}

పూర్ణాంకం ul_మార్గం_vreadf(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t len, స్థిరరాశి అక్షరం *మార్గం, va_list ap)
{
	స్థిరరాశి అక్షరం *p = ul_మార్గం_mkpath(pc, మార్గం, ap);

	తిరిగి !p ? -errno : ul_మార్గం_read(pc, buf, len, p);
}

పూర్ణాంకం ul_మార్గం_readf(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t len, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	va_list ap;
	పూర్ణాంకం rc;

	va_start(ap, మార్గం);
	rc = ul_మార్గం_vreadf(pc, buf, len, మార్గం, ap);
	va_end(ap);

	తిరిగి rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
పూర్ణాంకం ul_మార్గం_read_string(నిర్మాణం మార్గం_cxt *pc, అక్షరం **str, స్థిరరాశి అక్షరం *మార్గం)
{
	అక్షరం buf[BUFSIZ];
	పూర్ణాంకం rc;

	ఒకవేళ (!str)
		తిరిగి -EINVAL;

	*str = ఖాళీ;

	rc = ul_మార్గం_read_బఫరు(pc, buf, పరిమాణం(buf), మార్గం);
	ఒకవేళ (rc < 0)
		తిరిగి rc;

	*str = strdup(buf);
	ఒకవేళ (!*str)
		rc = -ENOMEM;

	తిరిగి rc;
}

పూర్ణాంకం ul_మార్గం_readf_string(నిర్మాణం మార్గం_cxt *pc, అక్షరం **str, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	స్థిరరాశి అక్షరం *p;
	va_list ap;

	va_start(ap, మార్గం);
	p = ul_మార్గం_mkpath(pc, మార్గం, ap);
	va_end(ap);

	తిరిగి !p ? -errno : ul_మార్గం_read_string(pc, str, p);
}

పూర్ణాంకం ul_మార్గం_read_బఫరు(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t bufsz, స్థిరరాశి అక్షరం *మార్గం)
{
	పూర్ణాంకం rc = ul_మార్గం_read(pc, buf, bufsz - 1, మార్గం);

	ఒకవేళ (rc == 0)
		buf[0] = '\0';

	లేకపోతే ఒకవేళ (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		ఒకవేళ (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		లేకపోతే
			buf[rc] = '\0';
	}

	తిరిగి rc;
}

పూర్ణాంకం ul_మార్గం_vreadf_బఫరు(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t bufsz, స్థిరరాశి అక్షరం *మార్గం, va_list ap)
{
	స్థిరరాశి అక్షరం *p;

	p = ul_మార్గం_mkpath(pc, మార్గం, ap);

	తిరిగి !p ? -errno : ul_మార్గం_read_బఫరు(pc, buf, bufsz, p);
}

పూర్ణాంకం ul_మార్గం_readf_బఫరు(నిర్మాణం మార్గం_cxt *pc, అక్షరం *buf, size_t bufsz, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	va_list ap;
	పూర్ణాంకం rc;

	va_start(ap, మార్గం);
	rc = ul_మార్గం_vreadf_బఫరు(pc, buf, bufsz, మార్గం, ap);
	va_end(ap);

	తిరిగి rc;
}

పూర్ణాంకం ul_మార్గం_scanf(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *మార్గం, స్థిరరాశి అక్షరం *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	పూర్ణాంకం rc;

	f = ul_మార్గం_fopen(pc, "r" UL_CLOEXECSTR, మార్గం);
	ఒకవేళ (!f)
		తిరిగి -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, మార్గం));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	తిరిగి rc;
}

పూర్ణాంకం ul_మార్గం_scanff(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *మార్గం, va_list ap, స్థిరరాశి అక్షరం *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	పూర్ణాంకం rc;

	f = ul_మార్గం_vfopenf(pc, "r" UL_CLOEXECSTR, మార్గం, ap);
	ఒకవేళ (!f)
		తిరిగి -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	తిరిగి rc;
}


పూర్ణాంకం ul_మార్గం_read_s64(నిర్మాణం మార్గం_cxt *pc, int64_t *res, స్థిరరాశి అక్షరం *మార్గం)
{
	int64_t x = 0;
	పూర్ణాంకం rc;

	rc = ul_మార్గం_scanf(pc, మార్గం, "%"SCNd64, &x);
	ఒకవేళ (rc != 1)
		తిరిగి -1;
	ఒకవేళ (res)
		*res = x;
	తిరిగి 0;
}

పూర్ణాంకం ul_మార్గం_readf_s64(నిర్మాణం మార్గం_cxt *pc, int64_t *res, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	స్థిరరాశి అక్షరం *p;
	va_list ap;

	va_start(ap, మార్గం);
	p = ul_మార్గం_mkpath(pc, మార్గం, ap);
	va_end(ap);

	తిరిగి !p ? -errno : ul_మార్గం_read_s64(pc, res, p);
}

పూర్ణాంకం ul_మార్గం_read_u64(నిర్మాణం మార్గం_cxt *pc, uint64_t *res, స్థిరరాశి అక్షరం *మార్గం)
{
	uint64_t x = 0;
	పూర్ణాంకం rc;

	rc = ul_మార్గం_scanf(pc, మార్గం, "%"SCNu64, &x);
	ఒకవేళ (rc != 1)
		తిరిగి -1;
	ఒకవేళ (res)
		*res = x;
	తిరిగి 0;
}

పూర్ణాంకం ul_మార్గం_readf_u64(నిర్మాణం మార్గం_cxt *pc, uint64_t *res, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	స్థిరరాశి అక్షరం *p;
	va_list ap;

	va_start(ap, మార్గం);
	p = ul_మార్గం_mkpath(pc, మార్గం, ap);
	va_end(ap);

	తిరిగి !p ? -errno : ul_మార్గం_read_u64(pc, res, p);
}

పూర్ణాంకం ul_మార్గం_read_s32(నిర్మాణం మార్గం_cxt *pc, పూర్ణాంకం *res, స్థిరరాశి అక్షరం *మార్గం)
{
	పూర్ణాంకం rc, x = 0;

	rc = ul_మార్గం_scanf(pc, మార్గం, "%d", &x);
	ఒకవేళ (rc != 1)
		తిరిగి -1;
	ఒకవేళ (res)
		*res = x;
	తిరిగి 0;
}

పూర్ణాంకం ul_మార్గం_readf_s32(నిర్మాణం మార్గం_cxt *pc, పూర్ణాంకం *res, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	స్థిరరాశి అక్షరం *p;
	va_list ap;

	va_start(ap, మార్గం);
	p = ul_మార్గం_mkpath(pc, మార్గం, ap);
	va_end(ap);

	తిరిగి !p ? -errno : ul_మార్గం_read_s32(pc, res, p);
}

పూర్ణాంకం ul_మార్గం_read_u32(నిర్మాణం మార్గం_cxt *pc, గుర్తులేని పూర్ణాంకం *res, స్థిరరాశి అక్షరం *మార్గం)
{
	పూర్ణాంకం rc;
	గుర్తులేని పూర్ణాంకం x = 0;

	rc = ul_మార్గం_scanf(pc, మార్గం, "%u", &x);
	ఒకవేళ (rc != 1)
		తిరిగి -1;
	ఒకవేళ (res)
		*res = x;
	తిరిగి 0;
}

పూర్ణాంకం ul_మార్గం_readf_u32(నిర్మాణం మార్గం_cxt *pc, గుర్తులేని పూర్ణాంకం *res, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	స్థిరరాశి అక్షరం *p;
	va_list ap;

	va_start(ap, మార్గం);
	p = ul_మార్గం_mkpath(pc, మార్గం, ap);
	va_end(ap);

	తిరిగి !p ? -errno : ul_మార్గం_read_u32(pc, res, p);
}

పూర్ణాంకం ul_మార్గం_read_majmin(నిర్మాణం మార్గం_cxt *pc, dev_t *res, స్థిరరాశి అక్షరం *మార్గం)
{
	పూర్ణాంకం rc, maj = 0, min = 0;

	rc = ul_మార్గం_scanf(pc, మార్గం, "%d:%d", &maj, &min);
	ఒకవేళ (rc != 2)
		తిరిగి -1;
	ఒకవేళ (res)
		*res = makedev(maj, min);
	తిరిగి 0;
}

పూర్ణాంకం ul_మార్గం_readf_majmin(నిర్మాణం మార్గం_cxt *pc, dev_t *res, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	స్థిరరాశి అక్షరం *p;
	va_list ap;

	va_start(ap, మార్గం);
	p = ul_మార్గం_mkpath(pc, మార్గం, ap);
	va_end(ap);

	తిరిగి !p ? -errno : ul_మార్గం_read_majmin(pc, res, p);
}

పూర్ణాంకం ul_మార్గం_write_string(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *str, స్థిరరాశి అక్షరం *మార్గం)
{
	పూర్ణాంకం rc, errsv;
	పూర్ణాంకం fd;

	fd = ul_మార్గం_open(pc, O_WRONLY|O_CLOEXEC, మార్గం);
	ఒకవేళ (fd < 0)
		తిరిగి -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : ఖాళీ, fd, str, అక్షరశ్రేణి_పొడవు(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ఖాళీ, fd);
	errno = errsv;
	తిరిగి rc;
}

పూర్ణాంకం ul_మార్గం_writef_string(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *str, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	స్థిరరాశి అక్షరం *p;
	va_list ap;

	va_start(ap, మార్గం);
	p = ul_మార్గం_mkpath(pc, మార్గం, ap);
	va_end(ap);

	తిరిగి !p ? -errno : ul_మార్గం_write_string(pc, str, p);
}

పూర్ణాంకం ul_మార్గం_write_s64(నిర్మాణం మార్గం_cxt *pc, int64_t అంకె, స్థిరరాశి అక్షరం *మార్గం)
{
	అక్షరం buf[పరిమాణం(stringify_విలువ(LLONG_MAX))];
	పూర్ణాంకం rc, errsv;
	పూర్ణాంకం fd, len;

	fd = ul_మార్గం_open(pc, O_WRONLY|O_CLOEXEC, మార్గం);
	ఒకవేళ (fd < 0)
		తిరిగి -errno;

	len = snprintf(buf, పరిమాణం(buf), "%" PRId64, అంకె);
	ఒకవేళ (len < 0 || (size_t) len >= పరిమాణం(buf))
		rc = len < 0 ? -errno : -E2BIG;
	లేకపోతే
		rc = ul_vfs_write_all(pc ? pc->vfs : ఖాళీ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ఖాళీ, fd);
	errno = errsv;
	తిరిగి rc;
}

పూర్ణాంకం ul_మార్గం_write_u64(నిర్మాణం మార్గం_cxt *pc, uint64_t అంకె, స్థిరరాశి అక్షరం *మార్గం)
{
	అక్షరం buf[పరిమాణం(stringify_విలువ(ULLONG_MAX))];
	పూర్ణాంకం rc, errsv;
	పూర్ణాంకం fd, len;

	fd = ul_మార్గం_open(pc, O_WRONLY|O_CLOEXEC, మార్గం);
	ఒకవేళ (fd < 0)
		తిరిగి -errno;

	len = snprintf(buf, పరిమాణం(buf), "%" PRIu64, అంకె);
	ఒకవేళ (len < 0 || (size_t) len >= పరిమాణం(buf))
		rc = len < 0 ? -errno : -E2BIG;
	లేకపోతే
		rc = ul_vfs_write_all(pc ? pc->vfs : ఖాళీ, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : ఖాళీ, fd);
	errno = errsv;
	తిరిగి rc;
}

పూర్ణాంకం ul_మార్గం_writef_u64(నిర్మాణం మార్గం_cxt *pc, uint64_t అంకె, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	స్థిరరాశి అక్షరం *p;
	va_list ap;

	va_start(ap, మార్గం);
	p = ul_మార్గం_mkpath(pc, మార్గం, ap);
	va_end(ap);

	తిరిగి !p ? -errno : ul_మార్గం_write_u64(pc, అంకె, p);

}

పూర్ణాంకం ul_మార్గం_లెక్క_dirents(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *మార్గం)
{
	DIR *dir;
	పూర్ణాంకం r = 0;

	dir = ul_మార్గం_opendir(pc, మార్గం);
	ఒకవేళ (!dir)
		తిరిగి 0;

	అయితే (xreaddir(dir)) r++;

	closedir(dir);
	తిరిగి r;
}

పూర్ణాంకం ul_మార్గం_countf_dirents(నిర్మాణం మార్గం_cxt *pc, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	స్థిరరాశి అక్షరం *p;
	va_list ap;

	va_start(ap, మార్గం);
	p = ul_మార్గం_mkpath(pc, మార్గం, ap);
	va_end(ap);

	తిరిగి !p ? -errno : ul_మార్గం_లెక్క_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
పూర్ణాంకం ul_మార్గం_తదుపరి_dirent(నిర్మాణం మార్గం_cxt *pc, DIR **sub, స్థిరరాశి అక్షరం *dirname, నిర్మాణం dirent **d)
{
	ఒకవేళ (!pc || !sub || !d)
		తిరిగి -EINVAL;

	ఒకవేళ (!*sub) {
		*sub = ul_మార్గం_opendir(pc, dirname);
		ఒకవేళ (!*sub)
			తిరిగి -errno;
	}

	*d = xreaddir(*sub);
	ఒకవేళ (*d)
		తిరిగి 0;

	closedir(*sub);
	*sub = ఖాళీ;
	తిరిగి 1;
}

#నిర్వచితమైతే HAVE_CPU_సమితి_T
స్థిర పూర్ణాంకం ul_మార్గం_cpuparse(నిర్మాణం మార్గం_cxt *pc, cpu_set_t **సమితి, పూర్ణాంకం maxcpus, పూర్ణాంకం islist, స్థిరరాశి అక్షరం *మార్గం, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	అక్షరం *buf;
	పూర్ణాంకం rc;

	*సమితి = ఖాళీ;

	buf = జ్ఞాపకం_ఇవ్వు(len);
	ఒకవేళ (!buf)
		తిరిగి -ENOMEM;

	rc = ul_మార్గం_vreadf_బఫరు(pc, buf, len, మార్గం, ap);
	ఒకవేళ (rc < 0)
		వెళ్ళు బహిర్గతం;

	*సమితి = cpuset_alloc(maxcpus, &setsize, ఖాళీ);
	ఒకవేళ (!*సమితి) {
		rc = -EINVAL;
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (islist) {
		ఒకవేళ (cpulist_parse(buf, *సమితి, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			వెళ్ళు బహిర్గతం;
		}
	} లేకపోతే {
		ఒకవేళ (cpumask_parse(buf, *సమితి, setsize)) {
			errno = EINVAL;
			rc = -errno;
			వెళ్ళు బహిర్గతం;
		}
	}
	rc = 0;

బహిర్గతం:
	ఒకవేళ (rc) {
		cpuset_free(*సమితి);
		*సమితి = ఖాళీ;
	}
	విడుదల(buf);
	తిరిగి rc;
}

పూర్ణాంకం ul_మార్గం_readf_cpuset(నిర్మాణం మార్గం_cxt *pc, cpu_set_t **సమితి, పూర్ణాంకం maxcpus, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	va_list ap;
	పూర్ణాంకం rc = 0;

	va_start(ap, మార్గం);
	rc = ul_మార్గం_cpuparse(pc, సమితి, maxcpus, 0, మార్గం, ap);
	va_end(ap);

	తిరిగి rc;
}

పూర్ణాంకం ul_మార్గం_readf_cpulist(నిర్మాణం మార్గం_cxt *pc, cpu_set_t **సమితి, పూర్ణాంకం maxcpus, స్థిరరాశి అక్షరం *మార్గం, ...)
{
	va_list ap;
	పూర్ణాంకం rc = 0;

	va_start(ap, మార్గం);
	rc = ul_మార్గం_cpuparse(pc, సమితి, maxcpus, 1, మార్గం, ap);
	va_end(ap);

	తిరిగి rc;
}

#అయితే_ముగింపు /* HAVE_CPU_SET_T */


#నిర్వచితమైతే TEST_కార్యక్రమం_మార్గం
#అన్తర్భావ <getopt.h>

స్థిర శూన్య __attribute__((__noreturn__)) usage(శూన్య)
{
	fprintf(stdout, " %s [options] <dir> <command>\n\n", కార్యక్రమం_invocation_short_name);
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

	నిష్క్రమించు(EXIT_SUCCESS);
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	పూర్ణాంకం c;
	స్థిరరాశి అక్షరం *ఉపసర్గ = ఖాళీ, *dir, *file, *command;
	నిర్మాణం మార్గం_cxt *pc = ఖాళీ;

	స్థిర స్థిరరాశి నిర్మాణం option longopts[] = {
		{ "prefix",	1, ఖాళీ, 'p' },
		{ "help",       0, ఖాళీ, 'h' },
		{ ఖాళీ, 0, ఖాళీ, 0 },
	};

	అయితే((c = getopt_long(argc, argv, "p:h", longopts, ఖాళీ)) != -1) {
		ఎంపిక(c) {
		స్థితి 'p':
			ఉపసర్గ = optarg;
			ఆపు;
		స్థితి 'h':
			usage();
			ఆపు;
		అప్రమేయ:
			err(EXIT_FAILURE, "try --help");
		}
	}

	ఒకవేళ (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_మార్గం_init_debug();

	pc = ul_new_మార్గం("%s", dir);
	ఒకవేళ (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	ఒకవేళ (ఉపసర్గ)
		ul_మార్గం_సమితి_ఉపసర్గ(pc, ఉపసర్గ);

	ఒకవేళ (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	ఒకవేళ (strcmp(command, "read-u32") == 0) {
		uint32_t res;

		ఒకవేళ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ఒకవేళ (ul_మార్గం_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		రాయి("read:  %s: %u\n", file, res);

		ఒకవేళ (ul_మార్గం_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		రాయి("readf: %s: %u\n", file, res);

	} లేకపోతే ఒకవేళ (strcmp(command, "read-s32") == 0) {
		int32_t res;

		ఒకవేళ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ఒకవేళ (ul_మార్గం_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		రాయి("read:  %s: %d\n", file, res);

		ఒకవేళ (ul_మార్గం_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		రాయి("readf: %s: %d\n", file, res);

	} లేకపోతే ఒకవేళ (strcmp(command, "read-u64") == 0) {
		uint64_t res;

		ఒకవేళ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ఒకవేళ (ul_మార్గం_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		రాయి("read:  %s: %" PRIu64 "\n", file, res);

		ఒకవేళ (ul_మార్గం_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		రాయి("readf: %s: %" PRIu64 "\n", file, res);

	} లేకపోతే ఒకవేళ (strcmp(command, "read-s64") == 0) {
		int64_t res;

		ఒకవేళ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ఒకవేళ (ul_మార్గం_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		రాయి("read:  %s: %" PRId64 "\n", file, res);

		ఒకవేళ (ul_మార్గం_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		రాయి("readf: %s: %" PRId64 "\n", file, res);

	} లేకపోతే ఒకవేళ (strcmp(command, "read-majmin") == 0) {
		dev_t res;

		ఒకవేళ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ఒకవేళ (ul_మార్గం_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		రాయి("read:  %s: %d\n", file, (పూర్ణాంకం) res);

		ఒకవేళ (ul_మార్గం_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		రాయి("readf: %s: %d\n", file, (పూర్ణాంకం) res);

	} లేకపోతే ఒకవేళ (strcmp(command, "read-string") == 0) {
		అక్షరం *res;

		ఒకవేళ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ఒకవేళ (ul_మార్గం_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		రాయి("read:  %s: %s\n", file, res);

		ఒకవేళ (ul_మార్గం_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		రాయి("readf: %s: %s\n", file, res);

	} లేకపోతే ఒకవేళ (strcmp(command, "read-link") == 0) {
		అక్షరం res[PATH_MAX];

		ఒకవేళ (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		ఒకవేళ (ul_మార్గం_readlink(pc, res, పరిమాణం(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		రాయి("read:  %s: %s\n", file, res);

		ఒకవేళ (ul_మార్గం_readlinkf(pc, res, పరిమాణం(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		రాయి("readf: %s: %s\n", file, res);

	} లేకపోతే ఒకవేళ (strcmp(command, "write-string") == 0) {
		అక్షరం *str;

		ఒకవేళ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		ఒకవేళ (ul_మార్గం_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		ఒకవేళ (ul_మార్గం_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} లేకపోతే ఒకవేళ (strcmp(command, "write-u64") == 0) {
		uint64_t అంకె;

		ఒకవేళ (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		అంకె = strtoumax(argv[optind++], ఖాళీ, 0);

		ఒకవేళ (ul_మార్గం_write_u64(pc, అంకె, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		ఒకవేళ (ul_మార్గం_writef_u64(pc, అంకె, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_మార్గం(pc);
	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు /* TEST_PROGRAM_PATH */
