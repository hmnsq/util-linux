/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#include "c.h"
#include "pathnames.h"
#include "procfs.h"
#include "fileutils.h"
#include "all-io.h"
#include "vfs.h"
#include "debug.h"
#include "strutils.h"
#include "statfs_magic.h"

static void procfs_process_deinit_path(sa_nracanaa path_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
static UL_DEBUG_DEFINE_MASK(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPROCFS_DEBUG_INIT	(1 << 1)
#define ULPROCFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

void ul_procfs_init_debug(void)
{
	ប្រសិនបើ (ulprocfs_debug_mask)
		ត្រឡប់;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

sa_nracanaa path_cxt *ul_new_procfs_path(pid_t pid, const var_na *prefix)
{
	sa_nracanaa path_cxt *pc = ul_new_path(NULL);

	ប្រសិនបើ (!pc)
		ត្រឡប់ NULL;
	ប្រសិនបើ (prefix)
		ul_path_set_prefix(pc, prefix);

	ប្រសិនបើ (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		ត្រឡប់ NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ត្រឡប់ pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
ចំនួនគត់ procfs_process_init_path(sa_nracanaa path_cxt *pc, pid_t pid)
{
	sa_nracanaa procfs_process *prc;
	ចំនួនគត់ rc;
	var_na buf[parimaa_na(_PATH_PROC) + parimaa_na(stringify_value(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, parimaa_na(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	ប្រសិនបើ (rc)
		ត្រឡប់ rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	ប្រសិនបើ (rc < 0)
		ត្រឡប់ rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	ប្រសិនបើ (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, parimaa_na(sa_nracanaa procfs_process));
		ប្រសិនបើ (!prc)
			ត្រឡប់ -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	ត្រឡប់ 0;
}

static void procfs_process_deinit_path(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa procfs_process *prc;

	ប្រសិនបើ (!pc)
		ត្រឡប់;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	ប្រសិនបើ (!prc)
		ត្រឡប់;

	free(prc);
	ul_path_set_dialect(pc, NULL, NULL);
}

static ssize_t read_procfs_file(const sa_nracanaa ul_vfs_ops *vfs, ចំនួនគត់ fd,
			       var_na *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	ប្រសិនបើ (fd < 0)
		ត្រឡប់ -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	ប្រសិនបើ (sz <= 0)
		ត្រឡប់ sz;

	សម្រាប់ (i = 0; i < (size_t) sz; i++) {
		ប្រសិនបើ (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	ត្រឡប់ sz;
}

static ssize_t procfs_process_get_data_for(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz,
					    const var_na *fname)
{
	ចំនួនគត់ fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	ប្រសិនបើ (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : NULL, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : NULL, fd);
		ត្រឡប់ sz;
	}
	ត្រឡប់ -errno;
}

ssize_t procfs_process_get_cmdline(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ត្រឡប់ procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ត្រឡប់ procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ត្រឡប់ procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ត្រឡប់ procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

ចំនួនគត់ procfs_process_get_stat_nth(sa_nracanaa path_cxt *pc, ចំនួនគត់ n, uint64_t *re)
{
	ssize_t rc;
	var_na *key = NULL, *tok, *p;
	var_na buf[BUFSIZ];
	ចំនួនគត់ i;

	ប្រសិនបើ (n == 2 || n == 3)		/* process name and status (strings) */
		ត្រឡប់ -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, parimaa_na(buf), "stat");
	ប្រសិនបើ (rc < 0)
		ត្រឡប់ rc;

	សម្រាប់ (i = 0, tok = strtok_r(buf, " ", &key); tok;
	     tok = strtok_r(NULL, " ", &key)) {

		i++;
		ប្រសិនបើ (i == n)
			ត្រឡប់ ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		ប្រសិនបើ (i == 2 && (p = strrchr(key, ')')))
			key = p + 2;
	}

	ត្រឡប់ -EINVAL;
}

ចំនួនគត់ procfs_process_get_uid(sa_nracanaa path_cxt *pc, uid_t *uid)
{
	sa_nracanaa stat sb;
	ចំនួនគត់ rc;

	ប្រសិនបើ ((rc = ul_path_stat(pc, &sb, 0, NULL)) == 0)
		*uid = sb.st_uid;
	ត្រឡប់ rc;
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
ចំនួនគត់ procfs_process_next_tid(sa_nracanaa path_cxt *pc, DIR **sub, pid_t *tid)
{
	sa_nracanaa dirent *d;

	ប្រសិនបើ (!pc || !sub || !tid)
		ត្រឡប់ -EINVAL;

	ប្រសិនបើ (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		ប្រសិនបើ (!*sub)
			ត្រឡប់ -errno;
	}

	ខណៈពេល ((d = xreaddir(*sub))) {
		ប្រសិនបើ (procfs_dirent_get_pid(d, tid) == 0)
			ត្រឡប់ 0;
	}

	closedir(*sub);
	*sub = NULL;
	ត្រឡប់ 1;
}

ចំនួនគត់ procfs_process_next_fd(sa_nracanaa path_cxt *pc, DIR **sub, ចំនួនគត់ *fd)
{
	sa_nracanaa dirent *d;

	ប្រសិនបើ (!pc || !sub || !fd)
		ត្រឡប់ -EINVAL;

	ប្រសិនបើ (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		ប្រសិនបើ (!*sub)
			ត្រឡប់ -errno;
	}

	ខណៈពេល ((d = xreaddir(*sub))) {
		uint64_t num;
#ifdef _DIRENT_HAVE_D_TYPE
		ប្រសិនបើ (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			continue;
#endif
		ប្រសិនបើ (ul_strtou64(d->d_name, &num, 10) < 0)
			continue;
		*fd = num;
		ត្រឡប់ 0;
	}

	closedir(*sub);
	*sub = NULL;
	ត្រឡប់ 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
ចំនួនគត់ procfs_dirent_is_process(sa_nracanaa dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	ប្រសិនបើ (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		ត្រឡប់ 0;
#endif
	ប្រសិនបើ (!isdigit((unsigned var_na) *d->d_name))
		ត្រឡប់ 0;

	ត្រឡប់ 1;
}

ចំនួនគត់ procfs_dirent_get_pid(sa_nracanaa dirent *d, pid_t *pid)
{
	uint64_t num;

	ប្រសិនបើ (!procfs_dirent_is_process(d))
		ត្រឡប់ -EINVAL;

	ប្រសិនបើ (ul_strtou64(d->d_name, &num, 10) < 0)
		ត្រឡប់ -EINVAL;

	*pid = (pid_t) num;
	ត្រឡប់ 0;
}

ចំនួនគត់ procfs_dirent_get_uid(DIR *procfs, sa_nracanaa dirent *d, uid_t *uid)
{
	sa_nracanaa stat st;

	ប្រសិនបើ (!procfs_dirent_is_process(d))
		ត្រឡប់ -EINVAL;

	ប្រសិនបើ (fstatat(dirfd(procfs), d->d_name, &st, 0))
		ត្រឡប់ -EINVAL;

	*uid = st.st_uid;
	ត្រឡប់ 0;
}

ចំនួនគត់ procfs_dirent_match_uid(DIR *procfs, sa_nracanaa dirent *d, uid_t uid)
{
	uid_t x;

	ប្រសិនបើ (procfs_dirent_get_uid(procfs, d, &x) == 0)
		ត្រឡប់ x == uid;

	ត្រឡប់ 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
ចំនួនគត់ procfs_dirent_get_name(DIR *procfs, sa_nracanaa dirent *d, var_na *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	var_na tmp[1024], *p, *end = NULL;

	ប្រសិនបើ (bufsz < 32)
		ត្រឡប់ -EINVAL;
	ប្រសិនបើ (!procfs_dirent_is_process(d))
		ត្រឡប់ -EINVAL;

	snprintf(tmp, parimaa_na(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	ប្រសិនបើ (!f)
		ត្រឡប់ -errno;

	p = fgets(tmp, parimaa_na(tmp), f);
	fclose(f);
	ប្រសិនបើ (!p)
		ត្រឡប់ -errno;

	/* skip PID */
	ខណៈពេល (*p && *p != '(')
		p++;

	/* skip extra '(' */
	ខណៈពេល (*p && *p == '(')
		p++;

	end = p;
	ខណៈពេល (*end && *end != ')')
		end++;

	sz = end - p;
	ប្រសិនបើ (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	ត្រឡប់ 0;
}

ចំនួនគត់ procfs_dirent_match_name(DIR *procfs, sa_nracanaa dirent *d, const var_na *name)
{
	var_na buf[33];

	ប្រសិនបើ (procfs_dirent_get_name(procfs, d, buf, parimaa_na(buf)) == 0)
		ត្រឡប់ strcmp(name, buf) == 0;

	ត្រឡប់ 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
ចំនួនគត់ fd_is_procfs(ចំនួនគត់ fd)
{
	sa_nracanaa statfs st;
	ចំនួនគត់ ret;

	do {
		errno = 0;
		ret = fstatfs(fd, &st);

		ប្រសិនបើ (ret < 0) {
			ប្រសិនបើ (errno != EINTR && errno != EAGAIN)
				ត្រឡប់ 0;
			xusleep(250000);
		}
	} ខណៈពេល (ret != 0);

	ត្រឡប់ F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#បើមិនដូច្នេះ
ចំនួនគត់ fd_is_procfs(ចំនួនគត់ fd __attribute__((__unused__)))
{
	ត្រឡប់ 0;
}
#endif

static var_na *strdup_procfs_file(pid_t pid, const var_na *name)
{
	var_na buf[BUFSIZ];
	var_na *re = NULL;
	ចំនួនគត់ fd;

	snprintf(buf, parimaa_na(buf), _PATH_PROC "/%d/%s", (ចំនួនគត់) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	ប្រសិនបើ (fd < 0)
		ត្រឡប់ NULL;

	ប្រសិនបើ (read_procfs_file(NULL, fd, buf, parimaa_na(buf)) > 0)
		re = strdup(buf);
	close(fd);
	ត្រឡប់ re;
}

var_na *pid_get_cmdname(pid_t pid)
{
	ត្រឡប់ strdup_procfs_file(pid, "comm");
}

var_na *pid_get_cmdline(pid_t pid)
{
	ត្រឡប់ strdup_procfs_file(pid, "cmdline");
}

var_na *pid_get_personality(pid_t pid)
{
	ត្រឡប់ strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

static ចំនួនគត់ test_tasks(ចំនួនគត់ argc, var_na *argv[], const var_na *prefix)
{
	DIR *sub = NULL;
	sa_nracanaa path_cxt *pc;
	pid_t tid = 0, pid;

	ប្រសិនបើ (argc != 2)
		ត្រឡប់ EXIT_FAILURE;

	pid = strtol(argv[1], (var_na **) NULL, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	ប្រសិនបើ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ខណៈពេល (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	ត្រឡប់ EXIT_SUCCESS;
}

static ចំនួនគត់ test_fds(ចំនួនគត់ argc, var_na *argv[], const var_na *prefix)
{
	DIR *sub = NULL;
	sa_nracanaa path_cxt *pc;
	pid_t pid;
	ចំនួនគត់ fd = -1;

	ប្រសិនបើ (argc != 2)
		ត្រឡប់ EXIT_FAILURE;

	pid = strtol(argv[1], (var_na **) NULL, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	ប្រសិនបើ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ខណៈពេល (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	ត្រឡប់ EXIT_SUCCESS;
}

static ចំនួនគត់ test_processes(ចំនួនគត់ argc, var_na *argv[])
{
	DIR *dir;
	sa_nracanaa dirent *d;
	var_na *name = NULL;
	uid_t uid = (uid_t) -1;
	var_na buf[128];

	ប្រសិនបើ (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	ប្រសិនបើ (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	ប្រសិនបើ (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	ខណៈពេល ((d = xreaddir(dir))) {
		pid_t pid = 0;

		ប្រសិនបើ (procfs_dirent_get_pid(d, &pid) != 0)
			continue;
		ប្រសិនបើ (name && !procfs_dirent_match_name(dir, d, name))
			continue;
		ប្រសិនបើ (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			continue;
		procfs_dirent_get_name(dir, d, buf, parimaa_na(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	ត្រឡប់ EXIT_SUCCESS;
}

static ចំនួនគត់ test_one_process(ចំនួនគត់ argc, var_na *argv[], const var_na *prefix)
{
	pid_t pid;
	sa_nracanaa path_cxt *pc;
	var_na buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	ប្រសិនបើ (argc != 2)
		ត្រឡប់ EXIT_FAILURE;
	pid = strtol(argv[1], (var_na **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	ប្រសិនបើ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (ចំនួនគត់) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, parimaa_na(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, parimaa_na(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	ត្រឡប់ EXIT_SUCCESS;
}

static ចំនួនគត់ test_isprocfs(ចំនួនគត់ argc, var_na *argv[])
{
	const var_na *name = argc > 1 ? argv[1] : "/proc";
	ចំនួនគត់ fd = open(name, O_RDONLY | O_CLOEXEC);
	ចំនួនគត់ is = 0;

	ប្រសិនបើ (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} បើមិនដូច្នេះ
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	ត្រឡប់ is ? EXIT_SUCCESS : EXIT_FAILURE;
}

static ចំនួនគត់ test_process_stat_nth(ចំនួនគត់ argc, var_na *argv[], const var_na *prefix)
{
	pid_t pid;
	sa_nracanaa path_cxt *pc;
	uint64_t num = 0;
	ចំនួនគត់ n, ret;

	ប្រសិនបើ (argc != 3)
		ត្រឡប់ EXIT_FAILURE;
	pid = strtol(argv[1], (var_na **) NULL, 10);
	n = strtol(argv[2], (var_na **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	ប្រសិនបើ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &num);
	ប្រសិនបើ (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (ចំនួនគត់) pid, n, num);
	ul_unref_path(pc);
	ត្រឡប់ EXIT_SUCCESS;
}

ចំនួនគត់ main(ចំនួនគត់ argc, var_na *argv[])
{
	const var_na *prefix = NULL;

	ប្រសិនបើ (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	ប្រសិនបើ (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		ត្រឡប់ EXIT_FAILURE;
	}

	ប្រសិនបើ (strcmp(argv[1], "--tasks") == 0)
		ត្រឡប់ test_tasks(argc - 1, argv + 1, prefix);
	ប្រសិនបើ (strcmp(argv[1], "--fds") == 0)
		ត្រឡប់ test_fds(argc - 1, argv + 1, prefix);
	ប្រសិនបើ (strcmp(argv[1], "--processes") == 0)
		ត្រឡប់ test_processes(argc - 1, argv + 1);
	ប្រសិនបើ (strcmp(argv[1], "--is-procfs") == 0)
		ត្រឡប់ test_isprocfs(argc - 1, argv + 1);
	ប្រសិនបើ (strcmp(argv[1], "--one") == 0)
		ត្រឡប់ test_one_process(argc - 1, argv + 1, prefix);
	ប្រសិនបើ (strcmp(argv[1], "--stat-nth") == 0)
		ត្រឡប់ test_process_stat_nth(argc - 1, argv + 1, prefix);

	ត្រឡប់ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
