/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#அந்தர்பாவ <ctype.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <errno.h>

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "pathnames.h"
#அந்தர்பாவ "procfs.h"
#அந்தர்பாவ "fileutils.h"
#அந்தர்பாவ "all-io.h"
#அந்தர்பாவ "vfs.h"
#அந்தர்பாவ "debug.h"
#அந்தர்பாவ "strutils.h"
#அந்தர்பாவ "statfs_magic.h"

நிலையான வெறுமை procfs_process_deinit_path(கட்டமைப்பு path_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
நிலையான UL_DEBUG_DEFINE_MASK(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPROCFS_DEBUG_INIT	(1 << 1)
#define ULPROCFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

வெறுமை ul_procfs_init_debug(வெறுமை)
{
	எனில் (ulprocfs_debug_mask)
		திரும்பு;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

கட்டமைப்பு path_cxt *ul_new_procfs_path(pid_t pid, மாறிலி எழுத்து *prefix)
{
	கட்டமைப்பு path_cxt *pc = ul_new_path(வெற்று);

	எனில் (!pc)
		திரும்பு வெற்று;
	எனில் (prefix)
		ul_path_set_prefix(pc, prefix);

	எனில் (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		திரும்பு வெற்று;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	திரும்பு pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
முழு procfs_process_init_path(கட்டமைப்பு path_cxt *pc, pid_t pid)
{
	கட்டமைப்பு procfs_process *prc;
	முழு rc;
	எழுத்து buf[அளவு(_PATH_PROC) + அளவு(stringify_மதிப்பு(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, அளவு(buf), _PATH_PROC "/%zu", (size_t) pid);
	rc = ul_path_set_dir(pc, buf);
	எனில் (rc)
		திரும்பு rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	எனில் (rc < 0)
		திரும்பு rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	எனில் (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = calloc(1, அளவு(கட்டமைப்பு procfs_process));
		எனில் (!prc)
			திரும்பு -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	திரும்பு 0;
}

நிலையான வெறுமை procfs_process_deinit_path(கட்டமைப்பு path_cxt *pc)
{
	கட்டமைப்பு procfs_process *prc;

	எனில் (!pc)
		திரும்பு;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	எனில் (!prc)
		திரும்பு;

	விடுவி(prc);
	ul_path_set_dialect(pc, வெற்று, வெற்று);
}

நிலையான ssize_t read_procfs_file(மாறிலி கட்டமைப்பு ul_vfs_ops *vfs, முழு fd,
			       எழுத்து *buf, size_t bufsz)
{
	ssize_t sz = 0;
	size_t i;

	எனில் (fd < 0)
		திரும்பு -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	எனில் (sz <= 0)
		திரும்பு sz;

	க்கு (i = 0; i < (size_t) sz; i++) {
		எனில் (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	திரும்பு sz;
}

நிலையான ssize_t procfs_process_get_data_for(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t bufsz,
					    மாறிலி எழுத்து *fname)
{
	முழு fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	எனில் (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : வெற்று, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : வெற்று, fd);
		திரும்பு sz;
	}
	திரும்பு -errno;
}

ssize_t procfs_process_get_cmdline(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t bufsz)
{
	திரும்பு procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t bufsz)
{
	திரும்பு procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t bufsz)
{
	திரும்பு procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(கட்டமைப்பு path_cxt *pc, எழுத்து *buf, size_t bufsz)
{
	திரும்பு procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

முழு procfs_process_get_stat_nth(கட்டமைப்பு path_cxt *pc, முழு n, uint64_t *re)
{
	ssize_t rc;
	எழுத்து *சாவி = வெற்று, *tok, *p;
	எழுத்து buf[BUFSIZ];
	முழு i;

	எனில் (n == 2 || n == 3)		/* process name and status (strings) */
		திரும்பு -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, அளவு(buf), "stat");
	எனில் (rc < 0)
		திரும்பு rc;

	க்கு (i = 0, tok = strtok_r(buf, " ", &சாவி); tok;
	     tok = strtok_r(வெற்று, " ", &சாவி)) {

		i++;
		எனில் (i == n)
			திரும்பு ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		எனில் (i == 2 && (p = strrchr(சாவி, ')')))
			சாவி = p + 2;
	}

	திரும்பு -EINVAL;
}

முழு procfs_process_get_uid(கட்டமைப்பு path_cxt *pc, uid_t *uid)
{
	கட்டமைப்பு stat sb;
	முழு rc;

	எனில் ((rc = ul_path_stat(pc, &sb, 0, வெற்று)) == 0)
		*uid = sb.st_uid;
	திரும்பு rc;
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
முழு procfs_process_next_tid(கட்டமைப்பு path_cxt *pc, DIR **sub, pid_t *tid)
{
	கட்டமைப்பு dirent *d;

	எனில் (!pc || !sub || !tid)
		திரும்பு -EINVAL;

	எனில் (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		எனில் (!*sub)
			திரும்பு -errno;
	}

	போது ((d = xreaddir(*sub))) {
		எனில் (procfs_dirent_get_pid(d, tid) == 0)
			திரும்பு 0;
	}

	closedir(*sub);
	*sub = வெற்று;
	திரும்பு 1;
}

முழு procfs_process_next_fd(கட்டமைப்பு path_cxt *pc, DIR **sub, முழு *fd)
{
	கட்டமைப்பு dirent *d;

	எனில் (!pc || !sub || !fd)
		திரும்பு -EINVAL;

	எனில் (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		எனில் (!*sub)
			திரும்பு -errno;
	}

	போது ((d = xreaddir(*sub))) {
		uint64_t இலக்கம்;
#ifdef _DIRENT_HAVE_D_TYPE
		எனில் (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			தொடர்;
#endif
		எனில் (ul_strtou64(d->d_name, &இலக்கம், 10) < 0)
			தொடர்;
		*fd = இலக்கம்;
		திரும்பு 0;
	}

	closedir(*sub);
	*sub = வெற்று;
	திரும்பு 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
முழு procfs_dirent_is_process(கட்டமைப்பு dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	எனில் (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		திரும்பு 0;
#endif
	எனில் (!isdigit((unsigned எழுத்து) *d->d_name))
		திரும்பு 0;

	திரும்பு 1;
}

முழு procfs_dirent_get_pid(கட்டமைப்பு dirent *d, pid_t *pid)
{
	uint64_t இலக்கம்;

	எனில் (!procfs_dirent_is_process(d))
		திரும்பு -EINVAL;

	எனில் (ul_strtou64(d->d_name, &இலக்கம், 10) < 0)
		திரும்பு -EINVAL;

	*pid = (pid_t) இலக்கம்;
	திரும்பு 0;
}

முழு procfs_dirent_get_uid(DIR *procfs, கட்டமைப்பு dirent *d, uid_t *uid)
{
	கட்டமைப்பு stat st;

	எனில் (!procfs_dirent_is_process(d))
		திரும்பு -EINVAL;

	எனில் (fstatat(dirfd(procfs), d->d_name, &st, 0))
		திரும்பு -EINVAL;

	*uid = st.st_uid;
	திரும்பு 0;
}

முழு procfs_dirent_match_uid(DIR *procfs, கட்டமைப்பு dirent *d, uid_t uid)
{
	uid_t x;

	எனில் (procfs_dirent_get_uid(procfs, d, &x) == 0)
		திரும்பு x == uid;

	திரும்பு 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
முழு procfs_dirent_get_name(DIR *procfs, கட்டமைப்பு dirent *d, எழுத்து *buf, size_t bufsz)
{
	FILE *f;
	size_t sz;
	எழுத்து tmp[1024], *p, *end = வெற்று;

	எனில் (bufsz < 32)
		திரும்பு -EINVAL;
	எனில் (!procfs_dirent_is_process(d))
		திரும்பு -EINVAL;

	snprintf(tmp, அளவு(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	எனில் (!f)
		திரும்பு -errno;

	p = fgets(tmp, அளவு(tmp), f);
	fclose(f);
	எனில் (!p)
		திரும்பு -errno;

	/* skip PID */
	போது (*p && *p != '(')
		p++;

	/* skip extra '(' */
	போது (*p && *p == '(')
		p++;

	end = p;
	போது (*end && *end != ')')
		end++;

	sz = end - p;
	எனில் (sz >= bufsz)
		sz = bufsz - 1;

	memcpy(buf, p, sz);
	buf[sz] = '\0';

	திரும்பு 0;
}

முழு procfs_dirent_match_name(DIR *procfs, கட்டமைப்பு dirent *d, மாறிலி எழுத்து *name)
{
	எழுத்து buf[33];

	எனில் (procfs_dirent_get_name(procfs, d, buf, அளவு(buf)) == 0)
		திரும்பு strcmp(name, buf) == 0;

	திரும்பு 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
முழு fd_is_procfs(முழு fd)
{
	கட்டமைப்பு statfs st;
	முழு ret;

	செய் {
		errno = 0;
		ret = fstatfs(fd, &st);

		எனில் (ret < 0) {
			எனில் (errno != EINTR && errno != EAGAIN)
				திரும்பு 0;
			xusleep(250000);
		}
	} போது (ret != 0);

	திரும்பு F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#இல்லையெனில்
முழு fd_is_procfs(முழு fd __attribute__((__unused__)))
{
	திரும்பு 0;
}
#endif

நிலையான எழுத்து *strdup_procfs_file(pid_t pid, மாறிலி எழுத்து *name)
{
	எழுத்து buf[BUFSIZ];
	எழுத்து *re = வெற்று;
	முழு fd;

	snprintf(buf, அளவு(buf), _PATH_PROC "/%d/%s", (முழு) pid, name);
	fd = open(buf, O_CLOEXEC|O_RDONLY);
	எனில் (fd < 0)
		திரும்பு வெற்று;

	எனில் (read_procfs_file(வெற்று, fd, buf, அளவு(buf)) > 0)
		re = strdup(buf);
	close(fd);
	திரும்பு re;
}

எழுத்து *pid_get_cmdname(pid_t pid)
{
	திரும்பு strdup_procfs_file(pid, "comm");
}

எழுத்து *pid_get_cmdline(pid_t pid)
{
	திரும்பு strdup_procfs_file(pid, "cmdline");
}

எழுத்து *pid_get_personality(pid_t pid)
{
	திரும்பு strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

நிலையான முழு test_tasks(முழு argc, எழுத்து *argv[], மாறிலி எழுத்து *prefix)
{
	DIR *sub = வெற்று;
	கட்டமைப்பு path_cxt *pc;
	pid_t tid = 0, pid;

	எனில் (argc != 2)
		திரும்பு EXIT_FAILURE;

	pid = strtol(argv[1], (எழுத்து **) வெற்று, 10);
	printf("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	எனில் (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	போது (procfs_process_next_tid(pc, &sub, &tid) == 0)
		printf(" %d", tid);

	printf("\n");
	ul_unref_path(pc);
	திரும்பு EXIT_SUCCESS;
}

நிலையான முழு test_fds(முழு argc, எழுத்து *argv[], மாறிலி எழுத்து *prefix)
{
	DIR *sub = வெற்று;
	கட்டமைப்பு path_cxt *pc;
	pid_t pid;
	முழு fd = -1;

	எனில் (argc != 2)
		திரும்பு EXIT_FAILURE;

	pid = strtol(argv[1], (எழுத்து **) வெற்று, 10);
	printf("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	எனில் (!pc)
		err(EXIT_FAILURE, "alloc procfs handler failed");

	போது (procfs_process_next_fd(pc, &sub, &fd) == 0)
		printf(" %d", fd);

	fputc('\n', stdout);
	ul_unref_path(pc);
	திரும்பு EXIT_SUCCESS;
}

நிலையான முழு test_processes(முழு argc, எழுத்து *argv[])
{
	DIR *dir;
	கட்டமைப்பு dirent *d;
	எழுத்து *name = வெற்று;
	uid_t uid = (uid_t) -1;
	எழுத்து buf[128];

	எனில் (argc >= 3 && strcmp(argv[1], "--name") == 0)
		name = argv[2];
	எனில் (argc >= 3 && strcmp(argv[1], "--uid") == 0)
		uid = (uid_t) atol(argv[2]);

	dir = opendir(_PATH_PROC);
	எனில் (!dir)
		err(EXIT_FAILURE, "cannot open proc");

	போது ((d = xreaddir(dir))) {
		pid_t pid = 0;

		எனில் (procfs_dirent_get_pid(d, &pid) != 0)
			தொடர்;
		எனில் (name && !procfs_dirent_match_name(dir, d, name))
			தொடர்;
		எனில் (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			தொடர்;
		procfs_dirent_get_name(dir, d, buf, அளவு(buf));
		printf(" %d [%s]", pid, buf);
	}

	fputc('\n', stdout);
	closedir(dir);
	திரும்பு EXIT_SUCCESS;
}

நிலையான முழு test_one_process(முழு argc, எழுத்து *argv[], மாறிலி எழுத்து *prefix)
{
	pid_t pid;
	கட்டமைப்பு path_cxt *pc;
	எழுத்து buf[BUFSIZ];
	uid_t uid = (uid_t) -1;

	எனில் (argc != 2)
		திரும்பு EXIT_FAILURE;
	pid = strtol(argv[1], (எழுத்து **) வெற்று, 10);

	pc = ul_new_procfs_path(pid, prefix);
	எனில் (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	printf("%d\n", (முழு) pid);

	procfs_process_get_uid(pc, &uid);
	printf("   UID: %zu\n", (size_t) uid);

	procfs_process_get_cmdline(pc, buf, அளவு(buf));
	printf("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, அளவு(buf));
	printf("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	திரும்பு EXIT_SUCCESS;
}

நிலையான முழு test_isprocfs(முழு argc, எழுத்து *argv[])
{
	மாறிலி எழுத்து *name = argc > 1 ? argv[1] : "/proc";
	முழு fd = open(name, O_RDONLY | O_CLOEXEC);
	முழு is = 0;

	எனில் (fd >= 0) {
		is = fd_is_procfs(fd);
		close(fd);
	} இல்லையெனில்
		err(EXIT_FAILURE, "cannot open %s", name);

	printf("%s: %s procfs\n", name, is ? "is" : "is NOT");
	திரும்பு is ? EXIT_SUCCESS : EXIT_FAILURE;
}

நிலையான முழு test_process_stat_nth(முழு argc, எழுத்து *argv[], மாறிலி எழுத்து *prefix)
{
	pid_t pid;
	கட்டமைப்பு path_cxt *pc;
	uint64_t இலக்கம் = 0;
	முழு n, ret;

	எனில் (argc != 3)
		திரும்பு EXIT_FAILURE;
	pid = strtol(argv[1], (எழுத்து **) வெற்று, 10);
	n = strtol(argv[2], (எழுத்து **) வெற்று, 10);

	pc = ul_new_procfs_path(pid, prefix);
	எனில் (!pc)
		err(EXIT_FAILURE, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &இலக்கம்);
	எனில் (ret)
		errx(EXIT_FAILURE, "read %dth number failed: %s", n, strerror(-ret));

	printf("%d: %dth %"PRIu64"\n", (முழு) pid, n, இலக்கம்);
	ul_unref_path(pc);
	திரும்பு EXIT_SUCCESS;
}

முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	மாறிலி எழுத்து *prefix = வெற்று;

	எனில் (argc > 2 && strcmp(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	எனில் (argc < 2) {
		fprintf(stderr, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		திரும்பு EXIT_FAILURE;
	}

	எனில் (strcmp(argv[1], "--tasks") == 0)
		திரும்பு test_tasks(argc - 1, argv + 1, prefix);
	எனில் (strcmp(argv[1], "--fds") == 0)
		திரும்பு test_fds(argc - 1, argv + 1, prefix);
	எனில் (strcmp(argv[1], "--processes") == 0)
		திரும்பு test_processes(argc - 1, argv + 1);
	எனில் (strcmp(argv[1], "--is-procfs") == 0)
		திரும்பு test_isprocfs(argc - 1, argv + 1);
	எனில் (strcmp(argv[1], "--one") == 0)
		திரும்பு test_one_process(argc - 1, argv + 1, prefix);
	எனில் (strcmp(argv[1], "--stat-nth") == 0)
		திரும்பு test_process_stat_nth(argc - 1, argv + 1, prefix);

	திரும்பு EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_PROCUTILS */
