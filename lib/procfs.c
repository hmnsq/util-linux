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
	නම් (ulprocfs_debug_mask)
		ආපසු;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

sa_nracanaa path_cxt *ul_new_procfs_path(pid_t pid, const var_na *prefix)
{
	sa_nracanaa path_cxt *pc = ul_new_path(NULL);

	නම් (!pc)
		ආපසු NULL;
	නම් (prefix)
		ul_path_set_prefix(pc, prefix);

	නම් (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		ආපසු NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ආපසු pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
පූර්ණ_සංඛ්‍යා procfs_process_init_path(sa_nracanaa path_cxt *pc, pid_t pid)
{
	sa_nracanaa procfs_process *prc;
	පූර්ණ_සංඛ්‍යා rc;
	var_na buf[parimaa_na(_PATH_PROC) + parimaa_na(stringify_value(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, parimaa_na(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	නම් (rc)
		ආපසු rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	නම් (rc < 0)
		ආපසු rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	නම් (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, parimaa_na(sa_nracanaa procfs_process));
		නම් (!prc)
			ආපසු -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	ආපසු 0;
}

static void procfs_process_deinit_path(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa procfs_process *prc;

	නම් (!pc)
		ආපසු;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	නම් (!prc)
		ආපසු;

	free(prc);
	ul_path_set_dialect(pc, NULL, NULL);
}

static ssize_t read_procfs_file(const sa_nracanaa ul_vfs_ops *vfs, පූර්ණ_සංඛ්‍යා fd,
			       var_na *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	නම් (fd < 0)
		ආපසු -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	නම් (sz <= 0)
		ආපසු sz;

	සඳහා (i = 0; i < (size_t) sz; i++) {
		නම් (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	ආපසු sz;
}

static ssize_t procfs_process_get_data_for(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz,
					    const var_na *fname)
{
	පූර්ණ_සංඛ්‍යා fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	නම් (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : NULL, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : NULL, fd);
		ආපසු sz;
	}
	ආපසු -errno;
}

ssize_t procfs_process_get_cmdline(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ආපසු procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ආපසු procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ආපසු procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ආපසු procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

පූර්ණ_සංඛ්‍යා procfs_process_get_stat_nth(sa_nracanaa path_cxt *pc, පූර්ණ_සංඛ්‍යා n, uint64_t *re)
{
	ssize_t rc;
	var_na *key = NULL, *tok, *p;
	var_na buf[BUFSIZ];
	පූර්ණ_සංඛ්‍යා i;

	නම් (n == 2 || n == 3)		/* process name and status (strings) */
		ආපසු -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, parimaa_na(buf), "stat");
	නම් (rc < 0)
		ආපසු rc;

	සඳහා (i = 0, tok = strtok_r(buf, " ", &key); tok;
	     tok = strtok_r(NULL, " ", &key)) {

		i++;
		නම් (i == n)
			ආපසු ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		නම් (i == 2 && (p = strrchr(key, ')')))
			key = p + 2;
	}

	ආපසු -EINVAL;
}

පූර්ණ_සංඛ්‍යා procfs_process_get_uid(sa_nracanaa path_cxt *pc, uid_t *uid)
{
	sa_nracanaa stat sb;
	පූර්ණ_සංඛ්‍යා rc;

	නම් ((rc = ul_path_stat(pc, &sb, 0, NULL)) == 0)
		*uid = sb.st_uid;
	ආපසු rc;
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
පූර්ණ_සංඛ්‍යා procfs_process_next_tid(sa_nracanaa path_cxt *pc, DIR **sub, pid_t *tid)
{
	sa_nracanaa dirent *d;

	නම් (!pc || !sub || !tid)
		ආපසු -EINVAL;

	නම් (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		නම් (!*sub)
			ආපසු -errno;
	}

	අතරතුර ((d = xreaddir(*sub))) {
		නම් (procfs_dirent_get_pid(d, tid) == 0)
			ආපසු 0;
	}

	closedir(*sub);
	*sub = NULL;
	ආපසු 1;
}

පූර්ණ_සංඛ්‍යා procfs_process_next_fd(sa_nracanaa path_cxt *pc, DIR **sub, පූර්ණ_සංඛ්‍යා *fd)
{
	sa_nracanaa dirent *d;

	නම් (!pc || !sub || !fd)
		ආපසු -EINVAL;

	නම් (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		නම් (!*sub)
			ආපසු -errno;
	}

	අතරතුර ((d = xreaddir(*sub))) {
		uint64_t num;
#ifdef _DIRENT_HAVE_D_TYPE
		නම් (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			continue;
#endif
		නම් (ul_strtou64(d->d_name, &num, 10) < 0)
			continue;
		*fd = num;
		ආපසු 0;
	}

	closedir(*sub);
	*sub = NULL;
	ආපසු 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
පූර්ණ_සංඛ්‍යා procfs_dirent_is_process(sa_nracanaa dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	නම් (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		ආපසු 0;
#endif
	නම් (!isdigit((unsigned var_na) *d->d_name))
		ආපසු 0;

	ආපසු 1;
}

පූර්ණ_සංඛ්‍යා procfs_dirent_get_pid(sa_nracanaa dirent *d, pid_t *pid)
{
	uint64_t num;

	නම් (!procfs_dirent_is_process(d))
		ආපසු -EINVAL;

	නම් (ul_strtou64(d->d_name, &num, 10) < 0)
		ආපසු -EINVAL;

	*pid = (pid_t) num;
	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා procfs_dirent_get_uid(DIR *procfs, sa_nracanaa dirent *d, uid_t *uid)
{
	sa_nracanaa stat st;

	නම් (!procfs_dirent_is_process(d))
		ආපසු -EINVAL;

	නම් (fstatat(dirfd(procfs), d->d_name, &st, 0))
		ආපසු -EINVAL;

	*uid = st.st_uid;
	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා procfs_dirent_match_uid(DIR *procfs, sa_nracanaa dirent *d, uid_t uid)
{
	uid_t x;

	නම් (procfs_dirent_get_uid(procfs, d, &x) == 0)
		ආපසු x == uid;

	ආපසු 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
පූර්ණ_සංඛ්‍යා procfs_dirent_get_name(DIR *procfs, sa_nracanaa dirent *d, var_na *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	var_na tmp[1024], *p, *end = NULL;

	නම් (bufsz < 32)
		ආපසු -EINVAL;
	නම් (!procfs_dirent_is_process(d))
		ආපසු -EINVAL;

	snprintf(tmp, parimaa_na(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	නම් (!f)
		ආපසු -errno;

	p = fgets(tmp, parimaa_na(tmp), f);
	fclose(f);
	නම් (!p)
		ආපසු -errno;

	/* skip PID */
	අතරතුර (*p && *p != '(')
		p++;

	/* skip extra '(' */
	අතරතුර (*p && *p == '(')
		p++;

	end = p;
	අතරතුර (*end && *end != ')')
		end++;

	sz = end - p;
	නම් (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා procfs_dirent_match_name(DIR *procfs, sa_nracanaa dirent *d, const var_na *name)
{
	var_na buf[33];

	නම් (procfs_dirent_get_name(procfs, d, buf, parimaa_na(buf)) == 0)
		ආපසු strcmp(name, buf) == 0;

	ආපසු 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
පූර්ණ_සංඛ්‍යා fd_is_procfs(පූර්ණ_සංඛ්‍යා fd)
{
	sa_nracanaa statfs st;
	පූර්ණ_සංඛ්‍යා ret;

	do {
		errno = 0;
		ret = fstatfs(fd, &st);

		නම් (ret < 0) {
			නම් (errno != EINTR && errno != EAGAIN)
				ආපසු 0;
			xusleep(250000);
		}
	} අතරතුර (ret != 0);

	ආපසු F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#නැතහොත්
පූර්ණ_සංඛ්‍යා fd_is_procfs(පූර්ණ_සංඛ්‍යා fd __attribute__((__unused__)))
{
	ආපසු 0;
}
#endif

static var_na *strdup_procfs_file(pid_t pid, const var_na *name)
{
	var_na buf[BUFSIZ];
	var_na *re = NULL;
	පූර්ණ_සංඛ්‍යා fd;

	snprintf(buf, parimaa_na(buf), _PATH_PROC "/%d/%s", (පූර්ණ_සංඛ්‍යා) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	නම් (fd < 0)
		ආපසු NULL;

	නම් (read_procfs_file(NULL, fd, buf, parimaa_na(buf)) > 0)
		re = strdup(buf);
	close(fd);
	ආපසු re;
}

var_na *pid_get_cmdname(pid_t pid)
{
	ආපසු strdup_procfs_file(pid, "comm");
}

var_na *pid_get_cmdline(pid_t pid)
{
	ආපසු strdup_procfs_file(pid, "cmdline");
}

var_na *pid_get_personality(pid_t pid)
{
	ආපසු strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

static පූර්ණ_සංඛ්‍යා test_tasks(පූර්ණ_සංඛ්‍යා argc, var_na *argv[], const var_na *prefix)
{
	DIR *sub = NULL;
	sa_nracanaa path_cxt *pc;
	pid_t tid = 0, pid;

	නම් (argc != 2)
		ආපසු EXIT_FAILURE;

	pid = strtol(argv[1], (var_na **) NULL, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	නම් (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	අතරතුර (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	ආපසු EXIT_SUCCESS;
}

static පූර්ණ_සංඛ්‍යා test_fds(පූර්ණ_සංඛ්‍යා argc, var_na *argv[], const var_na *prefix)
{
	DIR *sub = NULL;
	sa_nracanaa path_cxt *pc;
	pid_t pid;
	පූර්ණ_සංඛ්‍යා fd = -1;

	නම් (argc != 2)
		ආපසු EXIT_FAILURE;

	pid = strtol(argv[1], (var_na **) NULL, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	නම් (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	අතරතුර (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	ආපසු EXIT_SUCCESS;
}

static පූර්ණ_සංඛ්‍යා test_processes(පූර්ණ_සංඛ්‍යා argc, var_na *argv[])
{
	DIR *dir;
	sa_nracanaa dirent *d;
	var_na *name = NULL;
	uid_t uid = (uid_t) -1;
	var_na buf[128];

	නම් (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	නම් (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	නම් (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	අතරතුර ((d = xreaddir(dir))) {
		pid_t pid = 0;

		නම් (procfs_dirent_get_pid(d, &pid) != 0)
			continue;
		නම් (name && !procfs_dirent_match_name(dir, d, name))
			continue;
		නම් (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			continue;
		procfs_dirent_get_name(dir, d, buf, parimaa_na(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	ආපසු EXIT_SUCCESS;
}

static පූර්ණ_සංඛ්‍යා test_one_process(පූර්ණ_සංඛ්‍යා argc, var_na *argv[], const var_na *prefix)
{
	pid_t pid;
	sa_nracanaa path_cxt *pc;
	var_na buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	නම් (argc != 2)
		ආපසු EXIT_FAILURE;
	pid = strtol(argv[1], (var_na **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	නම් (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (පූර්ණ_සංඛ්‍යා) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, parimaa_na(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, parimaa_na(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	ආපසු EXIT_SUCCESS;
}

static පූර්ණ_සංඛ්‍යා test_isprocfs(පූර්ණ_සංඛ්‍යා argc, var_na *argv[])
{
	const var_na *name = argc > 1 ? argv[1] : "/proc";
	පූර්ණ_සංඛ්‍යා fd = open(name, O_RDONLY | O_CLOEXEC);
	පූර්ණ_සංඛ්‍යා is = 0;

	නම් (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} නැතහොත්
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	ආපසු is ? EXIT_SUCCESS : EXIT_FAILURE;
}

static පූර්ණ_සංඛ්‍යා test_process_stat_nth(පූර්ණ_සංඛ්‍යා argc, var_na *argv[], const var_na *prefix)
{
	pid_t pid;
	sa_nracanaa path_cxt *pc;
	uint64_t num = 0;
	පූර්ණ_සංඛ්‍යා n, ret;

	නම් (argc != 3)
		ආපසු EXIT_FAILURE;
	pid = strtol(argv[1], (var_na **) NULL, 10);
	n = strtol(argv[2], (var_na **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	නම් (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &num);
	නම් (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (පූර්ණ_සංඛ්‍යා) pid, n, num);
	ul_unref_path(pc);
	ආපසු EXIT_SUCCESS;
}

පූර්ණ_සංඛ්‍යා main(පූර්ණ_සංඛ්‍යා argc, var_na *argv[])
{
	const var_na *prefix = NULL;

	නම් (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	නම් (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		ආපසු EXIT_FAILURE;
	}

	නම් (strcmp(argv[1], "--tasks") == 0)
		ආපසු test_tasks(argc - 1, argv + 1, prefix);
	නම් (strcmp(argv[1], "--fds") == 0)
		ආපසු test_fds(argc - 1, argv + 1, prefix);
	නම් (strcmp(argv[1], "--processes") == 0)
		ආපසු test_processes(argc - 1, argv + 1);
	නම් (strcmp(argv[1], "--is-procfs") == 0)
		ආපසු test_isprocfs(argc - 1, argv + 1);
	නම් (strcmp(argv[1], "--one") == 0)
		ආපසු test_one_process(argc - 1, argv + 1, prefix);
	නම් (strcmp(argv[1], "--stat-nth") == 0)
		ආපසු test_process_stat_nth(argc - 1, argv + 1, prefix);

	ආපසු EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
