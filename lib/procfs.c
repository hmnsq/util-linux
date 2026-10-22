/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#അന്തര്ഭാവ <ctype.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <errno.h>

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "pathnames.h"
#അന്തര്ഭാവ "procfs.h"
#അന്തര്ഭാവ "fileutils.h"
#അന്തര്ഭാവ "all-io.h"
#അന്തര്ഭാവ "vfs.h"
#അന്തര്ഭാവ "debug.h"
#അന്തര്ഭാവ "strutils.h"
#അന്തര്ഭാവ "statfs_magic.h"

സ്ഥിതം ശൂന്യം procfs_process_deinit_path(ഘടന path_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
സ്ഥിതം UL_DEBUG_DEFINE_MASK(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPROCFS_DEBUG_INIT	(1 << 1)
#define ULPROCFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

ശൂന്യം ul_procfs_init_debug(ശൂന്യം)
{
	എങ്കിൽ (ulprocfs_debug_mask)
		തിരികെ;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

ഘടന path_cxt *ul_new_procfs_path(pid_t pid, സ്ഥിരം അക്ഷരം *prefix)
{
	ഘടന path_cxt *pc = ul_new_path(ശൂന്യ);

	എങ്കിൽ (!pc)
		തിരികെ ശൂന്യ;
	എങ്കിൽ (prefix)
		ul_path_set_prefix(pc, prefix);

	എങ്കിൽ (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		തിരികെ ശൂന്യ;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	തിരികെ pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
പൂർണ്ണസംഖ്യ procfs_process_init_path(ഘടന path_cxt *pc, pid_t pid)
{
	ഘടന procfs_process *prc;
	പൂർണ്ണസംഖ്യ rc;
	അക്ഷരം buf[വലുപ്പം(_PATH_PROC) + വലുപ്പം(stringify_മൂല്യം(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, വലുപ്പം(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	എങ്കിൽ (rc)
		തിരികെ rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	എങ്കിൽ (rc < 0)
		തിരികെ rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	എങ്കിൽ (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, വലുപ്പം(ഘടന procfs_process));
		എങ്കിൽ (!prc)
			തിരികെ -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം procfs_process_deinit_path(ഘടന path_cxt *pc)
{
	ഘടന procfs_process *prc;

	എങ്കിൽ (!pc)
		തിരികെ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	എങ്കിൽ (!prc)
		തിരികെ;

	സ്വതന്ത്രം(prc);
	ul_path_set_dialect(pc, ശൂന്യ, ശൂന്യ);
}

സ്ഥിതം ssize_t read_procfs_file(സ്ഥിരം ഘടന ul_vfs_ops *vfs, പൂർണ്ണസംഖ്യ fd,
			       അക്ഷരം *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	എങ്കിൽ (fd < 0)
		തിരികെ -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	എങ്കിൽ (sz <= 0)
		തിരികെ sz;

	വേണ്ടി (i = 0; i < (size_t) sz; i++) {
		എങ്കിൽ (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	തിരികെ sz;
}

സ്ഥിതം ssize_t procfs_process_get_data_for(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t bufsz,
					    സ്ഥിരം അക്ഷരം *fname)
{
	പൂർണ്ണസംഖ്യ fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	എങ്കിൽ (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : ശൂന്യ, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : ശൂന്യ, fd);
		തിരികെ sz;
	}
	തിരികെ -errno;
}

ssize_t procfs_process_get_cmdline(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t bufsz)
{
	തിരികെ procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t bufsz)
{
	തിരികെ procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t bufsz)
{
	തിരികെ procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(ഘടന path_cxt *pc, അക്ഷരം *buf, size_t bufsz)
{
	തിരികെ procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

പൂർണ്ണസംഖ്യ procfs_process_get_stat_nth(ഘടന path_cxt *pc, പൂർണ്ണസംഖ്യ n, uint64_t *re)
{
	ssize_t rc;
	അക്ഷരം *താക്കോൽ = ശൂന്യ, *tok, *p;
	അക്ഷരം buf[BUFSIZ];
	പൂർണ്ണസംഖ്യ i;

	എങ്കിൽ (n == 2 || n == 3)		/* process name and status (strings) */
		തിരികെ -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, വലുപ്പം(buf), "stat");
	എങ്കിൽ (rc < 0)
		തിരികെ rc;

	വേണ്ടി (i = 0, tok = strtok_r(buf, " ", &താക്കോൽ); tok;
	     tok = strtok_r(ശൂന്യ, " ", &താക്കോൽ)) {

		i++;
		എങ്കിൽ (i == n)
			തിരികെ ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		എങ്കിൽ (i == 2 && (p = strrchr(താക്കോൽ, ')')))
			താക്കോൽ = p + 2;
	}

	തിരികെ -EINVAL;
}

പൂർണ്ണസംഖ്യ procfs_process_get_uid(ഘടന path_cxt *pc, uid_t *uid)
{
	ഘടന stat sb;
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ ((rc = ul_path_stat(pc, &sb, 0, ശൂന്യ)) == 0)
		*uid = sb.st_uid;
	തിരികെ rc;
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
പൂർണ്ണസംഖ്യ procfs_process_next_tid(ഘടന path_cxt *pc, DIR **sub, pid_t *tid)
{
	ഘടന dirent *d;

	എങ്കിൽ (!pc || !sub || !tid)
		തിരികെ -EINVAL;

	എങ്കിൽ (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		എങ്കിൽ (!*sub)
			തിരികെ -errno;
	}

	ആകുമ്പോൾ ((d = xreaddir(*sub))) {
		എങ്കിൽ (procfs_dirent_get_pid(d, tid) == 0)
			തിരികെ 0;
	}

	closedir(*sub);
	*sub = ശൂന്യ;
	തിരികെ 1;
}

പൂർണ്ണസംഖ്യ procfs_process_next_fd(ഘടന path_cxt *pc, DIR **sub, പൂർണ്ണസംഖ്യ *fd)
{
	ഘടന dirent *d;

	എങ്കിൽ (!pc || !sub || !fd)
		തിരികെ -EINVAL;

	എങ്കിൽ (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		എങ്കിൽ (!*sub)
			തിരികെ -errno;
	}

	ആകുമ്പോൾ ((d = xreaddir(*sub))) {
		uint64_t അക്കം;
#ifdef _DIRENT_HAVE_D_TYPE
		എങ്കിൽ (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			തുടരുക;
#endif
		എങ്കിൽ (ul_strtou64(d->d_name, &അക്കം, 10) < 0)
			തുടരുക;
		*fd = അക്കം;
		തിരികെ 0;
	}

	closedir(*sub);
	*sub = ശൂന്യ;
	തിരികെ 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
പൂർണ്ണസംഖ്യ procfs_dirent_is_process(ഘടന dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	എങ്കിൽ (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		തിരികെ 0;
#endif
	എങ്കിൽ (!isdigit((unsigned അക്ഷരം) *d->d_name))
		തിരികെ 0;

	തിരികെ 1;
}

പൂർണ്ണസംഖ്യ procfs_dirent_get_pid(ഘടന dirent *d, pid_t *pid)
{
	uint64_t അക്കം;

	എങ്കിൽ (!procfs_dirent_is_process(d))
		തിരികെ -EINVAL;

	എങ്കിൽ (ul_strtou64(d->d_name, &അക്കം, 10) < 0)
		തിരികെ -EINVAL;

	*pid = (pid_t) അക്കം;
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ procfs_dirent_get_uid(DIR *procfs, ഘടന dirent *d, uid_t *uid)
{
	ഘടന stat st;

	എങ്കിൽ (!procfs_dirent_is_process(d))
		തിരികെ -EINVAL;

	എങ്കിൽ (fstatat(dirfd(procfs), d->d_name, &st, 0))
		തിരികെ -EINVAL;

	*uid = st.st_uid;
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ procfs_dirent_match_uid(DIR *procfs, ഘടന dirent *d, uid_t uid)
{
	uid_t x;

	എങ്കിൽ (procfs_dirent_get_uid(procfs, d, &x) == 0)
		തിരികെ x == uid;

	തിരികെ 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
പൂർണ്ണസംഖ്യ procfs_dirent_get_name(DIR *procfs, ഘടന dirent *d, അക്ഷരം *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	അക്ഷരം tmp[1024], *p, *end = ശൂന്യ;

	എങ്കിൽ (bufsz < 32)
		തിരികെ -EINVAL;
	എങ്കിൽ (!procfs_dirent_is_process(d))
		തിരികെ -EINVAL;

	snprintf(tmp, വലുപ്പം(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	എങ്കിൽ (!f)
		തിരികെ -errno;

	p = fgets(tmp, വലുപ്പം(tmp), f);
	fclose(f);
	എങ്കിൽ (!p)
		തിരികെ -errno;

	/* skip PID */
	ആകുമ്പോൾ (*p && *p != '(')
		p++;

	/* skip extra '(' */
	ആകുമ്പോൾ (*p && *p == '(')
		p++;

	end = p;
	ആകുമ്പോൾ (*end && *end != ')')
		end++;

	sz = end - p;
	എങ്കിൽ (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ procfs_dirent_match_name(DIR *procfs, ഘടന dirent *d, സ്ഥിരം അക്ഷരം *name)
{
	അക്ഷരം buf[33];

	എങ്കിൽ (procfs_dirent_get_name(procfs, d, buf, വലുപ്പം(buf)) == 0)
		തിരികെ strcmp(name, buf) == 0;

	തിരികെ 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
പൂർണ്ണസംഖ്യ fd_is_procfs(പൂർണ്ണസംഖ്യ fd)
{
	ഘടന statfs st;
	പൂർണ്ണസംഖ്യ ret;

	ചെയ്യുക {
		errno = 0;
		ret = fstatfs(fd, &st);

		എങ്കിൽ (ret < 0) {
			എങ്കിൽ (errno != EINTR && errno != EAGAIN)
				തിരികെ 0;
			xusleep(250000);
		}
	} ആകുമ്പോൾ (ret != 0);

	തിരികെ F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ fd_is_procfs(പൂർണ്ണസംഖ്യ fd __attribute__((__unused__)))
{
	തിരികെ 0;
}
#endif

സ്ഥിതം അക്ഷരം *strdup_procfs_file(pid_t pid, സ്ഥിരം അക്ഷരം *name)
{
	അക്ഷരം buf[BUFSIZ];
	അക്ഷരം *re = ശൂന്യ;
	പൂർണ്ണസംഖ്യ fd;

	snprintf(buf, വലുപ്പം(buf), _PATH_PROC "/%d/%s", (പൂർണ്ണസംഖ്യ) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	എങ്കിൽ (fd < 0)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (read_procfs_file(ശൂന്യ, fd, buf, വലുപ്പം(buf)) > 0)
		re = strdup(buf);
	close(fd);
	തിരികെ re;
}

അക്ഷരം *pid_get_cmdname(pid_t pid)
{
	തിരികെ strdup_procfs_file(pid, "comm");
}

അക്ഷരം *pid_get_cmdline(pid_t pid)
{
	തിരികെ strdup_procfs_file(pid, "cmdline");
}

അക്ഷരം *pid_get_personality(pid_t pid)
{
	തിരികെ strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

സ്ഥിതം പൂർണ്ണസംഖ്യ test_tasks(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[], സ്ഥിരം അക്ഷരം *prefix)
{
	DIR *sub = ശൂന്യ;
	ഘടന path_cxt *pc;
	pid_t tid = 0, pid;

	എങ്കിൽ (argc != 2)
		തിരികെ EXIT_FAILURE;

	pid = strtol(argv[1], (അക്ഷരം **) ശൂന്യ, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	എങ്കിൽ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ആകുമ്പോൾ (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	തിരികെ EXIT_SUCCESS;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ test_fds(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[], സ്ഥിരം അക്ഷരം *prefix)
{
	DIR *sub = ശൂന്യ;
	ഘടന path_cxt *pc;
	pid_t pid;
	പൂർണ്ണസംഖ്യ fd = -1;

	എങ്കിൽ (argc != 2)
		തിരികെ EXIT_FAILURE;

	pid = strtol(argv[1], (അക്ഷരം **) ശൂന്യ, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	എങ്കിൽ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ആകുമ്പോൾ (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	തിരികെ EXIT_SUCCESS;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ test_processes(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	DIR *dir;
	ഘടന dirent *d;
	അക്ഷരം *name = ശൂന്യ;
	uid_t uid = (uid_t) -1;
	അക്ഷരം buf[128];

	എങ്കിൽ (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	എങ്കിൽ (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	എങ്കിൽ (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	ആകുമ്പോൾ ((d = xreaddir(dir))) {
		pid_t pid = 0;

		എങ്കിൽ (procfs_dirent_get_pid(d, &pid) != 0)
			തുടരുക;
		എങ്കിൽ (name && !procfs_dirent_match_name(dir, d, name))
			തുടരുക;
		എങ്കിൽ (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			തുടരുക;
		procfs_dirent_get_name(dir, d, buf, വലുപ്പം(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	തിരികെ EXIT_SUCCESS;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ test_one_process(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[], സ്ഥിരം അക്ഷരം *prefix)
{
	pid_t pid;
	ഘടന path_cxt *pc;
	അക്ഷരം buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	എങ്കിൽ (argc != 2)
		തിരികെ EXIT_FAILURE;
	pid = strtol(argv[1], (അക്ഷരം **) ശൂന്യ, 10);

	pc = ul_new_procfs_path(pid, prefix);
	എങ്കിൽ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (പൂർണ്ണസംഖ്യ) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, വലുപ്പം(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, വലുപ്പം(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	തിരികെ EXIT_SUCCESS;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ test_isprocfs(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	സ്ഥിരം അക്ഷരം *name = argc > 1 ? argv[1] : "/proc";
	പൂർണ്ണസംഖ്യ fd = open(name, O_RDONLY | O_CLOEXEC);
	പൂർണ്ണസംഖ്യ is = 0;

	എങ്കിൽ (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} അല്ലെങ്കിൽ
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	തിരികെ is ? EXIT_SUCCESS : EXIT_FAILURE;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ test_process_stat_nth(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[], സ്ഥിരം അക്ഷരം *prefix)
{
	pid_t pid;
	ഘടന path_cxt *pc;
	uint64_t അക്കം = 0;
	പൂർണ്ണസംഖ്യ n, ret;

	എങ്കിൽ (argc != 3)
		തിരികെ EXIT_FAILURE;
	pid = strtol(argv[1], (അക്ഷരം **) ശൂന്യ, 10);
	n = strtol(argv[2], (അക്ഷരം **) ശൂന്യ, 10);

	pc = ul_new_procfs_path(pid, prefix);
	എങ്കിൽ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &അക്കം);
	എങ്കിൽ (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (പൂർണ്ണസംഖ്യ) pid, n, അക്കം);
	ul_unref_path(pc);
	തിരികെ EXIT_SUCCESS;
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	സ്ഥിരം അക്ഷരം *prefix = ശൂന്യ;

	എങ്കിൽ (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	എങ്കിൽ (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		തിരികെ EXIT_FAILURE;
	}

	എങ്കിൽ (strcmp(argv[1], "--tasks") == 0)
		തിരികെ test_tasks(argc - 1, argv + 1, prefix);
	എങ്കിൽ (strcmp(argv[1], "--fds") == 0)
		തിരികെ test_fds(argc - 1, argv + 1, prefix);
	എങ്കിൽ (strcmp(argv[1], "--processes") == 0)
		തിരികെ test_processes(argc - 1, argv + 1);
	എങ്കിൽ (strcmp(argv[1], "--is-procfs") == 0)
		തിരികെ test_isprocfs(argc - 1, argv + 1);
	എങ്കിൽ (strcmp(argv[1], "--one") == 0)
		തിരികെ test_one_process(argc - 1, argv + 1, prefix);
	എങ്കിൽ (strcmp(argv[1], "--stat-nth") == 0)
		തിരികെ test_process_stat_nth(argc - 1, argv + 1, prefix);

	തിരികെ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
