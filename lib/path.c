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
स्थिर UL_DEBUG_DEFINE_MASK(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPATH_DEBUG_INIT	(1 << 1)
#define ULPATH_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

शून्य ul_path_init_debug(शून्य)
{
	जर (ulpath_debug_mask)
		परत;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

संरचना path_cxt *ul_new_path(स्थिरांक अक्षर *dir, ...)
{
	संरचना path_cxt *pc = सुस्मृति(1, आकार(*pc));

	जर (!pc)
		परत रिक्त;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	जर (dir) {
		पूर्णांक rc;
		बहु_सूची ap;

		बहु_शुरू(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		बहु_पूर्ण(ap);

		जर (rc < 0 || !pc->dir_path)
			जा fail;
	}
	परत pc;
fail:
	ul_unref_path(pc);
	परत रिक्त;
}

शून्य ul_ref_path(संरचना path_cxt *pc)
{
	जर (pc)
		pc->refcount++;
}

शून्य ul_unref_path(संरचना path_cxt *pc)
{
	जर (!pc)
		परत;

	pc->refcount--;

	जर (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		जर (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		मुक्त(pc->dir_path);
		मुक्त(pc->prefix);
		मुक्त(pc);
	}
}

शून्य ul_path_refer_vfs(संरचना path_cxt *pc, स्थिरांक संरचना ul_vfs_ops *vfs)
{
	जर (pc)
		pc->vfs = vfs;
}

पूर्णांक ul_path_set_prefix(संरचना path_cxt *pc, स्थिरांक अक्षर *prefix)
{
	अक्षर *p = रिक्त;

	प्रतिज्ञा(pc->dir_fd < 0);

	जर (prefix) {
		p = strdup(prefix);
		जर (!p)
			परत -ENOMEM;
	}

	मुक्त(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	परत 0;
}

स्थिरांक अक्षर *ul_path_get_prefix(संरचना path_cxt *pc)
{
	परत pc ? pc->prefix : रिक्त;
}

पूर्णांक ul_path_set_dir(संरचना path_cxt *pc, स्थिरांक अक्षर *dir)
{
	अक्षर *p = रिक्त;

	जर (dir) {
		p = strdup(dir);
		जर (!p)
			परत -ENOMEM;
	}

	जर (pc->dir_fd >= 0) {
		बंद(pc->dir_fd);
		pc->dir_fd = -1;
	}

	मुक्त(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	परत 0;
}

स्थिरांक अक्षर *ul_path_get_dir(संरचना path_cxt *pc)
{
	परत pc ? pc->dir_path : रिक्त;
}

पूर्णांक ul_path_set_dialect(संरचना path_cxt *pc, शून्य *data, शून्य free_data(संरचना path_cxt *))
{
	pc->dialect = data;
	pc->free_dialect = free_data;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	परत 0;
}

शून्य *ul_path_get_dialect(संरचना path_cxt *pc)
{
	परत pc ? pc->dialect : रिक्त;
}

पूर्णांक ul_path_set_enoent_redirect(संरचना path_cxt *pc, पूर्णांक (*func)(संरचना path_cxt *, स्थिरांक अक्षर *, पूर्णांक *))
{
	pc->redirect_on_enoent = func;
	परत 0;
}

स्थिर स्थिरांक अक्षर *get_absdir(संरचना path_cxt *pc)
{
	पूर्णांक rc;
	स्थिरांक अक्षर *dirpath;

	जर (!pc->prefix)
		परत pc->dir_path;

	dirpath = pc->dir_path;
	जर (!dirpath)
		परत pc->prefix;
	जर (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_buffer, आकार(pc->path_buffer), "%s/%s", pc->prefix, dirpath);
	जर (rc < 0)
		परत रिक्त;
	जर ((माप_प्रकार)rc >= आकार(pc->path_buffer)) {
		त्रुटि_सं = ENAMETOOLONG;
		परत रिक्त;
	}

	परत pc->path_buffer;
}

पूर्णांक ul_path_is_accessible(संरचना path_cxt *pc)
{
	स्थिरांक अक्षर *path;
	प्रतिज्ञा(pc);

	जर (pc->dir_fd >= 0)
		परत 1;

	path = get_absdir(pc);
	जर (!path)
		परत 0;
	परत access(path, F_OK) == 0;
}

पूर्णांक ul_path_get_dirfd(संरचना path_cxt *pc)
{
	प्रतिज्ञा(pc);
	प्रतिज्ञा(pc->dir_path);

	जर (pc->dir_fd < 0) {
		स्थिरांक अक्षर *path = get_absdir(pc);
		जर (!path)
			परत -त्रुटि_सं;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = खोलो(path, O_RDONLY|O_CLOEXEC);
	}

	परत pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
शून्य ul_path_close_dirfd(संरचना path_cxt *pc)
{
	प्रतिज्ञा(pc);

	जर (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		बंद(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

पूर्णांक ul_path_isopen_dirfd(संरचना path_cxt *pc)
{
	परत pc && pc->dir_fd >= 0;
}

स्थिर स्थिरांक अक्षर *ul_path_mkpath(संरचना path_cxt *pc, स्थिरांक अक्षर *path, बहु_सूची ap)
{
	पूर्णांक rc;

	त्रुटि_सं = 0;

	rc = vsnprintf(pc->path_buffer, आकार(pc->path_buffer), path, ap);
	जर (rc < 0) {
		जर (!त्रुटि_सं)
			त्रुटि_सं = EINVAL;
		परत रिक्त;
	}

	जर ((माप_प्रकार)rc >= आकार(pc->path_buffer)) {
		त्रुटि_सं = ENAMETOOLONG;
		परत रिक्त;
	}

	परत pc->path_buffer;
}

अक्षर *ul_path_get_abspath(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz, स्थिरांक अक्षर *path, ...)
{
	जर (path) {
		पूर्णांक rc;
		बहु_सूची ap;
		स्थिरांक अक्षर *tail = रिक्त, *dirpath = pc->dir_path;

		बहु_शुरू(ap, path);
		tail = ul_path_mkpath(pc, path, ap);
		बहु_पूर्ण(ap);

		जर (dirpath && *dirpath == '/')
			dirpath++;
		जर (tail && *tail == '/')
			tail++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				tail ? tail : "");

		जर ((माप_प्रकार)rc >= bufsz) {
			त्रुटि_सं = ENAMETOOLONG;
			परत रिक्त;
		}
	} अन्यथा {
		स्थिरांक अक्षर *tmp = get_absdir(pc);

		जर (!tmp)
			परत रिक्त;
		xstrncpy(buf, tmp, bufsz);
	}

	परत buf;
}


पूर्णांक ul_path_access(संरचना path_cxt *pc, पूर्णांक mode, स्थिरांक अक्षर *path)
{
	पूर्णांक rc;

	जर (!path)
		परत -EINVAL;
	जर (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} अन्यथा {
		पूर्णांक dir = ul_path_get_dirfd(pc);
		जर (dir < 0)
			परत dir;
		जर (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		जर (rc && त्रुटि_सं == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	परत rc;
}

पूर्णांक ul_path_accessf(संरचना path_cxt *pc, पूर्णांक mode, स्थिरांक अक्षर *path, ...)
{
	बहु_सूची ap;
	स्थिरांक अक्षर *p;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	परत !p ? -त्रुटि_सं : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
पूर्णांक ul_path_stat(संरचना path_cxt *pc, संरचना stat *sb, पूर्णांक flags, स्थिरांक अक्षर *path)
{
	पूर्णांक rc;

	जर (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} अन्यथा {
		पूर्णांक dir = ul_path_get_dirfd(pc);
		जर (dir < 0)
			परत dir;
		जर (path) {
			जर  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} अन्यथा
			rc = ख_स्थिति(dir, sb);	/* dir itself */

		जर (rc && त्रुटि_सं == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	परत rc;
}

पूर्णांक ul_path_vstatf(संरचना path_cxt *pc, संरचना stat *sb, पूर्णांक flags, स्थिरांक अक्षर *path, बहु_सूची ap)
{
	स्थिरांक अक्षर *p = ul_path_mkpath(pc, path, ap);

	परत !p ? -त्रुटि_सं : ul_path_stat(pc, sb, flags, p);
}

पूर्णांक ul_path_statf(संरचना path_cxt *pc, संरचना stat *sb, पूर्णांक flags, स्थिरांक अक्षर *path, ...)
{
	बहु_सूची ap;
	पूर्णांक rc;

	बहु_शुरू(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	बहु_पूर्ण(ap);

	परत rc;
}
#ifdef HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
पूर्णांक ul_path_statx(संरचना path_cxt *pc, संरचना statx *stx, पूर्णांक flags, अचिन्हित पूर्णांक mask,
		  स्थिरांक अक्षर *path)
{
	पूर्णांक rc;

	जर (!path)
		परत -EINVAL;
	जर (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	अन्यथा {
		पूर्णांक dir = ul_path_get_dirfd(pc);
		जर (dir < 0)
			परत dir;
		जर (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	परत rc;
}
#अन्यथा
पूर्णांक ul_path_statx(संरचना path_cxt *pc __attribute__((__unused__)),
		  संरचना statx *stx __attribute__((__unused__)),
		  पूर्णांक flags __attribute__((__unused__)),
		  अचिन्हित पूर्णांक mask __attribute__((__unused__)),
		  स्थिरांक अक्षर *path __attribute__((__unused__)))
{
	त्रुटि_सं = ENOSYS;
	परत -1;
}
#endif /* HAVE_STATX */

पूर्णांक ul_path_vstatxf(संरचना path_cxt *pc, संरचना statx *stx, पूर्णांक flags, अचिन्हित पूर्णांक mask,
		    स्थिरांक अक्षर *path, बहु_सूची ap)
{
	स्थिरांक अक्षर *p = ul_path_mkpath(pc, path, ap);

	परत !p ? -त्रुटि_सं : ul_path_statx(pc, stx, flags, mask, p);
}

पूर्णांक ul_path_statxf(संरचना path_cxt *pc, संरचना statx *stx, पूर्णांक flags, अचिन्हित पूर्णांक mask,
		   स्थिरांक अक्षर *path, ...)
{
	बहु_सूची ap;
	पूर्णांक rc;

	बहु_शुरू(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	बहु_पूर्ण(ap);

	परत rc;
}

पूर्णांक ul_path_open(संरचना path_cxt *pc, पूर्णांक flags, स्थिरांक अक्षर *path)
{
	पूर्णांक fd;

	जर (!path)
		परत -EINVAL;
	जर (!pc) {
		fd = खोलो(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} अन्यथा {
		पूर्णांक fdx;
		पूर्णांक dir = ul_path_get_dirfd(pc);
		जर (dir < 0)
			परत dir;

		जर (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		जर (fd < 0 && त्रुटि_सं == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	परत fd;
}

पूर्णांक ul_path_vopenf(संरचना path_cxt *pc, पूर्णांक flags, स्थिरांक अक्षर *path, बहु_सूची ap)
{
	स्थिरांक अक्षर *p = ul_path_mkpath(pc, path, ap);

	परत !p ? -त्रुटि_सं : ul_path_open(pc, flags, p);
}

पूर्णांक ul_path_openf(संरचना path_cxt *pc, पूर्णांक flags, स्थिरांक अक्षर *path, ...)
{
	बहु_सूची ap;
	पूर्णांक rc;

	बहु_शुरू(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	बहु_पूर्ण(ap);

	परत rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
खाता *ul_path_fopen(संरचना path_cxt *pc, स्थिरांक अक्षर *mode, स्थिरांक अक्षर *path)
{
	पूर्णांक flags = ul_mode_to_flags(mode);
	पूर्णांक fd = ul_path_open(pc, flags, path);

	जर (fd < 0)
		परत रिक्त;

	परत ul_vfs_fdopen(pc ? pc->vfs : रिक्त, fd, mode);
}


खाता *ul_path_vfopenf(संरचना path_cxt *pc, स्थिरांक अक्षर *mode, स्थिरांक अक्षर *path, बहु_सूची ap)
{
	स्थिरांक अक्षर *p = ul_path_mkpath(pc, path, ap);

	परत !p ? रिक्त : ul_path_fopen(pc, mode, p);
}

खाता *ul_path_fopenf(संरचना path_cxt *pc, स्थिरांक अक्षर *mode, स्थिरांक अक्षर *path, ...)
{
	खाता *f;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	बहु_पूर्ण(ap);

	परत f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
सूची *ul_path_opendir(संरचना path_cxt *pc, स्थिरांक अक्षर *path)
{
	सूची *dir;
	पूर्णांक fd = -1;

	जर (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	अन्यथा जर (pc->dir_path) {
		पूर्णांक dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		जर (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	जर (fd < 0)
		परत रिक्त;

	dir = fdopendir(fd);
	जर (!dir) {
		बंद(fd);
		परत रिक्त;
	}
	जर (!path)
		 सूची_शुरु(dir);
	परत dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
सूची *ul_path_vopendirf(संरचना path_cxt *pc, स्थिरांक अक्षर *path, बहु_सूची ap)
{
	स्थिरांक अक्षर *p = ul_path_mkpath(pc, path, ap);

	परत !p ? रिक्त : ul_path_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
सूची *ul_path_opendirf(संरचना path_cxt *pc, स्थिरांक अक्षर *path, ...)
{
	बहु_सूची ap;
	सूची *dir;

	बहु_शुरू(ap, path);
	dir = ul_path_vopendirf(pc, path, ap);
	बहु_पूर्ण(ap);

	परत dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार bufsiz, स्थिरांक अक्षर *path)
{
	पूर्णांक dirfd;
	ssize_t ssz;

	जर (!path) {
		स्थिरांक अक्षर *p = get_absdir(pc);
		जर (!p)
			परत -त्रुटि_सं;
		ssz = readlink(p, buf, bufsiz - 1);
	} अन्यथा {
		dirfd = ul_path_get_dirfd(pc);
		जर (dirfd < 0)
			परत dirfd;

		जर (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	जर (ssz >= 0)
		buf[ssz] = '\0';
	परत ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlinkf(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार bufsiz, स्थिरांक अक्षर *path, ...)
{
	स्थिरांक अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	परत !p ? -त्रुटि_सं : ul_path_readlink(pc, buf, bufsiz, p);
}

पूर्णांक ul_path_read(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार len, स्थिरांक अक्षर *path)
{
	पूर्णांक rc, errsv;
	पूर्णांक fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	जर (fd < 0)
		परत -त्रुटि_सं;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : रिक्त, fd, buf, len);

	errsv = त्रुटि_सं;
	ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
	त्रुटि_सं = errsv;
	परत rc;
}

पूर्णांक ul_path_vreadf(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार len, स्थिरांक अक्षर *path, बहु_सूची ap)
{
	स्थिरांक अक्षर *p = ul_path_mkpath(pc, path, ap);

	परत !p ? -त्रुटि_सं : ul_path_read(pc, buf, len, p);
}

पूर्णांक ul_path_readf(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार len, स्थिरांक अक्षर *path, ...)
{
	बहु_सूची ap;
	पूर्णांक rc;

	बहु_शुरू(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	बहु_पूर्ण(ap);

	परत rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
पूर्णांक ul_path_read_string(संरचना path_cxt *pc, अक्षर **str, स्थिरांक अक्षर *path)
{
	अक्षर buf[बफ_मान];
	पूर्णांक rc;

	जर (!str)
		परत -EINVAL;

	*str = रिक्त;

	rc = ul_path_read_buffer(pc, buf, आकार(buf), path);
	जर (rc < 0)
		परत rc;

	*str = strdup(buf);
	जर (!*str)
		rc = -ENOMEM;

	परत rc;
}

पूर्णांक ul_path_readf_string(संरचना path_cxt *pc, अक्षर **str, स्थिरांक अक्षर *path, ...)
{
	स्थिरांक अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	परत !p ? -त्रुटि_सं : ul_path_read_string(pc, str, p);
}

पूर्णांक ul_path_read_buffer(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz, स्थिरांक अक्षर *path)
{
	पूर्णांक rc = ul_path_read(pc, buf, bufsz - 1, path);

	जर (rc == 0)
		buf[0] = '\0';

	अन्यथा जर (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		जर (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		अन्यथा
			buf[rc] = '\0';
	}

	परत rc;
}

पूर्णांक ul_path_vreadf_buffer(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz, स्थिरांक अक्षर *path, बहु_सूची ap)
{
	स्थिरांक अक्षर *p;

	p = ul_path_mkpath(pc, path, ap);

	परत !p ? -त्रुटि_सं : ul_path_read_buffer(pc, buf, bufsz, p);
}

पूर्णांक ul_path_readf_buffer(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz, स्थिरांक अक्षर *path, ...)
{
	बहु_सूची ap;
	पूर्णांक rc;

	बहु_शुरू(ap, path);
	rc = ul_path_vreadf_buffer(pc, buf, bufsz, path, ap);
	बहु_पूर्ण(ap);

	परत rc;
}

पूर्णांक ul_path_scanf(संरचना path_cxt *pc, स्थिरांक अक्षर *path, स्थिरांक अक्षर *fmt, ...)
{
	खाता *f;
	बहु_सूची fmt_ap;
	पूर्णांक rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	जर (!f)
		परत -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	बहु_शुरू(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	बहु_पूर्ण(fmt_ap);

	ख_बंद(f);
	परत rc;
}

पूर्णांक ul_path_scanff(संरचना path_cxt *pc, स्थिरांक अक्षर *path, बहु_सूची ap, स्थिरांक अक्षर *fmt, ...)
{
	खाता *f;
	बहु_सूची fmt_ap;
	पूर्णांक rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	जर (!f)
		परत -EINVAL;

	बहु_शुरू(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	बहु_पूर्ण(fmt_ap);

	ख_बंद(f);
	परत rc;
}


पूर्णांक ul_path_read_s64(संरचना path_cxt *pc, int64_t *res, स्थिरांक अक्षर *path)
{
	int64_t x = 0;
	पूर्णांक rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	जर (rc != 1)
		परत -1;
	जर (res)
		*res = x;
	परत 0;
}

पूर्णांक ul_path_readf_s64(संरचना path_cxt *pc, int64_t *res, स्थिरांक अक्षर *path, ...)
{
	स्थिरांक अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	परत !p ? -त्रुटि_सं : ul_path_read_s64(pc, res, p);
}

पूर्णांक ul_path_read_u64(संरचना path_cxt *pc, uint64_t *res, स्थिरांक अक्षर *path)
{
	uint64_t x = 0;
	पूर्णांक rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	जर (rc != 1)
		परत -1;
	जर (res)
		*res = x;
	परत 0;
}

पूर्णांक ul_path_readf_u64(संरचना path_cxt *pc, uint64_t *res, स्थिरांक अक्षर *path, ...)
{
	स्थिरांक अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	परत !p ? -त्रुटि_सं : ul_path_read_u64(pc, res, p);
}

पूर्णांक ul_path_read_s32(संरचना path_cxt *pc, पूर्णांक *res, स्थिरांक अक्षर *path)
{
	पूर्णांक rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	जर (rc != 1)
		परत -1;
	जर (res)
		*res = x;
	परत 0;
}

पूर्णांक ul_path_readf_s32(संरचना path_cxt *pc, पूर्णांक *res, स्थिरांक अक्षर *path, ...)
{
	स्थिरांक अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	परत !p ? -त्रुटि_सं : ul_path_read_s32(pc, res, p);
}

पूर्णांक ul_path_read_u32(संरचना path_cxt *pc, अचिन्हित पूर्णांक *res, स्थिरांक अक्षर *path)
{
	पूर्णांक rc;
	अचिन्हित पूर्णांक x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	जर (rc != 1)
		परत -1;
	जर (res)
		*res = x;
	परत 0;
}

पूर्णांक ul_path_readf_u32(संरचना path_cxt *pc, अचिन्हित पूर्णांक *res, स्थिरांक अक्षर *path, ...)
{
	स्थिरांक अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	परत !p ? -त्रुटि_सं : ul_path_read_u32(pc, res, p);
}

पूर्णांक ul_path_read_majmin(संरचना path_cxt *pc, dev_t *res, स्थिरांक अक्षर *path)
{
	पूर्णांक rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	जर (rc != 2)
		परत -1;
	जर (res)
		*res = makedev(maj, min);
	परत 0;
}

पूर्णांक ul_path_readf_majmin(संरचना path_cxt *pc, dev_t *res, स्थिरांक अक्षर *path, ...)
{
	स्थिरांक अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	परत !p ? -त्रुटि_सं : ul_path_read_majmin(pc, res, p);
}

पूर्णांक ul_path_write_string(संरचना path_cxt *pc, स्थिरांक अक्षर *str, स्थिरांक अक्षर *path)
{
	पूर्णांक rc, errsv;
	पूर्णांक fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	जर (fd < 0)
		परत -त्रुटि_सं;

	rc = ul_vfs_write_all(pc ? pc->vfs : रिक्त, fd, str, म_माप(str));

	errsv = त्रुटि_सं;
	ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
	त्रुटि_सं = errsv;
	परत rc;
}

पूर्णांक ul_path_writef_string(संरचना path_cxt *pc, स्थिरांक अक्षर *str, स्थिरांक अक्षर *path, ...)
{
	स्थिरांक अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	परत !p ? -त्रुटि_सं : ul_path_write_string(pc, str, p);
}

पूर्णांक ul_path_write_s64(संरचना path_cxt *pc, int64_t अंक, स्थिरांक अक्षर *path)
{
	अक्षर buf[आकार(stringify_मूल्य(LLONG_MAX))];
	पूर्णांक rc, errsv;
	पूर्णांक fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	जर (fd < 0)
		परत -त्रुटि_सं;

	len = snprintf(buf, आकार(buf), "%" PRId64, अंक);
	जर (len < 0 || (माप_प्रकार) len >= आकार(buf))
		rc = len < 0 ? -त्रुटि_सं : -E2BIG;
	अन्यथा
		rc = ul_vfs_write_all(pc ? pc->vfs : रिक्त, fd, buf, len);

	errsv = त्रुटि_सं;
	ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
	त्रुटि_सं = errsv;
	परत rc;
}

पूर्णांक ul_path_write_u64(संरचना path_cxt *pc, uint64_t अंक, स्थिरांक अक्षर *path)
{
	अक्षर buf[आकार(stringify_मूल्य(ULLONG_MAX))];
	पूर्णांक rc, errsv;
	पूर्णांक fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	जर (fd < 0)
		परत -त्रुटि_सं;

	len = snprintf(buf, आकार(buf), "%" PRIu64, अंक);
	जर (len < 0 || (माप_प्रकार) len >= आकार(buf))
		rc = len < 0 ? -त्रुटि_सं : -E2BIG;
	अन्यथा
		rc = ul_vfs_write_all(pc ? pc->vfs : रिक्त, fd, buf, len);

	errsv = त्रुटि_सं;
	ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
	त्रुटि_सं = errsv;
	परत rc;
}

पूर्णांक ul_path_writef_u64(संरचना path_cxt *pc, uint64_t अंक, स्थिरांक अक्षर *path, ...)
{
	स्थिरांक अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	परत !p ? -त्रुटि_सं : ul_path_write_u64(pc, अंक, p);

}

पूर्णांक ul_path_मोजणी_dirents(संरचना path_cxt *pc, स्थिरांक अक्षर *path)
{
	सूची *dir;
	पूर्णांक r = 0;

	dir = ul_path_opendir(pc, path);
	जर (!dir)
		परत 0;

	पर्यंत (xreaddir(dir)) r++;

	बंद_सूची(dir);
	परत r;
}

पूर्णांक ul_path_countf_dirents(संरचना path_cxt *pc, स्थिरांक अक्षर *path, ...)
{
	स्थिरांक अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	परत !p ? -त्रुटि_सं : ul_path_मोजणी_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
पूर्णांक ul_path_next_dirent(संरचना path_cxt *pc, सूची **sub, स्थिरांक अक्षर *स_नाम, संरचना dirent **d)
{
	जर (!pc || !sub || !d)
		परत -EINVAL;

	जर (!*sub) {
		*sub = ul_path_opendir(pc, स_नाम);
		जर (!*sub)
			परत -त्रुटि_सं;
	}

	*d = xreaddir(*sub);
	जर (*d)
		परत 0;

	बंद_सूची(*sub);
	*sub = रिक्त;
	परत 1;
}

#ifdef HAVE_CPU_SET_T
स्थिर पूर्णांक ul_path_cpuparse(संरचना path_cxt *pc, cpu_set_t **set, पूर्णांक maxcpus, पूर्णांक islist, स्थिरांक अक्षर *path, बहु_सूची ap)
{
	माप_प्रकार setsize, len = maxcpus * 7;
	अक्षर *buf;
	पूर्णांक rc;

	*set = रिक्त;

	buf = दो_स्मृति(len);
	जर (!buf)
		परत -ENOMEM;

	rc = ul_path_vreadf_buffer(pc, buf, len, path, ap);
	जर (rc < 0)
		जा out;

	*set = cpuset_alloc(maxcpus, &setsize, रिक्त);
	जर (!*set) {
		rc = -EINVAL;
		जा out;
	}

	जर (islist) {
		जर (cpulist_parse(buf, *set, setsize, 0)) {
			त्रुटि_सं = EINVAL;
			rc = -त्रुटि_सं;
			जा out;
		}
	} अन्यथा {
		जर (cpumask_parse(buf, *set, setsize)) {
			त्रुटि_सं = EINVAL;
			rc = -त्रुटि_सं;
			जा out;
		}
	}
	rc = 0;

out:
	जर (rc) {
		cpuset_free(*set);
		*set = रिक्त;
	}
	मुक्त(buf);
	परत rc;
}

पूर्णांक ul_path_readf_cpuset(संरचना path_cxt *pc, cpu_set_t **set, पूर्णांक maxcpus, स्थिरांक अक्षर *path, ...)
{
	बहु_सूची ap;
	पूर्णांक rc = 0;

	बहु_शुरू(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	बहु_पूर्ण(ap);

	परत rc;
}

पूर्णांक ul_path_readf_cpulist(संरचना path_cxt *pc, cpu_set_t **set, पूर्णांक maxcpus, स्थिरांक अक्षर *path, ...)
{
	बहु_सूची ap;
	पूर्णांक rc = 0;

	बहु_शुरू(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	बहु_पूर्ण(ap);

	परत rc;
}

#endif /* HAVE_CPU_SET_T */


#ifdef TEST_PROGRAM_PATH
#अन्तर्भाव <getopt.h>

स्थिर शून्य __attribute__((__noreturn__)) usage(शून्य)
{
	ख_लिखो(मानक_निकास, " %s [options] <dir> <command>\n\n", program_invocation_short_name);
	ख_माला_दो(" -p, --prefix <dir>      redirect hardcoded paths to <dir>\n", मानक_निकास);

	ख_माला_दो(" Commands:\n", मानक_निकास);
	ख_माला_दो(" read-u64 <file>            read uint64_t from file\n", मानक_निकास);
	ख_माला_दो(" read-s64 <file>            read  int64_t from file\n", मानक_निकास);
	ख_माला_दो(" read-u32 <file>            read uint32_t from file\n", मानक_निकास);
	ख_माला_दो(" read-s32 <file>            read  int32_t from file\n", मानक_निकास);
	ख_माला_दो(" read-string <file>         read string  from file\n", मानक_निकास);
	ख_माला_दो(" read-majmin <file>         read devno from file\n", मानक_निकास);
	ख_माला_दो(" read-link <file>           read symlink\n", मानक_निकास);
	ख_माला_दो(" write-string <file> <str>  write string from file\n", मानक_निकास);
	ख_माला_दो(" write-u64 <file> <str>     write uint64_t from file\n", मानक_निकास);

	निकास(निकास_सफल);
}

पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
	पूर्णांक c;
	स्थिरांक अक्षर *prefix = रिक्त, *dir, *file, *command;
	संरचना path_cxt *pc = रिक्त;

	स्थिर स्थिरांक संरचना option longopts[] = {
		{ "prefix",	1, रिक्त, 'p' },
		{ "help",       0, रिक्त, 'h' },
		{ रिक्त, 0, रिक्त, 0 },
	};

	पर्यंत((c = getopt_long(argc, argv, "p:h", longopts, रिक्त)) != -1) {
		निवड(c) {
		स्थिती 'p':
			prefix = optarg;
			थांबा;
		स्थिती 'h':
			usage();
			थांबा;
		पूर्वनिर्धारित:
			err(निकास_त्रुटि, "try --help");
		}
	}

	जर (optind == argc)
		errx(निकास_त्रुटि, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	जर (!pc)
		err(निकास_त्रुटि, "failed to initialize path context");
	जर (prefix)
		ul_path_set_prefix(pc, prefix);

	जर (optind == argc)
		errx(निकास_त्रुटि, "<command> not defined");
	command = argv[optind++];

	जर (म_भेद(command, "read-u32") == 0) {
		uint32_t res;

		जर (optind == argc)
			errx(निकास_त्रुटि, "<file> not defined");
		file = argv[optind++];

		जर (ul_path_read_u32(pc, &res, file) != 0)
			err(निकास_त्रुटि, "read u64 failed");
		म_लिखो("read:  %s: %u\n", file, res);

		जर (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(निकास_त्रुटि, "readf u64 failed");
		म_लिखो("readf: %s: %u\n", file, res);

	} अन्यथा जर (म_भेद(command, "read-s32") == 0) {
		int32_t res;

		जर (optind == argc)
			errx(निकास_त्रुटि, "<file> not defined");
		file = argv[optind++];

		जर (ul_path_read_s32(pc, &res, file) != 0)
			err(निकास_त्रुटि, "read u64 failed");
		म_लिखो("read:  %s: %d\n", file, res);

		जर (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(निकास_त्रुटि, "readf u64 failed");
		म_लिखो("readf: %s: %d\n", file, res);

	} अन्यथा जर (म_भेद(command, "read-u64") == 0) {
		uint64_t res;

		जर (optind == argc)
			errx(निकास_त्रुटि, "<file> not defined");
		file = argv[optind++];

		जर (ul_path_read_u64(pc, &res, file) != 0)
			err(निकास_त्रुटि, "read u64 failed");
		म_लिखो("read:  %s: %" PRIu64 "\n", file, res);

		जर (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(निकास_त्रुटि, "readf u64 failed");
		म_लिखो("readf: %s: %" PRIu64 "\n", file, res);

	} अन्यथा जर (म_भेद(command, "read-s64") == 0) {
		int64_t res;

		जर (optind == argc)
			errx(निकास_त्रुटि, "<file> not defined");
		file = argv[optind++];

		जर (ul_path_read_s64(pc, &res, file) != 0)
			err(निकास_त्रुटि, "read u64 failed");
		म_लिखो("read:  %s: %" PRId64 "\n", file, res);

		जर (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(निकास_त्रुटि, "readf u64 failed");
		म_लिखो("readf: %s: %" PRId64 "\n", file, res);

	} अन्यथा जर (म_भेद(command, "read-majmin") == 0) {
		dev_t res;

		जर (optind == argc)
			errx(निकास_त्रुटि, "<file> not defined");
		file = argv[optind++];

		जर (ul_path_read_majmin(pc, &res, file) != 0)
			err(निकास_त्रुटि, "read maj:min failed");
		म_लिखो("read:  %s: %d\n", file, (पूर्णांक) res);

		जर (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(निकास_त्रुटि, "readf maj:min failed");
		म_लिखो("readf: %s: %d\n", file, (पूर्णांक) res);

	} अन्यथा जर (म_भेद(command, "read-string") == 0) {
		अक्षर *res;

		जर (optind == argc)
			errx(निकास_त्रुटि, "<file> not defined");
		file = argv[optind++];

		जर (ul_path_read_string(pc, &res, file) <= 0)
			err(निकास_त्रुटि, "read string failed");
		म_लिखो("read:  %s: %s\n", file, res);

		जर (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(निकास_त्रुटि, "readf string failed");
		म_लिखो("readf: %s: %s\n", file, res);

	} अन्यथा जर (म_भेद(command, "read-link") == 0) {
		अक्षर res[PATH_MAX];

		जर (optind == argc)
			errx(निकास_त्रुटि, "<file> not defined");
		file = argv[optind++];

		जर (ul_path_readlink(pc, res, आकार(res), file) < 0)
			err(निकास_त्रुटि, "read symlink failed");
		म_लिखो("read:  %s: %s\n", file, res);

		जर (ul_path_readlinkf(pc, res, आकार(res), "%s", file) < 0)
			err(निकास_त्रुटि, "readf symlink failed");
		म_लिखो("readf: %s: %s\n", file, res);

	} अन्यथा जर (म_भेद(command, "write-string") == 0) {
		अक्षर *str;

		जर (optind + 1 == argc)
			errx(निकास_त्रुटि, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		जर (ul_path_write_string(pc, str, file) != 0)
			err(निकास_त्रुटि, "write string failed");
		जर (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(निकास_त्रुटि, "writef string failed");

	} अन्यथा जर (म_भेद(command, "write-u64") == 0) {
		uint64_t अंक;

		जर (optind + 1 == argc)
			errx(निकास_त्रुटि, "<file> <num> not defined");
		file = argv[optind++];
		अंक = strtoumax(argv[optind++], रिक्त, 0);

		जर (ul_path_write_u64(pc, अंक, file) != 0)
			err(निकास_त्रुटि, "write u64 failed");
		जर (ul_path_writef_u64(pc, अंक, "%s", file) != 0)
			err(निकास_त्रुटि, "writef u64 failed");
	}

	ul_unref_path(pc);
	परत निकास_सफल;
}
#endif /* TEST_PROGRAM_PATH */
