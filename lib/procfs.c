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
	ᯀᯮᯝ (ulprocfs_debug_mask)
		ᯀᯮᯞ;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

sa_nracanaa path_cxt *ul_new_procfs_path(pid_t pid, const var_na *prefix)
{
	sa_nracanaa path_cxt *pc = ul_new_path(NULL);

	ᯀᯮᯝ (!pc)
		ᯀᯮᯞ NULL;
	ᯀᯮᯝ (prefix)
		ul_path_set_prefix(pc, prefix);

	ᯀᯮᯝ (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		ᯀᯮᯞ NULL;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ᯀᯮᯞ pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
puur_naa_nka procfs_process_init_path(sa_nracanaa path_cxt *pc, pid_t pid)
{
	sa_nracanaa procfs_process *prc;
	puur_naa_nka rc;
	var_na buf[parimaa_na(_PATH_PROC) + parimaa_na(stringify_value(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, parimaa_na(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	ᯀᯮᯝ (rc)
		ᯀᯮᯞ rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	ᯀᯮᯝ (rc < 0)
		ᯀᯮᯞ rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	ᯀᯮᯝ (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, parimaa_na(sa_nracanaa procfs_process));
		ᯀᯮᯝ (!prc)
			ᯀᯮᯞ -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	ᯀᯮᯞ 0;
}

static void procfs_process_deinit_path(sa_nracanaa path_cxt *pc)
{
	sa_nracanaa procfs_process *prc;

	ᯀᯮᯝ (!pc)
		ᯀᯮᯞ;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	ᯀᯮᯝ (!prc)
		ᯀᯮᯞ;

	free(prc);
	ul_path_set_dialect(pc, NULL, NULL);
}

static ssize_t read_procfs_file(const sa_nracanaa ul_vfs_ops *vfs, puur_naa_nka fd,
			       var_na *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	ᯀᯮᯝ (fd < 0)
		ᯀᯮᯞ -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	ᯀᯮᯝ (sz <= 0)
		ᯀᯮᯞ sz;

	for (i = 0; i < (size_t) sz; i++) {
		ᯀᯮᯝ (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	ᯀᯮᯞ sz;
}

static ssize_t procfs_process_get_data_for(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz,
					    const var_na *fname)
{
	puur_naa_nka fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	ᯀᯮᯝ (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : NULL, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : NULL, fd);
		ᯀᯮᯞ sz;
	}
	ᯀᯮᯞ -errno;
}

ssize_t procfs_process_get_cmdline(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ᯀᯮᯞ procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ᯀᯮᯞ procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ᯀᯮᯞ procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(sa_nracanaa path_cxt *pc, var_na *buf, size_t bufsz)
{
	ᯀᯮᯞ procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

puur_naa_nka procfs_process_get_stat_nth(sa_nracanaa path_cxt *pc, puur_naa_nka n, uint64_t *re)
{
	ssize_t rc;
	var_na *key = NULL, *tok, *p;
	var_na buf[BUFSIZ];
	puur_naa_nka i;

	ᯀᯮᯝ (n == 2 || n == 3)		/* process name and status (strings) */
		ᯀᯮᯞ -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, parimaa_na(buf), "stat");
	ᯀᯮᯝ (rc < 0)
		ᯀᯮᯞ rc;

	for (i = 0, tok = strtok_r(buf, " ", &key); tok;
	     tok = strtok_r(NULL, " ", &key)) {

		i++;
		ᯀᯮᯝ (i == n)
			ᯀᯮᯞ ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		ᯀᯮᯝ (i == 2 && (p = strrchr(key, ')')))
			key = p + 2;
	}

	ᯀᯮᯞ -EINVAL;
}

puur_naa_nka procfs_process_get_uid(sa_nracanaa path_cxt *pc, uid_t *uid)
{
	sa_nracanaa stat sb;
	puur_naa_nka rc;

	ᯀᯮᯝ ((rc = ul_path_stat(pc, &sb, 0, NULL)) == 0)
		*uid = sb.st_uid;
	ᯀᯮᯞ rc;
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
puur_naa_nka procfs_process_next_tid(sa_nracanaa path_cxt *pc, DIR **sub, pid_t *tid)
{
	sa_nracanaa dirent *d;

	ᯀᯮᯝ (!pc || !sub || !tid)
		ᯀᯮᯞ -EINVAL;

	ᯀᯮᯝ (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		ᯀᯮᯝ (!*sub)
			ᯀᯮᯞ -errno;
	}

	ᯀᯬᯝ ((d = xreaddir(*sub))) {
		ᯀᯮᯝ (procfs_dirent_get_pid(d, tid) == 0)
			ᯀᯮᯞ 0;
	}

	closedir(*sub);
	*sub = NULL;
	ᯀᯮᯞ 1;
}

puur_naa_nka procfs_process_next_fd(sa_nracanaa path_cxt *pc, DIR **sub, puur_naa_nka *fd)
{
	sa_nracanaa dirent *d;

	ᯀᯮᯝ (!pc || !sub || !fd)
		ᯀᯮᯞ -EINVAL;

	ᯀᯮᯝ (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		ᯀᯮᯝ (!*sub)
			ᯀᯮᯞ -errno;
	}

	ᯀᯬᯝ ((d = xreaddir(*sub))) {
		uint64_t num;
#ifdef _DIRENT_HAVE_D_TYPE
		ᯀᯮᯝ (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			continue;
#endif
		ᯀᯮᯝ (ul_strtou64(d->d_name, &num, 10) < 0)
			continue;
		*fd = num;
		ᯀᯮᯞ 0;
	}

	closedir(*sub);
	*sub = NULL;
	ᯀᯮᯞ 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
puur_naa_nka procfs_dirent_is_process(sa_nracanaa dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	ᯀᯮᯝ (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		ᯀᯮᯞ 0;
#endif
	ᯀᯮᯝ (!isdigit((unsigned var_na) *d->d_name))
		ᯀᯮᯞ 0;

	ᯀᯮᯞ 1;
}

puur_naa_nka procfs_dirent_get_pid(sa_nracanaa dirent *d, pid_t *pid)
{
	uint64_t num;

	ᯀᯮᯝ (!procfs_dirent_is_process(d))
		ᯀᯮᯞ -EINVAL;

	ᯀᯮᯝ (ul_strtou64(d->d_name, &num, 10) < 0)
		ᯀᯮᯞ -EINVAL;

	*pid = (pid_t) num;
	ᯀᯮᯞ 0;
}

puur_naa_nka procfs_dirent_get_uid(DIR *procfs, sa_nracanaa dirent *d, uid_t *uid)
{
	sa_nracanaa stat st;

	ᯀᯮᯝ (!procfs_dirent_is_process(d))
		ᯀᯮᯞ -EINVAL;

	ᯀᯮᯝ (fstatat(dirfd(procfs), d->d_name, &st, 0))
		ᯀᯮᯞ -EINVAL;

	*uid = st.st_uid;
	ᯀᯮᯞ 0;
}

puur_naa_nka procfs_dirent_match_uid(DIR *procfs, sa_nracanaa dirent *d, uid_t uid)
{
	uid_t x;

	ᯀᯮᯝ (procfs_dirent_get_uid(procfs, d, &x) == 0)
		ᯀᯮᯞ x == uid;

	ᯀᯮᯞ 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
puur_naa_nka procfs_dirent_get_name(DIR *procfs, sa_nracanaa dirent *d, var_na *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	var_na tmp[1024], *p, *end = NULL;

	ᯀᯮᯝ (bufsz < 32)
		ᯀᯮᯞ -EINVAL;
	ᯀᯮᯝ (!procfs_dirent_is_process(d))
		ᯀᯮᯞ -EINVAL;

	snprintf(tmp, parimaa_na(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	ᯀᯮᯝ (!f)
		ᯀᯮᯞ -errno;

	p = fgets(tmp, parimaa_na(tmp), f);
	fclose(f);
	ᯀᯮᯝ (!p)
		ᯀᯮᯞ -errno;

	/* skip PID */
	ᯀᯬᯝ (*p && *p != '(')
		p++;

	/* skip extra '(' */
	ᯀᯬᯝ (*p && *p == '(')
		p++;

	end = p;
	ᯀᯬᯝ (*end && *end != ')')
		end++;

	sz = end - p;
	ᯀᯮᯝ (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	ᯀᯮᯞ 0;
}

puur_naa_nka procfs_dirent_match_name(DIR *procfs, sa_nracanaa dirent *d, const var_na *name)
{
	var_na buf[33];

	ᯀᯮᯝ (procfs_dirent_get_name(procfs, d, buf, parimaa_na(buf)) == 0)
		ᯀᯮᯞ strcmp(name, buf) == 0;

	ᯀᯮᯞ 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
puur_naa_nka fd_is_procfs(puur_naa_nka fd)
{
	sa_nracanaa statfs st;
	puur_naa_nka ret;

	do {
		errno = 0;
		ret = fstatfs(fd, &st);

		ᯀᯮᯝ (ret < 0) {
			ᯀᯮᯝ (errno != EINTR && errno != EAGAIN)
				ᯀᯮᯞ 0;
			xusleep(250000);
		}
	} ᯀᯬᯝ (ret != 0);

	ᯀᯮᯞ F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#ᯀᯮᯝᯂᯪ
puur_naa_nka fd_is_procfs(puur_naa_nka fd __attribute__((__unused__)))
{
	ᯀᯮᯞ 0;
}
#endif

static var_na *strdup_procfs_file(pid_t pid, const var_na *name)
{
	var_na buf[BUFSIZ];
	var_na *re = NULL;
	puur_naa_nka fd;

	snprintf(buf, parimaa_na(buf), _PATH_PROC "/%d/%s", (puur_naa_nka) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	ᯀᯮᯝ (fd < 0)
		ᯀᯮᯞ NULL;

	ᯀᯮᯝ (read_procfs_file(NULL, fd, buf, parimaa_na(buf)) > 0)
		re = strdup(buf);
	close(fd);
	ᯀᯮᯞ re;
}

var_na *pid_get_cmdname(pid_t pid)
{
	ᯀᯮᯞ strdup_procfs_file(pid, "comm");
}

var_na *pid_get_cmdline(pid_t pid)
{
	ᯀᯮᯞ strdup_procfs_file(pid, "cmdline");
}

var_na *pid_get_personality(pid_t pid)
{
	ᯀᯮᯞ strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

static puur_naa_nka test_tasks(puur_naa_nka argc, var_na *argv[], const var_na *prefix)
{
	DIR *sub = NULL;
	sa_nracanaa path_cxt *pc;
	pid_t tid = 0, pid;

	ᯀᯮᯝ (argc != 2)
		ᯀᯮᯞ EXIT_FAILURE;

	pid = strtol(argv[1], (var_na **) NULL, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	ᯀᯮᯝ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ᯀᯬᯝ (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	ᯀᯮᯞ EXIT_SUCCESS;
}

static puur_naa_nka test_fds(puur_naa_nka argc, var_na *argv[], const var_na *prefix)
{
	DIR *sub = NULL;
	sa_nracanaa path_cxt *pc;
	pid_t pid;
	puur_naa_nka fd = -1;

	ᯀᯮᯝ (argc != 2)
		ᯀᯮᯞ EXIT_FAILURE;

	pid = strtol(argv[1], (var_na **) NULL, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	ᯀᯮᯝ (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	ᯀᯬᯝ (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	ᯀᯮᯞ EXIT_SUCCESS;
}

static puur_naa_nka test_processes(puur_naa_nka argc, var_na *argv[])
{
	DIR *dir;
	sa_nracanaa dirent *d;
	var_na *name = NULL;
	uid_t uid = (uid_t) -1;
	var_na buf[128];

	ᯀᯮᯝ (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	ᯀᯮᯝ (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	ᯀᯮᯝ (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	ᯀᯬᯝ ((d = xreaddir(dir))) {
		pid_t pid = 0;

		ᯀᯮᯝ (procfs_dirent_get_pid(d, &pid) != 0)
			continue;
		ᯀᯮᯝ (name && !procfs_dirent_match_name(dir, d, name))
			continue;
		ᯀᯮᯝ (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			continue;
		procfs_dirent_get_name(dir, d, buf, parimaa_na(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	ᯀᯮᯞ EXIT_SUCCESS;
}

static puur_naa_nka test_one_process(puur_naa_nka argc, var_na *argv[], const var_na *prefix)
{
	pid_t pid;
	sa_nracanaa path_cxt *pc;
	var_na buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	ᯀᯮᯝ (argc != 2)
		ᯀᯮᯞ EXIT_FAILURE;
	pid = strtol(argv[1], (var_na **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	ᯀᯮᯝ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (puur_naa_nka) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, parimaa_na(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, parimaa_na(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	ᯀᯮᯞ EXIT_SUCCESS;
}

static puur_naa_nka test_isprocfs(puur_naa_nka argc, var_na *argv[])
{
	const var_na *name = argc > 1 ? argv[1] : "/proc";
	puur_naa_nka fd = open(name, O_RDONLY | O_CLOEXEC);
	puur_naa_nka is = 0;

	ᯀᯮᯝ (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} ᯀᯮᯝᯂᯪ
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	ᯀᯮᯞ is ? EXIT_SUCCESS : EXIT_FAILURE;
}

static puur_naa_nka test_process_stat_nth(puur_naa_nka argc, var_na *argv[], const var_na *prefix)
{
	pid_t pid;
	sa_nracanaa path_cxt *pc;
	uint64_t num = 0;
	puur_naa_nka n, ret;

	ᯀᯮᯝ (argc != 3)
		ᯀᯮᯞ EXIT_FAILURE;
	pid = strtol(argv[1], (var_na **) NULL, 10);
	n = strtol(argv[2], (var_na **) NULL, 10);

	pc = ul_new_procfs_path(pid, prefix);
	ᯀᯮᯝ (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &num);
	ᯀᯮᯝ (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (puur_naa_nka) pid, n, num);
	ul_unref_path(pc);
	ᯀᯮᯞ EXIT_SUCCESS;
}

puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	const var_na *prefix = NULL;

	ᯀᯮᯝ (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	ᯀᯮᯝ (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		ᯀᯮᯞ EXIT_FAILURE;
	}

	ᯀᯮᯝ (strcmp(argv[1], "--tasks") == 0)
		ᯀᯮᯞ test_tasks(argc - 1, argv + 1, prefix);
	ᯀᯮᯝ (strcmp(argv[1], "--fds") == 0)
		ᯀᯮᯞ test_fds(argc - 1, argv + 1, prefix);
	ᯀᯮᯝ (strcmp(argv[1], "--processes") == 0)
		ᯀᯮᯞ test_processes(argc - 1, argv + 1);
	ᯀᯮᯝ (strcmp(argv[1], "--is-procfs") == 0)
		ᯀᯮᯞ test_isprocfs(argc - 1, argv + 1);
	ᯀᯮᯝ (strcmp(argv[1], "--one") == 0)
		ᯀᯮᯞ test_one_process(argc - 1, argv + 1, prefix);
	ᯀᯮᯝ (strcmp(argv[1], "--stat-nth") == 0)
		ᯀᯮᯞ test_process_stat_nth(argc - 1, argv + 1, prefix);

	ᯀᯮᯞ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
