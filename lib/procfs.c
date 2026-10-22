/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#અન્તર્ભાવ <ctype.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <errno.h>

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "pathnames.h"
#અન્તર્ભાવ "procfs.h"
#અન્તર્ભાવ "fileutils.h"
#અન્તર્ભાવ "all-io.h"
#અન્તર્ભાવ "vfs.h"
#અન્તર્ભાવ "debug.h"
#અન્તર્ભાવ "strutils.h"
#અન્તર્ભાવ "statfs_magic.h"

સ્થિર શૂન્ય procfs_process_deinit_path(સંરચના path_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
સ્થિર UL_DEBUG_DEFINE_MASK(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPROCFS_DEBUG_INIT	(1 << 1)
#define ULPROCFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

શૂન્ય ul_procfs_init_debug(શૂન્ય)
{
	જો (ulprocfs_debug_mask)
		પાછા;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

સંરચના path_cxt *ul_new_procfs_path(pid_t pid, અચલ અક્ષર *prefix)
{
	સંરચના path_cxt *pc = ul_new_path(ખાલી);

	જો (!pc)
		પાછા ખાલી;
	જો (prefix)
		ul_path_set_prefix(pc, prefix);

	જો (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		પાછા ખાલી;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	પાછા pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
પૂર્ણાંક procfs_process_init_path(સંરચના path_cxt *pc, pid_t pid)
{
	સંરચના procfs_process *prc;
	પૂર્ણાંક rc;
	અક્ષર buf[કદ(_PATH_PROC) + કદ(stringify_કિંમત(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, કદ(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	જો (rc)
		પાછા rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	જો (rc < 0)
		પાછા rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	જો (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, કદ(સંરચના procfs_process));
		જો (!prc)
			પાછા -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	પાછા 0;
}

સ્થિર શૂન્ય procfs_process_deinit_path(સંરચના path_cxt *pc)
{
	સંરચના procfs_process *prc;

	જો (!pc)
		પાછા;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	જો (!prc)
		પાછા;

	મુક્ત(prc);
	ul_path_set_dialect(pc, ખાલી, ખાલી);
}

સ્થિર ssize_t read_procfs_file(અચલ સંરચના ul_vfs_ops *vfs, પૂર્ણાંક fd,
			       અક્ષર *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	જો (fd < 0)
		પાછા -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	જો (sz <= 0)
		પાછા sz;

	માટે (i = 0; i < (size_t) sz; i++) {
		જો (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	પાછા sz;
}

સ્થિર ssize_t procfs_process_get_data_for(સંરચના path_cxt *pc, અક્ષર *buf, size_t bufsz,
					    અચલ અક્ષર *fname)
{
	પૂર્ણાંક fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	જો (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : ખાલી, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : ખાલી, fd);
		પાછા sz;
	}
	પાછા -errno;
}

ssize_t procfs_process_get_cmdline(સંરચના path_cxt *pc, અક્ષર *buf, size_t bufsz)
{
	પાછા procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(સંરચના path_cxt *pc, અક્ષર *buf, size_t bufsz)
{
	પાછા procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(સંરચના path_cxt *pc, અક્ષર *buf, size_t bufsz)
{
	પાછા procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(સંરચના path_cxt *pc, અક્ષર *buf, size_t bufsz)
{
	પાછા procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

પૂર્ણાંક procfs_process_get_stat_nth(સંરચના path_cxt *pc, પૂર્ણાંક n, uint64_t *re)
{
	ssize_t rc;
	અક્ષર *ચાવી = ખાલી, *tok, *p;
	અક્ષર buf[BUFSIZ];
	પૂર્ણાંક i;

	જો (n == 2 || n == 3)		/* process name and status (strings) */
		પાછા -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, કદ(buf), "stat");
	જો (rc < 0)
		પાછા rc;

	માટે (i = 0, tok = strtok_r(buf, " ", &ચાવી); tok;
	     tok = strtok_r(ખાલી, " ", &ચાવી)) {

		i++;
		જો (i == n)
			પાછા ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		જો (i == 2 && (p = strrchr(ચાવી, ')')))
			ચાવી = p + 2;
	}

	પાછા -EINVAL;
}

પૂર્ણાંક procfs_process_get_uid(સંરચના path_cxt *pc, uid_t *uid)
{
	સંરચના stat sb;
	પૂર્ણાંક rc;

	જો ((rc = ul_path_stat(pc, &sb, 0, ખાલી)) == 0)
		*uid = sb.st_uid;
	પાછા rc;
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
પૂર્ણાંક procfs_process_next_tid(સંરચના path_cxt *pc, DIR **sub, pid_t *tid)
{
	સંરચના dirent *d;

	જો (!pc || !sub || !tid)
		પાછા -EINVAL;

	જો (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		જો (!*sub)
			પાછા -errno;
	}

	જ્યારે ((d = xreaddir(*sub))) {
		જો (procfs_dirent_get_pid(d, tid) == 0)
			પાછા 0;
	}

	closedir(*sub);
	*sub = ખાલી;
	પાછા 1;
}

પૂર્ણાંક procfs_process_next_fd(સંરચના path_cxt *pc, DIR **sub, પૂર્ણાંક *fd)
{
	સંરચના dirent *d;

	જો (!pc || !sub || !fd)
		પાછા -EINVAL;

	જો (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		જો (!*sub)
			પાછા -errno;
	}

	જ્યારે ((d = xreaddir(*sub))) {
		uint64_t અંક;
#ifdef _DIRENT_HAVE_D_TYPE
		જો (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			ચાલુ;
#endif
		જો (ul_strtou64(d->d_name, &અંક, 10) < 0)
			ચાલુ;
		*fd = અંક;
		પાછા 0;
	}

	closedir(*sub);
	*sub = ખાલી;
	પાછા 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
પૂર્ણાંક procfs_dirent_is_process(સંરચના dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	જો (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		પાછા 0;
#endif
	જો (!isdigit((unsigned અક્ષર) *d->d_name))
		પાછા 0;

	પાછા 1;
}

પૂર્ણાંક procfs_dirent_get_pid(સંરચના dirent *d, pid_t *pid)
{
	uint64_t અંક;

	જો (!procfs_dirent_is_process(d))
		પાછા -EINVAL;

	જો (ul_strtou64(d->d_name, &અંક, 10) < 0)
		પાછા -EINVAL;

	*pid = (pid_t) અંક;
	પાછા 0;
}

પૂર્ણાંક procfs_dirent_get_uid(DIR *procfs, સંરચના dirent *d, uid_t *uid)
{
	સંરચના stat st;

	જો (!procfs_dirent_is_process(d))
		પાછા -EINVAL;

	જો (fstatat(dirfd(procfs), d->d_name, &st, 0))
		પાછા -EINVAL;

	*uid = st.st_uid;
	પાછા 0;
}

પૂર્ણાંક procfs_dirent_match_uid(DIR *procfs, સંરચના dirent *d, uid_t uid)
{
	uid_t x;

	જો (procfs_dirent_get_uid(procfs, d, &x) == 0)
		પાછા x == uid;

	પાછા 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
પૂર્ણાંક procfs_dirent_get_name(DIR *procfs, સંરચના dirent *d, અક્ષર *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	અક્ષર tmp[1024], *p, *end = ખાલી;

	જો (bufsz < 32)
		પાછા -EINVAL;
	જો (!procfs_dirent_is_process(d))
		પાછા -EINVAL;

	snprintf(tmp, કદ(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	જો (!f)
		પાછા -errno;

	p = fgets(tmp, કદ(tmp), f);
	fclose(f);
	જો (!p)
		પાછા -errno;

	/* skip PID */
	જ્યારે (*p && *p != '(')
		p++;

	/* skip extra '(' */
	જ્યારે (*p && *p == '(')
		p++;

	end = p;
	જ્યારે (*end && *end != ')')
		end++;

	sz = end - p;
	જો (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	પાછા 0;
}

પૂર્ણાંક procfs_dirent_match_name(DIR *procfs, સંરચના dirent *d, અચલ અક્ષર *name)
{
	અક્ષર buf[33];

	જો (procfs_dirent_get_name(procfs, d, buf, કદ(buf)) == 0)
		પાછા strcmp(name, buf) == 0;

	પાછા 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
પૂર્ણાંક fd_is_procfs(પૂર્ણાંક fd)
{
	સંરચના statfs st;
	પૂર્ણાંક ret;

	કરો {
		errno = 0;
		ret = fstatfs(fd, &st);

		જો (ret < 0) {
			જો (errno != EINTR && errno != EAGAIN)
				પાછા 0;
			xusleep(250000);
		}
	} જ્યારે (ret != 0);

	પાછા F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#નહીંતર
પૂર્ણાંક fd_is_procfs(પૂર્ણાંક fd __attribute__((__unused__)))
{
	પાછા 0;
}
#endif

સ્થિર અક્ષર *strdup_procfs_file(pid_t pid, અચલ અક્ષર *name)
{
	અક્ષર buf[BUFSIZ];
	અક્ષર *re = ખાલી;
	પૂર્ણાંક fd;

	snprintf(buf, કદ(buf), _PATH_PROC "/%d/%s", (પૂર્ણાંક) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	જો (fd < 0)
		પાછા ખાલી;

	જો (read_procfs_file(ખાલી, fd, buf, કદ(buf)) > 0)
		re = strdup(buf);
	close(fd);
	પાછા re;
}

અક્ષર *pid_get_cmdname(pid_t pid)
{
	પાછા strdup_procfs_file(pid, "comm");
}

અક્ષર *pid_get_cmdline(pid_t pid)
{
	પાછા strdup_procfs_file(pid, "cmdline");
}

અક્ષર *pid_get_personality(pid_t pid)
{
	પાછા strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

સ્થિર પૂર્ણાંક test_tasks(પૂર્ણાંક argc, અક્ષર *argv[], અચલ અક્ષર *prefix)
{
	DIR *sub = ખાલી;
	સંરચના path_cxt *pc;
	pid_t tid = 0, pid;

	જો (argc != 2)
		પાછા EXIT_FAILURE;

	pid = strtol(argv[1], (અક્ષર **) ખાલી, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	જો (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	જ્યારે (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	પાછા EXIT_SUCCESS;
}

સ્થિર પૂર્ણાંક test_fds(પૂર્ણાંક argc, અક્ષર *argv[], અચલ અક્ષર *prefix)
{
	DIR *sub = ખાલી;
	સંરચના path_cxt *pc;
	pid_t pid;
	પૂર્ણાંક fd = -1;

	જો (argc != 2)
		પાછા EXIT_FAILURE;

	pid = strtol(argv[1], (અક્ષર **) ખાલી, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	જો (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	જ્યારે (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	પાછા EXIT_SUCCESS;
}

સ્થિર પૂર્ણાંક test_processes(પૂર્ણાંક argc, અક્ષર *argv[])
{
	DIR *dir;
	સંરચના dirent *d;
	અક્ષર *name = ખાલી;
	uid_t uid = (uid_t) -1;
	અક્ષર buf[128];

	જો (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	જો (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	જો (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	જ્યારે ((d = xreaddir(dir))) {
		pid_t pid = 0;

		જો (procfs_dirent_get_pid(d, &pid) != 0)
			ચાલુ;
		જો (name && !procfs_dirent_match_name(dir, d, name))
			ચાલુ;
		જો (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			ચાલુ;
		procfs_dirent_get_name(dir, d, buf, કદ(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	પાછા EXIT_SUCCESS;
}

સ્થિર પૂર્ણાંક test_one_process(પૂર્ણાંક argc, અક્ષર *argv[], અચલ અક્ષર *prefix)
{
	pid_t pid;
	સંરચના path_cxt *pc;
	અક્ષર buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	જો (argc != 2)
		પાછા EXIT_FAILURE;
	pid = strtol(argv[1], (અક્ષર **) ખાલી, 10);

	pc = ul_new_procfs_path(pid, prefix);
	જો (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (પૂર્ણાંક) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, કદ(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, કદ(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	પાછા EXIT_SUCCESS;
}

સ્થિર પૂર્ણાંક test_isprocfs(પૂર્ણાંક argc, અક્ષર *argv[])
{
	અચલ અક્ષર *name = argc > 1 ? argv[1] : "/proc";
	પૂર્ણાંક fd = open(name, O_RDONLY | O_CLOEXEC);
	પૂર્ણાંક is = 0;

	જો (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} નહીંતર
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	પાછા is ? EXIT_SUCCESS : EXIT_FAILURE;
}

સ્થિર પૂર્ણાંક test_process_stat_nth(પૂર્ણાંક argc, અક્ષર *argv[], અચલ અક્ષર *prefix)
{
	pid_t pid;
	સંરચના path_cxt *pc;
	uint64_t અંક = 0;
	પૂર્ણાંક n, ret;

	જો (argc != 3)
		પાછા EXIT_FAILURE;
	pid = strtol(argv[1], (અક્ષર **) ખાલી, 10);
	n = strtol(argv[2], (અક્ષર **) ખાલી, 10);

	pc = ul_new_procfs_path(pid, prefix);
	જો (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &અંક);
	જો (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (પૂર્ણાંક) pid, n, અંક);
	ul_unref_path(pc);
	પાછા EXIT_SUCCESS;
}

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	અચલ અક્ષર *prefix = ખાલી;

	જો (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	જો (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		પાછા EXIT_FAILURE;
	}

	જો (strcmp(argv[1], "--tasks") == 0)
		પાછા test_tasks(argc - 1, argv + 1, prefix);
	જો (strcmp(argv[1], "--fds") == 0)
		પાછા test_fds(argc - 1, argv + 1, prefix);
	જો (strcmp(argv[1], "--processes") == 0)
		પાછા test_processes(argc - 1, argv + 1);
	જો (strcmp(argv[1], "--is-procfs") == 0)
		પાછા test_isprocfs(argc - 1, argv + 1);
	જો (strcmp(argv[1], "--one") == 0)
		પાછા test_one_process(argc - 1, argv + 1, prefix);
	જો (strcmp(argv[1], "--stat-nth") == 0)
		પાછા test_process_stat_nth(argc - 1, argv + 1, prefix);

	પાછા EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
