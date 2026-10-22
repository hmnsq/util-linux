/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "pathnames.h"
#ಅನ್ತರ್ಭಾವ "procfs.h"
#ಅನ್ತರ್ಭಾವ "fileutils.h"
#ಅನ್ತರ್ಭಾವ "all-io.h"
#ಅನ್ತರ್ಭಾವ "vfs.h"
#ಅನ್ತರ್ಭಾವ "debug.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "statfs_magic.h"

ಸ್ಥಿರ ಶೂನ್ಯ procfs_process_deinit_path(ಸಂರಚನಾ path_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
ಸ್ಥಿರ UL_DEBUG_DEFINE_MASK(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPROCFS_DEBUG_INIT	(1 << 1)
#define ULPROCFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

ಶೂನ್ಯ ul_procfs_init_debug(ಶೂನ್ಯ)
{
	ಯದಿ (ulprocfs_debug_mask)
		ಪ್ರತ್ಯಾಗಮ;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

ಸಂರಚನಾ path_cxt *ul_new_procfs_path(pid_t pid, ಧ್ರುವ ವರ್ಣ *prefix)
{
	ಸಂರಚನಾ path_cxt *pc = ul_new_path(ರಿಕ್ತ);

	ಯದಿ (!pc)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಯದಿ (prefix)
		ul_path_set_prefix(pc, prefix);

	ಯದಿ (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ಪ್ರತ್ಯಾಗಮ pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
ಪೂರ್ಣಾಙ್ಕ procfs_process_init_path(ಸಂರಚನಾ path_cxt *pc, pid_t pid)
{
	ಸಂರಚನಾ procfs_process *prc;
	ಪೂರ್ಣಾಙ್ಕ rc;
	ವರ್ಣ buf[ಪರಿಮಾಣ(_PATH_PROC) + ಪರಿಮಾಣ(stringify_ಮೌಲ್ಯ(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, ಪರಿಮಾಣ(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	ಯದಿ (rc)
		ಪ್ರತ್ಯಾಗಮ rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	ಯದಿ (rc < 0)
		ಪ್ರತ್ಯಾಗಮ rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	ಯದಿ (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, ಪರಿಮಾಣ(ಸಂರಚನಾ procfs_process));
		ಯದಿ (!prc)
			ಪ್ರತ್ಯಾಗಮ -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ procfs_process_deinit_path(ಸಂರಚನಾ path_cxt *pc)
{
	ಸಂರಚನಾ procfs_process *prc;

	ಯದಿ (!pc)
		ಪ್ರತ್ಯಾಗಮ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	ಯದಿ (!prc)
		ಪ್ರತ್ಯಾಗಮ;

	ಮುಕ್ತ(prc);
	ul_path_set_dialect(pc, ರಿಕ್ತ, ರಿಕ್ತ);
}

ಸ್ಥಿರ ssize_t read_procfs_file(ಧ್ರುವ ಸಂರಚನಾ ul_vfs_ops *vfs, ಪೂರ್ಣಾಙ್ಕ fd,
			       ವರ್ಣ *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	ಯದಿ (sz <= 0)
		ಪ್ರತ್ಯಾಗಮ sz;

	ಪ್ರತಿ (i = 0; i < (size_t) sz; i++) {
		ಯದಿ (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	ಪ್ರತ್ಯಾಗಮ sz;
}

ಸ್ಥಿರ ssize_t procfs_process_get_data_for(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t bufsz,
					    ಧ್ರುವ ವರ್ಣ *fname)
{
	ಪೂರ್ಣಾಙ್ಕ fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	ಯದಿ (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : ರಿಕ್ತ, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : ರಿಕ್ತ, fd);
		ಪ್ರತ್ಯಾಗಮ sz;
	}
	ಪ್ರತ್ಯಾಗಮ -errno;
}

ssize_t procfs_process_get_cmdline(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t bufsz)
{
	ಪ್ರತ್ಯಾಗಮ procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t bufsz)
{
	ಪ್ರತ್ಯಾಗಮ procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t bufsz)
{
	ಪ್ರತ್ಯಾಗಮ procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(ಸಂರಚನಾ path_cxt *pc, ವರ್ಣ *buf, size_t bufsz)
{
	ಪ್ರತ್ಯಾಗಮ procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

ಪೂರ್ಣಾಙ್ಕ procfs_process_get_stat_nth(ಸಂರಚನಾ path_cxt *pc, ಪೂರ್ಣಾಙ್ಕ n, uint64_t *re)
{
	ssize_t rc;
	ವರ್ಣ *ಕೀಲಿ = ರಿಕ್ತ, *tok, *p;
	ವರ್ಣ buf[BUFSIZ];
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (n == 2 || n == 3)		/* process name and status (strings) */
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, ಪರಿಮಾಣ(buf), "stat");
	ಯದಿ (rc < 0)
		ಪ್ರತ್ಯಾಗಮ rc;

	ಪ್ರತಿ (i = 0, tok = strtok_r(buf, " ", &ಕೀಲಿ); tok;
	     tok = strtok_r(ರಿಕ್ತ, " ", &ಕೀಲಿ)) {

		i++;
		ಯದಿ (i == n)
			ಪ್ರತ್ಯಾಗಮ ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		ಯದಿ (i == 2 && (p = strrchr(ಕೀಲಿ, ')')))
			ಕೀಲಿ = p + 2;
	}

	ಪ್ರತ್ಯಾಗಮ -EINVAL;
}

ಪೂರ್ಣಾಙ್ಕ procfs_process_get_uid(ಸಂರಚನಾ path_cxt *pc, uid_t *uid)
{
	ಸಂರಚನಾ stat sb;
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ ((rc = ul_path_stat(pc, &sb, 0, ರಿಕ್ತ)) == 0)
		*uid = sb.st_uid;
	ಪ್ರತ್ಯಾಗಮ rc;
}

/*
 * returns the next task TID, the @sub is automatically initialized
 * when called first time and closed after last call or you can
 * call closedir()* when you need to break the loop.
 *
 * Returns: <0 on error, 0 on success, >1 done
 *
 * Example:
 *
 * pid_t tid;
 * DIR *sub = NULL;
 * path_cxt *pc = ul_new_procfs_path(123, NULL);
 *
 * while (procfs_process_next_tid(pc, &sub, &tid) == 0)
 *	printf("task: %d", (int) tid);
 *
 */
ಪೂರ್ಣಾಙ್ಕ procfs_process_next_tid(ಸಂರಚನಾ path_cxt *pc, DIR **sub, pid_t *tid)
{
	ಸಂರಚನಾ dirent *d;

	ಯದಿ (!pc || !sub || !tid)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ಯದಿ (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		ಯದಿ (!*sub)
			ಪ್ರತ್ಯಾಗಮ -errno;
	}

	ಯಾವತ್ ((d = xreaddir(*sub))) {
		ಯದಿ (procfs_dirent_get_pid(d, tid) == 0)
			ಪ್ರತ್ಯಾಗಮ 0;
	}

	closedir(*sub);
	*sub = ರಿಕ್ತ;
	ಪ್ರತ್ಯಾಗಮ 1;
}

ಪೂರ್ಣಾಙ್ಕ procfs_process_next_fd(ಸಂರಚನಾ path_cxt *pc, DIR **sub, ಪೂರ್ಣಾಙ್ಕ *fd)
{
	ಸಂರಚನಾ dirent *d;

	ಯದಿ (!pc || !sub || !fd)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ಯದಿ (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		ಯದಿ (!*sub)
			ಪ್ರತ್ಯಾಗಮ -errno;
	}

	ಯಾವತ್ ((d = xreaddir(*sub))) {
		uint64_t ಅಂಕೆ;
#ifdef _DIRENT_HAVE_D_TYPE
		ಯದಿ (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			ಅನುವರ್ತ;
#endif
		ಯದಿ (ul_strtou64(d->d_name, &ಅಂಕೆ, 10) < 0)
			ಅನುವರ್ತ;
		*fd = ಅಂಕೆ;
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	closedir(*sub);
	*sub = ರಿಕ್ತ;
	ಪ್ರತ್ಯಾಗಮ 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
ಪೂರ್ಣಾಙ್ಕ procfs_dirent_is_process(ಸಂರಚನಾ dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	ಯದಿ (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		ಪ್ರತ್ಯಾಗಮ 0;
#endif
	ಯದಿ (!isdigit((unsigned ವರ್ಣ) *d->d_name))
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ 1;
}

ಪೂರ್ಣಾಙ್ಕ procfs_dirent_get_pid(ಸಂರಚನಾ dirent *d, pid_t *pid)
{
	uint64_t ಅಂಕೆ;

	ಯದಿ (!procfs_dirent_is_process(d))
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ಯದಿ (ul_strtou64(d->d_name, &ಅಂಕೆ, 10) < 0)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	*pid = (pid_t) ಅಂಕೆ;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ procfs_dirent_get_uid(DIR *procfs, ಸಂರಚನಾ dirent *d, uid_t *uid)
{
	ಸಂರಚನಾ stat st;

	ಯದಿ (!procfs_dirent_is_process(d))
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	ಯದಿ (fstatat(dirfd(procfs), d->d_name, &st, 0))
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	*uid = st.st_uid;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ procfs_dirent_match_uid(DIR *procfs, ಸಂರಚನಾ dirent *d, uid_t uid)
{
	uid_t x;

	ಯದಿ (procfs_dirent_get_uid(procfs, d, &x) == 0)
		ಪ್ರತ್ಯಾಗಮ x == uid;

	ಪ್ರತ್ಯಾಗಮ 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
ಪೂರ್ಣಾಙ್ಕ procfs_dirent_get_name(DIR *procfs, ಸಂರಚನಾ dirent *d, ವರ್ಣ *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	ವರ್ಣ tmp[1024], *p, *end = ರಿಕ್ತ;

	ಯದಿ (bufsz < 32)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	ಯದಿ (!procfs_dirent_is_process(d))
		ಪ್ರತ್ಯಾಗಮ -EINVAL;

	snprintf(tmp, ಪರಿಮಾಣ(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	ಯದಿ (!f)
		ಪ್ರತ್ಯಾಗಮ -errno;

	p = fgets(tmp, ಪರಿಮಾಣ(tmp), f);
	fclose(f);
	ಯದಿ (!p)
		ಪ್ರತ್ಯಾಗಮ -errno;

	/* skip PID */
	ಯಾವತ್ (*p && *p != '(')
		p++;

	/* skip extra '(' */
	ಯಾವತ್ (*p && *p == '(')
		p++;

	end = p;
	ಯಾವತ್ (*end && *end != ')')
		end++;

	sz = end - p;
	ಯದಿ (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ procfs_dirent_match_name(DIR *procfs, ಸಂರಚನಾ dirent *d, ಧ್ರುವ ವರ್ಣ *name)
{
	ವರ್ಣ buf[33];

	ಯದಿ (procfs_dirent_get_name(procfs, d, buf, ಪರಿಮಾಣ(buf)) == 0)
		ಪ್ರತ್ಯಾಗಮ strcmp(name, buf) == 0;

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
ಪೂರ್ಣಾಙ್ಕ fd_is_procfs(ಪೂರ್ಣಾಙ್ಕ fd)
{
	ಸಂರಚನಾ statfs st;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಕುರು {
		errno = 0;
		ret = fstatfs(fd, &st);

		ಯದಿ (ret < 0) {
			ಯದಿ (errno != EINTR && errno != EAGAIN)
				ಪ್ರತ್ಯಾಗಮ 0;
			xusleep(250000);
		}
	} ಯಾವತ್ (ret != 0);

	ಪ್ರತ್ಯಾಗಮ F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ fd_is_procfs(ಪೂರ್ಣಾಙ್ಕ fd __attribute__((__unused__)))
{
	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

ಸ್ಥಿರ ವರ್ಣ *strdup_procfs_file(pid_t pid, ಧ್ರುವ ವರ್ಣ *name)
{
	ವರ್ಣ buf[BUFSIZ];
	ವರ್ಣ *re = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ fd;

	snprintf(buf, ಪರಿಮಾಣ(buf), _PATH_PROC "/%d/%s", (ಪೂರ್ಣಾಙ್ಕ) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	ಯದಿ (fd < 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (read_procfs_file(ರಿಕ್ತ, fd, buf, ಪರಿಮಾಣ(buf)) > 0)
		re = strdup(buf);
	close(fd);
	ಪ್ರತ್ಯಾಗಮ re;
}

ವರ್ಣ *pid_get_cmdname(pid_t pid)
{
	ಪ್ರತ್ಯಾಗಮ strdup_procfs_file(pid, "comm");
}

ವರ್ಣ *pid_get_cmdline(pid_t pid)
{
	ಪ್ರತ್ಯಾಗಮ strdup_procfs_file(pid, "cmdline");
}

ವರ್ಣ *pid_get_personality(pid_t pid)
{
	ಪ್ರತ್ಯಾಗಮ strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_tasks(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[], ಧ್ರುವ ವರ್ಣ *prefix)
{
	DIR *sub = ರಿಕ್ತ;
	ಸಂರಚನಾ path_cxt *pc;
	pid_t tid = 0, pid;

	ಯದಿ (argc != 2)
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;

	pid = strtol(argv[1], (ವರ್ಣ **) ರಿಕ್ತ, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	ಯದಿ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ಯಾವತ್ (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_fds(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[], ಧ್ರುವ ವರ್ಣ *prefix)
{
	DIR *sub = ರಿಕ್ತ;
	ಸಂರಚನಾ path_cxt *pc;
	pid_t pid;
	ಪೂರ್ಣಾಙ್ಕ fd = -1;

	ಯದಿ (argc != 2)
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;

	pid = strtol(argv[1], (ವರ್ಣ **) ರಿಕ್ತ, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	ಯದಿ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ಯಾವತ್ (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_processes(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	DIR *dir;
	ಸಂರಚನಾ dirent *d;
	ವರ್ಣ *name = ರಿಕ್ತ;
	uid_t uid = (uid_t) -1;
	ವರ್ಣ buf[128];

	ಯದಿ (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	ಯದಿ (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	ಯದಿ (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	ಯಾವತ್ ((d = xreaddir(dir))) {
		pid_t pid = 0;

		ಯದಿ (procfs_dirent_get_pid(d, &pid) != 0)
			ಅನುವರ್ತ;
		ಯದಿ (name && !procfs_dirent_match_name(dir, d, name))
			ಅನುವರ್ತ;
		ಯದಿ (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			ಅನುವರ್ತ;
		procfs_dirent_get_name(dir, d, buf, ಪರಿಮಾಣ(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_one_process(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[], ಧ್ರುವ ವರ್ಣ *prefix)
{
	pid_t pid;
	ಸಂರಚನಾ path_cxt *pc;
	ವರ್ಣ buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	ಯದಿ (argc != 2)
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	pid = strtol(argv[1], (ವರ್ಣ **) ರಿಕ್ತ, 10);

	pc = ul_new_procfs_path(pid, prefix);
	ಯದಿ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (ಪೂರ್ಣಾಙ್ಕ) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, ಪರಿಮಾಣ(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, ಪರಿಮಾಣ(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_isprocfs(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಧ್ರುವ ವರ್ಣ *name = argc > 1 ? argv[1] : "/proc";
	ಪೂರ್ಣಾಙ್ಕ fd = open(name, O_RDONLY | O_CLOEXEC);
	ಪೂರ್ಣಾಙ್ಕ is = 0;

	ಯದಿ (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} ಅನ್ಯಥಾ
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	ಪ್ರತ್ಯಾಗಮ is ? EXIT_SUCCESS : EXIT_FAILURE;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ test_process_stat_nth(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[], ಧ್ರುವ ವರ್ಣ *prefix)
{
	pid_t pid;
	ಸಂರಚನಾ path_cxt *pc;
	uint64_t ಅಂಕೆ = 0;
	ಪೂರ್ಣಾಙ್ಕ n, ret;

	ಯದಿ (argc != 3)
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	pid = strtol(argv[1], (ವರ್ಣ **) ರಿಕ್ತ, 10);
	n = strtol(argv[2], (ವರ್ಣ **) ರಿಕ್ತ, 10);

	pc = ul_new_procfs_path(pid, prefix);
	ಯದಿ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &ಅಂಕೆ);
	ಯದಿ (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (ಪೂರ್ಣಾಙ್ಕ) pid, n, ಅಂಕೆ);
	ul_unref_path(pc);
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	ಧ್ರುವ ವರ್ಣ *prefix = ರಿಕ್ತ;

	ಯದಿ (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	ಯದಿ (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	}

	ಯದಿ (strcmp(argv[1], "--tasks") == 0)
		ಪ್ರತ್ಯಾಗಮ test_tasks(argc - 1, argv + 1, prefix);
	ಯದಿ (strcmp(argv[1], "--fds") == 0)
		ಪ್ರತ್ಯಾಗಮ test_fds(argc - 1, argv + 1, prefix);
	ಯದಿ (strcmp(argv[1], "--processes") == 0)
		ಪ್ರತ್ಯಾಗಮ test_processes(argc - 1, argv + 1);
	ಯದಿ (strcmp(argv[1], "--is-procfs") == 0)
		ಪ್ರತ್ಯಾಗಮ test_isprocfs(argc - 1, argv + 1);
	ಯದಿ (strcmp(argv[1], "--one") == 0)
		ಪ್ರತ್ಯಾಗಮ test_one_process(argc - 1, argv + 1, prefix);
	ಯದಿ (strcmp(argv[1], "--stat-nth") == 0)
		ಪ್ರತ್ಯಾಗಮ test_process_stat_nth(argc - 1, argv + 1, prefix);

	ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
