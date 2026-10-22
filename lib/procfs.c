/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#অন্তর্ভাব <ctype.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <errno.h>

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "pathnames.h"
#অন্তর্ভাব "procfs.h"
#অন্তর্ভাব "fileutils.h"
#অন্তর্ভাব "all-io.h"
#অন্তর্ভাব "vfs.h"
#অন্তর্ভাব "debug.h"
#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "statfs_magic.h"

স্থির শূন্য procfs_process_deinit_path(সংরচনা path_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
স্থির UL_DEBUG_DEFINE_MASK(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPROCFS_DEBUG_INIT	(1 << 1)
#define ULPROCFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

শূন্য ul_procfs_init_debug(শূন্য)
{
	যদি (ulprocfs_debug_mask)
		প্রত্যাগম;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

সংরচনা path_cxt *ul_new_procfs_path(pid_t pid, ধ্রুব বর্ণ *prefix)
{
	সংরচনা path_cxt *pc = ul_new_path(রিক্ত);

	যদি (!pc)
		প্রত্যাগম রিক্ত;
	যদি (prefix)
		ul_path_set_prefix(pc, prefix);

	যদি (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		প্রত্যাগম রিক্ত;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	প্রত্যাগম pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
পূর্ণাঙ্ক procfs_process_init_path(সংরচনা path_cxt *pc, pid_t pid)
{
	সংরচনা procfs_process *prc;
	পূর্ণাঙ্ক rc;
	বর্ণ buf[পরিমাণ(_PATH_PROC) + পরিমাণ(stringify_মান(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, পরিমাণ(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	যদি (rc)
		প্রত্যাগম rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	যদি (rc < 0)
		প্রত্যাগম rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	যদি (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, পরিমাণ(সংরচনা procfs_process));
		যদি (!prc)
			প্রত্যাগম -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	প্রত্যাগম 0;
}

স্থির শূন্য procfs_process_deinit_path(সংরচনা path_cxt *pc)
{
	সংরচনা procfs_process *prc;

	যদি (!pc)
		প্রত্যাগম;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	যদি (!prc)
		প্রত্যাগম;

	মুক্ত(prc);
	ul_path_set_dialect(pc, রিক্ত, রিক্ত);
}

স্থির ssize_t read_procfs_file(ধ্রুব সংরচনা ul_vfs_ops *vfs, পূর্ণাঙ্ক fd,
			       বর্ণ *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	যদি (fd < 0)
		প্রত্যাগম -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	যদি (sz <= 0)
		প্রত্যাগম sz;

	প্রতি (i = 0; i < (size_t) sz; i++) {
		যদি (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	প্রত্যাগম sz;
}

স্থির ssize_t procfs_process_get_data_for(সংরচনা path_cxt *pc, বর্ণ *buf, size_t bufsz,
					    ধ্রুব বর্ণ *fname)
{
	পূর্ণাঙ্ক fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	যদি (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : রিক্ত, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : রিক্ত, fd);
		প্রত্যাগম sz;
	}
	প্রত্যাগম -errno;
}

ssize_t procfs_process_get_cmdline(সংরচনা path_cxt *pc, বর্ণ *buf, size_t bufsz)
{
	প্রত্যাগম procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(সংরচনা path_cxt *pc, বর্ণ *buf, size_t bufsz)
{
	প্রত্যাগম procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(সংরচনা path_cxt *pc, বর্ণ *buf, size_t bufsz)
{
	প্রত্যাগম procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(সংরচনা path_cxt *pc, বর্ণ *buf, size_t bufsz)
{
	প্রত্যাগম procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

পূর্ণাঙ্ক procfs_process_get_stat_nth(সংরচনা path_cxt *pc, পূর্ণাঙ্ক n, uint64_t *re)
{
	ssize_t rc;
	বর্ণ *চাবি = রিক্ত, *tok, *p;
	বর্ণ buf[BUFSIZ];
	পূর্ণাঙ্ক i;

	যদি (n == 2 || n == 3)		/* process name and status (strings) */
		প্রত্যাগম -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, পরিমাণ(buf), "stat");
	যদি (rc < 0)
		প্রত্যাগম rc;

	প্রতি (i = 0, tok = strtok_r(buf, " ", &চাবি); tok;
	     tok = strtok_r(রিক্ত, " ", &চাবি)) {

		i++;
		যদি (i == n)
			প্রত্যাগম ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		যদি (i == 2 && (p = strrchr(চাবি, ')')))
			চাবি = p + 2;
	}

	প্রত্যাগম -EINVAL;
}

পূর্ণাঙ্ক procfs_process_get_uid(সংরচনা path_cxt *pc, uid_t *uid)
{
	সংরচনা stat sb;
	পূর্ণাঙ্ক rc;

	যদি ((rc = ul_path_stat(pc, &sb, 0, রিক্ত)) == 0)
		*uid = sb.st_uid;
	প্রত্যাগম rc;
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
পূর্ণাঙ্ক procfs_process_next_tid(সংরচনা path_cxt *pc, DIR **sub, pid_t *tid)
{
	সংরচনা dirent *d;

	যদি (!pc || !sub || !tid)
		প্রত্যাগম -EINVAL;

	যদি (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		যদি (!*sub)
			প্রত্যাগম -errno;
	}

	যাবত্ ((d = xreaddir(*sub))) {
		যদি (procfs_dirent_get_pid(d, tid) == 0)
			প্রত্যাগম 0;
	}

	closedir(*sub);
	*sub = রিক্ত;
	প্রত্যাগম 1;
}

পূর্ণাঙ্ক procfs_process_next_fd(সংরচনা path_cxt *pc, DIR **sub, পূর্ণাঙ্ক *fd)
{
	সংরচনা dirent *d;

	যদি (!pc || !sub || !fd)
		প্রত্যাগম -EINVAL;

	যদি (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		যদি (!*sub)
			প্রত্যাগম -errno;
	}

	যাবত্ ((d = xreaddir(*sub))) {
		uint64_t অঙ্ক;
#ifdef _DIRENT_HAVE_D_TYPE
		যদি (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			অনুবর্ত;
#endif
		যদি (ul_strtou64(d->d_name, &অঙ্ক, 10) < 0)
			অনুবর্ত;
		*fd = অঙ্ক;
		প্রত্যাগম 0;
	}

	closedir(*sub);
	*sub = রিক্ত;
	প্রত্যাগম 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
পূর্ণাঙ্ক procfs_dirent_is_process(সংরচনা dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	যদি (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		প্রত্যাগম 0;
#endif
	যদি (!isdigit((unsigned বর্ণ) *d->d_name))
		প্রত্যাগম 0;

	প্রত্যাগম 1;
}

পূর্ণাঙ্ক procfs_dirent_get_pid(সংরচনা dirent *d, pid_t *pid)
{
	uint64_t অঙ্ক;

	যদি (!procfs_dirent_is_process(d))
		প্রত্যাগম -EINVAL;

	যদি (ul_strtou64(d->d_name, &অঙ্ক, 10) < 0)
		প্রত্যাগম -EINVAL;

	*pid = (pid_t) অঙ্ক;
	প্রত্যাগম 0;
}

পূর্ণাঙ্ক procfs_dirent_get_uid(DIR *procfs, সংরচনা dirent *d, uid_t *uid)
{
	সংরচনা stat st;

	যদি (!procfs_dirent_is_process(d))
		প্রত্যাগম -EINVAL;

	যদি (fstatat(dirfd(procfs), d->d_name, &st, 0))
		প্রত্যাগম -EINVAL;

	*uid = st.st_uid;
	প্রত্যাগম 0;
}

পূর্ণাঙ্ক procfs_dirent_match_uid(DIR *procfs, সংরচনা dirent *d, uid_t uid)
{
	uid_t x;

	যদি (procfs_dirent_get_uid(procfs, d, &x) == 0)
		প্রত্যাগম x == uid;

	প্রত্যাগম 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
পূর্ণাঙ্ক procfs_dirent_get_name(DIR *procfs, সংরচনা dirent *d, বর্ণ *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	বর্ণ tmp[1024], *p, *end = রিক্ত;

	যদি (bufsz < 32)
		প্রত্যাগম -EINVAL;
	যদি (!procfs_dirent_is_process(d))
		প্রত্যাগম -EINVAL;

	snprintf(tmp, পরিমাণ(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	যদি (!f)
		প্রত্যাগম -errno;

	p = fgets(tmp, পরিমাণ(tmp), f);
	fclose(f);
	যদি (!p)
		প্রত্যাগম -errno;

	/* skip PID */
	যাবত্ (*p && *p != '(')
		p++;

	/* skip extra '(' */
	যাবত্ (*p && *p == '(')
		p++;

	end = p;
	যাবত্ (*end && *end != ')')
		end++;

	sz = end - p;
	যদি (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক procfs_dirent_match_name(DIR *procfs, সংরচনা dirent *d, ধ্রুব বর্ণ *name)
{
	বর্ণ buf[33];

	যদি (procfs_dirent_get_name(procfs, d, buf, পরিমাণ(buf)) == 0)
		প্রত্যাগম strcmp(name, buf) == 0;

	প্রত্যাগম 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
পূর্ণাঙ্ক fd_is_procfs(পূর্ণাঙ্ক fd)
{
	সংরচনা statfs st;
	পূর্ণাঙ্ক ret;

	কুরু {
		errno = 0;
		ret = fstatfs(fd, &st);

		যদি (ret < 0) {
			যদি (errno != EINTR && errno != EAGAIN)
				প্রত্যাগম 0;
			xusleep(250000);
		}
	} যাবত্ (ret != 0);

	প্রত্যাগম F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#অন্যথা
পূর্ণাঙ্ক fd_is_procfs(পূর্ণাঙ্ক fd __attribute__((__unused__)))
{
	প্রত্যাগম 0;
}
#endif

স্থির বর্ণ *strdup_procfs_file(pid_t pid, ধ্রুব বর্ণ *name)
{
	বর্ণ buf[BUFSIZ];
	বর্ণ *re = রিক্ত;
	পূর্ণাঙ্ক fd;

	snprintf(buf, পরিমাণ(buf), _PATH_PROC "/%d/%s", (পূর্ণাঙ্ক) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	যদি (fd < 0)
		প্রত্যাগম রিক্ত;

	যদি (read_procfs_file(রিক্ত, fd, buf, পরিমাণ(buf)) > 0)
		re = strdup(buf);
	close(fd);
	প্রত্যাগম re;
}

বর্ণ *pid_get_cmdname(pid_t pid)
{
	প্রত্যাগম strdup_procfs_file(pid, "comm");
}

বর্ণ *pid_get_cmdline(pid_t pid)
{
	প্রত্যাগম strdup_procfs_file(pid, "cmdline");
}

বর্ণ *pid_get_personality(pid_t pid)
{
	প্রত্যাগম strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

স্থির পূর্ণাঙ্ক test_tasks(পূর্ণাঙ্ক argc, বর্ণ *argv[], ধ্রুব বর্ণ *prefix)
{
	DIR *sub = রিক্ত;
	সংরচনা path_cxt *pc;
	pid_t tid = 0, pid;

	যদি (argc != 2)
		প্রত্যাগম EXIT_FAILURE;

	pid = strtol(argv[1], (বর্ণ **) রিক্ত, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	যদি (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	যাবত্ (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	প্রত্যাগম EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক test_fds(পূর্ণাঙ্ক argc, বর্ণ *argv[], ধ্রুব বর্ণ *prefix)
{
	DIR *sub = রিক্ত;
	সংরচনা path_cxt *pc;
	pid_t pid;
	পূর্ণাঙ্ক fd = -1;

	যদি (argc != 2)
		প্রত্যাগম EXIT_FAILURE;

	pid = strtol(argv[1], (বর্ণ **) রিক্ত, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	যদি (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	যাবত্ (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	প্রত্যাগম EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক test_processes(পূর্ণাঙ্ক argc, বর্ণ *argv[])
{
	DIR *dir;
	সংরচনা dirent *d;
	বর্ণ *name = রিক্ত;
	uid_t uid = (uid_t) -1;
	বর্ণ buf[128];

	যদি (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	যদি (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	যদি (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	যাবত্ ((d = xreaddir(dir))) {
		pid_t pid = 0;

		যদি (procfs_dirent_get_pid(d, &pid) != 0)
			অনুবর্ত;
		যদি (name && !procfs_dirent_match_name(dir, d, name))
			অনুবর্ত;
		যদি (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			অনুবর্ত;
		procfs_dirent_get_name(dir, d, buf, পরিমাণ(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	প্রত্যাগম EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক test_one_process(পূর্ণাঙ্ক argc, বর্ণ *argv[], ধ্রুব বর্ণ *prefix)
{
	pid_t pid;
	সংরচনা path_cxt *pc;
	বর্ণ buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	যদি (argc != 2)
		প্রত্যাগম EXIT_FAILURE;
	pid = strtol(argv[1], (বর্ণ **) রিক্ত, 10);

	pc = ul_new_procfs_path(pid, prefix);
	যদি (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (পূর্ণাঙ্ক) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, পরিমাণ(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, পরিমাণ(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	প্রত্যাগম EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক test_isprocfs(পূর্ণাঙ্ক argc, বর্ণ *argv[])
{
	ধ্রুব বর্ণ *name = argc > 1 ? argv[1] : "/proc";
	পূর্ণাঙ্ক fd = open(name, O_RDONLY | O_CLOEXEC);
	পূর্ণাঙ্ক is = 0;

	যদি (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} অন্যথা
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	প্রত্যাগম is ? EXIT_SUCCESS : EXIT_FAILURE;
}

স্থির পূর্ণাঙ্ক test_process_stat_nth(পূর্ণাঙ্ক argc, বর্ণ *argv[], ধ্রুব বর্ণ *prefix)
{
	pid_t pid;
	সংরচনা path_cxt *pc;
	uint64_t অঙ্ক = 0;
	পূর্ণাঙ্ক n, ret;

	যদি (argc != 3)
		প্রত্যাগম EXIT_FAILURE;
	pid = strtol(argv[1], (বর্ণ **) রিক্ত, 10);
	n = strtol(argv[2], (বর্ণ **) রিক্ত, 10);

	pc = ul_new_procfs_path(pid, prefix);
	যদি (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &অঙ্ক);
	যদি (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (পূর্ণাঙ্ক) pid, n, অঙ্ক);
	ul_unref_path(pc);
	প্রত্যাগম EXIT_SUCCESS;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, বর্ণ *argv[])
{
	ধ্রুব বর্ণ *prefix = রিক্ত;

	যদি (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	যদি (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		প্রত্যাগম EXIT_FAILURE;
	}

	যদি (strcmp(argv[1], "--tasks") == 0)
		প্রত্যাগম test_tasks(argc - 1, argv + 1, prefix);
	যদি (strcmp(argv[1], "--fds") == 0)
		প্রত্যাগম test_fds(argc - 1, argv + 1, prefix);
	যদি (strcmp(argv[1], "--processes") == 0)
		প্রত্যাগম test_processes(argc - 1, argv + 1);
	যদি (strcmp(argv[1], "--is-procfs") == 0)
		প্রত্যাগম test_isprocfs(argc - 1, argv + 1);
	যদি (strcmp(argv[1], "--one") == 0)
		প্রত্যাগম test_one_process(argc - 1, argv + 1, prefix);
	যদি (strcmp(argv[1], "--stat-nth") == 0)
		প্রত্যাগম test_process_stat_nth(argc - 1, argv + 1, prefix);

	প্রত্যাগম EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
