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

ساكن UL_DEBUG_DEFINE_MASK(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPTY_DEBUG_INIT	(1 << 1)
#define ULPTY_DEBUG_SETUP	(1 << 2)
#define ULPTY_DEBUG_SIG		(1 << 3)
#define ULPTY_DEBUG_IO		(1 << 4)
#define ULPTY_DEBUG_DONE	(1 << 5)

#define DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

فراغ ul_pty_init_debug(صحيح mask)
{
	אם (ulpty_debug_mask)
		החזר;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

بنية ul_pty *ul_new_pty(صحيح is_stdin_tty)
{
	بنية ul_pty *pty = calloc(1, حجم(*pty));

	אם (!pty)
		החזר عدم;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	החזר pty;
}

فراغ ul_free_pty(بنية ul_pty *pty)
{
	بنية ul_pty_child_buffer *hd;

	כלעוד ((hd = pty->child_buffer_head)) {
		pty->child_buffer_head = hd->next;
		حرر(hd);
	}

	כלעוד ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->next;
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
	החזר pty->delivered_signal;
}

بنية ul_pty_callbacks *ul_pty_get_callbacks(بنية ul_pty *pty)
{
	أكد(pty);
	החזר &pty->callbacks;
}

فراغ ul_pty_set_callback_data(بنية ul_pty *pty, فراغ *data)
{
	أكد(pty);
	pty->callback_data = data;
}

فراغ ul_pty_set_child(بنية ul_pty *pty, pid_t child)
{
	أكد(pty);
	pty->child = child;
}

صحيح ul_pty_get_childfd(بنية ul_pty *pty)
{
	أكد(pty);
	החזר pty->master;
}

pid_t ul_pty_get_child(بنية ul_pty *pty)
{
	أكد(pty);
	החזר pty->child;
}

/* it's active when signals are redirected to sigfd */
صحيح ul_pty_is_running(بنية ul_pty *pty)
{
	أكد(pty);
	החזר pty->sigfd >= 0;
}

فراغ ul_pty_set_mainloop_time(بنية ul_pty *pty, بنية timeval *tv)
{
	أكد(pty);
	אם (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->next_callback_time);
	} אחרת {
		pty->next_callback_time.tv_sec = tv->tv_sec;
		pty->next_callback_time.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

ساكن فراغ pty_signals_cleanup(بنية ul_pty *pty)
{
	אם (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, عدم);
}

/* call me before fork() */
صحيح ul_pty_setup(بنية ul_pty *pty)
{
	بنية termios attrs;
	صحيح rc = 0;

	أكد(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, عدم, &pty->orgsig);

	אם (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		אם (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			اذهب done;
		}

		attrs = pty->stdin_attrs;
		אם (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		אחרת
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (حرف *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, عدم, &attrs, &pty->win);
		אם (rc)
			اذهب done;

	} אחרת {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, عدم, عدم, عدم);
		אם (rc)
			اذهب done;

		tcgetattr(pty->slave, &attrs);

		אם (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		אחרת
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

done:
	אם (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	החזר rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
صحيح ul_pty_terminal_setup(بنية ul_pty *pty)
{
	بنية termios attrs;

	אם (!pty->isterm)
		החזר 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	אם (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		החזר -errno;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	החזר 0;
}

/* call me before fork() */
صحيح ul_pty_signals_setup(بنية ul_pty *pty)
{
	sigset_t ourset;
	صحيح rc = 0;

	أكد(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, عدم, &pty->orgsig);

	sigfillset(&ourset);
	אם (sigprocmask(SIG_BLOCK, &ourset, عدم)) {
		rc = -errno;
		اذهب done;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, SIGTERM);
	sigaddset(&ourset, SIGINT);
	sigaddset(&ourset, SIGQUIT);

	אם (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	אם ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
done:
	אם (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	החזר rc;
}

/* cleanup in parent process */
فراغ ul_pty_cleanup(بنية ul_pty *pty)
{
	بنية termios rtt;

	pty_signals_cleanup(pty);

	אם (pty->master == -1 || !pty->isterm)
		החזר;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

صحيح ul_pty_chownmod_slave(بنية ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	אם (fchown(pty->slave, uid, gid))
		החזר -errno;
	אם (fchmod(pty->slave, mode))
		החזר -errno;
	החזר 0;
}

/* call me in child process */
فراغ ul_pty_init_slave(بنية ul_pty *pty)
{
	DBG_OBJ(SETUP, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	אם (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, عدم);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

ساكن صحيح write_output(حرف *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	אם (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		החזר -errno;
	}

	החזר 0;
}

ساكن صحيح schedule_child_write(بنية ul_pty *pty, حرف *buf, size_t bufsz, صحيح final)
{
	بنية ul_pty_child_buffer *stash;

	אם (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->next;
		memset(stash, 0, حجم(*stash));
	} אחרת
		stash = calloc(1, حجم(*stash));
	אם (!stash)
		החזר -1;

	أكد(bufsz <= حجم(stash->buf));

	memcpy(stash->buf, buf, bufsz);
	stash->size = bufsz;
	stash->final_input = final ? 1 : 0;

	אם (pty->child_buffer_head)
		pty->child_buffer_tail = pty->child_buffer_tail->next = stash;
	אחרת
		pty->child_buffer_head = pty->child_buffer_tail = stash;
	החזר 0;
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
ساكن فراغ drain_child_buffers(بنية ul_pty *pty)
{
	unsigned صحيح tries = 0;
	بنية pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	כלעוד (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	אם (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

ساكن صحيح flush_child_buffers(بنية ul_pty *pty, صحيح *anything)
{
	صحيح rc = 0, any = 0;

	כלעוד (pty->child_buffer_head) {
		بنية ul_pty_child_buffer *hd = pty->child_buffer_head;
		ssize_t ret;

		אם (hd->final_input)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->size - hd->cursor));

		ret = اكتب(pty->master, hd->buf + hd->cursor, hd->size - hd->cursor);
		אם (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			אם (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			اذهب out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		אם (hd->cursor == hd->size) {
			pty->child_buffer_head = hd->next;
			אם (!hd->next)
				pty->child_buffer_tail = عدم;

			hd->next = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
out:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	אם (any)
		fdatasync(pty->master);

	אם (anything)
		*anything = any;
	החזר rc;
}

فراغ ul_pty_write_eof_to_child(بنية ul_pty *pty)
{
	حرف c = DEF_EOF;
	schedule_child_write(pty, &c, حجم(حرف), 1);
}

ساكن صحيح mainloop_callback(بنية ul_pty *pty)
{
	صحيح rc;

	אם (!pty->callbacks.mainloop)
		החזר 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_data);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	החזר rc;
}

ساكن صحيح handle_io(بنية ul_pty *pty, صحيح fd, صحيح *eof)
{
	حرف buf[BUFSIZ];
	ssize_t bytes;
	صحيح rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, عدم);
	/* read from active FD */
	bytes = اقرأ(fd, buf, حجم(buf));
	sigprocmask(SIG_BLOCK, &set, عدم);
	אם (bytes == -1) {
		אם (errno == EAGAIN || errno == EINTR)
			החזר 0;
		החזר -errno;
	}

	אם (bytes == 0) {
		*eof = 1;
		החזר 0;
	}

	/* from stdin (user) to command */
	אם (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		אם (schedule_child_write(pty, buf, bytes, 0))
			החזר -errno;

	/* from command (master) to stdout */
	} אחרת אם (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	אם (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_data, fd, buf, bytes);

	החזר rc;
}

فراغ ul_pty_wait_for_child(بنية ul_pty *pty)
{
	صحيح status;
	pid_t pid;
	صحيح options = 0;

	אם (pty->child == (pid_t) -1)
		החזר;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (صحيح) pty->child));

	אם (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		עבור (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (صحيح) pid));
			אם (pid != (pid_t) - 1) {
				אם (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} אחרת
				اكسر;
		}
	} אחרת {
		/* final wait */
		כלעוד ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (صحيح) pid));
			אם (pid == pty->child) {
				אם (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

ساكن صحيح handle_signal(بنية ul_pty *pty, صحيح fd)
{
	بنية signalfd_siginfo info;
	ssize_t bytes;
	صحيح rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = اقرأ(fd, &info, حجم(info));
	אם (bytes != حجم(info)) {
		אם (bytes < 0 && (errno == EAGAIN || errno == EINTR))
			החזר 0;
		החזר -errno;
	}

	بدل (info.ssi_signo) {
	حالة SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		אם (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			אם (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_data,
							  pty->child);
			אחרת
				ul_pty_wait_for_child(pty);

		} אחרת אם (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_data,
						     pty->child);
		}

		אם (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->next_callback_time);
		}
		החזר 0;
	حالة SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		אם (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (حرف *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (حرف *)&pty->win);

			אם (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_data,
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
		אם (pty->child > 0)
			kill(pty->child, SIGTERM);

		אם (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_data,
					&info, (فراغ *) &pty->win);
		اكسر;
	حالة SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		אם (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_data);
		اكسر;
	افتراضي:
		abort();
	}

	החזר rc;
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

	כלעוד (!pty->delivered_signal) {
		size_t i;
		صحيح errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		אם (timerisset(&pty->next_callback_time)) {
			بنية timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			אם (timercmp(&now, &pty->next_callback_time, >)) {
				rc = mainloop_callback(pty);
				אם (rc)
					اكسر;
			}
		}

		/* set timeout */
		אם (timerisset(&pty->next_callback_time)) {
			بنية timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->next_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} אחרת
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		אם (pty->child_buffer_head)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		אחרת
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_SIZE(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		אם (ret < 0) {
			אם (errsv == EAGAIN)
				استمر;
			rc = -errno;
			اكسر;
		}

		/* timeout */
		אם (ret == 0) {
			אם (timerisset(&pty->next_callback_time)) {
				rc = mainloop_callback(pty);
				אם (rc == 0)
					استمر;
			} אחרת {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			اكسر;
		}
		/* event */
		עבור (i = 0; i < ARRAY_SIZE(pfd); i++) {
			אם (pfd[i].revents == 0)
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

			אם (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			אחרת {
				אם (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /* data */
				אם (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, عدم);
			}

			אם (rc) {
				صحيح anything = 1;

				ul_pty_write_eof_to_child(pty);
				עבור (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				اكسر;
			}

			אם (i == POLLFD_SIGNAL)
				استمر;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			אם ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				אם (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_to_child(pty);
				} אחרת /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		אם (rc)
			اكسر;
	}

	אם (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->child, SIGTERM);
		sleep(2);
		kill(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	החזר rc;
}

#ifdef TEST_PROGRAM_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
ساكن فراغ child_sigstop(فراغ *data __attribute__((__unused__)), pid_t child)
{
	kill(getpid(), SIGSTOP);
	kill(child, SIGCONT);
}

صحيح main(صحيح argc, حرف *argv[])
{
	بنية ul_pty_callbacks *cb;
	ثابت حرف *shell, *command = عدم, *shname = عدم;
	صحيح caught_signal = 0;
	pid_t child;
	بنية ul_pty *pty;

	shell = getenv("SHELL");
	אם (shell == عدم)
		shell = _PATH_BSHELL;
	אם (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	אם (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->child_sigstop = child_sigstop;

	אם (ul_pty_setup(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	אם (ul_pty_signals_setup(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	אם (ul_pty_terminal_setup(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	بدل ((صحيح) (child = fork())) {
	حالة -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		اكسر;

	حالة 0: /* child */
		ul_pty_init_slave(pty);

		إشارة(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		אם (command)
			execl(shell, shname, "-c", command, (حرف *)عدم);
		אחרת
			execl(shell, shname, "-i", (حرف *)عدم);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		اكسر;

	افتراضي:
		اكسر;
	}

	/* parent */
	ul_pty_set_child(pty, child);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	אם (!caught_signal && ul_pty_get_child(pty) != (pid_t)-1)
		ul_pty_wait_for_child(pty);	/* final wait */

	אם (caught_signal && ul_pty_get_child(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(child, SIGTERM);
		sleep(2);
		kill(child, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	החזר EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM */

