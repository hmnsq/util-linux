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

ایستا UL_DEBUG_DEFINE_MASK(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPTY_DEBUG_INIT	(1 << 1)
#define ULPTY_DEBUG_SETUP	(1 << 2)
#define ULPTY_DEBUG_SIG		(1 << 3)
#define ULPTY_DEBUG_IO		(1 << 4)
#define ULPTY_DEBUG_DONE	(1 << 5)

#define DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

تهی ul_pty_init_debug(صحیح mask)
{
	اگر (ulpty_debug_mask)
		بازگشت;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

ساختار ul_pty *ul_new_pty(صحیح is_stdin_tty)
{
	ساختار ul_pty *pty = calloc(1, اندازه(*pty));

	اگر (!pty)
		بازگشت پوچ;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	بازگشت pty;
}

تهی ul_free_pty(ساختار ul_pty *pty)
{
	ساختار ul_pty_child_buffer *hd;

	وقتی ((hd = pty->child_buffer_head)) {
		pty->child_buffer_head = hd->next;
		آزاد(hd);
	}

	وقتی ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->next;
		آزاد(hd);
	}
	آزاد(pty);
}

تهی ul_pty_slave_echo(ساختار ul_pty *pty, صحیح enable)
{
	أكد(pty);
	pty->slave_echo = enable ? 1 : 0;
}

صحیح ul_pty_get_delivered_signal(ساختار ul_pty *pty)
{
	أكد(pty);
	بازگشت pty->delivered_signal;
}

ساختار ul_pty_callbacks *ul_pty_get_callbacks(ساختار ul_pty *pty)
{
	أكد(pty);
	بازگشت &pty->callbacks;
}

تهی ul_pty_set_callback_data(ساختار ul_pty *pty, تهی *data)
{
	أكد(pty);
	pty->callback_data = data;
}

تهی ul_pty_set_child(ساختار ul_pty *pty, pid_t child)
{
	أكد(pty);
	pty->child = child;
}

صحیح ul_pty_get_childfd(ساختار ul_pty *pty)
{
	أكد(pty);
	بازگشت pty->master;
}

pid_t ul_pty_get_child(ساختار ul_pty *pty)
{
	أكد(pty);
	بازگشت pty->child;
}

/* it's active when signals are redirected to sigfd */
صحیح ul_pty_is_running(ساختار ul_pty *pty)
{
	أكد(pty);
	بازگشت pty->sigfd >= 0;
}

تهی ul_pty_set_mainloop_time(ساختار ul_pty *pty, ساختار timeval *tv)
{
	أكد(pty);
	اگر (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->next_callback_time);
	} وگرنه {
		pty->next_callback_time.tv_sec = tv->tv_sec;
		pty->next_callback_time.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

ایستا تهی pty_signals_cleanup(ساختار ul_pty *pty)
{
	اگر (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, پوچ);
}

/* call me before fork() */
صحیح ul_pty_setup(ساختار ul_pty *pty)
{
	ساختار termios attrs;
	صحیح rc = 0;

	أكد(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, پوچ, &pty->orgsig);

	اگر (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		اگر (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			برو done;
		}

		attrs = pty->stdin_attrs;
		اگر (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		وگرنه
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (نویسه *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, پوچ, &attrs, &pty->win);
		اگر (rc)
			برو done;

	} وگرنه {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, پوچ, پوچ, پوچ);
		اگر (rc)
			برو done;

		tcgetattr(pty->slave, &attrs);

		اگر (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		وگرنه
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

done:
	اگر (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	بازگشت rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
صحیح ul_pty_terminal_setup(ساختار ul_pty *pty)
{
	ساختار termios attrs;

	اگر (!pty->isterm)
		بازگشت 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	اگر (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		بازگشت -errno;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	بازگشت 0;
}

/* call me before fork() */
صحیح ul_pty_signals_setup(ساختار ul_pty *pty)
{
	sigset_t ourset;
	صحیح rc = 0;

	أكد(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, پوچ, &pty->orgsig);

	sigfillset(&ourset);
	اگر (sigprocmask(SIG_BLOCK, &ourset, پوچ)) {
		rc = -errno;
		برو done;
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
done:
	اگر (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	بازگشت rc;
}

/* cleanup in parent process */
تهی ul_pty_cleanup(ساختار ul_pty *pty)
{
	ساختار termios rtt;

	pty_signals_cleanup(pty);

	اگر (pty->master == -1 || !pty->isterm)
		بازگشت;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

صحیح ul_pty_chownmod_slave(ساختار ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	اگر (fchown(pty->slave, uid, gid))
		بازگشت -errno;
	اگر (fchmod(pty->slave, mode))
		بازگشت -errno;
	بازگشت 0;
}

/* call me in child process */
تهی ul_pty_init_slave(ساختار ul_pty *pty)
{
	DBG_OBJ(SETUP, pty, ul_debug("initialize slave"));

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

	sigprocmask(SIG_SETMASK, &pty->orgsig, پوچ);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

ایستا صحیح write_output(نویسه *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	اگر (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		بازگشت -errno;
	}

	بازگشت 0;
}

ایستا صحیح schedule_child_write(ساختار ul_pty *pty, نویسه *buf, size_t bufsz, صحیح final)
{
	ساختار ul_pty_child_buffer *stash;

	اگر (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->next;
		memset(stash, 0, اندازه(*stash));
	} وگرنه
		stash = calloc(1, اندازه(*stash));
	اگر (!stash)
		بازگشت -1;

	أكد(bufsz <= اندازه(stash->buf));

	memcpy(stash->buf, buf, bufsz);
	stash->size = bufsz;
	stash->final_درون_داد = final ? 1 : 0;

	اگر (pty->child_buffer_head)
		pty->child_buffer_tail = pty->child_buffer_tail->next = stash;
	وگرنه
		pty->child_buffer_head = pty->child_buffer_tail = stash;
	بازگشت 0;
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
ایستا تهی drain_child_buffers(ساختار ul_pty *pty)
{
	unsigned صحیح tries = 0;
	ساختار pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	وقتی (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	اگر (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

ایستا صحیح flush_child_buffers(ساختار ul_pty *pty, صحیح *anything)
{
	صحیح rc = 0, any = 0;

	وقتی (pty->child_buffer_head) {
		ساختار ul_pty_child_buffer *hd = pty->child_buffer_head;
		ssize_t ret;

		اگر (hd->final_درون_داد)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->size - hd->cursor));

		ret = اكتب(pty->master, hd->buf + hd->cursor, hd->size - hd->cursor);
		اگر (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			اگر (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			برو out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		اگر (hd->cursor == hd->size) {
			pty->child_buffer_head = hd->next;
			اگر (!hd->next)
				pty->child_buffer_tail = پوچ;

			hd->next = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
out:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	اگر (any)
		fdatasync(pty->master);

	اگر (anything)
		*anything = any;
	بازگشت rc;
}

تهی ul_pty_write_eof_to_child(ساختار ul_pty *pty)
{
	نویسه c = DEF_EOF;
	schedule_child_write(pty, &c, اندازه(نویسه), 1);
}

ایستا صحیح mainloop_callback(ساختار ul_pty *pty)
{
	صحیح rc;

	اگر (!pty->callbacks.mainloop)
		بازگشت 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_data);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	بازگشت rc;
}

ایستا صحیح handle_io(ساختار ul_pty *pty, صحیح fd, صحیح *eof)
{
	نویسه buf[BUFSIZ];
	ssize_t bytes;
	صحیح rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, پوچ);
	/* read from active FD */
	bytes = اقرأ(fd, buf, اندازه(buf));
	sigprocmask(SIG_BLOCK, &set, پوچ);
	اگر (bytes == -1) {
		اگر (errno == EAGAIN || errno == EINTR)
			بازگشت 0;
		بازگشت -errno;
	}

	اگر (bytes == 0) {
		*eof = 1;
		بازگشت 0;
	}

	/* from stdin (user) to command */
	اگر (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		اگر (schedule_child_write(pty, buf, bytes, 0))
			بازگشت -errno;

	/* from command (master) to stdout */
	} وگرنه اگر (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	اگر (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_data, fd, buf, bytes);

	بازگشت rc;
}

تهی ul_pty_wait_for_child(ساختار ul_pty *pty)
{
	صحیح status;
	pid_t pid;
	صحیح options = 0;

	اگر (pty->child == (pid_t) -1)
		بازگشت;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (صحیح) pty->child));

	اگر (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		برای (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (صحیح) pid));
			اگر (pid != (pid_t) - 1) {
				اگر (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} وگرنه
				بشکن;
		}
	} وگرنه {
		/* final wait */
		وقتی ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (صحیح) pid));
			اگر (pid == pty->child) {
				اگر (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

ایستا صحیح handle_signal(ساختار ul_pty *pty, صحیح fd)
{
	ساختار signalfd_siginfo info;
	ssize_t bytes;
	صحیح rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = اقرأ(fd, &info, اندازه(info));
	اگر (bytes != اندازه(info)) {
		اگر (bytes < 0 && (errno == EAGAIN || errno == EINTR))
			بازگشت 0;
		بازگشت -errno;
	}

	انتخاب (info.ssi_signo) {
	حالت SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		اگر (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			اگر (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_data,
							  pty->child);
			وگرنه
				ul_pty_wait_for_child(pty);

		} وگرنه اگر (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_data,
						     pty->child);
		}

		اگر (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->next_callback_time);
		}
		بازگشت 0;
	حالت SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		اگر (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (نویسه *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (نویسه *)&pty->win);

			اگر (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_data,
							&info, (تهی *) &pty->win);
		}
		بشکن;
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
		اگر (pty->child > 0)
			kill(pty->child, SIGTERM);

		اگر (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_data,
					&info, (تهی *) &pty->win);
		بشکن;
	حالت SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		اگر (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_data);
		بشکن;
	پیشفرض:
		abort();
	}

	بازگشت rc;
}

/* loop in parent */
صحیح ul_pty_proxy_master(ساختار ul_pty *pty)
{
	صحیح rc = 0, ret, eof = 0;
	شمارش {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	ساختار pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	أكد(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	وقتی (!pty->delivered_signal) {
		size_t i;
		صحیح errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		اگر (timerisset(&pty->next_callback_time)) {
			ساختار timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			اگر (timercmp(&now, &pty->next_callback_time, >)) {
				rc = mainloop_callback(pty);
				اگر (rc)
					بشکن;
			}
		}

		/* set timeout */
		اگر (timerisset(&pty->next_callback_time)) {
			ساختار timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->next_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} وگرنه
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		اگر (pty->child_buffer_head)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		وگرنه
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_SIZE(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		اگر (ret < 0) {
			اگر (errsv == EAGAIN)
				ادامه;
			rc = -errno;
			بشکن;
		}

		/* timeout */
		اگر (ret == 0) {
			اگر (timerisset(&pty->next_callback_time)) {
				rc = mainloop_callback(pty);
				اگر (rc == 0)
					ادامه;
			} وگرنه {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			بشکن;
		}
		/* event */
		برای (i = 0; i < ARRAY_SIZE(pfd); i++) {
			اگر (pfd[i].revents == 0)
				ادامه;

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
			وگرنه {
				اگر (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /* data */
				اگر (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, پوچ);
			}

			اگر (rc) {
				صحیح anything = 1;

				ul_pty_write_eof_to_child(pty);
				برای (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				بشکن;
			}

			اگر (i == POLLFD_SIGNAL)
				ادامه;

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
					ul_pty_write_eof_to_child(pty);
				} وگرنه /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		اگر (rc)
			بشکن;
	}

	اگر (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->child, SIGTERM);
		sleep(2);
		kill(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	بازگشت rc;
}

#ifdef TEST_PROGRAM_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
ایستا تهی child_sigstop(تهی *data __attribute__((__unused__)), pid_t child)
{
	kill(getpid(), SIGSTOP);
	kill(child, SIGCONT);
}

صحیح اصلی(صحیح argc, نویسه *argv[])
{
	ساختار ul_pty_callbacks *cb;
	ثابت نویسه *shell, *command = پوچ, *shname = پوچ;
	صحیح caught_signal = 0;
	pid_t child;
	ساختار ul_pty *pty;

	shell = getenv("SHELL");
	اگر (shell == پوچ)
		shell = _PATH_BSHELL;
	اگر (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	اگر (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->child_sigstop = child_sigstop;

	اگر (ul_pty_setup(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	اگر (ul_pty_signals_setup(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	اگر (ul_pty_terminal_setup(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	انتخاب ((صحیح) (child = fork())) {
	حالت -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		بشکن;

	حالت 0: /* child */
		ul_pty_init_slave(pty);

		سیگنال(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		اگر (command)
			execl(shell, shname, "-c", command, (نویسه *)پوچ);
		وگرنه
			execl(shell, shname, "-i", (نویسه *)پوچ);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		بشکن;

	پیشفرض:
		بشکن;
	}

	/* parent */
	ul_pty_set_child(pty, child);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	اگر (!caught_signal && ul_pty_get_child(pty) != (pid_t)-1)
		ul_pty_wait_for_child(pty);	/* final wait */

	اگر (caught_signal && ul_pty_get_child(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(child, SIGTERM);
		sleep(2);
		kill(child, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	بازگشت EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM */

