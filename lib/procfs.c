/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [2021]
 */
#अन्तर्भाव <ctype.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <errno.h>

#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif

#अन्तर्भाव "c.h"
#अन्तर्भाव "pathnames.h"
#अन्तर्भाव "procfs.h"
#अन्तर्भाव "fileutils.h"
#अन्तर्भाव "all-io.h"
#अन्तर्भाव "vfs.h"
#अन्तर्भाव "debug.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "statfs_magic.h"

स्थिर शून्य procfs_process_deinit_path(संरचना path_cxt *pc);

/*
 * Debug stuff (based on include/debug.h)
 */
स्थिर UL_DEBUG_DEFINE_MASK(ulprocfs);
UL_DEBUG_DEFINE_MASKNAMES(ulprocfs) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPROCFS_DEBUG_INIT	(1 << 1)
#define ULPROCFS_DEBUG_CXT	(1 << 2)

#define DBG(m, x)		__UL_DBG(ulprocfs, ULPROCFS_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulprocfs, ULPROCFS_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulprocfs, ULPROCFS_DEBUG_, m, x)

शून्य ul_procfs_init_debug(शून्य)
{
	यदि (ulprocfs_debug_mask)
		प्रत्यागम;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

संरचना path_cxt *ul_new_procfs_path(pid_t pid, ध्रुव वर्ण *prefix)
{
	संरचना path_cxt *pc = ul_new_path(रिक्त);

	यदि (!pc)
		प्रत्यागम रिक्त;
	यदि (prefix)
		ul_path_set_prefix(pc, prefix);

	यदि (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		प्रत्यागम रिक्त;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	प्रत्यागम pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
पूर्णाङ्क procfs_process_init_path(संरचना path_cxt *pc, pid_t pid)
{
	संरचना procfs_process *prc;
	पूर्णाङ्क rc;
	वर्ण buf[परिमाण(_PATH_PROC) + परिमाण(stringify_मूल्य(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, परिमाण(buf), _PATH_PROC "/%zu", (माप_प्रकार) pid);
	rc = ul_path_set_dir(pc, buf);
	यदि (rc)
		प्रत्यागम rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	यदि (rc < 0)
		प्रत्यागम rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	यदि (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = सुस्मृति(1, परिमाण(संरचना procfs_process));
		यदि (!prc)
			प्रत्यागम -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	प्रत्यागम 0;
}

स्थिर शून्य procfs_process_deinit_path(संरचना path_cxt *pc)
{
	संरचना procfs_process *prc;

	यदि (!pc)
		प्रत्यागम;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	यदि (!prc)
		प्रत्यागम;

	मुक्त(prc);
	ul_path_set_dialect(pc, रिक्त, रिक्त);
}

स्थिर ssize_t read_procfs_file(ध्रुव संरचना ul_vfs_ops *vfs, पूर्णाङ्क fd,
			       वर्ण *buf, माप_प्रकार bufsz)
{
	ssize_t sz = 0;
	माप_प्रकार i;

	यदि (fd < 0)
		प्रत्यागम -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	यदि (sz <= 0)
		प्रत्यागम sz;

	प्रति (i = 0; i < (माप_प्रकार) sz; i++) {
		यदि (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	प्रत्यागम sz;
}

स्थिर ssize_t procfs_process_get_data_for(संरचना path_cxt *pc, वर्ण *buf, माप_प्रकार bufsz,
					    ध्रुव वर्ण *fname)
{
	पूर्णाङ्क fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	यदि (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : रिक्त, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
		प्रत्यागम sz;
	}
	प्रत्यागम -त्रुटि_सं;
}

ssize_t procfs_process_get_cmdline(संरचना path_cxt *pc, वर्ण *buf, माप_प्रकार bufsz)
{
	प्रत्यागम procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(संरचना path_cxt *pc, वर्ण *buf, माप_प्रकार bufsz)
{
	प्रत्यागम procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(संरचना path_cxt *pc, वर्ण *buf, माप_प्रकार bufsz)
{
	प्रत्यागम procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(संरचना path_cxt *pc, वर्ण *buf, माप_प्रकार bufsz)
{
	प्रत्यागम procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

पूर्णाङ्क procfs_process_get_stat_nth(संरचना path_cxt *pc, पूर्णाङ्क n, uint64_t *re)
{
	ssize_t rc;
	वर्ण *कुंजी = रिक्त, *tok, *p;
	वर्ण buf[बफ_मान];
	पूर्णाङ्क i;

	यदि (n == 2 || n == 3)		/* process name and status (strings) */
		प्रत्यागम -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, परिमाण(buf), "stat");
	यदि (rc < 0)
		प्रत्यागम rc;

	प्रति (i = 0, tok = strtok_r(buf, " ", &कुंजी); tok;
	     tok = strtok_r(रिक्त, " ", &कुंजी)) {

		i++;
		यदि (i == n)
			प्रत्यागम ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		यदि (i == 2 && (p = म_खोजप(कुंजी, ')')))
			कुंजी = p + 2;
	}

	प्रत्यागम -EINVAL;
}

पूर्णाङ्क procfs_process_get_uid(संरचना path_cxt *pc, uid_t *uid)
{
	संरचना stat sb;
	पूर्णाङ्क rc;

	यदि ((rc = ul_path_stat(pc, &sb, 0, रिक्त)) == 0)
		*uid = sb.st_uid;
	प्रत्यागम rc;
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
पूर्णाङ्क procfs_process_next_tid(संरचना path_cxt *pc, सूची **sub, pid_t *tid)
{
	संरचना dirent *d;

	यदि (!pc || !sub || !tid)
		प्रत्यागम -EINVAL;

	यदि (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		यदि (!*sub)
			प्रत्यागम -त्रुटि_सं;
	}

	यावत् ((d = xreaddir(*sub))) {
		यदि (procfs_dirent_get_pid(d, tid) == 0)
			प्रत्यागम 0;
	}

	बंद_सूची(*sub);
	*sub = रिक्त;
	प्रत्यागम 1;
}

पूर्णाङ्क procfs_process_next_fd(संरचना path_cxt *pc, सूची **sub, पूर्णाङ्क *fd)
{
	संरचना dirent *d;

	यदि (!pc || !sub || !fd)
		प्रत्यागम -EINVAL;

	यदि (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		यदि (!*sub)
			प्रत्यागम -त्रुटि_सं;
	}

	यावत् ((d = xreaddir(*sub))) {
		uint64_t अंक;
#ifdef _DIRENT_HAVE_D_TYPE
		यदि (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			अनुवर्त;
#endif
		यदि (ul_strtou64(d->d_name, &अंक, 10) < 0)
			अनुवर्त;
		*fd = अंक;
		प्रत्यागम 0;
	}

	बंद_सूची(*sub);
	*sub = रिक्त;
	प्रत्यागम 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
पूर्णाङ्क procfs_dirent_is_process(संरचना dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	यदि (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		प्रत्यागम 0;
#endif
	यदि (!है_अंक((अचिन्हित वर्ण) *d->d_name))
		प्रत्यागम 0;

	प्रत्यागम 1;
}

पूर्णाङ्क procfs_dirent_get_pid(संरचना dirent *d, pid_t *pid)
{
	uint64_t अंक;

	यदि (!procfs_dirent_is_process(d))
		प्रत्यागम -EINVAL;

	यदि (ul_strtou64(d->d_name, &अंक, 10) < 0)
		प्रत्यागम -EINVAL;

	*pid = (pid_t) अंक;
	प्रत्यागम 0;
}

पूर्णाङ्क procfs_dirent_get_uid(सूची *procfs, संरचना dirent *d, uid_t *uid)
{
	संरचना stat st;

	यदि (!procfs_dirent_is_process(d))
		प्रत्यागम -EINVAL;

	यदि (fstatat(dirfd(procfs), d->d_name, &st, 0))
		प्रत्यागम -EINVAL;

	*uid = st.st_uid;
	प्रत्यागम 0;
}

पूर्णाङ्क procfs_dirent_match_uid(सूची *procfs, संरचना dirent *d, uid_t uid)
{
	uid_t x;

	यदि (procfs_dirent_get_uid(procfs, d, &x) == 0)
		प्रत्यागम x == uid;

	प्रत्यागम 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
पूर्णाङ्क procfs_dirent_get_name(सूची *procfs, संरचना dirent *d, वर्ण *buf, माप_प्रकार bufsz)
{
	खाता *f;
	माप_प्रकार sz;
	वर्ण tmp[1024], *p, *end = रिक्त;

	यदि (bufsz < 32)
		प्रत्यागम -EINVAL;
	यदि (!procfs_dirent_is_process(d))
		प्रत्यागम -EINVAL;

	snprintf(tmp, परिमाण(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	यदि (!f)
		प्रत्यागम -त्रुटि_सं;

	p = ख_माला_लो(tmp, परिमाण(tmp), f);
	ख_बंद(f);
	यदि (!p)
		प्रत्यागम -त्रुटि_सं;

	/* skip PID */
	यावत् (*p && *p != '(')
		p++;

	/* skip extra '(' */
	यावत् (*p && *p == '(')
		p++;

	end = p;
	यावत् (*end && *end != ')')
		end++;

	sz = end - p;
	यदि (sz >= bufsz)
		sz = bufsz - 1;

	स_नकल(buf, p, sz);
	buf[sz] = '\0';

	प्रत्यागम 0;
}

पूर्णाङ्क procfs_dirent_match_name(सूची *procfs, संरचना dirent *d, ध्रुव वर्ण *name)
{
	वर्ण buf[33];

	यदि (procfs_dirent_get_name(procfs, d, buf, परिमाण(buf)) == 0)
		प्रत्यागम म_भेद(name, buf) == 0;

	प्रत्यागम 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
पूर्णाङ्क fd_is_procfs(पूर्णाङ्क fd)
{
	संरचना statfs st;
	पूर्णाङ्क ret;

	कुरु {
		त्रुटि_सं = 0;
		ret = fstatfs(fd, &st);

		यदि (ret < 0) {
			यदि (त्रुटि_सं != EINTR && त्रुटि_सं != EAGAIN)
				प्रत्यागम 0;
			xusleep(250000);
		}
	} यावत् (ret != 0);

	प्रत्यागम F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#अन्यथा
पूर्णाङ्क fd_is_procfs(पूर्णाङ्क fd __attribute__((__unused__)))
{
	प्रत्यागम 0;
}
#endif

स्थिर वर्ण *strdup_procfs_file(pid_t pid, ध्रुव वर्ण *name)
{
	वर्ण buf[बफ_मान];
	वर्ण *re = रिक्त;
	पूर्णाङ्क fd;

	snprintf(buf, परिमाण(buf), _PATH_PROC "/%d/%s", (पूर्णाङ्क) pid, name);
	fd = खोलो(buf, O_CLOEXEC|O_RDONLY);
	यदि (fd < 0)
		प्रत्यागम रिक्त;

	यदि (read_procfs_file(रिक्त, fd, buf, परिमाण(buf)) > 0)
		re = strdup(buf);
	बंद(fd);
	प्रत्यागम re;
}

वर्ण *pid_get_cmdname(pid_t pid)
{
	प्रत्यागम strdup_procfs_file(pid, "comm");
}

वर्ण *pid_get_cmdline(pid_t pid)
{
	प्रत्यागम strdup_procfs_file(pid, "cmdline");
}

वर्ण *pid_get_personality(pid_t pid)
{
	प्रत्यागम strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

स्थिर पूर्णाङ्क test_tasks(पूर्णाङ्क argc, वर्ण *argv[], ध्रुव वर्ण *prefix)
{
	सूची *sub = रिक्त;
	संरचना path_cxt *pc;
	pid_t tid = 0, pid;

	यदि (argc != 2)
		प्रत्यागम निकास_त्रुटि;

	pid = म_से_दीर्घ(argv[1], (वर्ण **) रिक्त, 10);
	म_लिखो("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	यदि (!pc)
		err(निकास_त्रुटि, "alloc procfs handler failed");

	यावत् (procfs_process_next_tid(pc, &sub, &tid) == 0)
		म_लिखो(" %d", tid);

	म_लिखो("\n");
	ul_unref_path(pc);
	प्रत्यागम निकास_सफल;
}

स्थिर पूर्णाङ्क test_fds(पूर्णाङ्क argc, वर्ण *argv[], ध्रुव वर्ण *prefix)
{
	सूची *sub = रिक्त;
	संरचना path_cxt *pc;
	pid_t pid;
	पूर्णाङ्क fd = -1;

	यदि (argc != 2)
		प्रत्यागम निकास_त्रुटि;

	pid = म_से_दीर्घ(argv[1], (वर्ण **) रिक्त, 10);
	म_लिखो("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	यदि (!pc)
		err(निकास_त्रुटि, "alloc procfs handler failed");

	यावत् (procfs_process_next_fd(pc, &sub, &fd) == 0)
		म_लिखो(" %d", fd);

	ख_अक्षर_दो('\n', मानक_निकास);
	ul_unref_path(pc);
	प्रत्यागम निकास_सफल;
}

स्थिर पूर्णाङ्क test_processes(पूर्णाङ्क argc, वर्ण *argv[])
{
	सूची *dir;
	संरचना dirent *d;
	वर्ण *name = रिक्त;
	uid_t uid = (uid_t) -1;
	वर्ण buf[128];

	यदि (argc >= 3 && म_भेद(argv[1], "--name") == 0)
		name = argv[2];
	यदि (argc >= 3 && म_भेद(argv[1], "--uid") == 0)
		uid = (uid_t) म_से_द(argv[2]);

	dir = सूची_खोलो(_PATH_PROC);
	यदि (!dir)
		err(निकास_त्रुटि, "cannot open proc");

	यावत् ((d = xreaddir(dir))) {
		pid_t pid = 0;

		यदि (procfs_dirent_get_pid(d, &pid) != 0)
			अनुवर्त;
		यदि (name && !procfs_dirent_match_name(dir, d, name))
			अनुवर्त;
		यदि (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			अनुवर्त;
		procfs_dirent_get_name(dir, d, buf, परिमाण(buf));
		म_लिखो(" %d [%s]", pid, buf);
	}

	ख_अक्षर_दो('\n', मानक_निकास);
	बंद_सूची(dir);
	प्रत्यागम निकास_सफल;
}

स्थिर पूर्णाङ्क test_one_process(पूर्णाङ्क argc, वर्ण *argv[], ध्रुव वर्ण *prefix)
{
	pid_t pid;
	संरचना path_cxt *pc;
	वर्ण buf[बफ_मान];
	uid_t uid = (uid_t) -1;

	यदि (argc != 2)
		प्रत्यागम निकास_त्रुटि;
	pid = म_से_दीर्घ(argv[1], (वर्ण **) रिक्त, 10);

	pc = ul_new_procfs_path(pid, prefix);
	यदि (!pc)
		err(निकास_त्रुटि, "cannot alloc procfs handler");

	म_लिखो("%d\n", (पूर्णाङ्क) pid);

	procfs_process_get_uid(pc, &uid);
	म_लिखो("   UID: %zu\n", (माप_प्रकार) uid);

	procfs_process_get_cmdline(pc, buf, परिमाण(buf));
	म_लिखो("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, परिमाण(buf));
	म_लिखो("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	प्रत्यागम निकास_सफल;
}

स्थिर पूर्णाङ्क test_isprocfs(पूर्णाङ्क argc, वर्ण *argv[])
{
	ध्रुव वर्ण *name = argc > 1 ? argv[1] : "/proc";
	पूर्णाङ्क fd = खोलो(name, O_RDONLY | O_CLOEXEC);
	पूर्णाङ्क is = 0;

	यदि (fd >= 0) {
		is = fd_is_procfs(fd);
		बंद(fd);
	} अन्यथा
		err(निकास_त्रुटि, "cannot open %s", name);

	म_लिखो("%s: %s procfs\n", name, is ? "is" : "is NOT");
	प्रत्यागम is ? निकास_सफल : निकास_त्रुटि;
}

स्थिर पूर्णाङ्क test_process_stat_nth(पूर्णाङ्क argc, वर्ण *argv[], ध्रुव वर्ण *prefix)
{
	pid_t pid;
	संरचना path_cxt *pc;
	uint64_t अंक = 0;
	पूर्णाङ्क n, ret;

	यदि (argc != 3)
		प्रत्यागम निकास_त्रुटि;
	pid = म_से_दीर्घ(argv[1], (वर्ण **) रिक्त, 10);
	n = म_से_दीर्घ(argv[2], (वर्ण **) रिक्त, 10);

	pc = ul_new_procfs_path(pid, prefix);
	यदि (!pc)
		err(निकास_त्रुटि, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &अंक);
	यदि (ret)
		errx(निकास_त्रुटि, "read %dth number failed: %s", n, म_त्रुटि(-ret));

	म_लिखो("%d: %dth %"PRIu64"\n", (पूर्णाङ्क) pid, n, अंक);
	ul_unref_path(pc);
	प्रत्यागम निकास_सफल;
}

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	ध्रुव वर्ण *prefix = रिक्त;

	यदि (argc > 2 && म_भेद(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	यदि (argc < 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		प्रत्यागम निकास_त्रुटि;
	}

	यदि (म_भेद(argv[1], "--tasks") == 0)
		प्रत्यागम test_tasks(argc - 1, argv + 1, prefix);
	यदि (म_भेद(argv[1], "--fds") == 0)
		प्रत्यागम test_fds(argc - 1, argv + 1, prefix);
	यदि (म_भेद(argv[1], "--processes") == 0)
		प्रत्यागम test_processes(argc - 1, argv + 1);
	यदि (म_भेद(argv[1], "--is-procfs") == 0)
		प्रत्यागम test_isprocfs(argc - 1, argv + 1);
	यदि (म_भेद(argv[1], "--one") == 0)
		प्रत्यागम test_one_process(argc - 1, argv + 1, prefix);
	यदि (म_भेद(argv[1], "--stat-nth") == 0)
		प्रत्यागम test_process_stat_nth(argc - 1, argv + 1, prefix);

	प्रत्यागम निकास_त्रुटि;
}
#endif /* TEST_PROGRAM_PROCUTILS */
