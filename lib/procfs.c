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

static void procfs_process_deinit_path(struct path_cxt *pc);

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
	もし (ulprocfs_debug_mask)
		戻る;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

struct path_cxt *ul_new_procfs_path(pid_t pid, const char *prefix)
{
	struct path_cxt *pc = ul_new_path(NULL);

	もし (!pc)
		戻る NULL;
	もし (prefix)
		ul_path_set_prefix(pc, prefix);

	もし (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		戻る NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	戻る pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
整数 procfs_process_init_path(struct path_cxt *pc, pid_t pid)
{
	struct procfs_process *prc;
	整数 rc;
	char buf[sizeof(_PATH_PROC) + sizeof(stringify_value(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, sizeof(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	もし (rc)
		戻る rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	もし (rc < 0)
		戻る rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	もし (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, sizeof(struct procfs_process));
		もし (!prc)
			戻る -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	戻る 0;
}

static void procfs_process_deinit_path(struct path_cxt *pc)
{
	struct procfs_process *prc;

	もし (!pc)
		戻る;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	もし (!prc)
		戻る;

	free(prc);
	ul_path_set_dialect(pc, NULL, NULL);
}

static ssize_t read_procfs_file(const struct ul_vfs_ops *vfs, 整数 fd,
			       char *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	もし (fd < 0)
		戻る -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	もし (sz <= 0)
		戻る sz;

	ために (i = 0; i < (size_t) sz; i++) {
		もし (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	戻る sz;
}

static ssize_t procfs_process_get_data_for(struct path_cxt *pc, char *buf, size_t bufsz,
					    const char *fname)
{
	整数 fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	もし (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : NULL, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : NULL, fd);
		戻る sz;
	}
	戻る -errno;
}

ssize_t procfs_process_get_cmdline(struct path_cxt *pc, char *buf, size_t bufsz)
{
	戻る procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(struct path_cxt *pc, char *buf, size_t bufsz)
{
	戻る procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(struct path_cxt *pc, char *buf, size_t bufsz)
{
	戻る procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(struct path_cxt *pc, char *buf, size_t bufsz)
{
	戻る procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

整数 procfs_process_get_stat_nth(struct path_cxt *pc, 整数 n, uint64_t *re)
{
	ssize_t rc;
	char *key = NULL, *tok, *p;
	char buf[BUFSIZ];
	整数 i;

	もし (n == 2 || n == 3)		/* process name and status (strings) */
		戻る -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, sizeof(buf), "stat");
	もし (rc < 0)
		戻る rc;

	ために (i = 0, tok = strtok_r(buf, " ", &key); tok;
	     tok = strtok_r(NULL, " ", &key)) {

		i++;
		もし (i == n)
			戻る ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		もし (i == 2 && (p = strrchr(key, ')')))
			key = p + 2;
	}

	戻る -EINVAL;
}

整数 procfs_process_get_uid(struct path_cxt *pc, uid_t *uid)
{
	struct stat sb;
	整数 rc;

	もし ((rc = ul_path_stat(pc, &sb, 0, NULL)) == 0)
		*uid = sb.st_uid;
	戻る rc;
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
整数 procfs_process_next_tid(struct path_cxt *pc, DIR **sub, pid_t *tid)
{
	struct dirent *d;

	もし (!pc || !sub || !tid)
		戻る -EINVAL;

	もし (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		もし (!*sub)
			戻る -errno;
	}

	ながら ((d = xreaddir(*sub))) {
		もし (procfs_dirent_get_pid(d, tid) == 0)
			戻る 0;
	}

	closedir(*sub);
	*sub = NULL;
	戻る 1;
}

整数 procfs_process_next_fd(struct path_cxt *pc, DIR **sub, 整数 *fd)
{
	struct dirent *d;

	もし (!pc || !sub || !fd)
		戻る -EINVAL;

	もし (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		もし (!*sub)
			戻る -errno;
	}

	ながら ((d = xreaddir(*sub))) {
		uint64_t num;
#ifdef _DIRENT_HAVE_D_TYPE
		もし (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			continue;
#endif
		もし (ul_strtou64(d->d_name, &num, 10) < 0)
			continue;
		*fd = num;
		戻る 0;
	}

	closedir(*sub);
	*sub = NULL;
	戻る 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
整数 procfs_dirent_is_process(struct dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	もし (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		戻る 0;
#endif
	もし (!isdigit((unsigned char) *d->d_name))
		戻る 0;

	戻る 1;
}

整数 procfs_dirent_get_pid(struct dirent *d, pid_t *pid)
{
	uint64_t num;

	もし (!procfs_dirent_is_process(d))
		戻る -EINVAL;

	もし (ul_strtou64(d->d_name, &num, 10) < 0)
		戻る -EINVAL;

	*pid = (pid_t) num;
	戻る 0;
}

整数 procfs_dirent_get_uid(DIR *procfs, struct dirent *d, uid_t *uid)
{
	struct stat st;

	もし (!procfs_dirent_is_process(d))
		戻る -EINVAL;

	もし (fstatat(dirfd(procfs), d->d_name, &st, 0))
		戻る -EINVAL;

	*uid = st.st_uid;
	戻る 0;
}

整数 procfs_dirent_match_uid(DIR *procfs, struct dirent *d, uid_t uid)
{
	uid_t x;

	もし (procfs_dirent_get_uid(procfs, d, &x) == 0)
		戻る x == uid;

	戻る 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
整数 procfs_dirent_get_name(DIR *procfs, struct dirent *d, char *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	char tmp[1024], *p, *end = NULL;

	もし (bufsz < 32)
		戻る -EINVAL;
	もし (!procfs_dirent_is_process(d))
		戻る -EINVAL;

	snprintf(tmp, sizeof(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	もし (!f)
		戻る -errno;

	p = fgets(tmp, sizeof(tmp), f);
	fclose(f);
	もし (!p)
		戻る -errno;

	/* skip PID */
	ながら (*p && *p != '(')
		p++;

	/* skip extra '(' */
	ながら (*p && *p == '(')
		p++;

	end = p;
	ながら (*end && *end != ')')
		end++;

	sz = end - p;
	もし (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	戻る 0;
}

整数 procfs_dirent_match_name(DIR *procfs, struct dirent *d, const char *name)
{
	char buf[33];

	もし (procfs_dirent_get_name(procfs, d, buf, sizeof(buf)) == 0)
		戻る strcmp(name, buf) == 0;

	戻る 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
整数 fd_is_procfs(整数 fd)
{
	struct statfs st;
	整数 ret;

	do {
		errno = 0;
		ret = fstatfs(fd, &st);

		もし (ret < 0) {
			もし (errno != EINTR && errno != EAGAIN)
				戻る 0;
			xusleep(250000);
		}
	} ながら (ret != 0);

	戻る F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#否则
整数 fd_is_procfs(整数 fd __attribute__((__unused__)))
{
	戻る 0;
}
#endif

static char *strdup_procfs_file(pid_t pid, const char *name)
{
	char buf[BUFSIZ];
	char *re = NULL;
	整数 fd;

	snprintf(buf, sizeof(buf), _PATH_PROC "/%d/%s", (整数) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	もし (fd < 0)
		戻る NULL;

	もし (read_procfs_file(NULL, fd, buf, sizeof(buf)) > 0)
		re = strdup(buf);
	close(fd);
	戻る re;
}

char *pid_get_cmdname(pid_t pid)
{
	戻る strdup_procfs_file(pid, "comm");
}

char *pid_get_cmdline(pid_t pid)
{
	戻る strdup_procfs_file(pid, "cmdline");
}

char *pid_get_personality(pid_t pid)
{
	戻る strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

static 整数 test_tasks(整数 argc, char *argv[], const char *prefix)
{
	DIR *sub = NULL;
	struct path_cxt *pc;
	pid_t tid = 0, pid;

	もし (argc != 2)
		戻る EXIT_FAILURE;

	pid = strtol(argv[1], (char **) NULL, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	もし (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ながら (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	戻る EXIT_SUCCESS;
}

static 整数 test_fds(整数 argc, char *argv[], const char *prefix)
{
	DIR *sub = NULL;
	struct path_cxt *pc;
	pid_t pid;
	整数 fd = -1;

	もし (argc != 2)
		戻る EXIT_FAILURE;

	pid = strtol(argv[1], (char **) NULL, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	もし (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ながら (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	戻る EXIT_SUCCESS;
}

static 整数 test_processes(整数 argc, char *argv[])
{
	DIR *dir;
	struct dirent *d;
	char *name = NULL;
	uid_t uid = (uid_t) -1;
	char buf[128];

	もし (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	もし (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	もし (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	ながら ((d = xreaddir(dir))) {
		pid_t pid = 0;

		もし (procfs_dirent_get_pid(d, &pid) != 0)
			continue;
		もし (name && !procfs_dirent_match_name(dir, d, name))
			continue;
		もし (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			continue;
		procfs_dirent_get_name(dir, d, buf, sizeof(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	戻る EXIT_SUCCESS;
}

static 整数 test_one_process(整数 argc, char *argv[], const char *prefix)
{
	pid_t pid;
	struct path_cxt *pc;
	char buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	もし (argc != 2)
		戻る EXIT_FAILURE;
	pid = strtol(argv[1], (char **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	もし (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (整数) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, sizeof(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, sizeof(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	戻る EXIT_SUCCESS;
}

static 整数 test_isprocfs(整数 argc, char *argv[])
{
	const char *name = argc > 1 ? argv[1] : "/proc";
	整数 fd = open(name, O_RDONLY | O_CLOEXEC);
	整数 is = 0;

	もし (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} 否则
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	戻る is ? EXIT_SUCCESS : EXIT_FAILURE;
}

static 整数 test_process_stat_nth(整数 argc, char *argv[], const char *prefix)
{
	pid_t pid;
	struct path_cxt *pc;
	uint64_t num = 0;
	整数 n, ret;

	もし (argc != 3)
		戻る EXIT_FAILURE;
	pid = strtol(argv[1], (char **) NULL, 10);
	n = strtol(argv[2], (char **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	もし (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &num);
	もし (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (整数) pid, n, num);
	ul_unref_path(pc);
	戻る EXIT_SUCCESS;
}

整数 メインページ(整数 argc, char *argv[])
{
	const char *prefix = NULL;

	もし (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	もし (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		戻る EXIT_FAILURE;
	}

	もし (strcmp(argv[1], "--tasks") == 0)
		戻る test_tasks(argc - 1, argv + 1, prefix);
	もし (strcmp(argv[1], "--fds") == 0)
		戻る test_fds(argc - 1, argv + 1, prefix);
	もし (strcmp(argv[1], "--processes") == 0)
		戻る test_processes(argc - 1, argv + 1);
	もし (strcmp(argv[1], "--is-procfs") == 0)
		戻る test_isprocfs(argc - 1, argv + 1);
	もし (strcmp(argv[1], "--one") == 0)
		戻る test_one_process(argc - 1, argv + 1, prefix);
	もし (strcmp(argv[1], "--stat-nth") == 0)
		戻る test_process_stat_nth(argc - 1, argv + 1, prefix);

	戻る EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
