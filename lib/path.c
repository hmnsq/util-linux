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
#समावेश <stdarg.h>
#समावेश <string.h>
#समावेश <unistd.h>
#समावेश <stdio.h>
#समावेश <inttypes.h>
#समावेश <errno.h>

#समावेश "c.h"
#समावेश "fileutils.h"
#समावेश "all-io.h"
#समावेश "vfs.h"
#समावेश "path.h"
#समावेश "debug.h"
#समावेश "strutils.h"

/*
 * Debug stuff (based on include/debug.h)
 */
अटल UL_DEBUG_DEFINE_MASK(ulpath);
UL_DEBUG_DEFINE_MASKNAMES(ulpath) = UL_DEBUG_EMPTY_MASKNAMES;

#घोषणा ULPATH_DEBUG_INIT	(1 << 1)
#घोषणा ULPATH_DEBUG_CXT	(1 << 2)

#घोषणा DBG(m, x)		__UL_DBG(ulpath, ULPATH_DEBUG_, m, x)
#घोषणा DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpath, ULPATH_DEBUG_, m, h, x)
#घोषणा ON_DBG(m, x)		__UL_DBG_CALL(ulpath, ULPATH_DEBUG_, m, x)

व्योम ul_path_init_debug(व्योम)
{
	अगर (ulpath_debug_mask)
		वापस;
	__UL_INIT_DEBUG_FROM_ENV(ulpath, ULPATH_DEBUG_, 0, ULPATH_DEBUG);
}

काष्ठा path_cxt *ul_new_path(स्थिर अक्षर *dir, ...)
{
	काष्ठा path_cxt *pc = सुस्मृति(1, माप(*pc));

	अगर (!pc)
		वापस शून्य;

	DBG_OBJ(CXT, pc, ul_debug("alloc"));

	pc->refcount = 1;
	pc->dir_fd = -1;

	अगर (dir) {
		पूर्णांक rc;
		बहु_सूची ap;

		बहु_शुरू(ap, dir);
		rc = vasprintf(&pc->dir_path, dir, ap);
		बहु_पूर्ण(ap);

		अगर (rc < 0 || !pc->dir_path)
			जाओ fail;
	}
	वापस pc;
fail:
	ul_unref_path(pc);
	वापस शून्य;
}

व्योम ul_ref_path(काष्ठा path_cxt *pc)
{
	अगर (pc)
		pc->refcount++;
}

व्योम ul_unref_path(काष्ठा path_cxt *pc)
{
	अगर (!pc)
		वापस;

	pc->refcount--;

	अगर (pc->refcount <= 0) {
		DBG_OBJ(CXT, pc, ul_debug("dealloc"));
		अगर (pc->dialect)
			pc->free_dialect(pc);
		ul_path_close_dirfd(pc);
		मुक्त(pc->dir_path);
		मुक्त(pc->prefix);
		मुक्त(pc);
	}
}

व्योम ul_path_refer_vfs(काष्ठा path_cxt *pc, स्थिर काष्ठा ul_vfs_ops *vfs)
{
	अगर (pc)
		pc->vfs = vfs;
}

पूर्णांक ul_path_set_prefix(काष्ठा path_cxt *pc, स्थिर अक्षर *prefix)
{
	अक्षर *p = शून्य;

	निश्चित(pc->dir_fd < 0);

	अगर (prefix) {
		p = strdup(prefix);
		अगर (!p)
			वापस -ENOMEM;
	}

	मुक्त(pc->prefix);
	pc->prefix = p;
	DBG_OBJ(CXT, pc, ul_debug("new prefix: '%s'", p));
	वापस 0;
}

स्थिर अक्षर *ul_path_get_prefix(काष्ठा path_cxt *pc)
{
	वापस pc ? pc->prefix : शून्य;
}

पूर्णांक ul_path_set_dir(काष्ठा path_cxt *pc, स्थिर अक्षर *dir)
{
	अक्षर *p = शून्य;

	अगर (dir) {
		p = strdup(dir);
		अगर (!p)
			वापस -ENOMEM;
	}

	अगर (pc->dir_fd >= 0) {
		बंद(pc->dir_fd);
		pc->dir_fd = -1;
	}

	मुक्त(pc->dir_path);
	pc->dir_path = p;
	DBG_OBJ(CXT, pc, ul_debug("new dir: '%s'", p));
	वापस 0;
}

स्थिर अक्षर *ul_path_get_dir(काष्ठा path_cxt *pc)
{
	वापस pc ? pc->dir_path : शून्य;
}

पूर्णांक ul_path_set_dialect(काष्ठा path_cxt *pc, व्योम *तथ्य, व्योम free_तथ्य(काष्ठा path_cxt *))
{
	pc->dialect = तथ्य;
	pc->free_dialect = free_तथ्य;
	DBG_OBJ(CXT, pc, ul_debug("(re)set dialect"));
	वापस 0;
}

व्योम *ul_path_get_dialect(काष्ठा path_cxt *pc)
{
	वापस pc ? pc->dialect : शून्य;
}

पूर्णांक ul_path_set_enoent_redirect(काष्ठा path_cxt *pc, पूर्णांक (*func)(काष्ठा path_cxt *, स्थिर अक्षर *, पूर्णांक *))
{
	pc->redirect_on_enoent = func;
	वापस 0;
}

अटल स्थिर अक्षर *get_absdir(काष्ठा path_cxt *pc)
{
	पूर्णांक rc;
	स्थिर अक्षर *dirpath;

	अगर (!pc->prefix)
		वापस pc->dir_path;

	dirpath = pc->dir_path;
	अगर (!dirpath)
		वापस pc->prefix;
	अगर (*dirpath == '/')
		dirpath++;

	rc = snprintf(pc->path_बफर, माप(pc->path_बफर), "%s/%s", pc->prefix, dirpath);
	अगर (rc < 0)
		वापस शून्य;
	अगर ((माप_प्रकार)rc >= माप(pc->path_बफर)) {
		त्रुटि_सं = ENAMETOOLONG;
		वापस शून्य;
	}

	वापस pc->path_बफर;
}

पूर्णांक ul_path_is_accessible(काष्ठा path_cxt *pc)
{
	स्थिर अक्षर *path;
	निश्चित(pc);

	अगर (pc->dir_fd >= 0)
		वापस 1;

	path = get_absdir(pc);
	अगर (!path)
		वापस 0;
	वापस access(path, F_OK) == 0;
}

पूर्णांक ul_path_get_dirfd(काष्ठा path_cxt *pc)
{
	निश्चित(pc);
	निश्चित(pc->dir_path);

	अगर (pc->dir_fd < 0) {
		स्थिर अक्षर *path = get_absdir(pc);
		अगर (!path)
			वापस -त्रुटि_सं;

		DBG_OBJ(CXT, pc, ul_debug("opening dir: '%s'", path));
		pc->dir_fd = खोलो(path, O_RDONLY|O_CLOEXEC);
	}

	वापस pc->dir_fd;
}

/* Note that next ul_path_get_dirfd() will reopen the directory */
व्योम ul_path_close_dirfd(काष्ठा path_cxt *pc)
{
	निश्चित(pc);

	अगर (pc->dir_fd >= 0) {
		DBG_OBJ(CXT, pc, ul_debug("closing dir"));
		बंद(pc->dir_fd);
		pc->dir_fd = -1;
	}
}

पूर्णांक ul_path_isopen_dirfd(काष्ठा path_cxt *pc)
{
	वापस pc && pc->dir_fd >= 0;
}

अटल स्थिर अक्षर *ul_path_mkpath(काष्ठा path_cxt *pc, स्थिर अक्षर *path, बहु_सूची ap)
{
	पूर्णांक rc;

	त्रुटि_सं = 0;

	rc = vsnprintf(pc->path_बफर, माप(pc->path_बफर), path, ap);
	अगर (rc < 0) {
		अगर (!त्रुटि_सं)
			त्रुटि_सं = EINVAL;
		वापस शून्य;
	}

	अगर ((माप_प्रकार)rc >= माप(pc->path_बफर)) {
		त्रुटि_सं = ENAMETOOLONG;
		वापस शून्य;
	}

	वापस pc->path_बफर;
}

अक्षर *ul_path_get_abspath(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz, स्थिर अक्षर *path, ...)
{
	अगर (path) {
		पूर्णांक rc;
		बहु_सूची ap;
		स्थिर अक्षर *पुच्छ = शून्य, *dirpath = pc->dir_path;

		बहु_शुरू(ap, path);
		पुच्छ = ul_path_mkpath(pc, path, ap);
		बहु_पूर्ण(ap);

		अगर (dirpath && *dirpath == '/')
			dirpath++;
		अगर (पुच्छ && *पुच्छ == '/')
			पुच्छ++;

		rc = snprintf(buf, bufsz, "%s/%s/%s",
				pc->prefix ? pc->prefix : "",
				dirpath ? dirpath : "",
				पुच्छ ? पुच्छ : "");

		अगर ((माप_प्रकार)rc >= bufsz) {
			त्रुटि_सं = ENAMETOOLONG;
			वापस शून्य;
		}
	} अन्यथा {
		स्थिर अक्षर *tmp = get_absdir(pc);

		अगर (!tmp)
			वापस शून्य;
		xstrncpy(buf, tmp, bufsz);
	}

	वापस buf;
}


पूर्णांक ul_path_access(काष्ठा path_cxt *pc, पूर्णांक mode, स्थिर अक्षर *path)
{
	पूर्णांक rc;

	अगर (!path)
		वापस -EINVAL;
	अगर (!pc) {
		rc = access(path, mode);
		DBG(CXT, ul_debug("access '%s' [no context, rc=%d]", path, rc));
	} अन्यथा {
		पूर्णांक dir = ul_path_get_dirfd(pc);
		अगर (dir < 0)
			वापस dir;
		अगर (*path == '/')
			path++;

		rc = faccessat(dir, path, mode, 0);

		अगर (rc && त्रुटि_सं == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = faccessat(dir, path, mode, 0);

		DBG_OBJ(CXT, pc, ul_debug("access: '%s' [rc=%d]", path, rc));
	}
	वापस rc;
}

पूर्णांक ul_path_accessf(काष्ठा path_cxt *pc, पूर्णांक mode, स्थिर अक्षर *path, ...)
{
	बहु_सूची ap;
	स्थिर अक्षर *p;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	वापस !p ? -त्रुटि_सं : ul_path_access(pc, mode, p);
}

/*
 * If @path is NULL, then stat() is called for the directory itself addressed by @pc.
 */
पूर्णांक ul_path_stat(काष्ठा path_cxt *pc, काष्ठा stat *sb, पूर्णांक flags, स्थिर अक्षर *path)
{
	पूर्णांक rc;

	अगर (!pc) {
		rc = path ? stat(path, sb) : -EINVAL;
		DBG(CXT, ul_debug("stat '%s' [no context, rc=%d]", path, rc));
	} अन्यथा {
		पूर्णांक dir = ul_path_get_dirfd(pc);
		अगर (dir < 0)
			वापस dir;
		अगर (path) {
			अगर  (*path == '/')
				path++;
			rc = fstatat(dir, path, sb, flags);

		} अन्यथा
			rc = ख_स्थिति(dir, sb);	/* dir itself */

		अगर (rc && त्रुटि_सं == ENOENT
		    && path
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			rc = fstatat(dir, path, sb, 0);

		DBG_OBJ(CXT, pc, ul_debug("stat '%s' [rc=%d]", path, rc));
	}
	वापस rc;
}

पूर्णांक ul_path_vstatf(काष्ठा path_cxt *pc, काष्ठा stat *sb, पूर्णांक flags, स्थिर अक्षर *path, बहु_सूची ap)
{
	स्थिर अक्षर *p = ul_path_mkpath(pc, path, ap);

	वापस !p ? -त्रुटि_सं : ul_path_stat(pc, sb, flags, p);
}

पूर्णांक ul_path_statf(काष्ठा path_cxt *pc, काष्ठा stat *sb, पूर्णांक flags, स्थिर अक्षर *path, ...)
{
	बहु_सूची ap;
	पूर्णांक rc;

	बहु_शुरू(ap, path);
	rc = ul_path_vstatf(pc, sb, flags, path, ap);
	बहु_पूर्ण(ap);

	वापस rc;
}
#अगर_घोषित HAVE_STATX
/*
* This function follows the semantics of statx(). To call statx() for the directory
* itself addressed by @pc, use an empty string and the AT_EMPTY_PATH @flag.
*/
पूर्णांक ul_path_statx(काष्ठा path_cxt *pc, काष्ठा statx *stx, पूर्णांक flags, अचिन्हित पूर्णांक mask,
		  स्थिर अक्षर *path)
{
	पूर्णांक rc;

	अगर (!path)
		वापस -EINVAL;
	अगर (!pc)
		rc = path ? statx(AT_FDCWD, path, flags, mask, stx) : - EINVAL;
	अन्यथा {
		पूर्णांक dir = ul_path_get_dirfd(pc);
		अगर (dir < 0)
			वापस dir;
		अगर (*path == '/')
			path++;

		rc = statx(dir, path, flags, mask, stx);
	}

	वापस rc;
}
#अन्यथा
पूर्णांक ul_path_statx(काष्ठा path_cxt *pc __attribute__((__unused__)),
		  काष्ठा statx *stx __attribute__((__unused__)),
		  पूर्णांक flags __attribute__((__unused__)),
		  अचिन्हित पूर्णांक mask __attribute__((__unused__)),
		  स्थिर अक्षर *path __attribute__((__unused__)))
{
	त्रुटि_सं = ENOSYS;
	वापस -1;
}
#पूर्ण_अगर /* HAVE_STATX */

पूर्णांक ul_path_vstatxf(काष्ठा path_cxt *pc, काष्ठा statx *stx, पूर्णांक flags, अचिन्हित पूर्णांक mask,
		    स्थिर अक्षर *path, बहु_सूची ap)
{
	स्थिर अक्षर *p = ul_path_mkpath(pc, path, ap);

	वापस !p ? -त्रुटि_सं : ul_path_statx(pc, stx, flags, mask, p);
}

पूर्णांक ul_path_statxf(काष्ठा path_cxt *pc, काष्ठा statx *stx, पूर्णांक flags, अचिन्हित पूर्णांक mask,
		   स्थिर अक्षर *path, ...)
{
	बहु_सूची ap;
	पूर्णांक rc;

	बहु_शुरू(ap, path);
	rc = ul_path_vstatxf(pc, stx, flags, mask, path, ap);
	बहु_पूर्ण(ap);

	वापस rc;
}

पूर्णांक ul_path_open(काष्ठा path_cxt *pc, पूर्णांक flags, स्थिर अक्षर *path)
{
	पूर्णांक fd;

	अगर (!path)
		वापस -EINVAL;
	अगर (!pc) {
		fd = खोलो(path, flags);
		DBG(CXT, ul_debug("opening '%s' [no context]", path));
	} अन्यथा {
		पूर्णांक fdx;
		पूर्णांक dir = ul_path_get_dirfd(pc);
		अगर (dir < 0)
			वापस dir;

		अगर (*path == '/')
			path++;

		fdx = fd = openat(dir, path, flags);

		अगर (fd < 0 && त्रुटि_सं == ENOENT
		    && pc->redirect_on_enoent
		    && pc->redirect_on_enoent(pc, path, &dir) == 0)
			fd = openat(dir, path, flags);

		DBG_OBJ(CXT, pc, ul_debug("opening '%s'%s", path, fdx != fd ? " [redirected]" : ""));
	}
	वापस fd;
}

पूर्णांक ul_path_vopenf(काष्ठा path_cxt *pc, पूर्णांक flags, स्थिर अक्षर *path, बहु_सूची ap)
{
	स्थिर अक्षर *p = ul_path_mkpath(pc, path, ap);

	वापस !p ? -त्रुटि_सं : ul_path_open(pc, flags, p);
}

पूर्णांक ul_path_openf(काष्ठा path_cxt *pc, पूर्णांक flags, स्थिर अक्षर *path, ...)
{
	बहु_सूची ap;
	पूर्णांक rc;

	बहु_शुरू(ap, path);
	rc = ul_path_vopenf(pc, flags, path, ap);
	बहु_पूर्ण(ap);

	वापस rc;
}

/*
 * Maybe stupid, but good enough ;-)
 */
खाता *ul_path_fopen(काष्ठा path_cxt *pc, स्थिर अक्षर *mode, स्थिर अक्षर *path)
{
	पूर्णांक flags = ul_mode_to_flags(mode);
	पूर्णांक fd = ul_path_open(pc, flags, path);

	अगर (fd < 0)
		वापस शून्य;

	वापस ul_vfs_fdopen(pc ? pc->vfs : शून्य, fd, mode);
}


खाता *ul_path_vfopenf(काष्ठा path_cxt *pc, स्थिर अक्षर *mode, स्थिर अक्षर *path, बहु_सूची ap)
{
	स्थिर अक्षर *p = ul_path_mkpath(pc, path, ap);

	वापस !p ? शून्य : ul_path_fopen(pc, mode, p);
}

खाता *ul_path_fopenf(काष्ठा path_cxt *pc, स्थिर अक्षर *mode, स्थिर अक्षर *path, ...)
{
	खाता *f;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	f = ul_path_vfopenf(pc, mode, path, ap);
	बहु_पूर्ण(ap);

	वापस f;
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
सूची *ul_path_opendir(काष्ठा path_cxt *pc, स्थिर अक्षर *path)
{
	सूची *dir;
	पूर्णांक fd = -1;

	अगर (path)
		fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	अन्यथा अगर (pc->dir_path) {
		पूर्णांक dirfd;

		DBG_OBJ(CXT, pc, ul_debug("duplicate dir path"));
		dirfd = ul_path_get_dirfd(pc);
		अगर (dirfd >= 0)
			fd = dup_fd_cloexec(dirfd, STDERR_FILENO + 1);
	}

	अगर (fd < 0)
		वापस शून्य;

	dir = fdopendir(fd);
	अगर (!dir) {
		बंद(fd);
		वापस शून्य;
	}
	अगर (!path)
		 सूची_शुरु(dir);
	वापस dir;
}


/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
सूची *ul_path_vopendirf(काष्ठा path_cxt *pc, स्थिर अक्षर *path, बहु_सूची ap)
{
	स्थिर अक्षर *p = ul_path_mkpath(pc, path, ap);

	वापस !p ? शून्य : ul_path_opendir(pc, p);
}

/*
 * Open directory @path in read-only mode. If the path is NULL then duplicate FD
 * to the directory addressed by @pc.
 */
सूची *ul_path_opendirf(काष्ठा path_cxt *pc, स्थिर अक्षर *path, ...)
{
	बहु_सूची ap;
	सूची *dir;

	बहु_शुरू(ap, path);
	dir = ul_path_vopendirf(pc, path, ap);
	बहु_पूर्ण(ap);

	वापस dir;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlink(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार bufsiz, स्थिर अक्षर *path)
{
	पूर्णांक dirfd;
	ssize_t ssz;

	अगर (!path) {
		स्थिर अक्षर *p = get_absdir(pc);
		अगर (!p)
			वापस -त्रुटि_सं;
		ssz = readlink(p, buf, bufsiz - 1);
	} अन्यथा {
		dirfd = ul_path_get_dirfd(pc);
		अगर (dirfd < 0)
			वापस dirfd;

		अगर (*path == '/')
			path++;

		ssz = readlinkat(dirfd, path, buf, bufsiz - 1);
	}

	अगर (ssz >= 0)
		buf[ssz] = '\0';
	वापस ssz;
}

/*
 * If @path is NULL then readlink is called on @pc directory.
 */
ssize_t ul_path_readlinkf(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार bufsiz, स्थिर अक्षर *path, ...)
{
	स्थिर अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	वापस !p ? -त्रुटि_सं : ul_path_readlink(pc, buf, bufsiz, p);
}

पूर्णांक ul_path_read(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार len, स्थिर अक्षर *path)
{
	पूर्णांक rc, errsv;
	पूर्णांक fd;

	fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, path);
	अगर (fd < 0)
		वापस -त्रुटि_सं;

	DBG(CXT, ul_debug(" reading '%s'", path));
	rc = ul_vfs_read_all(pc ? pc->vfs : शून्य, fd, buf, len);

	errsv = त्रुटि_सं;
	ul_vfs_close(pc ? pc->vfs : शून्य, fd);
	त्रुटि_सं = errsv;
	वापस rc;
}

पूर्णांक ul_path_vreadf(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार len, स्थिर अक्षर *path, बहु_सूची ap)
{
	स्थिर अक्षर *p = ul_path_mkpath(pc, path, ap);

	वापस !p ? -त्रुटि_सं : ul_path_read(pc, buf, len, p);
}

पूर्णांक ul_path_readf(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार len, स्थिर अक्षर *path, ...)
{
	बहु_सूची ap;
	पूर्णांक rc;

	बहु_शुरू(ap, path);
	rc = ul_path_vreadf(pc, buf, len, path, ap);
	बहु_पूर्ण(ap);

	वापस rc;
}


/*
 * Returns newly allocated buffer with data from file. Maximal size is BUFSIZ
 * (send patch if you need something bigger;-)
 *
 * Returns size of the string without \0, nothing is allocated if returns <= 0.
 */
पूर्णांक ul_path_read_string(काष्ठा path_cxt *pc, अक्षर **str, स्थिर अक्षर *path)
{
	अक्षर buf[बफ_मान];
	पूर्णांक rc;

	अगर (!str)
		वापस -EINVAL;

	*str = शून्य;

	rc = ul_path_read_बफर(pc, buf, माप(buf), path);
	अगर (rc < 0)
		वापस rc;

	*str = strdup(buf);
	अगर (!*str)
		rc = -ENOMEM;

	वापस rc;
}

पूर्णांक ul_path_readf_string(काष्ठा path_cxt *pc, अक्षर **str, स्थिर अक्षर *path, ...)
{
	स्थिर अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	वापस !p ? -त्रुटि_सं : ul_path_read_string(pc, str, p);
}

पूर्णांक ul_path_read_बफर(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz, स्थिर अक्षर *path)
{
	पूर्णांक rc = ul_path_read(pc, buf, bufsz - 1, path);

	अगर (rc == 0)
		buf[0] = '\0';

	अन्यथा अगर (rc > 0) {
		/* Remove trailing newline (usual in sysfs) */
		अगर (*(buf + rc - 1) == '\n')
			buf[--rc] = '\0';
		अन्यथा
			buf[rc] = '\0';
	}

	वापस rc;
}

पूर्णांक ul_path_vreadf_बफर(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz, स्थिर अक्षर *path, बहु_सूची ap)
{
	स्थिर अक्षर *p;

	p = ul_path_mkpath(pc, path, ap);

	वापस !p ? -त्रुटि_सं : ul_path_read_बफर(pc, buf, bufsz, p);
}

पूर्णांक ul_path_readf_बफर(काष्ठा path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz, स्थिर अक्षर *path, ...)
{
	बहु_सूची ap;
	पूर्णांक rc;

	बहु_शुरू(ap, path);
	rc = ul_path_vreadf_बफर(pc, buf, bufsz, path, ap);
	बहु_पूर्ण(ap);

	वापस rc;
}

पूर्णांक ul_path_scanf(काष्ठा path_cxt *pc, स्थिर अक्षर *path, स्थिर अक्षर *fmt, ...)
{
	खाता *f;
	बहु_सूची fmt_ap;
	पूर्णांक rc;

	f = ul_path_fopen(pc, "r" UL_CLOEXECSTR, path);
	अगर (!f)
		वापस -EINVAL;

	DBG(CXT, ul_debug(" fscanf [%s] '%s'", fmt, path));

	बहु_शुरू(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	बहु_पूर्ण(fmt_ap);

	ख_बंद(f);
	वापस rc;
}

पूर्णांक ul_path_scanff(काष्ठा path_cxt *pc, स्थिर अक्षर *path, बहु_सूची ap, स्थिर अक्षर *fmt, ...)
{
	खाता *f;
	बहु_सूची fmt_ap;
	पूर्णांक rc;

	f = ul_path_vfopenf(pc, "r" UL_CLOEXECSTR, path, ap);
	अगर (!f)
		वापस -EINVAL;

	बहु_शुरू(fmt_ap, fmt);
	rc = vfscanf(f, fmt, fmt_ap);
	बहु_पूर्ण(fmt_ap);

	ख_बंद(f);
	वापस rc;
}


पूर्णांक ul_path_read_s64(काष्ठा path_cxt *pc, int64_t *res, स्थिर अक्षर *path)
{
	int64_t x = 0;
	पूर्णांक rc;

	rc = ul_path_scanf(pc, path, "%"SCNd64, &x);
	अगर (rc != 1)
		वापस -1;
	अगर (res)
		*res = x;
	वापस 0;
}

पूर्णांक ul_path_readf_s64(काष्ठा path_cxt *pc, int64_t *res, स्थिर अक्षर *path, ...)
{
	स्थिर अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	वापस !p ? -त्रुटि_सं : ul_path_read_s64(pc, res, p);
}

पूर्णांक ul_path_read_u64(काष्ठा path_cxt *pc, uint64_t *res, स्थिर अक्षर *path)
{
	uint64_t x = 0;
	पूर्णांक rc;

	rc = ul_path_scanf(pc, path, "%"SCNu64, &x);
	अगर (rc != 1)
		वापस -1;
	अगर (res)
		*res = x;
	वापस 0;
}

पूर्णांक ul_path_readf_u64(काष्ठा path_cxt *pc, uint64_t *res, स्थिर अक्षर *path, ...)
{
	स्थिर अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	वापस !p ? -त्रुटि_सं : ul_path_read_u64(pc, res, p);
}

पूर्णांक ul_path_read_s32(काष्ठा path_cxt *pc, पूर्णांक *res, स्थिर अक्षर *path)
{
	पूर्णांक rc, x = 0;

	rc = ul_path_scanf(pc, path, "%d", &x);
	अगर (rc != 1)
		वापस -1;
	अगर (res)
		*res = x;
	वापस 0;
}

पूर्णांक ul_path_readf_s32(काष्ठा path_cxt *pc, पूर्णांक *res, स्थिर अक्षर *path, ...)
{
	स्थिर अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	वापस !p ? -त्रुटि_सं : ul_path_read_s32(pc, res, p);
}

पूर्णांक ul_path_read_u32(काष्ठा path_cxt *pc, अचिन्हित पूर्णांक *res, स्थिर अक्षर *path)
{
	पूर्णांक rc;
	अचिन्हित पूर्णांक x = 0;

	rc = ul_path_scanf(pc, path, "%u", &x);
	अगर (rc != 1)
		वापस -1;
	अगर (res)
		*res = x;
	वापस 0;
}

पूर्णांक ul_path_readf_u32(काष्ठा path_cxt *pc, अचिन्हित पूर्णांक *res, स्थिर अक्षर *path, ...)
{
	स्थिर अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	वापस !p ? -त्रुटि_सं : ul_path_read_u32(pc, res, p);
}

पूर्णांक ul_path_read_majmin(काष्ठा path_cxt *pc, dev_t *res, स्थिर अक्षर *path)
{
	पूर्णांक rc, maj = 0, min = 0;

	rc = ul_path_scanf(pc, path, "%d:%d", &maj, &min);
	अगर (rc != 2)
		वापस -1;
	अगर (res)
		*res = makedev(maj, min);
	वापस 0;
}

पूर्णांक ul_path_readf_majmin(काष्ठा path_cxt *pc, dev_t *res, स्थिर अक्षर *path, ...)
{
	स्थिर अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	वापस !p ? -त्रुटि_सं : ul_path_read_majmin(pc, res, p);
}

पूर्णांक ul_path_write_string(काष्ठा path_cxt *pc, स्थिर अक्षर *str, स्थिर अक्षर *path)
{
	पूर्णांक rc, errsv;
	पूर्णांक fd;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	अगर (fd < 0)
		वापस -त्रुटि_सं;

	rc = ul_vfs_write_all(pc ? pc->vfs : शून्य, fd, str, म_माप(str));

	errsv = त्रुटि_सं;
	ul_vfs_close(pc ? pc->vfs : शून्य, fd);
	त्रुटि_सं = errsv;
	वापस rc;
}

पूर्णांक ul_path_writef_string(काष्ठा path_cxt *pc, स्थिर अक्षर *str, स्थिर अक्षर *path, ...)
{
	स्थिर अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	वापस !p ? -त्रुटि_सं : ul_path_write_string(pc, str, p);
}

पूर्णांक ul_path_write_s64(काष्ठा path_cxt *pc, int64_t अंक, स्थिर अक्षर *path)
{
	अक्षर buf[माप(stringify_मूल्य(LLONG_MAX))];
	पूर्णांक rc, errsv;
	पूर्णांक fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	अगर (fd < 0)
		वापस -त्रुटि_सं;

	len = snprintf(buf, माप(buf), "%" PRId64, अंक);
	अगर (len < 0 || (माप_प्रकार) len >= माप(buf))
		rc = len < 0 ? -त्रुटि_सं : -E2BIG;
	अन्यथा
		rc = ul_vfs_write_all(pc ? pc->vfs : शून्य, fd, buf, len);

	errsv = त्रुटि_सं;
	ul_vfs_close(pc ? pc->vfs : शून्य, fd);
	त्रुटि_सं = errsv;
	वापस rc;
}

पूर्णांक ul_path_write_u64(काष्ठा path_cxt *pc, uint64_t अंक, स्थिर अक्षर *path)
{
	अक्षर buf[माप(stringify_मूल्य(ULLONG_MAX))];
	पूर्णांक rc, errsv;
	पूर्णांक fd, len;

	fd = ul_path_open(pc, O_WRONLY|O_CLOEXEC, path);
	अगर (fd < 0)
		वापस -त्रुटि_सं;

	len = snprintf(buf, माप(buf), "%" PRIu64, अंक);
	अगर (len < 0 || (माप_प्रकार) len >= माप(buf))
		rc = len < 0 ? -त्रुटि_सं : -E2BIG;
	अन्यथा
		rc = ul_vfs_write_all(pc ? pc->vfs : शून्य, fd, buf, len);

	errsv = त्रुटि_सं;
	ul_vfs_close(pc ? pc->vfs : शून्य, fd);
	त्रुटि_सं = errsv;
	वापस rc;
}

पूर्णांक ul_path_writef_u64(काष्ठा path_cxt *pc, uint64_t अंक, स्थिर अक्षर *path, ...)
{
	स्थिर अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	वापस !p ? -त्रुटि_सं : ul_path_write_u64(pc, अंक, p);

}

पूर्णांक ul_path_गिनती_dirents(काष्ठा path_cxt *pc, स्थिर अक्षर *path)
{
	सूची *dir;
	पूर्णांक r = 0;

	dir = ul_path_opendir(pc, path);
	अगर (!dir)
		वापस 0;

	जबतक (xreaddir(dir)) r++;

	बंद_सूची(dir);
	वापस r;
}

पूर्णांक ul_path_countf_dirents(काष्ठा path_cxt *pc, स्थिर अक्षर *path, ...)
{
	स्थिर अक्षर *p;
	बहु_सूची ap;

	बहु_शुरू(ap, path);
	p = ul_path_mkpath(pc, path, ap);
	बहु_पूर्ण(ap);

	वापस !p ? -त्रुटि_सं : ul_path_गिनती_dirents(pc, p);
}

/* first call (when @sub is NULL) opens the directory, last call closes the directory */
पूर्णांक ul_path_अग्र_dirent(काष्ठा path_cxt *pc, सूची **sub, स्थिर अक्षर *स_नाम, काष्ठा dirent **d)
{
	अगर (!pc || !sub || !d)
		वापस -EINVAL;

	अगर (!*sub) {
		*sub = ul_path_opendir(pc, स_नाम);
		अगर (!*sub)
			वापस -त्रुटि_सं;
	}

	*d = xreaddir(*sub);
	अगर (*d)
		वापस 0;

	बंद_सूची(*sub);
	*sub = शून्य;
	वापस 1;
}

#अगर_घोषित HAVE_CPU_SET_T
अटल पूर्णांक ul_path_cpuparse(काष्ठा path_cxt *pc, cpu_set_t **set, पूर्णांक maxcpus, पूर्णांक islist, स्थिर अक्षर *path, बहु_सूची ap)
{
	माप_प्रकार setsize, len = maxcpus * 7;
	अक्षर *buf;
	पूर्णांक rc;

	*set = शून्य;

	buf = दो_स्मृति(len);
	अगर (!buf)
		वापस -ENOMEM;

	rc = ul_path_vreadf_बफर(pc, buf, len, path, ap);
	अगर (rc < 0)
		जाओ out;

	*set = cpuset_alloc(maxcpus, &setsize, शून्य);
	अगर (!*set) {
		rc = -EINVAL;
		जाओ out;
	}

	अगर (islist) {
		अगर (cpulist_parse(buf, *set, setsize, 0)) {
			त्रुटि_सं = EINVAL;
			rc = -त्रुटि_सं;
			जाओ out;
		}
	} अन्यथा {
		अगर (cpumask_parse(buf, *set, setsize)) {
			त्रुटि_सं = EINVAL;
			rc = -त्रुटि_सं;
			जाओ out;
		}
	}
	rc = 0;

out:
	अगर (rc) {
		cpuset_free(*set);
		*set = शून्य;
	}
	मुक्त(buf);
	वापस rc;
}

पूर्णांक ul_path_readf_cpuset(काष्ठा path_cxt *pc, cpu_set_t **set, पूर्णांक maxcpus, स्थिर अक्षर *path, ...)
{
	बहु_सूची ap;
	पूर्णांक rc = 0;

	बहु_शुरू(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 0, path, ap);
	बहु_पूर्ण(ap);

	वापस rc;
}

पूर्णांक ul_path_readf_cpulist(काष्ठा path_cxt *pc, cpu_set_t **set, पूर्णांक maxcpus, स्थिर अक्षर *path, ...)
{
	बहु_सूची ap;
	पूर्णांक rc = 0;

	बहु_शुरू(ap, path);
	rc = ul_path_cpuparse(pc, set, maxcpus, 1, path, ap);
	बहु_पूर्ण(ap);

	वापस rc;
}

#पूर्ण_अगर /* HAVE_CPU_SET_T */


#अगर_घोषित TEST_PROGRAM_PATH
#समावेश <getopt.h>

अटल व्योम __attribute__((__noreturn__)) usage(व्योम)
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

पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	पूर्णांक c;
	स्थिर अक्षर *prefix = शून्य, *dir, *file, *command;
	काष्ठा path_cxt *pc = शून्य;

	अटल स्थिर काष्ठा option longopts[] = {
		{ "prefix",	1, शून्य, 'p' },
		{ "help",       0, शून्य, 'h' },
		{ शून्य, 0, शून्य, 0 },
	};

	जबतक((c = getopt_long(argc, argv, "p:h", longopts, शून्य)) != -1) {
		चयन(c) {
		हाल 'p':
			prefix = optarg;
			अवरोध;
		हाल 'h':
			usage();
			अवरोध;
		शेष:
			err(निकास_त्रुटि, "try --help");
		}
	}

	अगर (optind == argc)
		errx(निकास_त्रुटि, "<dir> not defined");
	dir = argv[optind++];

	ul_path_init_debug();

	pc = ul_new_path("%s", dir);
	अगर (!pc)
		err(निकास_त्रुटि, "failed to initialize path context");
	अगर (prefix)
		ul_path_set_prefix(pc, prefix);

	अगर (optind == argc)
		errx(निकास_त्रुटि, "<command> not defined");
	command = argv[optind++];

	अगर (म_भेद(command, "read-u32") == 0) {
		uint32_t res;

		अगर (optind == argc)
			errx(निकास_त्रुटि, "<file> not defined");
		file = argv[optind++];

		अगर (ul_path_read_u32(pc, &res, file) != 0)
			err(निकास_त्रुटि, "read u64 failed");
		म_लिखो("read:  %s: %u\n", file, res);

		अगर (ul_path_readf_u32(pc, &res, "%s", file) != 0)
			err(निकास_त्रुटि, "readf u64 failed");
		म_लिखो("readf: %s: %u\n", file, res);

	} अन्यथा अगर (म_भेद(command, "read-s32") == 0) {
		int32_t res;

		अगर (optind == argc)
			errx(निकास_त्रुटि, "<file> not defined");
		file = argv[optind++];

		अगर (ul_path_read_s32(pc, &res, file) != 0)
			err(निकास_त्रुटि, "read u64 failed");
		म_लिखो("read:  %s: %d\n", file, res);

		अगर (ul_path_readf_s32(pc, &res, "%s", file) != 0)
			err(निकास_त्रुटि, "readf u64 failed");
		म_लिखो("readf: %s: %d\n", file, res);

	} अन्यथा अगर (म_भेद(command, "read-u64") == 0) {
		uint64_t res;

		अगर (optind == argc)
			errx(निकास_त्रुटि, "<file> not defined");
		file = argv[optind++];

		अगर (ul_path_read_u64(pc, &res, file) != 0)
			err(निकास_त्रुटि, "read u64 failed");
		म_लिखो("read:  %s: %" PRIu64 "\n", file, res);

		अगर (ul_path_readf_u64(pc, &res, "%s", file) != 0)
			err(निकास_त्रुटि, "readf u64 failed");
		म_लिखो("readf: %s: %" PRIu64 "\n", file, res);

	} अन्यथा अगर (म_भेद(command, "read-s64") == 0) {
		int64_t res;

		अगर (optind == argc)
			errx(निकास_त्रुटि, "<file> not defined");
		file = argv[optind++];

		अगर (ul_path_read_s64(pc, &res, file) != 0)
			err(निकास_त्रुटि, "read u64 failed");
		म_लिखो("read:  %s: %" PRId64 "\n", file, res);

		अगर (ul_path_readf_s64(pc, &res, "%s", file) != 0)
			err(निकास_त्रुटि, "readf u64 failed");
		म_लिखो("readf: %s: %" PRId64 "\n", file, res);

	} अन्यथा अगर (म_भेद(command, "read-majmin") == 0) {
		dev_t res;

		अगर (optind == argc)
			errx(निकास_त्रुटि, "<file> not defined");
		file = argv[optind++];

		अगर (ul_path_read_majmin(pc, &res, file) != 0)
			err(निकास_त्रुटि, "read maj:min failed");
		म_लिखो("read:  %s: %d\n", file, (पूर्णांक) res);

		अगर (ul_path_readf_majmin(pc, &res, "%s", file) != 0)
			err(निकास_त्रुटि, "readf maj:min failed");
		म_लिखो("readf: %s: %d\n", file, (पूर्णांक) res);

	} अन्यथा अगर (म_भेद(command, "read-string") == 0) {
		अक्षर *res;

		अगर (optind == argc)
			errx(निकास_त्रुटि, "<file> not defined");
		file = argv[optind++];

		अगर (ul_path_read_string(pc, &res, file) <= 0)
			err(निकास_त्रुटि, "read string failed");
		म_लिखो("read:  %s: %s\n", file, res);

		अगर (ul_path_readf_string(pc, &res, "%s", file) <= 0)
			err(निकास_त्रुटि, "readf string failed");
		म_लिखो("readf: %s: %s\n", file, res);

	} अन्यथा अगर (म_भेद(command, "read-link") == 0) {
		अक्षर res[PATH_MAX];

		अगर (optind == argc)
			errx(निकास_त्रुटि, "<file> not defined");
		file = argv[optind++];

		अगर (ul_path_readlink(pc, res, माप(res), file) < 0)
			err(निकास_त्रुटि, "read symlink failed");
		म_लिखो("read:  %s: %s\n", file, res);

		अगर (ul_path_readlinkf(pc, res, माप(res), "%s", file) < 0)
			err(निकास_त्रुटि, "readf symlink failed");
		म_लिखो("readf: %s: %s\n", file, res);

	} अन्यथा अगर (म_भेद(command, "write-string") == 0) {
		अक्षर *str;

		अगर (optind + 1 == argc)
			errx(निकास_त्रुटि, "<file> <string> not defined");
		file = argv[optind++];
		str = argv[optind++];

		अगर (ul_path_write_string(pc, str, file) != 0)
			err(निकास_त्रुटि, "write string failed");
		अगर (ul_path_writef_string(pc, str, "%s", file) != 0)
			err(निकास_त्रुटि, "writef string failed");

	} अन्यथा अगर (म_भेद(command, "write-u64") == 0) {
		uint64_t अंक;

		अगर (optind + 1 == argc)
			errx(निकास_त्रुटि, "<file> <num> not defined");
		file = argv[optind++];
		अंक = strtoumax(argv[optind++], शून्य, 0);

		अगर (ul_path_write_u64(pc, अंक, file) != 0)
			err(निकास_त्रुटि, "write u64 failed");
		अगर (ul_path_writef_u64(pc, अंक, "%s", file) != 0)
			err(निकास_त्रुटि, "writef u64 failed");
	}

	ul_unref_path(pc);
	वापस निकास_सफल;
}
#पूर्ण_अगर /* TEST_PROGRAM_PATH */
