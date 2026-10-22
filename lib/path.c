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
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <errno.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "fileutils.h"
#अन्तर्भाव "all-io.h"
#अन्तर्भाव "vfs.h"
#अन्तर्भाव "path.h"
#अन्तर्भाव "debug.h"
#अन्तर्भाव "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
स्थिर UL_DEBUG_DEFINE_आवरणम्(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_रिक्तानि_MASKNAMES;

#परिभाषय ULPATH_DEBUG_INIT	(1 << 1)
#परिभाषय ULPATH_DEBUG_CXT	(1 << 2)

#परिभाषय DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#परिभाषय DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#परिभाषय ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

शून्य ul_मार्गः_init_debug(शून्य)
{
	यदि (ulpath_debug_आवरणम्)
		प्रत्यागम;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

संरचना मार्गः_cxt *ul_new_मार्गः(ध्रुव वर्ण *dir, ...)
{
	संरचना मार्गः_cxt *pc = शून्यस्मृतिं_देहि(1, परिमाण(*pc));

	यदि (!pc)
		प्रत्यागम रिक्त;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	यदि (dir) {
		पूर्णाङ्क rc;
		va_list ap;

		va_start(ap, dir);
		rc = vasprintf(&pc->dir_मार्गः, dir, ap);
		va_end(ap);

		यदि (rc < 0 || !pc->dir_मार्गः)
			गच्छ fail;
	}
	प्रत्यागम pc;
fail:
	ul_unref_मार्गः(pc);
	प्रत्यागम रिक्त;
}

शून्य ul_ref_मार्गः(संरचना मार्गः_cxt *pc)
{
	यदि (pc)
		pc->refcount++;
}

शून्य ul_unref_मार्गः(संरचना मार्गः_cxt *pc)
{
	यदि (!pc)
		प्रत्यागम;

	pc->refcount--;

	यदि (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		यदि (pc->dialect)
			pc->free_dialect(pc);
		ul_मार्गः_close_dirfd(pc);
		मुक्त(pc->dir_मार्गः);
		मुक्त(pc->उपसर्गः);
		मुक्त(pc);
	}
}

शून्य ul_मार्गः_refer_vfs(संरचना मार्गः_cxt *pc, ध्रुव संरचना ul_vfs_ops *vfs)
{
	यदि (pc)
		pc->vfs = vfs;
}

पूर्णाङ्क ul_मार्गः_समुच्चयः_उपसर्गः(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *उपसर्गः)
{
	वर्ण *p = रिक्त;

	प्रतिज्ञा(pc->dir_fd < 0);

	यदि (उपसर्गः) {
		p = strdup(उपसर्गः);
		यदि (!p)
			प्रत्यागम -ENOMEM;
	}

	मुक्त(pc->उपसर्गः);
	pc->उपसर्गः = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	प्रत्यागम 0;
}

ध्रुव वर्ण *ul_मार्गः_get_उपसर्गः(संरचना मार्गः_cxt *pc)
{
	प्रत्यागम pc ? pc->उपसर्गः : रिक्त;
}

पूर्णाङ्क ul_मार्गः_समुच्चयः_dir(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *dir)
{
	वर्ण *p = रिक्त;

	यदि (dir) {
		p = strdup(dir);
		यदि (!p)
			प्रत्यागम -ENOMEM;
	}

	यदि (pc->dir_fd >= 0) {
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}

	मुक्त(pc->dir_मार्गः);
	pc->dir_मार्गः = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	प्रत्यागम 0;
}

ध्रुव वर्ण *ul_मार्गः_get_dir(संरचना मार्गः_cxt *pc)
{
	प्रत्यागम pc ? pc->dir_मार्गः : रिक्त;
}

पूर्णाङ्क ul_मार्गः_समुच्चयः_dialect(संरचना मार्गः_cxt *pc, शून्य *दत्त, शून्य free_दत्त(संरचना मार्गः_cxt *))
{
	pc->dialect = दत्त;
	pc->free_dialect = free_दत्त;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	प्रत्यागम 0;
}

शून्य *ul_मार्गः_get_dialect(संरचना मार्गः_cxt *pc)
{
	प्रत्यागम pc ? pc->dialect : रिक्त;
}

पूर्णाङ्क ul_मार्गः_समुच्चयः_enoent_redirect(संरचना मार्गः_cxt *pc, पूर्णाङ्क (*func)(संरचना मार्गः_cxt *, ध्रुव वर्ण *, पूर्णाङ्क *))
{
	pc->redirect_on_enoent = func;
	प्रत्यागम 0;
}

स्थिर ध्रुव वर्ण *get_absdir(संरचना मार्गः_cxt *pc)
{
	पूर्णाङ्क rc;
	ध्रुव वर्ण *dirpath;

	यदि (!pc->उपसर्गः)
		प्रत्यागम pc->dir_मार्गः;

	dirpath = pc->dir_मार्गः;
	यदि (!dirpath)
		प्रत्यागम pc->उपसर्गः;
	यदि (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->मार्गः_बफर, परिमाण(pc->मार्गः_बफर), "%s/%s", pc->उपसर्गः, dirpath);
	यदि (rc < 0)
		प्रत्यागम रिक्त;
	यदि ((size_t)rc >= परिमाण(pc->मार्गः_बफर)) {
		errno = ENAMETOOLONG;
		प्रत्यागम रिक्त;
	}

	प्रत्यागम pc->मार्गः_बफर;
}

पूर्णाङ्क ul_मार्गः_is_accessible(संरचना मार्गः_cxt *pc)
{
	ध्रुव वर्ण *मार्गः;
	प्रतिज्ञा(pc);

	यदि (pc->dir_fd >= 0)
		प्रत्यागम 1;

	मार्गः = get_absdir(pc);
	यदि (!मार्गः)
		प्रत्यागम 0;
	प्रत्यागम access(मार्गः, F_OK) == 0;
}

पूर्णाङ्क ul_मार्गः_get_dirfd(संरचना मार्गः_cxt *pc)
{
	प्रतिज्ञा(pc);
	प्रतिज्ञा(pc->dir_मार्गः);

	यदि (pc->dir_fd < 0) {
		ध्रुव वर्ण *मार्गः = get_absdir(pc);
		यदि (!मार्गः)
			प्रत्यागम -errno;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", मार्गः));
		pc->dir_fd = उद्घाटय(मार्गः, O_RDONLY|O_CLOEXEC);
	}

	प्रत्यागम pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
शून्य ul_मार्गः_close_dirfd(संरचना मार्गः_cxt *pc)
{
	प्रतिज्ञा(pc);

	यदि (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		close(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

पूर्णाङ्क ul_मार्गः_isopen_dirfd(संरचना मार्गः_cxt *pc)
{
	प्रत्यागम pc && pc->dir_fd >= 0;
}

स्थिर ध्रुव वर्ण *ul_मार्गः_mkpath(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *मार्गः, va_list ap)
{
	पूर्णाङ्क rc;

	errno = 0;

	rc = vsnprintf(pc->मार्गः_बफर, परिमाण(pc->मार्गः_बफर), मार्गः, ap);
	यदि (rc < 0) {
		यदि (!errno)
			errno = EINVAL;
		प्रत्यागम रिक्त;
	}

	यदि ((size_t)rc >= परिमाण(pc->मार्गः_बफर)) {
		errno = ENAMETOOLONG;
		प्रत्यागम रिक्त;
	}

	प्रत्यागम pc->मार्गः_बफर;
}

वर्ण *ul_मार्गः_get_abspath(संरचना मार्गः_cxt *pc, वर्ण *buf, size_t bufsz, ध्रुव वर्ण *मार्गः, ...)
{
	यदि (मार्गः) {
		पूर्णाङ्क rc;
		va_list ap;
		ध्रुव वर्ण *पुच्छ = रिक्त, *dirpath = pc->dir_मार्गः;

		va_start(ap, मार्गः);
		पुच्छ = ul_मार्गः_mkpath(pc, मार्गः, ap);
		va_end(ap);

		यदि (dirpath && *dirpath == '/')
			dirpath++;
		यदि (पुच्छ && *पुच्छ == '/')
			पुच्छ++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->उपसर्गः ? pc->उपसर्गः : "",
				dirpath ? dirpath : "",
				पुच्छ ? पुच्छ : "");

		यदि ((size_t)rc >= bufsz) {
			errno = ENAMETOOLONG;
			प्रत्यागम रिक्त;
		}
	} अन्यथा {
		ध्रुव वर्ण *tmp = get_absdir(pc);

		यदि (!tmp)
			प्रत्यागम रिक्त;
		xstrncpy(buf, tmp, bufsz);
	}

	प्रत्यागम buf;
}


पूर्णाङ्क ul_मार्गः_access(संरचना मार्गः_cxt *pc, पूर्णाङ्क बहुलकम्, ध्रुव वर्ण *मार्गः)
{
	पूर्णाङ्क rc;

	यदि (!मार्गः)
		प्रत्यागम -EINVAL;
	यदि (!pc) {
		rc = access(मार्गः, बहुलकम्);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", मार्गः, rc));
	} अन्यथा {
		पूर्णाङ्क dir = ul_मार्गः_get_dirfd(pc);
		यदि (dir < 0)
			प्रत्यागम dir;
		यदि (*मार्गः == '/')
			मार्गः++;

		rc = faccessat(dir, मार्गः, बहुलकम्, 0);

		यदि (rc && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, मार्गः, &dir) == 0)
			rc = faccessat(dir, मार्गः, बहुलकम्, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", मार्गः, rc));
	}
	प्रत्यागम rc;
}

पूर्णाङ्क ul_मार्गः_accessf(संरचना मार्गः_cxt *pc, पूर्णाङ्क बहुलकम्, ध्रुव वर्ण *मार्गः, ...)
{
	va_list ap;
	ध्रुव वर्ण *p;

	va_start(ap, मार्गः);
	p = ul_मार्गः_mkpath(pc, मार्गः, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_access(pc, बहुलकम्, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
पूर्णाङ्क ul_मार्गः_stat(संरचना मार्गः_cxt *pc, संरचना stat *sb, पूर्णाङ्क flags, ध्रुव वर्ण *मार्गः)
{
	पूर्णाङ्क rc;

	यदि (!pc) {
		rc = मार्गः ? stat(मार्गः, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", मार्गः, rc));
	} अन्यथा {
		पूर्णाङ्क dir = ul_मार्गः_get_dirfd(pc);
		यदि (dir < 0)
			प्रत्यागम dir;
		यदि (मार्गः) {
			यदि  (*मार्गः == '/')
				मार्गः++;
			rc = fstatat(dir, मार्गः, sb, flags);

		} अन्यथा
			rc = fstat(dir, sb);	/* dir itself */

		यदि (rc && errno == ENOENT
		    && मार्गः
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, मार्गः, &dir) == 0)
			rc = fstatat(dir, मार्गः, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", मार्गः, rc));
	}
	प्रत्यागम rc;
}

पूर्णाङ्क ul_मार्गः_vstatf(संरचना मार्गः_cxt *pc, संरचना stat *sb, पूर्णाङ्क flags, ध्रुव वर्ण *मार्गः, va_list ap)
{
	ध्रुव वर्ण *p = ul_मार्गः_mkpath(pc, मार्गः, ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_stat(pc, sb, flags, p);
}

पूर्णाङ्क ul_मार्गः_statf(संरचना मार्गः_cxt *pc, संरचना stat *sb, पूर्णाङ्क flags, ध्रुव वर्ण *मार्गः, ...)
{
	va_list ap;
	पूर्णाङ्क rc;

	va_start(ap, मार्गः);
	rc = ul_मार्गः_vstatf(pc, sb, flags, मार्गः, ap);
	va_end(ap);

	प्रत्यागम rc;
}
#यदि_परिभाषितम् HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
पूर्णाङ्क ul_मार्गः_statx(संरचना मार्गः_cxt *pc, संरचना statx *stx, पूर्णाङ्क flags, अचिह्नित पूर्णाङ्क आवरणम्,
		  ध्रुव वर्ण *मार्गः)
{
	पूर्णाङ्क rc;

	यदि (!मार्गः)
		प्रत्यागम -EINVAL;
	यदि (!pc)
		rc = मार्गः ? statx(AT_FDCWD, मार्गः, flags, आवरणम्, stx) : - EINVAL;
	अन्यथा {
		पूर्णाङ्क dir = ul_मार्गः_get_dirfd(pc);
		यदि (dir < 0)
			प्रत्यागम dir;
		यदि (*मार्गः == '/')
			मार्गः++;

		rc = statx(dir, मार्गः, flags, आवरणम्, stx);
	}

	प्रत्यागम rc;
}
#अन्यथा
पूर्णाङ्क ul_मार्गः_statx(संरचना मार्गः_cxt *pc __attribute__((__unused__)),
		  संरचना statx *stx __attribute__((__unused__)),
		  पूर्णाङ्क flags __attribute__((__unused__)),
		  अचिह्नित पूर्णाङ्क आवरणम् __attribute__((__unused__)),
		  ध्रुव वर्ण *मार्गः __attribute__((__unused__)))
{
	errno = ENOSYS;
	प्रत्यागम -1;
}
#यदि_अन्तः /* HAVE_STATX */

पूर्णाङ्क ul_मार्गः_vstatxf(संरचना मार्गः_cxt *pc, संरचना statx *stx, पूर्णाङ्क flags, अचिह्नित पूर्णाङ्क आवरणम्,
		    ध्रुव वर्ण *मार्गः, va_list ap)
{
	ध्रुव वर्ण *p = ul_मार्गः_mkpath(pc, मार्गः, ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_statx(pc, stx, flags, आवरणम्, p);
}

पूर्णाङ्क ul_मार्गः_statxf(संरचना मार्गः_cxt *pc, संरचना statx *stx, पूर्णाङ्क flags, अचिह्नित पूर्णाङ्क आवरणम्,
		   ध्रुव वर्ण *मार्गः, ...)
{
	va_list ap;
	पूर्णाङ्क rc;

	va_start(ap, मार्गः);
	rc = ul_मार्गः_vstatxf(pc, stx, flags, आवरणम्, मार्गः, ap);
	va_end(ap);

	प्रत्यागम rc;
}

पूर्णाङ्क ul_मार्गः_open(संरचना मार्गः_cxt *pc, पूर्णाङ्क flags, ध्रुव वर्ण *मार्गः)
{
	पूर्णाङ्क fd;

	यदि (!मार्गः)
		प्रत्यागम -EINVAL;
	यदि (!pc) {
		fd = उद्घाटय(मार्गः, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", मार्गः));
	} अन्यथा {
		पूर्णाङ्क fdx;
		पूर्णाङ्क dir = ul_मार्गः_get_dirfd(pc);
		यदि (dir < 0)
			प्रत्यागम dir;

		यदि (*मार्गः == '/')
			मार्गः++;

		fdx = fd = openat(dir, मार्गः, flags);

		यदि (fd < 0 && errno == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, मार्गः, &dir) == 0)
			fd = openat(dir, मार्गः, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", मार्गः, fdx != fd ? " [redirected]" : ""));
	}
	प्रत्यागम fd;
}

पूर्णाङ्क ul_मार्गः_vopenf(संरचना मार्गः_cxt *pc, पूर्णाङ्क flags, ध्रुव वर्ण *मार्गः, va_list ap)
{
	ध्रुव वर्ण *p = ul_मार्गः_mkpath(pc, मार्गः, ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_open(pc, flags, p);
}

पूर्णाङ्क ul_मार्गः_openf(संरचना मार्गः_cxt *pc, पूर्णाङ्क flags, ध्रुव वर्ण *मार्गः, ...)
{
	va_list ap;
	पूर्णाङ्क rc;

	va_start(ap, मार्गः);
	rc = ul_मार्गः_vopenf(pc, flags, मार्गः, ap);
	va_end(ap);

	प्रत्यागम rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
FILE *ul_मार्गः_fopen(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *बहुलकम्, ध्रुव वर्ण *मार्गः)
{
	पूर्णाङ्क flags = ul_बहुलकम्_गन्तव्यम्_flags(बहुलकम्);
	पूर्णाङ्क fd = ul_मार्गः_open(pc, flags, मार्गः);

	यदि (fd < 0)
		प्रत्यागम रिक्त;

	प्रत्यागम ul_vfs_fdopen(pc ? pc->vfs : रिक्त, fd, बहुलकम्);
}


FILE *ul_मार्गः_vfopenf(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *बहुलकम्, ध्रुव वर्ण *मार्गः, va_list ap)
{
	ध्रुव वर्ण *p = ul_मार्गः_mkpath(pc, मार्गः, ap);

	प्रत्यागम !p ? रिक्त : ul_मार्गः_fopen(pc, बहुलकम्, p);
}

FILE *ul_मार्गः_fopenf(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *बहुलकम्, ध्रुव वर्ण *मार्गः, ...)
{
	FILE *f;
	va_list ap;

	va_start(ap, मार्गः);
	f = ul_मार्गः_vfopenf(pc, बहुलकम्, मार्गः, ap);
	va_end(ap);

	प्रत्यागम f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_मार्गः_opendir(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *मार्गः)
{
	DIR *dir;
	पूर्णाङ्क fd = -1;

	यदि (मार्गः)
		fd = ul_मार्गः_open(pc, O_RDONLY|O_CLOEXEC, मार्गः);
	अन्यथा यदि (pc->dir_मार्गः) {
		पूर्णाङ्क dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_मार्गः_get_dirfd(pc);
		यदि (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	यदि (fd < 0)
		प्रत्यागम रिक्त;

	dir = fdopendir(fd);
	यदि (!dir) {
		close(fd);
		प्रत्यागम रिक्त;
	}
	यदि (!मार्गः)
		 rewinddir(dir);
	प्रत्यागम dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_मार्गः_vopendirf(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *मार्गः, va_list ap)
{
	ध्रुव वर्ण *p = ul_मार्गः_mkpath(pc, मार्गः, ap);

	प्रत्यागम !p ? रिक्त : ul_मार्गः_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
DIR *ul_मार्गः_opendirf(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *मार्गः, ...)
{
	va_list ap;
	DIR *dir;

	va_start(ap, मार्गः);
	dir = ul_मार्गः_vopendirf(pc, मार्गः, ap);
	va_end(ap);

	प्रत्यागम dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_मार्गः_readlink(संरचना मार्गः_cxt *pc, वर्ण *buf, size_t bufsiz, ध्रुव वर्ण *मार्गः)
{
	पूर्णाङ्क dirfd;
	ssize_t ssz;

	यदि (!मार्गः) {
		ध्रुव वर्ण *p = get_absdir(pc);
		यदि (!p)
			प्रत्यागम -errno;
		ssz = readlink(p, buf, bufsiz - 1);
	} अन्यथा {
		dirfd = ul_मार्गः_get_dirfd(pc);
		यदि (dirfd < 0)
			प्रत्यागम dirfd;

		यदि (*मार्गः == '/')
			मार्गः++;

		ssz = readlinkat(dirfd, मार्गः, buf, bufsiz - 1);
	}

	यदि (ssz >= 0)
		buf[ssz] = '\0';
	प्रत्यागम ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_मार्गः_readlinkf(संरचना मार्गः_cxt *pc, वर्ण *buf, size_t bufsiz, ध्रुव वर्ण *मार्गः, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, मार्गः);
	p = ul_मार्गः_mkpath(pc, मार्गः, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_readlink(pc, buf, bufsiz, p);
}

पूर्णाङ्क ul_मार्गः_read(संरचना मार्गः_cxt *pc, वर्ण *buf, size_t len, ध्रुव वर्ण *मार्गः)
{
	पूर्णाङ्क rc, errsv;
	पूर्णाङ्क fd;

	fd = ul_मार्गः_open(pc, O_RDONLY|O_CLOEXEC, मार्गः);
	यदि (fd < 0)
		प्रत्यागम -errno;

	DBG(CXT, ul_debug(" reading '%s'", मार्गः));
	rc = ul_vfs_read_all(pc ? pc->vfs : रिक्त, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
	errno = errsv;
	प्रत्यागम rc;
}

पूर्णाङ्क ul_मार्गः_vreadf(संरचना मार्गः_cxt *pc, वर्ण *buf, size_t len, ध्रुव वर्ण *मार्गः, va_list ap)
{
	ध्रुव वर्ण *p = ul_मार्गः_mkpath(pc, मार्गः, ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_read(pc, buf, len, p);
}

पूर्णाङ्क ul_मार्गः_readf(संरचना मार्गः_cxt *pc, वर्ण *buf, size_t len, ध्रुव वर्ण *मार्गः, ...)
{
	va_list ap;
	पूर्णाङ्क rc;

	va_start(ap, मार्गः);
	rc = ul_मार्गः_vreadf(pc, buf, len, मार्गः, ap);
	va_end(ap);

	प्रत्यागम rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
पूर्णाङ्क ul_मार्गः_read_string(संरचना मार्गः_cxt *pc, वर्ण **str, ध्रुव वर्ण *मार्गः)
{
	वर्ण buf[BUFSIZ];
	पूर्णाङ्क rc;

	यदि (!str)
		प्रत्यागम -EINVAL;

	*str = रिक्त;

	rc = ul_मार्गः_read_बफर(pc, buf, परिमाण(buf), मार्गः);
	यदि (rc < 0)
		प्रत्यागम rc;

	*str = strdup(buf);
	यदि (!*str)
		rc = -ENOMEM;

	प्रत्यागम rc;
}

पूर्णाङ्क ul_मार्गः_readf_string(संरचना मार्गः_cxt *pc, वर्ण **str, ध्रुव वर्ण *मार्गः, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, मार्गः);
	p = ul_मार्गः_mkpath(pc, मार्गः, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_read_string(pc, str, p);
}

पूर्णाङ्क ul_मार्गः_read_बफर(संरचना मार्गः_cxt *pc, वर्ण *buf, size_t bufsz, ध्रुव वर्ण *मार्गः)
{
	पूर्णाङ्क rc = ul_मार्गः_read(pc, buf, bufsz - 1, मार्गः);

	यदि (rc == 0)
		buf[0] = '\0';

	अन्यथा यदि (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		यदि (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		अन्यथा
			buf[rc] = '\0';
	}

	प्रत्यागम rc;
}

पूर्णाङ्क ul_मार्गः_vreadf_बफर(संरचना मार्गः_cxt *pc, वर्ण *buf, size_t bufsz, ध्रुव वर्ण *मार्गः, va_list ap)
{
	ध्रुव वर्ण *p;

	p = ul_मार्गः_mkpath(pc, मार्गः, ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_read_बफर(pc, buf, bufsz, p);
}

पूर्णाङ्क ul_मार्गः_readf_बफर(संरचना मार्गः_cxt *pc, वर्ण *buf, size_t bufsz, ध्रुव वर्ण *मार्गः, ...)
{
	va_list ap;
	पूर्णाङ्क rc;

	va_start(ap, मार्गः);
	rc = ul_मार्गः_vreadf_बफर(pc, buf, bufsz, मार्गः, ap);
	va_end(ap);

	प्रत्यागम rc;
}

पूर्णाङ्क ul_मार्गः_scanf(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *मार्गः, ध्रुव वर्ण *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	पूर्णाङ्क rc;

	f = ul_मार्गः_fopen(pc, "r" UL_CLOEXECSTR, मार्गः);
	यदि (!f)
		प्रत्यागम -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, मार्गः));

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	प्रत्यागम rc;
}

पूर्णाङ्क ul_मार्गः_scanff(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *मार्गः, va_list ap, ध्रुव वर्ण *fmt, ...)
{
	FILE *f;
	va_list fmt_ap;
	पूर्णाङ्क rc;

	f = ul_मार्गः_vfopenf(pc, "r" UL_CLOEXECSTR, मार्गः, ap);
	यदि (!f)
		प्रत्यागम -EINVAL;

	va_start(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	va_end(fmt_ap);

	fclose(f);
	प्रत्यागम rc;
}


पूर्णाङ्क ul_मार्गः_read_s64(संरचना मार्गः_cxt *pc, int64_t *res, ध्रुव वर्ण *मार्गः)
{
	int64_t x = 0;
	पूर्णाङ्क rc;

	rc = ul_मार्गः_scanf(pc, मार्गः, "%"SCNd64, &x);
	यदि (rc != 1)
		प्रत्यागम -1;
	यदि (res)
		*res = x;
	प्रत्यागम 0;
}

पूर्णाङ्क ul_मार्गः_readf_s64(संरचना मार्गः_cxt *pc, int64_t *res, ध्रुव वर्ण *मार्गः, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, मार्गः);
	p = ul_मार्गः_mkpath(pc, मार्गः, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_read_s64(pc, res, p);
}

पूर्णाङ्क ul_मार्गः_read_u64(संरचना मार्गः_cxt *pc, uint64_t *res, ध्रुव वर्ण *मार्गः)
{
	uint64_t x = 0;
	पूर्णाङ्क rc;

	rc = ul_मार्गः_scanf(pc, मार्गः, "%"SCNu64, &x);
	यदि (rc != 1)
		प्रत्यागम -1;
	यदि (res)
		*res = x;
	प्रत्यागम 0;
}

पूर्णाङ्क ul_मार्गः_readf_u64(संरचना मार्गः_cxt *pc, uint64_t *res, ध्रुव वर्ण *मार्गः, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, मार्गः);
	p = ul_मार्गः_mkpath(pc, मार्गः, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_read_u64(pc, res, p);
}

पूर्णाङ्क ul_मार्गः_read_s32(संरचना मार्गः_cxt *pc, पूर्णाङ्क *res, ध्रुव वर्ण *मार्गः)
{
	पूर्णाङ्क rc, x = 0;

	rc = ul_मार्गः_scanf(pc, मार्गः, "%d", &x);
	यदि (rc != 1)
		प्रत्यागम -1;
	यदि (res)
		*res = x;
	प्रत्यागम 0;
}

पूर्णाङ्क ul_मार्गः_readf_s32(संरचना मार्गः_cxt *pc, पूर्णाङ्क *res, ध्रुव वर्ण *मार्गः, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, मार्गः);
	p = ul_मार्गः_mkpath(pc, मार्गः, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_read_s32(pc, res, p);
}

पूर्णाङ्क ul_मार्गः_read_u32(संरचना मार्गः_cxt *pc, अचिह्नित पूर्णाङ्क *res, ध्रुव वर्ण *मार्गः)
{
	पूर्णाङ्क rc;
	अचिह्नित पूर्णाङ्क x = 0;

	rc = ul_मार्गः_scanf(pc, मार्गः, "%u", &x);
	यदि (rc != 1)
		प्रत्यागम -1;
	यदि (res)
		*res = x;
	प्रत्यागम 0;
}

पूर्णाङ्क ul_मार्गः_readf_u32(संरचना मार्गः_cxt *pc, अचिह्नित पूर्णाङ्क *res, ध्रुव वर्ण *मार्गः, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, मार्गः);
	p = ul_मार्गः_mkpath(pc, मार्गः, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_read_u32(pc, res, p);
}

पूर्णाङ्क ul_मार्गः_read_majmin(संरचना मार्गः_cxt *pc, dev_t *res, ध्रुव वर्ण *मार्गः)
{
	पूर्णाङ्क rc, maj = 0, min = 0;

	rc = ul_मार्गः_scanf(pc, मार्गः, "%d:%d", &maj, &min);
	यदि (rc != 2)
		प्रत्यागम -1;
	यदि (res)
		*res = makedev(maj, min);
	प्रत्यागम 0;
}

पूर्णाङ्क ul_मार्गः_readf_majmin(संरचना मार्गः_cxt *pc, dev_t *res, ध्रुव वर्ण *मार्गः, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, मार्गः);
	p = ul_मार्गः_mkpath(pc, मार्गः, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_read_majmin(pc, res, p);
}

पूर्णाङ्क ul_मार्गः_write_string(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *str, ध्रुव वर्ण *मार्गः)
{
	पूर्णाङ्क rc, errsv;
	पूर्णाङ्क fd;

	fd = ul_मार्गः_open(pc, O_WRONLY|O_CLOEXEC, मार्गः);
	यदि (fd < 0)
		प्रत्यागम -errno;

	rc = ul_vfs_write_all(pc ? pc->vfs : रिक्त, fd, str, वाक्यदैर्घ्यम्(str));

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
	errno = errsv;
	प्रत्यागम rc;
}

पूर्णाङ्क ul_मार्गः_writef_string(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *str, ध्रुव वर्ण *मार्गः, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, मार्गः);
	p = ul_मार्गः_mkpath(pc, मार्गः, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_write_string(pc, str, p);
}

पूर्णाङ्क ul_मार्गः_write_s64(संरचना मार्गः_cxt *pc, int64_t अङ्कः, ध्रुव वर्ण *मार्गः)
{
	वर्ण buf[परिमाण(stringify_मानम्(LLONG_MAX))];
	पूर्णाङ्क rc, errsv;
	पूर्णाङ्क fd, len;

	fd = ul_मार्गः_open(pc, O_WRONLY|O_CLOEXEC, मार्गः);
	यदि (fd < 0)
		प्रत्यागम -errno;

	len = snprintf(buf, परिमाण(buf), "%" PRId64, अङ्कः);
	यदि (len < 0 || (size_t) len >= परिमाण(buf))
		rc = len < 0 ? -errno : -E2BIG;
	अन्यथा
		rc = ul_vfs_write_all(pc ? pc->vfs : रिक्त, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
	errno = errsv;
	प्रत्यागम rc;
}

पूर्णाङ्क ul_मार्गः_write_u64(संरचना मार्गः_cxt *pc, uint64_t अङ्कः, ध्रुव वर्ण *मार्गः)
{
	वर्ण buf[परिमाण(stringify_मानम्(ULLONG_MAX))];
	पूर्णाङ्क rc, errsv;
	पूर्णाङ्क fd, len;

	fd = ul_मार्गः_open(pc, O_WRONLY|O_CLOEXEC, मार्गः);
	यदि (fd < 0)
		प्रत्यागम -errno;

	len = snprintf(buf, परिमाण(buf), "%" PRIu64, अङ्कः);
	यदि (len < 0 || (size_t) len >= परिमाण(buf))
		rc = len < 0 ? -errno : -E2BIG;
	अन्यथा
		rc = ul_vfs_write_all(pc ? pc->vfs : रिक्त, fd, buf, len);

	errsv = errno;
	ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
	errno = errsv;
	प्रत्यागम rc;
}

पूर्णाङ्क ul_मार्गः_writef_u64(संरचना मार्गः_cxt *pc, uint64_t अङ्कः, ध्रुव वर्ण *मार्गः, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, मार्गः);
	p = ul_मार्गः_mkpath(pc, मार्गः, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_write_u64(pc, अङ्कः, p);

}

पूर्णाङ्क ul_मार्गः_गणनम्_dirents(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *मार्गः)
{
	DIR *dir;
	पूर्णाङ्क r = 0;

	dir = ul_मार्गः_opendir(pc, मार्गः);
	यदि (!dir)
		प्रत्यागम 0;

	यावत् (xreaddir(dir)) r++;

	closedir(dir);
	प्रत्यागम r;
}

पूर्णाङ्क ul_मार्गः_countf_dirents(संरचना मार्गः_cxt *pc, ध्रुव वर्ण *मार्गः, ...)
{
	ध्रुव वर्ण *p;
	va_list ap;

	va_start(ap, मार्गः);
	p = ul_मार्गः_mkpath(pc, मार्गः, ap);
	va_end(ap);

	प्रत्यागम !p ? -errno : ul_मार्गः_गणनम्_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
पूर्णाङ्क ul_मार्गः_अग्र_dirent(संरचना मार्गः_cxt *pc, DIR **sub, ध्रुव वर्ण *dirname, संरचना dirent **d)
{
	यदि (!pc || !sub || !d)
		प्रत्यागम -EINVAL;

	यदि (!*sub) {
		*sub = ul_मार्गः_opendir(pc, dirname);
		यदि (!*sub)
			प्रत्यागम -errno;
	}

	*d = xreaddir(*sub);
	यदि (*d)
		प्रत्यागम 0;

	closedir(*sub);
	*sub = रिक्त;
	प्रत्यागम 1;
}

#यदि_परिभाषितम् HAVE_CPU_समुच्चयः_T
स्थिर पूर्णाङ्क ul_मार्गः_cpuparse(संरचना मार्गः_cxt *pc, cpu_set_t **समुच्चयः, पूर्णाङ्क maxcpus, पूर्णाङ्क islist, ध्रुव वर्ण *मार्गः, va_list ap)
{
	size_t setsize, len = maxcpus * 7;
	वर्ण *buf;
	पूर्णाङ्क rc;

	*समुच्चयः = रिक्त;

	buf = स्मृतिं_देहि(len);
	यदि (!buf)
		प्रत्यागम -ENOMEM;

	rc = ul_मार्गः_vreadf_बफर(pc, buf, len, मार्गः, ap);
	यदि (rc < 0)
		गच्छ बहिर्गमनम्;

	*समुच्चयः = cpuset_alloc(maxcpus, &setsize, रिक्त);
	यदि (!*समुच्चयः) {
		rc = -EINVAL;
		गच्छ बहिर्गमनम्;
	}

	यदि (islist) {
		यदि (cpulist_parse(buf, *समुच्चयः, setsize, 0)) {
			errno = EINVAL;
			rc = -errno;
			गच्छ बहिर्गमनम्;
		}
	} अन्यथा {
		यदि (cpumask_parse(buf, *समुच्चयः, setsize)) {
			errno = EINVAL;
			rc = -errno;
			गच्छ बहिर्गमनम्;
		}
	}
	rc = 0;

बहिर्गमनम्:
	यदि (rc) {
		cpuset_free(*समुच्चयः);
		*समुच्चयः = रिक्त;
	}
	मुक्त(buf);
	प्रत्यागम rc;
}

पूर्णाङ्क ul_मार्गः_readf_cpuset(संरचना मार्गः_cxt *pc, cpu_set_t **समुच्चयः, पूर्णाङ्क maxcpus, ध्रुव वर्ण *मार्गः, ...)
{
	va_list ap;
	पूर्णाङ्क rc = 0;

	va_start(ap, मार्गः);
	rc = ul_मार्गः_cpuparse(pc, समुच्चयः, maxcpus, 0, मार्गः, ap);
	va_end(ap);

	प्रत्यागम rc;
}

पूर्णाङ्क ul_मार्गः_readf_cpulist(संरचना मार्गः_cxt *pc, cpu_set_t **समुच्चयः, पूर्णाङ्क maxcpus, ध्रुव वर्ण *मार्गः, ...)
{
	va_list ap;
	पूर्णाङ्क rc = 0;

	va_start(ap, मार्गः);
	rc = ul_मार्गः_cpuparse(pc, समुच्चयः, maxcpus, 1, मार्गः, ap);
	va_end(ap);

	प्रत्यागम rc;
}

#यदि_अन्तः /* HAVE_CPU_SET_T */


#यदि_परिभाषितम् TEST_क्रमादेशः_मार्गः
#अन्तर्भाव <getopt.h>

स्थिर शून्य __attribute__((__noreturn__)) usage(शून्य)
{
	fprintf(stdout, " %s [options] <dir> <command>\n\n", क्रमादेशः_invocation_short_name);
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

	निर्गच्छ(EXIT_SUCCESS);
}

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[])
{
	पूर्णाङ्क c;
	ध्रुव वर्ण *उपसर्गः = रिक्त, *dir, *file, *command;
	संरचना मार्गः_cxt *pc = रिक्त;

	स्थिर ध्रुव संरचना option longopts[] = {
		{ "prefix",	1, रिक्त, 'p' },
		{ "help",       0, रिक्त, 'h' },
		{ रिक्त, 0, रिक्त, 0 },
	};

	यावत्((c = getopt_long(argc, argv, "p:h", longopts, रिक्त)) != -1) {
		विकल्प(c) {
		पक्ष 'p':
			उपसर्गः = optarg;
			भङ्ग;
		पक्ष 'h':
			usage();
			भङ्ग;
		स्वभाव:
			err(EXIT_FAILURE, "try --help");
		}
	}

	यदि (optind == argc)
		errx(EXIT_FAILURE, "<dir> not defined");
	dir = argv[optind++];

	ul_मार्गः_init_debug();

	pc = ul_new_मार्गः("%s", dir);
	यदि (!pc)
		err(EXIT_FAILURE, "failed to initialize path context");
	यदि (उपसर्गः)
		ul_मार्गः_समुच्चयः_उपसर्गः(pc, उपसर्गः);

	यदि (optind == argc)
		errx(EXIT_FAILURE, "<command> not defined");
	command = argv[optind++];

	यदि (वाक्यं_तुलय(command, "read-u32") == 0) {
		uint32_t res;

		यदि (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		यदि (ul_मार्गः_read_u32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		प्रारूप_लिख("read:  %s: %u\n", file, res);

		यदि (ul_मार्गः_readf_u32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		प्रारूप_लिख("readf: %s: %u\n", file, res);

	} अन्यथा यदि (वाक्यं_तुलय(command, "read-s32") == 0) {
		int32_t res;

		यदि (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		यदि (ul_मार्गः_read_s32(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		प्रारूप_लिख("read:  %s: %d\n", file, res);

		यदि (ul_मार्गः_readf_s32(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		प्रारूप_लिख("readf: %s: %d\n", file, res);

	} अन्यथा यदि (वाक्यं_तुलय(command, "read-u64") == 0) {
		uint64_t res;

		यदि (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		यदि (ul_मार्गः_read_u64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		प्रारूप_लिख("read:  %s: %" PRIu64 "\n", file, res);

		यदि (ul_मार्गः_readf_u64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		प्रारूप_लिख("readf: %s: %" PRIu64 "\n", file, res);

	} अन्यथा यदि (वाक्यं_तुलय(command, "read-s64") == 0) {
		int64_t res;

		यदि (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		यदि (ul_मार्गः_read_s64(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read u64 failed");
		प्रारूप_लिख("read:  %s: %" PRId64 "\n", file, res);

		यदि (ul_मार्गः_readf_s64(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf u64 failed");
		प्रारूप_लिख("readf: %s: %" PRId64 "\n", file, res);

	} अन्यथा यदि (वाक्यं_तुलय(command, "read-majmin") == 0) {
		dev_t res;

		यदि (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		यदि (ul_मार्गः_read_majmin(pc, &res, file) != 0)
			err(EXIT_FAILURE, "read maj:min failed");
		प्रारूप_लिख("read:  %s: %d\n", file, (पूर्णाङ्क) res);

		यदि (ul_मार्गः_readf_majmin(pc, &res, "%s", file) != 0)
			err(EXIT_FAILURE, "readf maj:min failed");
		प्रारूप_लिख("readf: %s: %d\n", file, (पूर्णाङ्क) res);

	} अन्यथा यदि (वाक्यं_तुलय(command, "read-string") == 0) {
		वर्ण *res;

		यदि (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		यदि (ul_मार्गः_read_string(pc, &res, file) <= 0)
			err(EXIT_FAILURE, "read string failed");
		प्रारूप_लिख("read:  %s: %s\n", file, res);

		यदि (ul_मार्गः_readf_string(pc, &res, "%s", file) <= 0)
			err(EXIT_FAILURE, "readf string failed");
		प्रारूप_लिख("readf: %s: %s\n", file, res);

	} अन्यथा यदि (वाक्यं_तुलय(command, "read-link") == 0) {
		वर्ण res[PATH_MAX];

		यदि (optind == argc)
			errx(EXIT_FAILURE, "<file> not defined");
		file = argv[optind++];

		यदि (ul_मार्गः_readlink(pc, res, परिमाण(res), file) < 0)
			err(EXIT_FAILURE, "read symlink failed");
		प्रारूप_लिख("read:  %s: %s\n", file, res);

		यदि (ul_मार्गः_readlinkf(pc, res, परिमाण(res), "%s", file) < 0)
			err(EXIT_FAILURE, "readf symlink failed");
		प्रारूप_लिख("readf: %s: %s\n", file, res);

	} अन्यथा यदि (वाक्यं_तुलय(command, "write-string") == 0) {
		वर्ण *str;

		यदि (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		यदि (ul_मार्गः_write_string(pc, str, file) != 0)
			err(EXIT_FAILURE, "write string failed");
		यदि (ul_मार्गः_writef_string(pc, str, "%s", file) != 0)
			err(EXIT_FAILURE, "writef string failed");

	} अन्यथा यदि (वाक्यं_तुलय(command, "write-u64") == 0) {
		uint64_t अङ्कः;

		यदि (optind + 1 == argc)
			errx(EXIT_FAILURE, "<file> <num> not defined");
		file = argv[optind++];
		अङ्कः = strtoumax(argv[optind++], रिक्त, 0);

		यदि (ul_मार्गः_write_u64(pc, अङ्कः, file) != 0)
			err(EXIT_FAILURE, "write u64 failed");
		यदि (ul_मार्गः_writef_u64(pc, अङ्कः, "%s", file) != 0)
			err(EXIT_FAILURE, "writef u64 failed");
	}

	ul_unref_मार्गः(pc);
	प्रत्यागम EXIT_SUCCESS;
}
#यदि_अन्तः /* TEST_PROGRAM_PATH */
