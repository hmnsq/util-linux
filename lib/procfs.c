/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#অন্তর্ভাৱ <ctype.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <errno.h>

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "pathnames.h"
#অন্তর্ভাৱ "procfs.h"
#অন্তর্ভাৱ "fileutils.h"
#অন্তর্ভাৱ "all-io.h"
#অন্তর্ভাৱ "vfs.h"
#অন্তর্ভাৱ "debug.h"
#অন্তর্ভাৱ "strutils.h"
#অন্তর্ভাৱ "statfs_magic.h"

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
		ঘূৰাই;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

সংরচনা path_cxt *ul_new_procfs_path(pid_t pid, ধ্রুৱ ৱর্ণ *prefix)
{
	সংরচনা path_cxt *pc = ul_new_path(রিক্ত);

	যদি (!pc)
		ঘূৰাই রিক্ত;
	যদি (prefix)
		ul_path_set_prefix(pc, prefix);

	যদি (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		ঘূৰাই রিক্ত;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	ঘূৰাই pc;
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
	ৱর্ণ buf[পরিমাণ(_PATH_PROC) + পরিমাণ(stringify_মান(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, পরিমাণ(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	যদি (rc)
		ঘূৰাই rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	যদি (rc < 0)
		ঘূৰাই rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	যদি (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, পরিমাণ(সংরচনা procfs_process));
		যদি (!prc)
			ঘূৰাই -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	ঘূৰাই 0;
}

স্থির শূন্য procfs_process_deinit_path(সংরচনা path_cxt *pc)
{
	সংরচনা procfs_process *prc;

	যদি (!pc)
		ঘূৰাই;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	যদি (!prc)
		ঘূৰাই;

	মুক্ত(prc);
	ul_path_set_dialect(pc, রিক্ত, রিক্ত);
}

স্থির ssize_t read_procfs_file(ধ্রুৱ সংরচনা ul_vfs_ops *vfs, পূর্ণাঙ্ক fd,
			       ৱর্ণ *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	যদি (fd < 0)
		ঘূৰাই -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	যদি (sz <= 0)
		ঘূৰাই sz;

	বাবে (i = 0; i < (size_t) sz; i++) {
		যদি (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	ঘূৰাই sz;
}

স্থির ssize_t procfs_process_get_data_for(সংরচনা path_cxt *pc, ৱর্ণ *buf, size_t bufsz,
					    ধ্রুৱ ৱর্ণ *fname)
{
	পূর্ণাঙ্ক fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	যদি (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : রিক্ত, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : রিক্ত, fd);
		ঘূৰাই sz;
	}
	ঘূৰাই -errno;
}

ssize_t procfs_process_get_cmdline(সংরচনা path_cxt *pc, ৱর্ণ *buf, size_t bufsz)
{
	ঘূৰাই procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(সংরচনা path_cxt *pc, ৱর্ণ *buf, size_t bufsz)
{
	ঘূৰাই procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(সংরচনা path_cxt *pc, ৱর্ণ *buf, size_t bufsz)
{
	ঘূৰাই procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(সংরচনা path_cxt *pc, ৱর্ণ *buf, size_t bufsz)
{
	ঘূৰাই procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

পূর্ণাঙ্ক procfs_process_get_stat_nth(সংরচনা path_cxt *pc, পূর্ণাঙ্ক n, uint64_t *re)
{
	ssize_t rc;
	ৱর্ণ *চাবি = রিক্ত, *tok, *p;
	ৱর্ণ buf[BUFSIZ];
	পূর্ণাঙ্ক i;

	যদি (n == 2 || n == 3)		/* process name and status (strings) */
		ঘূৰাই -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, পরিমাণ(buf), "stat");
	যদি (rc < 0)
		ঘূৰাই rc;

	বাবে (i = 0, tok = strtok_r(buf, " ", &চাবি); tok;
	     tok = strtok_r(রিক্ত, " ", &চাবি)) {

		i++;
		যদি (i == n)
			ঘূৰাই ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		যদি (i == 2 && (p = strrchr(চাবি, ')')))
			চাবি = p + 2;
	}

	ঘূৰাই -EINVAL;
}

পূর্ণাঙ্ক procfs_process_get_uid(সংরচনা path_cxt *pc, uid_t *uid)
{
	সংরচনা stat sb;
	পূর্ণাঙ্ক rc;

	যদি ((rc = ul_path_stat(pc, &sb, 0, রিক্ত)) == 0)
		*uid = sb.st_uid;
	ঘূৰাই rc;
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
		ঘূৰাই -EINVAL;

	যদি (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		যদি (!*sub)
			ঘূৰাই -errno;
	}

	যেতিয়ালৈ ((d = xreaddir(*sub))) {
		যদি (procfs_dirent_get_pid(d, tid) == 0)
			ঘূৰাই 0;
	}

	closedir(*sub);
	*sub = রিক্ত;
	ঘূৰাই 1;
}

পূর্ণাঙ্ক procfs_process_next_fd(সংরচনা path_cxt *pc, DIR **sub, পূর্ণাঙ্ক *fd)
{
	সংরচনা dirent *d;

	যদি (!pc || !sub || !fd)
		ঘূৰাই -EINVAL;

	যদি (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		যদি (!*sub)
			ঘূৰাই -errno;
	}

	যেতিয়ালৈ ((d = xreaddir(*sub))) {
		uint64_t অঙ্ক;
#ifdef _DIRENT_HAVE_D_TYPE
		যদি (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			অনুৱর্ত;
#endif
		যদি (ul_strtou64(d->d_name, &অঙ্ক, 10) < 0)
			অনুৱর্ত;
		*fd = অঙ্ক;
		ঘূৰাই 0;
	}

	closedir(*sub);
	*sub = রিক্ত;
	ঘূৰাই 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
পূর্ণাঙ্ক procfs_dirent_is_process(সংরচনা dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	যদি (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		ঘূৰাই 0;
#endif
	যদি (!isdigit((unsigned ৱর্ণ) *d->d_name))
		ঘূৰাই 0;

	ঘূৰাই 1;
}

পূর্ণাঙ্ক procfs_dirent_get_pid(সংরচনা dirent *d, pid_t *pid)
{
	uint64_t অঙ্ক;

	যদি (!procfs_dirent_is_process(d))
		ঘূৰাই -EINVAL;

	যদি (ul_strtou64(d->d_name, &অঙ্ক, 10) < 0)
		ঘূৰাই -EINVAL;

	*pid = (pid_t) অঙ্ক;
	ঘূৰাই 0;
}

পূর্ণাঙ্ক procfs_dirent_get_uid(DIR *procfs, সংরচনা dirent *d, uid_t *uid)
{
	সংরচনা stat st;

	যদি (!procfs_dirent_is_process(d))
		ঘূৰাই -EINVAL;

	যদি (fstatat(dirfd(procfs), d->d_name, &st, 0))
		ঘূৰাই -EINVAL;

	*uid = st.st_uid;
	ঘূৰাই 0;
}

পূর্ণাঙ্ক procfs_dirent_match_uid(DIR *procfs, সংরচনা dirent *d, uid_t uid)
{
	uid_t x;

	যদি (procfs_dirent_get_uid(procfs, d, &x) == 0)
		ঘূৰাই x == uid;

	ঘূৰাই 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
পূর্ণাঙ্ক procfs_dirent_get_name(DIR *procfs, সংরচনা dirent *d, ৱর্ণ *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	ৱর্ণ tmp[1024], *p, *end = রিক্ত;

	যদি (bufsz < 32)
		ঘূৰাই -EINVAL;
	যদি (!procfs_dirent_is_process(d))
		ঘূৰাই -EINVAL;

	snprintf(tmp, পরিমাণ(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	যদি (!f)
		ঘূৰাই -errno;

	p = fgets(tmp, পরিমাণ(tmp), f);
	fclose(f);
	যদি (!p)
		ঘূৰাই -errno;

	/* skip PID */
	যেতিয়ালৈ (*p && *p != '(')
		p++;

	/* skip extra '(' */
	যেতিয়ালৈ (*p && *p == '(')
		p++;

	end = p;
	যেতিয়ালৈ (*end && *end != ')')
		end++;

	sz = end - p;
	যদি (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	ঘূৰাই 0;
}

পূর্ণাঙ্ক procfs_dirent_match_name(DIR *procfs, সংরচনা dirent *d, ধ্রুৱ ৱর্ণ *name)
{
	ৱর্ণ buf[33];

	যদি (procfs_dirent_get_name(procfs, d, buf, পরিমাণ(buf)) == 0)
		ঘূৰাই strcmp(name, buf) == 0;

	ঘূৰাই 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
পূর্ণাঙ্ক fd_is_procfs(পূর্ণাঙ্ক fd)
{
	সংরচনা statfs st;
	পূর্ণাঙ্ক ret;

	কৰা {
		errno = 0;
		ret = fstatfs(fd, &st);

		যদি (ret < 0) {
			যদি (errno != EINTR && errno != EAGAIN)
				ঘূৰাই 0;
			xusleep(250000);
		}
	} যেতিয়ালৈ (ret != 0);

	ঘূৰাই F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#অন্যথা
পূর্ণাঙ্ক fd_is_procfs(পূর্ণাঙ্ক fd __attribute__((__unused__)))
{
	ঘূৰাই 0;
}
#endif

স্থির ৱর্ণ *strdup_procfs_file(pid_t pid, ধ্রুৱ ৱর্ণ *name)
{
	ৱর্ণ buf[BUFSIZ];
	ৱর্ণ *re = রিক্ত;
	পূর্ণাঙ্ক fd;

	snprintf(buf, পরিমাণ(buf), _PATH_PROC "/%d/%s", (পূর্ণাঙ্ক) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	যদি (fd < 0)
		ঘূৰাই রিক্ত;

	যদি (read_procfs_file(রিক্ত, fd, buf, পরিমাণ(buf)) > 0)
		re = strdup(buf);
	close(fd);
	ঘূৰাই re;
}

ৱর্ণ *pid_get_cmdname(pid_t pid)
{
	ঘূৰাই strdup_procfs_file(pid, "comm");
}

ৱর্ণ *pid_get_cmdline(pid_t pid)
{
	ঘূৰাই strdup_procfs_file(pid, "cmdline");
}

ৱর্ণ *pid_get_personality(pid_t pid)
{
	ঘূৰাই strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

স্থির পূর্ণাঙ্ক test_tasks(পূর্ণাঙ্ক argc, ৱর্ণ *argv[], ধ্রুৱ ৱর্ণ *prefix)
{
	DIR *sub = রিক্ত;
	সংরচনা path_cxt *pc;
	pid_t tid = 0, pid;

	যদি (argc != 2)
		ঘূৰাই EXIT_FAILURE;

	pid = strtol(argv[1], (ৱর্ণ **) রিক্ত, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	যদি (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	যেতিয়ালৈ (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	ঘূৰাই EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক test_fds(পূর্ণাঙ্ক argc, ৱর্ণ *argv[], ধ্রুৱ ৱর্ণ *prefix)
{
	DIR *sub = রিক্ত;
	সংরচনা path_cxt *pc;
	pid_t pid;
	পূর্ণাঙ্ক fd = -1;

	যদি (argc != 2)
		ঘূৰাই EXIT_FAILURE;

	pid = strtol(argv[1], (ৱর্ণ **) রিক্ত, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	যদি (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	যেতিয়ালৈ (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	ঘূৰাই EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক test_processes(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	DIR *dir;
	সংরচনা dirent *d;
	ৱর্ণ *name = রিক্ত;
	uid_t uid = (uid_t) -1;
	ৱর্ণ buf[128];

	যদি (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	যদি (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	যদি (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	যেতিয়ালৈ ((d = xreaddir(dir))) {
		pid_t pid = 0;

		যদি (procfs_dirent_get_pid(d, &pid) != 0)
			অনুৱর্ত;
		যদি (name && !procfs_dirent_match_name(dir, d, name))
			অনুৱর্ত;
		যদি (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			অনুৱর্ত;
		procfs_dirent_get_name(dir, d, buf, পরিমাণ(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	ঘূৰাই EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক test_one_process(পূর্ণাঙ্ক argc, ৱর্ণ *argv[], ধ্রুৱ ৱর্ণ *prefix)
{
	pid_t pid;
	সংরচনা path_cxt *pc;
	ৱর্ণ buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	যদি (argc != 2)
		ঘূৰাই EXIT_FAILURE;
	pid = strtol(argv[1], (ৱর্ণ **) রিক্ত, 10);

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
	ঘূৰাই EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক test_isprocfs(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	ধ্রুৱ ৱর্ণ *name = argc > 1 ? argv[1] : "/proc";
	পূর্ণাঙ্ক fd = open(name, O_RDONLY | O_CLOEXEC);
	পূর্ণাঙ্ক is = 0;

	যদি (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} অন্যথা
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	ঘূৰাই is ? EXIT_SUCCESS : EXIT_FAILURE;
}

স্থির পূর্ণাঙ্ক test_process_stat_nth(পূর্ণাঙ্ক argc, ৱর্ণ *argv[], ধ্রুৱ ৱর্ণ *prefix)
{
	pid_t pid;
	সংরচনা path_cxt *pc;
	uint64_t অঙ্ক = 0;
	পূর্ণাঙ্ক n, ret;

	যদি (argc != 3)
		ঘূৰাই EXIT_FAILURE;
	pid = strtol(argv[1], (ৱর্ণ **) রিক্ত, 10);
	n = strtol(argv[2], (ৱর্ণ **) রিক্ত, 10);

	pc = ul_new_procfs_path(pid, prefix);
	যদি (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &অঙ্ক);
	যদি (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (পূর্ণাঙ্ক) pid, n, অঙ্ক);
	ul_unref_path(pc);
	ঘূৰাই EXIT_SUCCESS;
}

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc, ৱর্ণ *argv[])
{
	ধ্রুৱ ৱর্ণ *prefix = রিক্ত;

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
		ঘূৰাই EXIT_FAILURE;
	}

	যদি (strcmp(argv[1], "--tasks") == 0)
		ঘূৰাই test_tasks(argc - 1, argv + 1, prefix);
	যদি (strcmp(argv[1], "--fds") == 0)
		ঘূৰাই test_fds(argc - 1, argv + 1, prefix);
	যদি (strcmp(argv[1], "--processes") == 0)
		ঘূৰাই test_processes(argc - 1, argv + 1);
	যদি (strcmp(argv[1], "--is-procfs") == 0)
		ঘূৰাই test_isprocfs(argc - 1, argv + 1);
	যদি (strcmp(argv[1], "--one") == 0)
		ঘূৰাই test_one_process(argc - 1, argv + 1, prefix);
	যদি (strcmp(argv[1], "--stat-nth") == 0)
		ঘূৰাই test_process_stat_nth(argc - 1, argv + 1, prefix);

	ঘূৰাই EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
