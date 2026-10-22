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
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <pty.h>
#شامل <poll.h>
#شامل <sys/signalfd.h>
#شامل <paths.h>
#شامل <sys/types.h>
#شامل <sys/wait.h>
#شامل <inttypes.h>

#شامل "c.h"
#شامل "all-io.h"
#شامل "ttyutils.h"
#شامل "pty-session.h"
#شامل "monotonic.h"
#شامل "debug.h"

ساکن UL_DEBUG_DEFINE_نقاب(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_خالی_خانے_MASKNAMES;

#تعریف_کرو ULPTY_DEBUG_INIT	(1 << 1)
#تعریف_کرو ULPTY_DEBUG_تیاری	(1 << 2)
#تعریف_کرو ULPTY_DEBUG_SIG		(1 << 3)
#تعریف_کرو ULPTY_DEBUG_IO		(1 << 4)
#تعریف_کرو ULPTY_DEBUG_مکمل	(1 << 5)

#تعریف_کرو DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#تعریف_کرو DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#تعریف_کرو ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

خالی ul_pty_init_debug(صحیح نقاب)
{
	اگر (ulpty_debug_نقاب)
		واپس;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, نقاب, ULPTY_DEBUG);
}

ساخت ul_pty *ul_new_pty(صحیح is_stdin_tty)
{
	ساخت ul_pty *pty = صفر_حافظہ_دو(1, حجم(*pty));

	اگر (!pty)
		واپس تہی;

	DBG_OBJ(تیاری, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->بچہ = (pid_t) -1;

	واپس pty;
}

خالی ul_free_pty(ساخت ul_pty *pty)
{
	ساخت ul_pty_بچہ_بفر *hd;

	جب ((hd = pty->بچہ_بفر_سر)) {
		pty->بچہ_بفر_سر = hd->اگلا;
		آزاد(hd);
	}

	جب ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->اگلا;
		آزاد(hd);
	}
	آزاد(pty);
}

خالی ul_pty_slave_echo(ساخت ul_pty *pty, صحیح enable)
{
	أكد(pty);
	pty->slave_echo = enable ? 1 : 0;
}

صحیح ul_pty_get_delivered_signal(ساخت ul_pty *pty)
{
	أكد(pty);
	واپس pty->delivered_signal;
}

ساخت ul_pty_callbacks *ul_pty_get_callbacks(ساخت ul_pty *pty)
{
	أكد(pty);
	واپس &pty->callbacks;
}

خالی ul_pty_سیٹ_callback_ڈیٹا(ساخت ul_pty *pty, خالی *ڈیٹا)
{
	أكد(pty);
	pty->callback_ڈیٹا = ڈیٹا;
}

خالی ul_pty_سیٹ_بچہ(ساخت ul_pty *pty, pid_t بچہ)
{
	أكد(pty);
	pty->بچہ = بچہ;
}

صحیح ul_pty_get_childfd(ساخت ul_pty *pty)
{
	أكد(pty);
	واپس pty->master;
}

pid_t ul_pty_get_بچہ(ساخت ul_pty *pty)
{
	أكد(pty);
	واپس pty->بچہ;
}

/* it's active when signals are redirected to sigfd */
صحیح ul_pty_is_running(ساخت ul_pty *pty)
{
	أكد(pty);
	واپس pty->sigfd >= 0;
}

خالی ul_pty_سیٹ_mainloop_وقت(ساخت ul_pty *pty, ساخت timeval *tv)
{
	أكد(pty);
	اگر (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->اگلا_callback_وقت);
	} ورنہ {
		pty->اگلا_callback_وقت.tv_sec = tv->tv_sec;
		pty->اگلا_callback_وقت.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

ساکن خالی pty_اشارات_cleanup(ساخت ul_pty *pty)
{
	اگر (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, تہی);
}

/* call me before fork() */
صحیح ul_pty_تیاری(ساخت ul_pty *pty)
{
	ساخت termios attrs;
	صحیح rc = 0;

	أكد(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, تہی, &pty->orgsig);

	اگر (pty->isterm) {
		DBG_OBJ(تیاری, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		اگر (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			جاؤ مکمل;
		}

		attrs = pty->stdin_attrs;
		اگر (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		ورنہ
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (حرف *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, تہی, &attrs, &pty->win);
		اگر (rc)
			جاؤ مکمل;

	} ورنہ {
		DBG_OBJ(تیاری, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, تہی, تہی, تہی);
		اگر (rc)
			جاؤ مکمل;

		tcgetattr(pty->slave, &attrs);

		اگر (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		ورنہ
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

مکمل:
	اگر (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(تیاری, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	واپس rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
صحیح ul_pty_terminal_تیاری(ساخت ul_pty *pty)
{
	ساخت termios attrs;

	اگر (!pty->isterm)
		واپس 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	اگر (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		واپس -errno;

	DBG_OBJ(تیاری, pty, ul_debug("terminal raw mode setup done"));
	واپس 0;
}

/* call me before fork() */
صحیح ul_pty_اشارات_تیاری(ساخت ul_pty *pty)
{
	sigset_t ourset;
	صحیح rc = 0;

	أكد(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, تہی, &pty->orgsig);

	sigfillset(&ourset);
	اگر (sigprocmask(SIG_BLOCK, &ourset, تہی)) {
		rc = -errno;
		جاؤ مکمل;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, SIGTERM);
	sigaddset(&ourset, SIGINT);
	sigaddset(&ourset, SIGQUIT);

	اگر (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	اگر ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
مکمل:
	اگر (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(تیاری, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	واپس rc;
}

/* cleanup in parent process */
خالی ul_pty_cleanup(ساخت ul_pty *pty)
{
	ساخت termios rtt;

	pty_اشارات_cleanup(pty);

	اگر (pty->master == -1 || !pty->isterm)
		واپس;

	DBG_OBJ(مکمل_2, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

صحیح ul_pty_chownmod_slave(ساخت ul_pty *pty, uid_t uid, gid_t gid, mode_t منوال)
{
	اگر (fchown(pty->slave, uid, gid))
		واپس -errno;
	اگر (fchmod(pty->slave, منوال))
		واپس -errno;
	واپس 0;
}

/* call me in child process */
خالی ul_pty_init_slave(ساخت ul_pty *pty)
{
	DBG_OBJ(تیاری, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	اگر (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, تہی);

	DBG_OBJ(تیاری, pty, ul_debug("... initialize slave done"));
}

ساکن صحیح write_output(حرف *obuf, ssize_t بائٹیں)
{
	DBG(IO, ul_debug(" writing output"));

	اگر (ul_write_all(STDOUT_FILENO, obuf, بائٹیں)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		واپس -errno;
	}

	واپس 0;
}

ساکن صحیح جدول_وقت_بچہ_write(ساخت ul_pty *pty, حرف *buf, size_t bufsz, صحیح final)
{
	ساخت ul_pty_بچہ_بفر *stash;

	اگر (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->اگلا;
		memset(stash, 0, حجم(*stash));
	} ورنہ
		stash = صفر_حافظہ_دو(1, حجم(*stash));
	اگر (!stash)
		واپس -1;

	أكد(bufsz <= حجم(stash->buf));

	حافظہ_نقل(stash->buf, buf, bufsz);
	stash->سائز = bufsz;
	stash->final_داخلہ = final ? 1 : 0;

	اگر (pty->بچہ_بفر_سر)
		pty->بچہ_بفر_دم = pty->بچہ_بفر_دم->اگلا = stash;
	ورنہ
		pty->بچہ_بفر_سر = pty->بچہ_بفر_دم = stash;
	واپس 0;
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
ساکن خالی drain_بچہ_buffers(ساخت ul_pty *pty)
{
	غیرعلامتی صحیح tries = 0;
	ساخت pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	جب (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	اگر (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

ساکن صحیح flush_بچہ_buffers(ساخت ul_pty *pty, صحیح *anything)
{
	صحیح rc = 0, any = 0;

	جب (pty->بچہ_بفر_سر) {
		ساخت ul_pty_بچہ_بفر *hd = pty->بچہ_بفر_سر;
		ssize_t ret;

		اگر (hd->final_داخلہ)
			drain_بچہ_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->سائز - hd->cursor));

		ret = اكتب(pty->master, hd->buf + hd->cursor, hd->سائز - hd->cursor);
		اگر (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			اگر (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			جاؤ برآمد;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		اگر (hd->cursor == hd->سائز) {
			pty->بچہ_بفر_سر = hd->اگلا;
			اگر (!hd->اگلا)
				pty->بچہ_بفر_دم = تہی;

			hd->اگلا = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
برآمد:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	اگر (any)
		fdatasync(pty->master);

	اگر (anything)
		*anything = any;
	واپس rc;
}

خالی ul_pty_write_eof_مقصد_بچہ(ساخت ul_pty *pty)
{
	حرف c = DEF_EOF;
	جدول_وقت_بچہ_write(pty, &c, حجم(حرف), 1);
}

ساکن صحیح mainloop_callback(ساخت ul_pty *pty)
{
	صحیح rc;

	اگر (!pty->callbacks.mainloop)
		واپس 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_ڈیٹا);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	واپس rc;
}

ساکن صحیح handle_io(ساخت ul_pty *pty, صحیح fd, صحیح *eof)
{
	حرف buf[BUFSIZ];
	ssize_t بائٹیں;
	صحیح rc = 0;
	sigset_t سیٹ;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&سیٹ);
	sigaddset(&سیٹ, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &سیٹ, تہی);
	/* read from active FD */
	بائٹیں = اقرأ(fd, buf, حجم(buf));
	sigprocmask(SIG_BLOCK, &سیٹ, تہی);
	اگر (بائٹیں == -1) {
		اگر (errno == EAGAIN || errno == EINTR)
			واپس 0;
		واپس -errno;
	}

	اگر (بائٹیں == 0) {
		*eof = 1;
		واپس 0;
	}

	/* from stdin (user) to command */
	اگر (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", بائٹیں));

		اگر (جدول_وقت_بچہ_write(pty, buf, بائٹیں, 0))
			واپس -errno;

	/* from command (master) to stdout */
	} ورنہ اگر (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", بائٹیں));
		write_output(buf, بائٹیں);
	}

	اگر (pty->callbacks.log_دھارا_activity)
		rc = pty->callbacks.log_دھارا_activity(
					pty->callback_ڈیٹا, fd, buf, بائٹیں);

	واپس rc;
}

خالی ul_pty_wait_for_بچہ(ساخت ul_pty *pty)
{
	صحیح حالت_2;
	pid_t pid;
	صحیح options = 0;

	اگر (pty->بچہ == (pid_t) -1)
		واپس;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (صحیح) pty->بچہ));

	اگر (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		برائے (;;) {
			pid = waitpid(pty->بچہ, &حالت_2, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (صحیح) pid));
			اگر (pid != (pid_t) - 1) {
				اگر (pty->callbacks.بچہ_die)
					pty->callbacks.بچہ_die(
							pty->callback_ڈیٹا,
							pty->بچہ, حالت_2);
				ul_pty_سیٹ_بچہ(pty, (pid_t) -1);
			} ورنہ
				توڑو;
		}
	} ورنہ {
		/* final wait */
		جب ((pid = waitpid(-1, &حالت_2, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (صحیح) pid));
			اگر (pid == pty->بچہ) {
				اگر (pty->callbacks.بچہ_die)
					pty->callbacks.بچہ_die(
							pty->callback_ڈیٹا,
							pty->بچہ, حالت_2);
				ul_pty_سیٹ_بچہ(pty, (pid_t) -1);
			}
		}
	}
}

ساکن صحیح handle_signal(ساخت ul_pty *pty, صحیح fd)
{
	ساخت signalfd_siginfo info;
	ssize_t بائٹیں;
	صحیح rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	بائٹیں = اقرأ(fd, &info, حجم(info));
	اگر (بائٹیں != حجم(info)) {
		اگر (بائٹیں < 0 && (errno == EAGAIN || errno == EINTR))
			واپس 0;
		واپس -errno;
	}

	انتخاب (info.ssi_signo) {
	حالت SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		اگر (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			اگر (pty->callbacks.بچہ_wait)
				pty->callbacks.بچہ_wait(pty->callback_ڈیٹا,
							  pty->بچہ);
			ورنہ
				ul_pty_wait_for_بچہ(pty);

		} ورنہ اگر (info.ssi_status == SIGSTOP && pty->بچہ > 0) {
			pty->callbacks.بچہ_sigstop(pty->callback_ڈیٹا,
						     pty->بچہ);
		}

		اگر (pty->بچہ <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->اگلا_callback_وقت);
		}
		واپس 0;
	حالت SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		اگر (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (حرف *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (حرف *)&pty->win);

			اگر (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_ڈیٹا,
							&info, (خالی *) &pty->win);
		}
		توڑو;
	حالت SIGALRM:
		FALLTHROUGH;
	حالت SIGHUP:
		FALLTHROUGH;
	حالت SIGTERM:
		FALLTHROUGH;
	حالت SIGINT:
		FALLTHROUGH;
	حالت SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		اگر (pty->بچہ > 0)
			kill(pty->بچہ, SIGTERM);

		اگر (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_ڈیٹا,
					&info, (خالی *) &pty->win);
		توڑو;
	حالت SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		اگر (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_ڈیٹا);
		توڑو;
	طےشدہ:
		abort();
	}

	واپس rc;
}

/* loop in parent */
صحیح ul_pty_proxy_master(ساخت ul_pty *pty)
{
	صحیح rc = 0, ret, eof = 0;
	شمار {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	ساخت pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	أكد(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	جب (!pty->delivered_signal) {
		size_t i;
		صحیح errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		اگر (timerisset(&pty->اگلا_callback_وقت)) {
			ساخت timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			اگر (timercmp(&now, &pty->اگلا_callback_وقت, >)) {
				rc = mainloop_callback(pty);
				اگر (rc)
					توڑو;
			}
		}

		/* set timeout */
		اگر (timerisset(&pty->اگلا_callback_وقت)) {
			ساخت timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->اگلا_callback_وقت, &now, &rest);
			timeout = (rest.tv_sec * MSEC_فی_SEC) +  (rest.tv_usec / USEC_فی_MSEC);
		} ورنہ
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		اگر (pty->بچہ_بفر_سر)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		ورنہ
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_سائز(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		اگر (ret < 0) {
			اگر (errsv == EAGAIN)
				جاری;
			rc = -errno;
			توڑو;
		}

		/* timeout */
		اگر (ret == 0) {
			اگر (timerisset(&pty->اگلا_callback_وقت)) {
				rc = mainloop_callback(pty);
				اگر (rc == 0)
					جاری;
			} ورنہ {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			توڑو;
		}
		/* event */
		برائے (i = 0; i < ARRAY_سائز(pfd); i++) {
			اگر (pfd[i].revents == 0)
				جاری;

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

			اگر (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			ورنہ {
				اگر (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /*ڈیٹا*/
				اگر (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_بچہ_buffers(pty, تہی);
			}

			اگر (rc) {
				صحیح anything = 1;

				ul_pty_write_eof_مقصد_بچہ(pty);
				برائے (anything = 1; anything;)
					flush_بچہ_buffers(pty, &anything);
				توڑو;
			}

			اگر (i == POLLFD_SIGNAL)
				جاری;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			اگر ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				اگر (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_مقصد_بچہ(pty);
				} ورنہ /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		اگر (rc)
			توڑو;
	}

	اگر (rc && pty->بچہ && pty->بچہ != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->بچہ, SIGTERM);
		sleep(2);
		kill(pty->بچہ, SIGKILL);
	}

	pty_اشارات_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	واپس rc;
}

#اگر_تعریف_شدہ TEST_پروگرام_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
ساکن خالی بچہ_sigstop(خالی *ڈیٹا __attribute__((__unused__)), pid_t بچہ)
{
	kill(getpid(), SIGSTOP);
	kill(بچہ, SIGCONT);
}

صحیح مین(صحیح argc, حرف *argv[])
{
	ساخت ul_pty_callbacks *cb;
	ثابت حرف *shell, *command = تہی, *shname = تہی;
	صحیح caught_signal = 0;
	pid_t بچہ;
	ساخت ul_pty *pty;

	shell = getenv("SHELL");
	اگر (shell == تہی)
		shell = _PATH_BSHELL;
	اگر (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	اگر (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->بچہ_sigstop = بچہ_sigstop;

	اگر (ul_pty_تیاری(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	اگر (ul_pty_اشارات_تیاری(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	اگر (ul_pty_terminal_تیاری(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	انتخاب ((صحیح) (بچہ = fork())) {
	حالت -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		توڑو;

	حالت 0: /*بچہ*/
		ul_pty_init_slave(pty);

		اشارہ(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		اگر (command)
			execl(shell, shname, "-c", command, (حرف *)تہی);
		ورنہ
			execl(shell, shname, "-i", (حرف *)تہی);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		توڑو;

	طےشدہ:
		توڑو;
	}

	/*والد*/
	ul_pty_سیٹ_بچہ(pty, بچہ);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	اگر (!caught_signal && ul_pty_get_بچہ(pty) != (pid_t)-1)
		ul_pty_wait_for_بچہ(pty);	/* final wait */

	اگر (caught_signal && ul_pty_get_بچہ(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(بچہ, SIGTERM);
		sleep(2);
		kill(بچہ, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	واپس EXIT_SUCCESS;
}

#اگر_ختم /* TEST_PROGRAM */

