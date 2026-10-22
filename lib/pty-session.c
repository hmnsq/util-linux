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

ساكن UL_DEBUG_DEFINE_نقاب(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_خالي_جايون_MASKNAMES;

#وصف_ڪريو ULPTY_DEBUG_INIT	(1 << 1)
#وصف_ڪريو ULPTY_DEBUG_تياري	(1 << 2)
#وصف_ڪريو ULPTY_DEBUG_SIG		(1 << 3)
#وصف_ڪريو ULPTY_DEBUG_IO		(1 << 4)
#وصف_ڪريو ULPTY_DEBUG_پورو	(1 << 5)

#وصف_ڪريو DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#وصف_ڪريو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#وصف_ڪريو ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

فراغ ul_pty_init_debug(صحيح نقاب)
{
	إذا (ulpty_debug_نقاب)
		أرجع;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, نقاب, ULPTY_DEBUG);
}

بنية ul_pty *ul_new_pty(صحيح is_stdin_tty)
{
	بنية ul_pty *pty = صفر_يادگيري_ڏيو(1, حجم(*pty));

	إذا (!pty)
		أرجع عدم;

	DBG_OBJ(تياري, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->ٻار = (pid_t) -1;

	أرجع pty;
}

فراغ ul_free_pty(بنية ul_pty *pty)
{
	بنية ul_pty_ٻار_بفر *hd;

	طالما ((hd = pty->ٻار_بفر_سر)) {
		pty->ٻار_بفر_سر = hd->ايندڙ;
		حرر(hd);
	}

	طالما ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->ايندڙ;
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

فراغ ul_pty_سيٽ_callback_معلومات(بنية ul_pty *pty, فراغ *معلومات)
{
	أكد(pty);
	pty->callback_معلومات = معلومات;
}

فراغ ul_pty_سيٽ_ٻار(بنية ul_pty *pty, pid_t ٻار)
{
	أكد(pty);
	pty->ٻار = ٻار;
}

صحيح ul_pty_get_childfd(بنية ul_pty *pty)
{
	أكد(pty);
	أرجع pty->master;
}

pid_t ul_pty_get_ٻار(بنية ul_pty *pty)
{
	أكد(pty);
	أرجع pty->ٻار;
}

/* it's active when signals are redirected to sigfd */
صحيح ul_pty_is_running(بنية ul_pty *pty)
{
	أكد(pty);
	أرجع pty->sigfd >= 0;
}

فراغ ul_pty_سيٽ_mainloop_وقت(بنية ul_pty *pty, بنية timeval *tv)
{
	أكد(pty);
	إذا (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->ايندڙ_callback_وقت);
	} وإلا {
		pty->ايندڙ_callback_وقت.tv_sec = tv->tv_sec;
		pty->ايندڙ_callback_وقت.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

ساكن فراغ pty_سگنل_cleanup(بنية ul_pty *pty)
{
	إذا (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, عدم);
}

/* call me before fork() */
صحيح ul_pty_تياري(بنية ul_pty *pty)
{
	بنية termios attrs;
	صحيح rc = 0;

	أكد(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, عدم, &pty->orgsig);

	إذا (pty->isterm) {
		DBG_OBJ(تياري, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		إذا (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			اذهب پورو;
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
			اذهب پورو;

	} وإلا {
		DBG_OBJ(تياري, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, عدم, عدم, عدم);
		إذا (rc)
			اذهب پورو;

		tcgetattr(pty->slave, &attrs);

		إذا (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		وإلا
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

پورو:
	إذا (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(تياري, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	أرجع rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
صحيح ul_pty_terminal_تياري(بنية ul_pty *pty)
{
	بنية termios attrs;

	إذا (!pty->isterm)
		أرجع 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	إذا (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		أرجع -errno;

	DBG_OBJ(تياري, pty, ul_debug("terminal raw mode setup done"));
	أرجع 0;
}

/* call me before fork() */
صحيح ul_pty_سگنل_تياري(بنية ul_pty *pty)
{
	sigset_t ourset;
	صحيح rc = 0;

	أكد(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, عدم, &pty->orgsig);

	sigfillset(&ourset);
	إذا (sigprocmask(SIG_BLOCK, &ourset, عدم)) {
		rc = -errno;
		اذهب پورو;
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
پورو:
	إذا (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(تياري, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	أرجع rc;
}

/* cleanup in parent process */
فراغ ul_pty_cleanup(بنية ul_pty *pty)
{
	بنية termios rtt;

	pty_سگنل_cleanup(pty);

	إذا (pty->master == -1 || !pty->isterm)
		أرجع;

	DBG_OBJ(پورو_2, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

صحيح ul_pty_chownmod_slave(بنية ul_pty *pty, uid_t uid, gid_t gid, mode_t گهڻائي)
{
	إذا (fchown(pty->slave, uid, gid))
		أرجع -errno;
	إذا (fchmod(pty->slave, گهڻائي))
		أرجع -errno;
	أرجع 0;
}

/* call me in child process */
فراغ ul_pty_init_slave(بنية ul_pty *pty)
{
	DBG_OBJ(تياري, pty, ul_debug("initialize slave"));

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

	DBG_OBJ(تياري, pty, ul_debug("... initialize slave done"));
}

ساكن صحيح write_output(حرف *obuf, ssize_t بائيٽون)
{
	DBG(IO, ul_debug(" writing output"));

	إذا (ul_write_all(STDOUT_FILENO, obuf, بائيٽون)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		أرجع -errno;
	}

	أرجع 0;
}

ساكن صحيح وقت_جدول_ٻار_write(بنية ul_pty *pty, حرف *buf, size_t bufsz, صحيح final)
{
	بنية ul_pty_ٻار_بفر *stash;

	إذا (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->ايندڙ;
		memset(stash, 0, حجم(*stash));
	} وإلا
		stash = صفر_يادگيري_ڏيو(1, حجم(*stash));
	إذا (!stash)
		أرجع -1;

	أكد(bufsz <= حجم(stash->buf));

	يادگيري_نقل(stash->buf, buf, bufsz);
	stash->سائيز = bufsz;
	stash->final_داخلا = final ? 1 : 0;

	إذا (pty->ٻار_بفر_سر)
		pty->ٻار_بفر_پڇ = pty->ٻار_بفر_پڇ->ايندڙ = stash;
	وإلا
		pty->ٻار_بفر_سر = pty->ٻار_بفر_پڇ = stash;
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
ساكن فراغ drain_ٻار_buffers(بنية ul_pty *pty)
{
	بنانشان صحيح tries = 0;
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

ساكن صحيح flush_ٻار_buffers(بنية ul_pty *pty, صحيح *anything)
{
	صحيح rc = 0, any = 0;

	طالما (pty->ٻار_بفر_سر) {
		بنية ul_pty_ٻار_بفر *hd = pty->ٻار_بفر_سر;
		ssize_t ret;

		إذا (hd->final_داخلا)
			drain_ٻار_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->سائيز - hd->cursor));

		ret = اكتب(pty->master, hd->buf + hd->cursor, hd->سائيز - hd->cursor);
		إذا (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			إذا (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			اذهب ٻاهريون;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		إذا (hd->cursor == hd->سائيز) {
			pty->ٻار_بفر_سر = hd->ايندڙ;
			إذا (!hd->ايندڙ)
				pty->ٻار_بفر_پڇ = عدم;

			hd->ايندڙ = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
ٻاهريون:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	إذا (any)
		fdatasync(pty->master);

	إذا (anything)
		*anything = any;
	أرجع rc;
}

فراغ ul_pty_write_eof_منزل_ٻار(بنية ul_pty *pty)
{
	حرف c = DEF_EOF;
	وقت_جدول_ٻار_write(pty, &c, حجم(حرف), 1);
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
	ssize_t بائيٽون;
	صحيح rc = 0;
	sigset_t سيٽ;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&سيٽ);
	sigaddset(&سيٽ, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &سيٽ, عدم);
	/* read from active FD */
	بائيٽون = اقرأ(fd, buf, حجم(buf));
	sigprocmask(SIG_BLOCK, &سيٽ, عدم);
	إذا (بائيٽون == -1) {
		إذا (errno == EAGAIN || errno == EINTR)
			أرجع 0;
		أرجع -errno;
	}

	إذا (بائيٽون == 0) {
		*eof = 1;
		أرجع 0;
	}

	/* from stdin (user) to command */
	إذا (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", بائيٽون));

		إذا (وقت_جدول_ٻار_write(pty, buf, بائيٽون, 0))
			أرجع -errno;

	/* from command (master) to stdout */
	} وإلا إذا (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", بائيٽون));
		write_output(buf, بائيٽون);
	}

	إذا (pty->callbacks.log_وهڪرو_activity)
		rc = pty->callbacks.log_وهڪرو_activity(
					pty->callback_معلومات, fd, buf, بائيٽون);

	أرجع rc;
}

فراغ ul_pty_wait_for_ٻار(بنية ul_pty *pty)
{
	صحيح حالت;
	pid_t pid;
	صحيح options = 0;

	إذا (pty->ٻار == (pid_t) -1)
		أرجع;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (صحيح) pty->ٻار));

	إذا (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		لكل (;;) {
			pid = waitpid(pty->ٻار, &حالت, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (صحيح) pid));
			إذا (pid != (pid_t) - 1) {
				إذا (pty->callbacks.ٻار_die)
					pty->callbacks.ٻار_die(
							pty->callback_معلومات,
							pty->ٻار, حالت);
				ul_pty_سيٽ_ٻار(pty, (pid_t) -1);
			} وإلا
				اكسر;
		}
	} وإلا {
		/* final wait */
		طالما ((pid = waitpid(-1, &حالت, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (صحيح) pid));
			إذا (pid == pty->ٻار) {
				إذا (pty->callbacks.ٻار_die)
					pty->callbacks.ٻار_die(
							pty->callback_معلومات,
							pty->ٻار, حالت);
				ul_pty_سيٽ_ٻار(pty, (pid_t) -1);
			}
		}
	}
}

ساكن صحيح handle_signal(بنية ul_pty *pty, صحيح fd)
{
	بنية signalfd_siginfo info;
	ssize_t بائيٽون;
	صحيح rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	بائيٽون = اقرأ(fd, &info, حجم(info));
	إذا (بائيٽون != حجم(info)) {
		إذا (بائيٽون < 0 && (errno == EAGAIN || errno == EINTR))
			أرجع 0;
		أرجع -errno;
	}

	بدل (info.ssi_signo) {
	حالة SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		إذا (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			إذا (pty->callbacks.ٻار_wait)
				pty->callbacks.ٻار_wait(pty->callback_معلومات,
							  pty->ٻار);
			وإلا
				ul_pty_wait_for_ٻار(pty);

		} وإلا إذا (info.ssi_status == SIGSTOP && pty->ٻار > 0) {
			pty->callbacks.ٻار_sigstop(pty->callback_معلومات,
						     pty->ٻار);
		}

		إذا (pty->ٻار <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->ايندڙ_callback_وقت);
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
		إذا (pty->ٻار > 0)
			kill(pty->ٻار, SIGTERM);

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
		إذا (timerisset(&pty->ايندڙ_callback_وقت)) {
			بنية timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			إذا (timercmp(&now, &pty->ايندڙ_callback_وقت, >)) {
				rc = mainloop_callback(pty);
				إذا (rc)
					اكسر;
			}
		}

		/* set timeout */
		إذا (timerisset(&pty->ايندڙ_callback_وقت)) {
			بنية timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->ايندڙ_callback_وقت, &now, &rest);
			timeout = (rest.tv_sec * MSEC_في_SEC) +  (rest.tv_usec / USEC_في_MSEC);
		} وإلا
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		إذا (pty->ٻار_بفر_سر)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		وإلا
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_سائيز(pfd), timeout);

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
			إذا (timerisset(&pty->ايندڙ_callback_وقت)) {
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
		لكل (i = 0; i < ARRAY_سائيز(pfd); i++) {
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
					rc = flush_ٻار_buffers(pty, عدم);
			}

			إذا (rc) {
				صحيح anything = 1;

				ul_pty_write_eof_منزل_ٻار(pty);
				لكل (anything = 1; anything;)
					flush_ٻار_buffers(pty, &anything);
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
					ul_pty_write_eof_منزل_ٻار(pty);
				} وإلا /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		إذا (rc)
			اكسر;
	}

	إذا (rc && pty->ٻار && pty->ٻار != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->ٻار, SIGTERM);
		sleep(2);
		kill(pty->ٻار, SIGKILL);
	}

	pty_سگنل_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	أرجع rc;
}

#جيڪڏهن_وصف_ٿيل TEST_پروگرام_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
ساكن فراغ ٻار_sigstop(فراغ *معلومات __attribute__((__unused__)), pid_t ٻار)
{
	kill(getpid(), SIGSTOP);
	kill(ٻار, SIGCONT);
}

صحيح مکيه(صحيح argc, حرف *argv[])
{
	بنية ul_pty_callbacks *cb;
	ثابت حرف *shell, *command = عدم, *shname = عدم;
	صحيح caught_signal = 0;
	pid_t ٻار;
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
	cb->ٻار_sigstop = ٻار_sigstop;

	إذا (ul_pty_تياري(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	إذا (ul_pty_سگنل_تياري(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	إذا (ul_pty_terminal_تياري(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	بدل ((صحيح) (ٻار = fork())) {
	حالة -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		اكسر;

	حالة 0: /*ٻار*/
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

	/*والد*/
	ul_pty_سيٽ_ٻار(pty, ٻار);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	إذا (!caught_signal && ul_pty_get_ٻار(pty) != (pid_t)-1)
		ul_pty_wait_for_ٻار(pty);	/* final wait */

	إذا (caught_signal && ul_pty_get_ٻار(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(ٻار, SIGTERM);
		sleep(2);
		kill(ٻار, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	أرجع EXIT_SUCCESS;
}

#جيڪڏهن_پورو /* TEST_PROGRAM */

