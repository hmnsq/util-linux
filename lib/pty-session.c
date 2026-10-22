/*
 * This is pseudo-terminal container for child process where parent creates a
 * proxy between the current std{in,out,etrr} and the child's pty. Advantages:
 *
 * - child has no access to parent's terminal (e.g. su --pty)
 * - parent can log all traffic between user and child's terminal (e.g. script(1))
 * - it's possible to start commands on terminal although parent has no terminal
 *
 * This code is in the public domain; do with it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> in Jul 2019
 */
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <pty.h>
#ضمن <poll.h>
#ضمن <sys/signalfd.h>
#ضمن <paths.h>
#ضمن <sys/types.h>
#ضمن <sys/wait.h>
#ضمن <inttypes.h>

#ضمن "c.h"
#ضمن "all-io.h"
#ضمن "ttyutils.h"
#ضمن "pty-session.h"
#ضمن "monotonic.h"
#ضمن "debug.h"

ساكن UL_DEBUG_DEFINE_ماسک(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_تشه_MASKNAMES;

#تعریف_کړه ULPTY_DEBUG_INIT	(1 << 1)
#تعریف_کړه ULPTY_DEBUG_تیاری	(1 << 2)
#تعریف_کړه ULPTY_DEBUG_SIG		(1 << 3)
#تعریف_کړه ULPTY_DEBUG_IO		(1 << 4)
#تعریف_کړه ULPTY_DEBUG_بشپړ	(1 << 5)

#تعریف_کړه DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#تعریف_کړه DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#تعریف_کړه ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

فراغ ul_pty_init_debug(صحيح ماسک)
{
	إذا (ulpty_debug_ماسک)
		أرجع;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, ماسک, ULPTY_DEBUG);
}

بنية ul_pty *ul_new_pty(صحيح is_stdin_tty)
{
	بنية ul_pty *pty = حافظه_صفر_ورکړه(1, حجم(*pty));

	إذا (!pty)
		أرجع عدم;

	DBG_OBJ(تیاری, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->ماشوم = (pid_t) -1;

	أرجع pty;
}

فراغ ul_free_pty(بنية ul_pty *pty)
{
	بنية ul_pty_ماشوم_بفر *hd;

	طالما ((hd = pty->ماشوم_بفر_سر)) {
		pty->ماشوم_بفر_سر = hd->بل;
		حرر(hd);
	}

	طالما ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->بل;
		حرر(hd);
	}
	حرر(pty);
}

فراغ ul_pty_slave_echo(بنية ul_pty *pty, صحيح enable)
{
	أكد(pty);
	pty->slave_echo = enable ? 1 : 0;
}

صحيح ul_pty_get_delivered_signal(بنية ul_pty *pty)
{
	أكد(pty);
	أرجع pty->delivered_signal;
}

بنية ul_pty_callbacks *ul_pty_get_callbacks(بنية ul_pty *pty)
{
	أكد(pty);
	أرجع &pty->callbacks;
}

فراغ ul_pty_سېټ_callback_معلومات(بنية ul_pty *pty, فراغ *معلومات)
{
	أكد(pty);
	pty->callback_معلومات = معلومات;
}

فراغ ul_pty_سېټ_ماشوم(بنية ul_pty *pty, pid_t ماشوم)
{
	أكد(pty);
	pty->ماشوم = ماشوم;
}

صحيح ul_pty_get_childfd(بنية ul_pty *pty)
{
	أكد(pty);
	أرجع pty->master;
}

pid_t ul_pty_get_ماشوم(بنية ul_pty *pty)
{
	أكد(pty);
	أرجع pty->ماشوم;
}

/* it's active when signals are redirected to sigfd */
صحيح ul_pty_is_running(بنية ul_pty *pty)
{
	أكد(pty);
	أرجع pty->sigfd >= 0;
}

فراغ ul_pty_سېټ_mainloop_وخت(بنية ul_pty *pty, بنية timeval *tv)
{
	أكد(pty);
	إذا (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->بل_callback_وخت);
	} وإلا {
		pty->بل_callback_وخت.tv_sec = tv->tv_sec;
		pty->بل_callback_وخت.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

ساكن فراغ pty_سیګنالونه_cleanup(بنية ul_pty *pty)
{
	إذا (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, عدم);
}

/* call me before fork() */
صحيح ul_pty_تیاری(بنية ul_pty *pty)
{
	بنية termios attrs;
	صحيح rc = 0;

	أكد(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, عدم, &pty->orgsig);

	إذا (pty->isterm) {
		DBG_OBJ(تیاری, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		إذا (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			اذهب بشپړ;
		}

		attrs = pty->stdin_attrs;
		إذا (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		وإلا
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (حرف *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, عدم, &attrs, &pty->win);
		إذا (rc)
			اذهب بشپړ;

	} وإلا {
		DBG_OBJ(تیاری, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, عدم, عدم, عدم);
		إذا (rc)
			اذهب بشپړ;

		tcgetattr(pty->slave, &attrs);

		إذا (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		وإلا
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

بشپړ:
	إذا (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(تیاری, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	أرجع rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
صحيح ul_pty_terminal_تیاری(بنية ul_pty *pty)
{
	بنية termios attrs;

	إذا (!pty->isterm)
		أرجع 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	إذا (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		أرجع -errno;

	DBG_OBJ(تیاری, pty, ul_debug("terminal raw mode setup done"));
	أرجع 0;
}

/* call me before fork() */
صحيح ul_pty_سیګنالونه_تیاری(بنية ul_pty *pty)
{
	sigset_t ourset;
	صحيح rc = 0;

	أكد(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, عدم, &pty->orgsig);

	sigfillset(&ourset);
	إذا (sigprocmask(SIG_BLOCK, &ourset, عدم)) {
		rc = -errno;
		اذهب بشپړ;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, SIGTERM);
	sigaddset(&ourset, SIGINT);
	sigaddset(&ourset, SIGQUIT);

	إذا (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	إذا ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
بشپړ:
	إذا (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(تیاری, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	أرجع rc;
}

/* cleanup in parent process */
فراغ ul_pty_cleanup(بنية ul_pty *pty)
{
	بنية termios rtt;

	pty_سیګنالونه_cleanup(pty);

	إذا (pty->master == -1 || !pty->isterm)
		أرجع;

	DBG_OBJ(بشپړ_2, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

صحيح ul_pty_chownmod_slave(بنية ul_pty *pty, uid_t uid, gid_t gid, mode_t ډېرپېښ)
{
	إذا (fchown(pty->slave, uid, gid))
		أرجع -errno;
	إذا (fchmod(pty->slave, ډېرپېښ))
		أرجع -errno;
	أرجع 0;
}

/* call me in child process */
فراغ ul_pty_init_slave(بنية ul_pty *pty)
{
	DBG_OBJ(تیاری, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	إذا (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, عدم);

	DBG_OBJ(تیاری, pty, ul_debug("... initialize slave done"));
}

ساكن صحيح write_output(حرف *obuf, ssize_t بایټونه)
{
	DBG(IO, ul_debug(" writing output"));

	إذا (ul_write_all(STDOUT_FILENO, obuf, بایټونه)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		أرجع -errno;
	}

	أرجع 0;
}

ساكن صحيح مهالوېش_ماشوم_write(بنية ul_pty *pty, حرف *buf, size_t bufsz, صحيح final)
{
	بنية ul_pty_ماشوم_بفر *stash;

	إذا (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->بل;
		memset(stash, 0, حجم(*stash));
	} وإلا
		stash = حافظه_صفر_ورکړه(1, حجم(*stash));
	إذا (!stash)
		أرجع -1;

	أكد(bufsz <= حجم(stash->buf));

	حافظه_نقل(stash->buf, buf, bufsz);
	stash->اندازه = bufsz;
	stash->final_ننوت = final ? 1 : 0;

	إذا (pty->ماشوم_بفر_سر)
		pty->ماشوم_بفر_لکۍ = pty->ماشوم_بفر_لکۍ->بل = stash;
	وإلا
		pty->ماشوم_بفر_سر = pty->ماشوم_بفر_لکۍ = stash;
	أرجع 0;
}

/*
 * The pty is usually faster than shell, so it's a good idea to wait until
 * the previous message has been already read by shell from slave before we
 * write to master. This is necessary especially for EOF situation when we can
 * send EOF to master before shell is fully initialized, to workaround this
 * problem we wait until slave is empty. For example:
 *
 *   echo "date" | su --pty
 *
 * Unfortunately, the child (usually shell) can ignore stdin at all, so we
 * don't wait forever to avoid dead locks...
 *
 * Note that su --pty is primarily designed for interactive sessions as it
 * maintains master+slave tty stuff within the session. Use pipe to write to
 * pty and assume non-interactive (tee-like) behavior is NOT well supported.
 */
ساكن فراغ drain_ماشوم_buffers(بنية ul_pty *pty)
{
	بینښه صحيح tries = 0;
	بنية pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	طالما (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	إذا (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

ساكن صحيح flush_ماشوم_buffers(بنية ul_pty *pty, صحيح *anything)
{
	صحيح rc = 0, any = 0;

	طالما (pty->ماشوم_بفر_سر) {
		بنية ul_pty_ماشوم_بفر *hd = pty->ماشوم_بفر_سر;
		ssize_t ret;

		إذا (hd->final_ننوت)
			drain_ماشوم_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->اندازه - hd->cursor));

		ret = اكتب(pty->master, hd->buf + hd->cursor, hd->اندازه - hd->cursor);
		إذا (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			إذا (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			اذهب بهر;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		إذا (hd->cursor == hd->اندازه) {
			pty->ماشوم_بفر_سر = hd->بل;
			إذا (!hd->بل)
				pty->ماشوم_بفر_لکۍ = عدم;

			hd->بل = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
بهر:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	إذا (any)
		fdatasync(pty->master);

	إذا (anything)
		*anything = any;
	أرجع rc;
}

فراغ ul_pty_write_eof_منزل_ماشوم(بنية ul_pty *pty)
{
	حرف c = DEF_EOF;
	مهالوېش_ماشوم_write(pty, &c, حجم(حرف), 1);
}

ساكن صحيح mainloop_callback(بنية ul_pty *pty)
{
	صحيح rc;

	إذا (!pty->callbacks.mainloop)
		أرجع 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_معلومات);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	أرجع rc;
}

ساكن صحيح handle_io(بنية ul_pty *pty, صحيح fd, صحيح *eof)
{
	حرف buf[BUFSIZ];
	ssize_t بایټونه;
	صحيح rc = 0;
	sigset_t سېټ;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&سېټ);
	sigaddset(&سېټ, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &سېټ, عدم);
	/* read from active FD */
	بایټونه = اقرأ(fd, buf, حجم(buf));
	sigprocmask(SIG_BLOCK, &سېټ, عدم);
	إذا (بایټونه == -1) {
		إذا (errno == EAGAIN || errno == EINTR)
			أرجع 0;
		أرجع -errno;
	}

	إذا (بایټونه == 0) {
		*eof = 1;
		أرجع 0;
	}

	/* from stdin (user) to command */
	إذا (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", بایټونه));

		إذا (مهالوېش_ماشوم_write(pty, buf, بایټونه, 0))
			أرجع -errno;

	/* from command (master) to stdout */
	} وإلا إذا (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", بایټونه));
		write_output(buf, بایټونه);
	}

	إذا (pty->callbacks.log_بهیر_activity)
		rc = pty->callbacks.log_بهیر_activity(
					pty->callback_معلومات, fd, buf, بایټونه);

	أرجع rc;
}

فراغ ul_pty_wait_for_ماشوم(بنية ul_pty *pty)
{
	صحيح حالت;
	pid_t pid;
	صحيح options = 0;

	إذا (pty->ماشوم == (pid_t) -1)
		أرجع;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (صحيح) pty->ماشوم));

	إذا (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		لكل (;;) {
			pid = waitpid(pty->ماشوم, &حالت, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (صحيح) pid));
			إذا (pid != (pid_t) - 1) {
				إذا (pty->callbacks.ماشوم_die)
					pty->callbacks.ماشوم_die(
							pty->callback_معلومات,
							pty->ماشوم, حالت);
				ul_pty_سېټ_ماشوم(pty, (pid_t) -1);
			} وإلا
				اكسر;
		}
	} وإلا {
		/* final wait */
		طالما ((pid = waitpid(-1, &حالت, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (صحيح) pid));
			إذا (pid == pty->ماشوم) {
				إذا (pty->callbacks.ماشوم_die)
					pty->callbacks.ماشوم_die(
							pty->callback_معلومات,
							pty->ماشوم, حالت);
				ul_pty_سېټ_ماشوم(pty, (pid_t) -1);
			}
		}
	}
}

ساكن صحيح handle_signal(بنية ul_pty *pty, صحيح fd)
{
	بنية signalfd_siginfo info;
	ssize_t بایټونه;
	صحيح rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	بایټونه = اقرأ(fd, &info, حجم(info));
	إذا (بایټونه != حجم(info)) {
		إذا (بایټونه < 0 && (errno == EAGAIN || errno == EINTR))
			أرجع 0;
		أرجع -errno;
	}

	بدل (info.ssi_signo) {
	حالة SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		إذا (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			إذا (pty->callbacks.ماشوم_wait)
				pty->callbacks.ماشوم_wait(pty->callback_معلومات,
							  pty->ماشوم);
			وإلا
				ul_pty_wait_for_ماشوم(pty);

		} وإلا إذا (info.ssi_status == SIGSTOP && pty->ماشوم > 0) {
			pty->callbacks.ماشوم_sigstop(pty->callback_معلومات,
						     pty->ماشوم);
		}

		إذا (pty->ماشوم <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->بل_callback_وخت);
		}
		أرجع 0;
	حالة SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		إذا (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (حرف *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (حرف *)&pty->win);

			إذا (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_معلومات,
							&info, (فراغ *) &pty->win);
		}
		اكسر;
	حالة SIGALRM:
		FALLTHROUGH;
	حالة SIGHUP:
		FALLTHROUGH;
	حالة SIGTERM:
		FALLTHROUGH;
	حالة SIGINT:
		FALLTHROUGH;
	حالة SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		إذا (pty->ماشوم > 0)
			kill(pty->ماشوم, SIGTERM);

		إذا (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_معلومات,
					&info, (فراغ *) &pty->win);
		اكسر;
	حالة SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		إذا (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_معلومات);
		اكسر;
	افتراضي:
		abort();
	}

	أرجع rc;
}

/* loop in parent */
صحيح ul_pty_proxy_master(بنية ul_pty *pty)
{
	صحيح rc = 0, ret, eof = 0;
	تعداد {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	بنية pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	أكد(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	طالما (!pty->delivered_signal) {
		size_t i;
		صحيح errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		إذا (timerisset(&pty->بل_callback_وخت)) {
			بنية timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			إذا (timercmp(&now, &pty->بل_callback_وخت, >)) {
				rc = mainloop_callback(pty);
				إذا (rc)
					اكسر;
			}
		}

		/* set timeout */
		إذا (timerisset(&pty->بل_callback_وخت)) {
			بنية timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->بل_callback_وخت, &now, &rest);
			timeout = (rest.tv_sec * MSEC_فی_SEC) +  (rest.tv_usec / USEC_فی_MSEC);
		} وإلا
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		إذا (pty->ماشوم_بفر_سر)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		وإلا
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_اندازه(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		إذا (ret < 0) {
			إذا (errsv == EAGAIN)
				استمر;
			rc = -errno;
			اكسر;
		}

		/* timeout */
		إذا (ret == 0) {
			إذا (timerisset(&pty->بل_callback_وخت)) {
				rc = mainloop_callback(pty);
				إذا (rc == 0)
					استمر;
			} وإلا {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			اكسر;
		}
		/* event */
		لكل (i = 0; i < ARRAY_اندازه(pfd); i++) {
			إذا (pfd[i].revents == 0)
				استمر;

			DBG_OBJ(IO, pty, ul_debug(" active pfd[%s].fd=%d %s %s %s %s %s",
						i == POLLFD_STDIN  ? "stdin" :
						i == POLLFD_MASTER ? "master" :
						i == POLLFD_SIGNAL ? "signal" : "???",
						pfd[i].fd,
						pfd[i].revents & POLLIN  ? "POLLIN" : "",
						pfd[i].revents & POLLOUT ? "POLLOUT" : "",
						pfd[i].revents & POLLHUP ? "POLLHUP" : "",
						pfd[i].revents & POLLERR ? "POLLERR" : "",
						pfd[i].revents & POLLNVAL ? "POLLNVAL" : ""));

			إذا (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			وإلا {
				إذا (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /*معلومات*/
				إذا (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_ماشوم_buffers(pty, عدم);
			}

			إذا (rc) {
				صحيح anything = 1;

				ul_pty_write_eof_منزل_ماشوم(pty);
				لكل (anything = 1; anything;)
					flush_ماشوم_buffers(pty, &anything);
				اكسر;
			}

			إذا (i == POLLFD_SIGNAL)
				استمر;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			إذا ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				إذا (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_منزل_ماشوم(pty);
				} وإلا /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		إذا (rc)
			اكسر;
	}

	إذا (rc && pty->ماشوم && pty->ماشوم != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->ماشوم, SIGTERM);
		sleep(2);
		kill(pty->ماشوم, SIGKILL);
	}

	pty_سیګنالونه_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	أرجع rc;
}

#که_تعریف_شوی TEST_پروګرام_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
ساكن فراغ ماشوم_sigstop(فراغ *معلومات __attribute__((__unused__)), pid_t ماشوم)
{
	kill(getpid(), SIGSTOP);
	kill(ماشوم, SIGCONT);
}

صحيح اصلي(صحيح argc, حرف *argv[])
{
	بنية ul_pty_callbacks *cb;
	ثابت حرف *shell, *command = عدم, *shname = عدم;
	صحيح caught_signal = 0;
	pid_t ماشوم;
	بنية ul_pty *pty;

	shell = getenv("SHELL");
	إذا (shell == عدم)
		shell = _PATH_BSHELL;
	إذا (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	إذا (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->ماشوم_sigstop = ماشوم_sigstop;

	إذا (ul_pty_تیاری(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	إذا (ul_pty_سیګنالونه_تیاری(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	إذا (ul_pty_terminal_تیاری(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	بدل ((صحيح) (ماشوم = fork())) {
	حالة -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		اكسر;

	حالة 0: /*ماشوم*/
		ul_pty_init_slave(pty);

		إشارة(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		إذا (command)
			execl(shell, shname, "-c", command, (حرف *)عدم);
		وإلا
			execl(shell, shname, "-i", (حرف *)عدم);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		اكسر;

	افتراضي:
		اكسر;
	}

	/*پلار*/
	ul_pty_سېټ_ماشوم(pty, ماشوم);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	إذا (!caught_signal && ul_pty_get_ماشوم(pty) != (pid_t)-1)
		ul_pty_wait_for_ماشوم(pty);	/* final wait */

	إذا (caught_signal && ul_pty_get_ماشوم(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(ماشوم, SIGTERM);
		sleep(2);
		kill(ماشوم, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	أرجع EXIT_SUCCESS;
}

#د_که_پای /* TEST_PROGRAM */

