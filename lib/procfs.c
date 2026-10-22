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
	만약 (ulprocfs_debug_mask)
		반환;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

struct path_cxt *ul_new_procfs_path(pid_t pid, const char *prefix)
{
	struct path_cxt *pc = ul_new_path(NULL);

	만약 (!pc)
		반환 NULL;
	만약 (prefix)
		ul_path_set_prefix(pc, prefix);

	만약 (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		반환 NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	반환 pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
정수 procfs_process_init_path(struct path_cxt *pc, pid_t pid)
{
	struct procfs_process *prc;
	정수 rc;
	char buf[sizeof(_PATH_PROC) + sizeof(stringify_value(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, sizeof(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	만약 (rc)
		반환 rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	만약 (rc < 0)
		반환 rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	만약 (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, sizeof(struct procfs_process));
		만약 (!prc)
			반환 -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	반환 0;
}

static void procfs_process_deinit_path(struct path_cxt *pc)
{
	struct procfs_process *prc;

	만약 (!pc)
		반환;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	만약 (!prc)
		반환;

	free(prc);
	ul_path_set_dialect(pc, NULL, NULL);
}

static ssize_t read_procfs_file(const struct ul_vfs_ops *vfs, 정수 fd,
			       char *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	만약 (fd < 0)
		반환 -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	만약 (sz <= 0)
		반환 sz;

	위해 (i = 0; i < (size_t) sz; i++) {
		만약 (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	반환 sz;
}

static ssize_t procfs_process_get_data_for(struct path_cxt *pc, char *buf, size_t bufsz,
					    const char *fname)
{
	정수 fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	만약 (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : NULL, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : NULL, fd);
		반환 sz;
	}
	반환 -errno;
}

ssize_t procfs_process_get_cmdline(struct path_cxt *pc, char *buf, size_t bufsz)
{
	반환 procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(struct path_cxt *pc, char *buf, size_t bufsz)
{
	반환 procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(struct path_cxt *pc, char *buf, size_t bufsz)
{
	반환 procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(struct path_cxt *pc, char *buf, size_t bufsz)
{
	반환 procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

정수 procfs_process_get_stat_nth(struct path_cxt *pc, 정수 n, uint64_t *re)
{
	ssize_t rc;
	char *key = NULL, *tok, *p;
	char buf[BUFSIZ];
	정수 i;

	만약 (n == 2 || n == 3)		/* process name and status (strings) */
		반환 -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, sizeof(buf), "stat");
	만약 (rc < 0)
		반환 rc;

	위해 (i = 0, tok = strtok_r(buf, " ", &key); tok;
	     tok = strtok_r(NULL, " ", &key)) {

		i++;
		만약 (i == n)
			반환 ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		만약 (i == 2 && (p = strrchr(key, ')')))
			key = p + 2;
	}

	반환 -EINVAL;
}

정수 procfs_process_get_uid(struct path_cxt *pc, uid_t *uid)
{
	struct stat sb;
	정수 rc;

	만약 ((rc = ul_path_stat(pc, &sb, 0, NULL)) == 0)
		*uid = sb.st_uid;
	반환 rc;
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
정수 procfs_process_next_tid(struct path_cxt *pc, DIR **sub, pid_t *tid)
{
	struct dirent *d;

	만약 (!pc || !sub || !tid)
		반환 -EINVAL;

	만약 (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		만약 (!*sub)
			반환 -errno;
	}

	동안 ((d = xreaddir(*sub))) {
		만약 (procfs_dirent_get_pid(d, tid) == 0)
			반환 0;
	}

	closedir(*sub);
	*sub = NULL;
	반환 1;
}

정수 procfs_process_next_fd(struct path_cxt *pc, DIR **sub, 정수 *fd)
{
	struct dirent *d;

	만약 (!pc || !sub || !fd)
		반환 -EINVAL;

	만약 (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		만약 (!*sub)
			반환 -errno;
	}

	동안 ((d = xreaddir(*sub))) {
		uint64_t num;
#ifdef _DIRENT_HAVE_D_TYPE
		만약 (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			continue;
#endif
		만약 (ul_strtou64(d->d_name, &num, 10) < 0)
			continue;
		*fd = num;
		반환 0;
	}

	closedir(*sub);
	*sub = NULL;
	반환 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
정수 procfs_dirent_is_process(struct dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	만약 (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		반환 0;
#endif
	만약 (!isdigit((unsigned char) *d->d_name))
		반환 0;

	반환 1;
}

정수 procfs_dirent_get_pid(struct dirent *d, pid_t *pid)
{
	uint64_t num;

	만약 (!procfs_dirent_is_process(d))
		반환 -EINVAL;

	만약 (ul_strtou64(d->d_name, &num, 10) < 0)
		반환 -EINVAL;

	*pid = (pid_t) num;
	반환 0;
}

정수 procfs_dirent_get_uid(DIR *procfs, struct dirent *d, uid_t *uid)
{
	struct stat st;

	만약 (!procfs_dirent_is_process(d))
		반환 -EINVAL;

	만약 (fstatat(dirfd(procfs), d->d_name, &st, 0))
		반환 -EINVAL;

	*uid = st.st_uid;
	반환 0;
}

정수 procfs_dirent_match_uid(DIR *procfs, struct dirent *d, uid_t uid)
{
	uid_t x;

	만약 (procfs_dirent_get_uid(procfs, d, &x) == 0)
		반환 x == uid;

	반환 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
정수 procfs_dirent_get_name(DIR *procfs, struct dirent *d, char *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	char tmp[1024], *p, *end = NULL;

	만약 (bufsz < 32)
		반환 -EINVAL;
	만약 (!procfs_dirent_is_process(d))
		반환 -EINVAL;

	snprintf(tmp, sizeof(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	만약 (!f)
		반환 -errno;

	p = fgets(tmp, sizeof(tmp), f);
	fclose(f);
	만약 (!p)
		반환 -errno;

	/* skip PID */
	동안 (*p && *p != '(')
		p++;

	/* skip extra '(' */
	동안 (*p && *p == '(')
		p++;

	end = p;
	동안 (*end && *end != ')')
		end++;

	sz = end - p;
	만약 (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	반환 0;
}

정수 procfs_dirent_match_name(DIR *procfs, struct dirent *d, const char *name)
{
	char buf[33];

	만약 (procfs_dirent_get_name(procfs, d, buf, sizeof(buf)) == 0)
		반환 strcmp(name, buf) == 0;

	반환 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
정수 fd_is_procfs(정수 fd)
{
	struct statfs st;
	정수 ret;

	do {
		errno = 0;
		ret = fstatfs(fd, &st);

		만약 (ret < 0) {
			만약 (errno != EINTR && errno != EAGAIN)
				반환 0;
			xusleep(250000);
		}
	} 동안 (ret != 0);

	반환 F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#아니면
정수 fd_is_procfs(정수 fd __attribute__((__unused__)))
{
	반환 0;
}
#endif

static char *strdup_procfs_file(pid_t pid, const char *name)
{
	char buf[BUFSIZ];
	char *re = NULL;
	정수 fd;

	snprintf(buf, sizeof(buf), _PATH_PROC "/%d/%s", (정수) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	만약 (fd < 0)
		반환 NULL;

	만약 (read_procfs_file(NULL, fd, buf, sizeof(buf)) > 0)
		re = strdup(buf);
	close(fd);
	반환 re;
}

char *pid_get_cmdname(pid_t pid)
{
	반환 strdup_procfs_file(pid, "comm");
}

char *pid_get_cmdline(pid_t pid)
{
	반환 strdup_procfs_file(pid, "cmdline");
}

char *pid_get_personality(pid_t pid)
{
	반환 strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

static 정수 test_tasks(정수 argc, char *argv[], const char *prefix)
{
	DIR *sub = NULL;
	struct path_cxt *pc;
	pid_t tid = 0, pid;

	만약 (argc != 2)
		반환 EXIT_FAILURE;

	pid = strtol(argv[1], (char **) NULL, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	만약 (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	동안 (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	반환 EXIT_SUCCESS;
}

static 정수 test_fds(정수 argc, char *argv[], const char *prefix)
{
	DIR *sub = NULL;
	struct path_cxt *pc;
	pid_t pid;
	정수 fd = -1;

	만약 (argc != 2)
		반환 EXIT_FAILURE;

	pid = strtol(argv[1], (char **) NULL, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	만약 (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	동안 (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	반환 EXIT_SUCCESS;
}

static 정수 test_processes(정수 argc, char *argv[])
{
	DIR *dir;
	struct dirent *d;
	char *name = NULL;
	uid_t uid = (uid_t) -1;
	char buf[128];

	만약 (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	만약 (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	만약 (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	동안 ((d = xreaddir(dir))) {
		pid_t pid = 0;

		만약 (procfs_dirent_get_pid(d, &pid) != 0)
			continue;
		만약 (name && !procfs_dirent_match_name(dir, d, name))
			continue;
		만약 (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			continue;
		procfs_dirent_get_name(dir, d, buf, sizeof(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	반환 EXIT_SUCCESS;
}

static 정수 test_one_process(정수 argc, char *argv[], const char *prefix)
{
	pid_t pid;
	struct path_cxt *pc;
	char buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	만약 (argc != 2)
		반환 EXIT_FAILURE;
	pid = strtol(argv[1], (char **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	만약 (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (정수) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, sizeof(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, sizeof(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	반환 EXIT_SUCCESS;
}

static 정수 test_isprocfs(정수 argc, char *argv[])
{
	const char *name = argc > 1 ? argv[1] : "/proc";
	정수 fd = open(name, O_RDONLY | O_CLOEXEC);
	정수 is = 0;

	만약 (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} 아니면
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	반환 is ? EXIT_SUCCESS : EXIT_FAILURE;
}

static 정수 test_process_stat_nth(정수 argc, char *argv[], const char *prefix)
{
	pid_t pid;
	struct path_cxt *pc;
	uint64_t num = 0;
	정수 n, ret;

	만약 (argc != 3)
		반환 EXIT_FAILURE;
	pid = strtol(argv[1], (char **) NULL, 10);
	n = strtol(argv[2], (char **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	만약 (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &num);
	만약 (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (정수) pid, n, num);
	ul_unref_path(pc);
	반환 EXIT_SUCCESS;
}

정수 main(정수 argc, char *argv[])
{
	const char *prefix = NULL;

	만약 (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	만약 (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		반환 EXIT_FAILURE;
	}

	만약 (strcmp(argv[1], "--tasks") == 0)
		반환 test_tasks(argc - 1, argv + 1, prefix);
	만약 (strcmp(argv[1], "--fds") == 0)
		반환 test_fds(argc - 1, argv + 1, prefix);
	만약 (strcmp(argv[1], "--processes") == 0)
		반환 test_processes(argc - 1, argv + 1);
	만약 (strcmp(argv[1], "--is-procfs") == 0)
		반환 test_isprocfs(argc - 1, argv + 1);
	만약 (strcmp(argv[1], "--one") == 0)
		반환 test_one_process(argc - 1, argv + 1, prefix);
	만약 (strcmp(argv[1], "--stat-nth") == 0)
		반환 test_process_stat_nth(argc - 1, argv + 1, prefix);

	반환 EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
