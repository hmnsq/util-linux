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
	जर (ulprocfs_debug_mask)
		परत;
	__UL_INIT_DEBUG_FROM_ENV(ulprocfs, ULPROCFS_DEBUG_, 0, ULPROCFS_DEBUG);
}

संरचना path_cxt *ul_new_procfs_path(pid_t pid, स्थिरांक अक्षर *prefix)
{
	संरचना path_cxt *pc = ul_new_path(रिक्त);

	जर (!pc)
		परत रिक्त;
	जर (prefix)
		ul_path_set_prefix(pc, prefix);

	जर (procfs_process_init_path(pc, pid) != 0) {
		ul_unref_path(pc);
		परत रिक्त;
	}

	DBG_OBJ(CXT, pc, ul_debug("alloc"));
	परत pc;
}

/*
 * procfs_blkdev_* is procfs extension to ul_path_* API to read info about process.
 *
 * The function is possible to call in loop and without sysfs_procfs_deinit_path().
 * The procfs_process_deinit_path() is automatically called by ul_unref_path().
 *
 */
पूर्णांक procfs_process_init_path(संरचना path_cxt *pc, pid_t pid)
{
	संरचना procfs_process *prc;
	पूर्णांक rc;
	अक्षर buf[आकार(_PATH_PROC) + आकार(stringify_मूल्य(UINT32_MAX)) + 2];

	/* define path to pid stuff */
	snprintf(buf, आकार(buf), _PATH_PROC "/%zu", (माप_प्रकार) pid);
	rc = ul_path_set_dir(pc, buf);
	जर (rc)
		परत rc;

	/* make sure path exists */
	rc = ul_path_get_dirfd(pc);
	जर (rc < 0)
		परत rc;

	/* initialize procfs specific stuff */
	prc = ul_path_get_dialect(pc);
	जर (!prc) {
		DBG_OBJ(CXT, pc, ul_debug("alloc new procfs handler"));
		prc = सुस्मृति(1, आकार(संरचना procfs_process));
		जर (!prc)
			परत -ENOMEM;

		ul_path_set_dialect(pc, prc, procfs_process_deinit_path);
	}

	DBG_OBJ(CXT, pc, ul_debug("init procfs stuff"));

	prc->pid = pid;
	परत 0;
}

स्थिर शून्य procfs_process_deinit_path(संरचना path_cxt *pc)
{
	संरचना procfs_process *prc;

	जर (!pc)
		परत;

	DBG_OBJ(CXT, pc, ul_debug("deinit"));

	prc = ul_path_get_dialect(pc);
	जर (!prc)
		परत;

	मुक्त(prc);
	ul_path_set_dialect(pc, रिक्त, रिक्त);
}

स्थिर ssize_t read_procfs_file(स्थिरांक संरचना ul_vfs_ops *vfs, पूर्णांक fd,
			       अक्षर *buf, माप_प्रकार bufsz)
{
	ssize_t sz = 0;
	माप_प्रकार i;

	जर (fd < 0)
		परत -EINVAL;

	sz = ul_vfs_read_all(vfs, fd, buf, bufsz);
	जर (sz <= 0)
		परत sz;

	साठी (i = 0; i < (माप_प्रकार) sz; i++) {
		जर (buf[i] == '\0')
			buf[i] = ' ';
	}
	buf[sz - 1] = '\0';
	परत sz;
}

स्थिर ssize_t procfs_process_get_data_for(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz,
					    स्थिरांक अक्षर *fname)
{
	पूर्णांक fd = ul_path_open(pc, O_RDONLY|O_CLOEXEC, fname);

	जर (fd >= 0) {
		ssize_t sz = read_procfs_file(pc ? pc->vfs : रिक्त, fd, buf, bufsz);
		ul_vfs_close(pc ? pc->vfs : रिक्त, fd);
		परत sz;
	}
	परत -त्रुटि_सं;
}

ssize_t procfs_process_get_cmdline(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz)
{
	परत procfs_process_get_data_for(pc, buf, bufsz, "cmdline");
}

ssize_t procfs_process_get_cmdname(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz)
{
	परत procfs_process_get_data_for(pc, buf, bufsz, "comm");
}

ssize_t procfs_process_get_stat(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz)
{
	परत procfs_process_get_data_for(pc, buf, bufsz, "stat");
}

ssize_t procfs_process_get_syscall(संरचना path_cxt *pc, अक्षर *buf, माप_प्रकार bufsz)
{
	परत procfs_process_get_data_for(pc, buf, bufsz, "syscall");
}

पूर्णांक procfs_process_get_stat_nth(संरचना path_cxt *pc, पूर्णांक n, uint64_t *re)
{
	ssize_t rc;
	अक्षर *किल्ली = रिक्त, *tok, *p;
	अक्षर buf[बफ_मान];
	पूर्णांक i;

	जर (n == 2 || n == 3)		/* process name and status (strings) */
		परत -EINVAL;

	rc = procfs_process_get_data_for(pc, buf, आकार(buf), "stat");
	जर (rc < 0)
		परत rc;

	साठी (i = 0, tok = strtok_r(buf, " ", &किल्ली); tok;
	     tok = strtok_r(रिक्त, " ", &किल्ली)) {

		i++;
		जर (i == n)
			परत ul_strtou64(tok, re, 10);

		/* skip rest of the process name */
		जर (i == 2 && (p = म_खोजप(किल्ली, ')')))
			किल्ली = p + 2;
	}

	परत -EINVAL;
}

पूर्णांक procfs_process_get_uid(संरचना path_cxt *pc, uid_t *uid)
{
	संरचना stat sb;
	पूर्णांक rc;

	जर ((rc = ul_path_stat(pc, &sb, 0, रिक्त)) == 0)
		*uid = sb.st_uid;
	परत rc;
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
पूर्णांक procfs_process_next_tid(संरचना path_cxt *pc, सूची **sub, pid_t *tid)
{
	संरचना dirent *d;

	जर (!pc || !sub || !tid)
		परत -EINVAL;

	जर (!*sub) {
		*sub = ul_path_opendir(pc, "task");
		जर (!*sub)
			परत -त्रुटि_सं;
	}

	पर्यंत ((d = xreaddir(*sub))) {
		जर (procfs_dirent_get_pid(d, tid) == 0)
			परत 0;
	}

	बंद_सूची(*sub);
	*sub = रिक्त;
	परत 1;
}

पूर्णांक procfs_process_next_fd(संरचना path_cxt *pc, सूची **sub, पूर्णांक *fd)
{
	संरचना dirent *d;

	जर (!pc || !sub || !fd)
		परत -EINVAL;

	जर (!*sub) {
		*sub = ul_path_opendir(pc, "fd");
		जर (!*sub)
			परत -त्रुटि_सं;
	}

	पर्यंत ((d = xreaddir(*sub))) {
		uint64_t अंक;
#ifdef _DIRENT_HAVE_D_TYPE
		जर (d->d_type != DT_LNK && d->d_type != DT_UNKNOWN)
			सुरूठेवा;
#endif
		जर (ul_strtou64(d->d_name, &अंक, 10) < 0)
			सुरूठेवा;
		*fd = अंक;
		परत 0;
	}

	बंद_सूची(*sub);
	*sub = रिक्त;
	परत 1;
}

/*
 * Simple 'dirent' based stuff for use-cases where procfs_process_* API is overkill
 */

/* stupid, but good enough as a basic filter */
पूर्णांक procfs_dirent_is_process(संरचना dirent *d)
{
#ifdef _DIRENT_HAVE_D_TYPE
	जर (d->d_type != DT_DIR && d->d_type != DT_UNKNOWN)
		परत 0;
#endif
	जर (!है_अंक((अचिन्हित अक्षर) *d->d_name))
		परत 0;

	परत 1;
}

पूर्णांक procfs_dirent_get_pid(संरचना dirent *d, pid_t *pid)
{
	uint64_t अंक;

	जर (!procfs_dirent_is_process(d))
		परत -EINVAL;

	जर (ul_strtou64(d->d_name, &अंक, 10) < 0)
		परत -EINVAL;

	*pid = (pid_t) अंक;
	परत 0;
}

पूर्णांक procfs_dirent_get_uid(सूची *procfs, संरचना dirent *d, uid_t *uid)
{
	संरचना stat st;

	जर (!procfs_dirent_is_process(d))
		परत -EINVAL;

	जर (fstatat(dirfd(procfs), d->d_name, &st, 0))
		परत -EINVAL;

	*uid = st.st_uid;
	परत 0;
}

पूर्णांक procfs_dirent_match_uid(सूची *procfs, संरचना dirent *d, uid_t uid)
{
	uid_t x;

	जर (procfs_dirent_get_uid(procfs, d, &x) == 0)
		परत x == uid;

	परत 0;
}

/* "name" of process; may be truncated, see prctl(2) and PR_SET_NAME.
 * The minimal of the @buf has to be 32 bytes. */
पूर्णांक procfs_dirent_get_name(सूची *procfs, संरचना dirent *d, अक्षर *buf, माप_प्रकार bufsz)
{
	खाता *f;
	माप_प्रकार sz;
	अक्षर tmp[1024], *p, *end = रिक्त;

	जर (bufsz < 32)
		परत -EINVAL;
	जर (!procfs_dirent_is_process(d))
		परत -EINVAL;

	snprintf(tmp, आकार(tmp), "%s/stat", d->d_name);
	f = fopen_at(dirfd(procfs), tmp, O_CLOEXEC|O_RDONLY, "r");
	जर (!f)
		परत -त्रुटि_सं;

	p = ख_माला_लो(tmp, आकार(tmp), f);
	ख_बंद(f);
	जर (!p)
		परत -त्रुटि_सं;

	/* skip PID */
	पर्यंत (*p && *p != '(')
		p++;

	/* skip extra '(' */
	पर्यंत (*p && *p == '(')
		p++;

	end = p;
	पर्यंत (*end && *end != ')')
		end++;

	sz = end - p;
	जर (sz >= bufsz)
		sz = bufsz - 1;

	स_नकल(buf, p, sz);
	buf[sz] = '\0';

	परत 0;
}

पूर्णांक procfs_dirent_match_name(सूची *procfs, संरचना dirent *d, स्थिरांक अक्षर *name)
{
	अक्षर buf[33];

	जर (procfs_dirent_get_name(procfs, d, buf, आकार(buf)) == 0)
		परत म_भेद(name, buf) == 0;

	परत 0;
}

#ifdef HAVE_SYS_VFS_H
/* checks if fd is file in a procfs;
 * returns 1 if true, 0 if false or couldn't determine */
पूर्णांक fd_is_procfs(पूर्णांक fd)
{
	संरचना statfs st;
	पूर्णांक ret;

	करा {
		त्रुटि_सं = 0;
		ret = fstatfs(fd, &st);

		जर (ret < 0) {
			जर (त्रुटि_सं != EINTR && त्रुटि_सं != EAGAIN)
				परत 0;
			xusleep(250000);
		}
	} पर्यंत (ret != 0);

	परत F_TYPE_EQUAL(st.f_type, STATFS_PROC_MAGIC);
}
#अन्यथा
पूर्णांक fd_is_procfs(पूर्णांक fd __attribute__((__unused__)))
{
	परत 0;
}
#endif

स्थिर अक्षर *strdup_procfs_file(pid_t pid, स्थिरांक अक्षर *name)
{
	अक्षर buf[बफ_मान];
	अक्षर *re = रिक्त;
	पूर्णांक fd;

	snprintf(buf, आकार(buf), _PATH_PROC "/%d/%s", (पूर्णांक) pid, name);
	fd = खोलो(buf, O_CLOEXEC|O_RDONLY);
	जर (fd < 0)
		परत रिक्त;

	जर (read_procfs_file(रिक्त, fd, buf, आकार(buf)) > 0)
		re = strdup(buf);
	बंद(fd);
	परत re;
}

अक्षर *pid_get_cmdname(pid_t pid)
{
	परत strdup_procfs_file(pid, "comm");
}

अक्षर *pid_get_cmdline(pid_t pid)
{
	परत strdup_procfs_file(pid, "cmdline");
}

अक्षर *pid_get_personality(pid_t pid)
{
	परत strdup_procfs_file(pid, "personality");
}

#ifdef TEST_PROGRAM_PROCFS

स्थिर पूर्णांक test_tasks(पूर्णांक argc, अक्षर *argv[], स्थिरांक अक्षर *prefix)
{
	सूची *sub = रिक्त;
	संरचना path_cxt *pc;
	pid_t tid = 0, pid;

	जर (argc != 2)
		परत निकास_त्रुटि;

	pid = म_से_दीर्घ(argv[1], (अक्षर **) रिक्त, 10);
	म_लिखो("PID=%d, TIDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	जर (!pc)
		err(निकास_त्रुटि, "alloc procfs handler failed");

	पर्यंत (procfs_process_next_tid(pc, &sub, &tid) == 0)
		म_लिखो(" %d", tid);

	म_लिखो("\n");
	ul_unref_path(pc);
	परत निकास_सफल;
}

स्थिर पूर्णांक test_fds(पूर्णांक argc, अक्षर *argv[], स्थिरांक अक्षर *prefix)
{
	सूची *sub = रिक्त;
	संरचना path_cxt *pc;
	pid_t pid;
	पूर्णांक fd = -1;

	जर (argc != 2)
		परत निकास_त्रुटि;

	pid = म_से_दीर्घ(argv[1], (अक्षर **) रिक्त, 10);
	म_लिखो("PID=%d, FDs:", pid);

	pc = ul_new_procfs_path(pid, prefix);
	जर (!pc)
		err(निकास_त्रुटि, "alloc procfs handler failed");

	पर्यंत (procfs_process_next_fd(pc, &sub, &fd) == 0)
		म_लिखो(" %d", fd);

	ख_अक्षर_दो('\n', मानक_निकास);
	ul_unref_path(pc);
	परत निकास_सफल;
}

स्थिर पूर्णांक test_processes(पूर्णांक argc, अक्षर *argv[])
{
	सूची *dir;
	संरचना dirent *d;
	अक्षर *name = रिक्त;
	uid_t uid = (uid_t) -1;
	अक्षर buf[128];

	जर (argc >= 3 && म_भेद(argv[1], "--name") == 0)
		name = argv[2];
	जर (argc >= 3 && म_भेद(argv[1], "--uid") == 0)
		uid = (uid_t) म_से_द(argv[2]);

	dir = सूची_खोलो(_PATH_PROC);
	जर (!dir)
		err(निकास_त्रुटि, "cannot open proc");

	पर्यंत ((d = xreaddir(dir))) {
		pid_t pid = 0;

		जर (procfs_dirent_get_pid(d, &pid) != 0)
			सुरूठेवा;
		जर (name && !procfs_dirent_match_name(dir, d, name))
			सुरूठेवा;
		जर (uid != (uid_t) -1 && !procfs_dirent_match_uid(dir, d, uid))
			सुरूठेवा;
		procfs_dirent_get_name(dir, d, buf, आकार(buf));
		म_लिखो(" %d [%s]", pid, buf);
	}

	ख_अक्षर_दो('\n', मानक_निकास);
	बंद_सूची(dir);
	परत निकास_सफल;
}

स्थिर पूर्णांक test_one_process(पूर्णांक argc, अक्षर *argv[], स्थिरांक अक्षर *prefix)
{
	pid_t pid;
	संरचना path_cxt *pc;
	अक्षर buf[बफ_मान];
	uid_t uid = (uid_t) -1;

	जर (argc != 2)
		परत निकास_त्रुटि;
	pid = म_से_दीर्घ(argv[1], (अक्षर **) रिक्त, 10);

	pc = ul_new_procfs_path(pid, prefix);
	जर (!pc)
		err(निकास_त्रुटि, "cannot alloc procfs handler");

	म_लिखो("%d\n", (पूर्णांक) pid);

	procfs_process_get_uid(pc, &uid);
	म_लिखो("   UID: %zu\n", (माप_प्रकार) uid);

	procfs_process_get_cmdline(pc, buf, आकार(buf));
	म_लिखो("   CMDLINE: '%s'\n", buf);

	procfs_process_get_cmdname(pc, buf, आकार(buf));
	म_लिखो("   COMM: '%s'\n", buf);

	ul_unref_path(pc);
	परत निकास_सफल;
}

स्थिर पूर्णांक test_isprocfs(पूर्णांक argc, अक्षर *argv[])
{
	स्थिरांक अक्षर *name = argc > 1 ? argv[1] : "/proc";
	पूर्णांक fd = खोलो(name, O_RDONLY | O_CLOEXEC);
	पूर्णांक is = 0;

	जर (fd >= 0) {
		is = fd_is_procfs(fd);
		बंद(fd);
	} अन्यथा
		err(निकास_त्रुटि, "cannot open %s", name);

	म_लिखो("%s: %s procfs\n", name, is ? "is" : "is NOT");
	परत is ? निकास_सफल : निकास_त्रुटि;
}

स्थिर पूर्णांक test_process_stat_nth(पूर्णांक argc, अक्षर *argv[], स्थिरांक अक्षर *prefix)
{
	pid_t pid;
	संरचना path_cxt *pc;
	uint64_t अंक = 0;
	पूर्णांक n, ret;

	जर (argc != 3)
		परत निकास_त्रुटि;
	pid = म_से_दीर्घ(argv[1], (अक्षर **) रिक्त, 10);
	n = म_से_दीर्घ(argv[2], (अक्षर **) रिक्त, 10);

	pc = ul_new_procfs_path(pid, prefix);
	जर (!pc)
		err(निकास_त्रुटि, "cannot alloc procfs handler");

	ret = procfs_process_get_stat_nth(pc, n, &अंक);
	जर (ret)
		errx(निकास_त्रुटि, "read %dth number failed: %s", n, म_त्रुटि(-ret));

	म_लिखो("%d: %dth %"PRIu64"\n", (पूर्णांक) pid, n, अंक);
	ul_unref_path(pc);
	परत निकास_सफल;
}

पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
	स्थिरांक अक्षर *prefix = रिक्त;

	जर (argc > 2 && म_भेद(argv[1], "--prefix") == 0) {
		prefix = argv[2];
		argc -= 2;
		argv += 2;
	}

	जर (argc < 2) {
		ख_लिखो(मानक_त्रुटि, "usage: %1$s [--prefix <prefix>] --tasks <pid>\n"
				"       %1$s [--prefix <prefix>] --fds <pid>\n"
				"       %1$s --is-procfs [<dir>]\n"
				"       %1$s --processes [--name <name>] [--uid <uid>]\n"
				"       %1$s [--prefix <prefix>] --one <pid>\n"
				"       %1$s [--prefix <prefix>] --stat-nth <pid> <n>\n",
				program_invocation_short_name);
		परत निकास_त्रुटि;
	}

	जर (म_भेद(argv[1], "--tasks") == 0)
		परत test_tasks(argc - 1, argv + 1, prefix);
	जर (म_भेद(argv[1], "--fds") == 0)
		परत test_fds(argc - 1, argv + 1, prefix);
	जर (म_भेद(argv[1], "--processes") == 0)
		परत test_processes(argc - 1, argv + 1);
	जर (म_भेद(argv[1], "--is-procfs") == 0)
		परत test_isprocfs(argc - 1, argv + 1);
	जर (म_भेद(argv[1], "--one") == 0)
		परत test_one_process(argc - 1, argv + 1, prefix);
	जर (म_भेद(argv[1], "--stat-nth") == 0)
		परत test_process_stat_nth(argc - 1, argv + 1, prefix);

	परत निकास_त्रुटि;
}
#endif /* TEST_PROGRAM_PROCUTILS */
