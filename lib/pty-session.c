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
#include <stdio.h>
#include <stdlib.h>
#include <pty.h>
#include <poll.h>
#include <sys/signalfd.h>
#include <paths.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <inttypes.h>

#include "c.h"
#include "all-io.h"
#include "ttyutils.h"
#include "pty-session.h"
#include "monotonic.h"
#include "debug.h"

static UL_DEBUG_DEFINE_MASK(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPTY_DEBUG_INIT	(1 << 1)
#define ULPTY_DEBUG_SETUP	(1 << 2)
#define ULPTY_DEBUG_SIG		(1 << 3)
#define ULPTY_DEBUG_IO		(1 << 4)
#define ULPTY_DEBUG_DONE	(1 << 5)

#define DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

void ul_pty_init_debug(ចំនួនគត់ mask)
{
	ប្រសិនបើ (ulpty_debug_mask)
		ត្រឡប់;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

sa_nracanaa ul_pty *ul_new_pty(ចំនួនគត់ is_stdin_tty)
{
	sa_nracanaa ul_pty *pty = calloc(1, parimaa_na(*pty));

	ប្រសិនបើ (!pty)
		ត្រឡប់ NULL;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	ត្រឡប់ pty;
}

void ul_free_pty(sa_nracanaa ul_pty *pty)
{
	sa_nracanaa ul_pty_child_buffer *hd;

	ខណៈពេល ((hd = pty->child_buffer_head)) {
		pty->child_buffer_head = hd->next;
		free(hd);
	}

	ខណៈពេល ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->next;
		free(hd);
	}
	free(pty);
}

void ul_pty_slave_echo(sa_nracanaa ul_pty *pty, ចំនួនគត់ enable)
{
	assert(pty);
	pty->slave_echo = enable ? 1 : 0;
}

ចំនួនគត់ ul_pty_get_delivered_signal(sa_nracanaa ul_pty *pty)
{
	assert(pty);
	ត្រឡប់ pty->delivered_signal;
}

sa_nracanaa ul_pty_callbacks *ul_pty_get_callbacks(sa_nracanaa ul_pty *pty)
{
	assert(pty);
	ត្រឡប់ &pty->callbacks;
}

void ul_pty_set_callback_data(sa_nracanaa ul_pty *pty, void *data)
{
	assert(pty);
	pty->callback_data = data;
}

void ul_pty_set_child(sa_nracanaa ul_pty *pty, pid_t child)
{
	assert(pty);
	pty->child = child;
}

ចំនួនគត់ ul_pty_get_childfd(sa_nracanaa ul_pty *pty)
{
	assert(pty);
	ត្រឡប់ pty->master;
}

pid_t ul_pty_get_child(sa_nracanaa ul_pty *pty)
{
	assert(pty);
	ត្រឡប់ pty->child;
}

/* it's active when signals are redirected to sigfd */
ចំនួនគត់ ul_pty_is_running(sa_nracanaa ul_pty *pty)
{
	assert(pty);
	ត្រឡប់ pty->sigfd >= 0;
}

void ul_pty_set_mainloop_time(sa_nracanaa ul_pty *pty, sa_nracanaa timeval *tv)
{
	assert(pty);
	ប្រសិនបើ (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->next_callback_time);
	} បើមិនដូច្នេះ {
		pty->next_callback_time.tv_sec = tv->tv_sec;
		pty->next_callback_time.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

static void pty_signals_cleanup(sa_nracanaa ul_pty *pty)
{
	ប្រសិនបើ (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, NULL);
}

/* call me before fork() */
ចំនួនគត់ ul_pty_setup(sa_nracanaa ul_pty *pty)
{
	sa_nracanaa termios attrs;
	ចំនួនគត់ rc = 0;

	assert(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, NULL, &pty->orgsig);

	ប្រសិនបើ (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		ប្រសិនបើ (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			goto done;
		}

		attrs = pty->stdin_attrs;
		ប្រសិនបើ (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		បើមិនដូច្នេះ
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (var_na *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, NULL, &attrs, &pty->win);
		ប្រសិនបើ (rc)
			goto done;

	} បើមិនដូច្នេះ {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, NULL, NULL, NULL);
		ប្រសិនបើ (rc)
			goto done;

		tcgetattr(pty->slave, &attrs);

		ប្រសិនបើ (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		បើមិនដូច្នេះ
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

done:
	ប្រសិនបើ (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	ត្រឡប់ rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
ចំនួនគត់ ul_pty_terminal_setup(sa_nracanaa ul_pty *pty)
{
	sa_nracanaa termios attrs;

	ប្រសិនបើ (!pty->isterm)
		ត្រឡប់ 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	ប្រសិនបើ (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		ត្រឡប់ -errno;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	ត្រឡប់ 0;
}

/* call me before fork() */
ចំនួនគត់ ul_pty_signals_setup(sa_nracanaa ul_pty *pty)
{
	sigset_t ourset;
	ចំនួនគត់ rc = 0;

	assert(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, NULL, &pty->orgsig);

	sigfillset(&ourset);
	ប្រសិនបើ (sigprocmask(SIG_BLOCK, &ourset, NULL)) {
		rc = -errno;
		goto done;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, SIGTERM);
	sigaddset(&ourset, SIGINT);
	sigaddset(&ourset, SIGQUIT);

	ប្រសិនបើ (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	ប្រសិនបើ ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
done:
	ប្រសិនបើ (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	ត្រឡប់ rc;
}

/* cleanup in parent process */
void ul_pty_cleanup(sa_nracanaa ul_pty *pty)
{
	sa_nracanaa termios rtt;

	pty_signals_cleanup(pty);

	ប្រសិនបើ (pty->master == -1 || !pty->isterm)
		ត្រឡប់;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

ចំនួនគត់ ul_pty_chownmod_slave(sa_nracanaa ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	ប្រសិនបើ (fchown(pty->slave, uid, gid))
		ត្រឡប់ -errno;
	ប្រសិនបើ (fchmod(pty->slave, mode))
		ត្រឡប់ -errno;
	ត្រឡប់ 0;
}

/* call me in child process */
void ul_pty_init_slave(sa_nracanaa ul_pty *pty)
{
	DBG_OBJ(SETUP, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	ប្រសិនបើ (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, NULL);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

static ចំនួនគត់ write_output(var_na *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	ប្រសិនបើ (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		ត្រឡប់ -errno;
	}

	ត្រឡប់ 0;
}

static ចំនួនគត់ schedule_child_write(sa_nracanaa ul_pty *pty, var_na *buf, size_t bufsz, ចំនួនគត់ final)
{
	sa_nracanaa ul_pty_child_buffer *stash;

	ប្រសិនបើ (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->next;
		memset(stash, 0, parimaa_na(*stash));
	} បើមិនដូច្នេះ
		stash = calloc(1, parimaa_na(*stash));
	ប្រសិនបើ (!stash)
		ត្រឡប់ -1;

	assert(bufsz <= parimaa_na(stash->buf));

	memcpy(stash->buf, buf, bufsz);
	stash->size = bufsz;
	stash->final_input = final ? 1 : 0;

	ប្រសិនបើ (pty->child_buffer_head)
		pty->child_buffer_tail = pty->child_buffer_tail->next = stash;
	បើមិនដូច្នេះ
		pty->child_buffer_head = pty->child_buffer_tail = stash;
	ត្រឡប់ 0;
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
static void drain_child_buffers(sa_nracanaa ul_pty *pty)
{
	unsigned ចំនួនគត់ tries = 0;
	sa_nracanaa pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	ខណៈពេល (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	ប្រសិនបើ (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

static ចំនួនគត់ flush_child_buffers(sa_nracanaa ul_pty *pty, ចំនួនគត់ *anything)
{
	ចំនួនគត់ rc = 0, any = 0;

	ខណៈពេល (pty->child_buffer_head) {
		sa_nracanaa ul_pty_child_buffer *hd = pty->child_buffer_head;
		ssize_t ret;

		ប្រសិនបើ (hd->final_input)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->size - hd->cursor));

		ret = write(pty->master, hd->buf + hd->cursor, hd->size - hd->cursor);
		ប្រសិនបើ (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			ប្រសិនបើ (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			goto out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		ប្រសិនបើ (hd->cursor == hd->size) {
			pty->child_buffer_head = hd->next;
			ប្រសិនបើ (!hd->next)
				pty->child_buffer_tail = NULL;

			hd->next = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
out:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	ប្រសិនបើ (any)
		fdatasync(pty->master);

	ប្រសិនបើ (anything)
		*anything = any;
	ត្រឡប់ rc;
}

void ul_pty_write_eof_to_child(sa_nracanaa ul_pty *pty)
{
	var_na c = DEF_EOF;
	schedule_child_write(pty, &c, parimaa_na(var_na), 1);
}

static ចំនួនគត់ mainloop_callback(sa_nracanaa ul_pty *pty)
{
	ចំនួនគត់ rc;

	ប្រសិនបើ (!pty->callbacks.mainloop)
		ត្រឡប់ 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_data);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	ត្រឡប់ rc;
}

static ចំនួនគត់ handle_io(sa_nracanaa ul_pty *pty, ចំនួនគត់ fd, ចំនួនគត់ *eof)
{
	var_na buf[BUFSIZ];
	ssize_t bytes;
	ចំនួនគត់ rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
	/* read from active FD */
	bytes = read(fd, buf, parimaa_na(buf));
	sigprocmask(SIG_BLOCK, &set, NULL);
	ប្រសិនបើ (bytes == -1) {
		ប្រសិនបើ (errno == EAGAIN || errno == EINTR)
			ត្រឡប់ 0;
		ត្រឡប់ -errno;
	}

	ប្រសិនបើ (bytes == 0) {
		*eof = 1;
		ត្រឡប់ 0;
	}

	/* from stdin (user) to command */
	ប្រសិនបើ (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		ប្រសិនបើ (schedule_child_write(pty, buf, bytes, 0))
			ត្រឡប់ -errno;

	/* from command (master) to stdout */
	} បើមិនដូច្នេះ ប្រសិនបើ (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	ប្រសិនបើ (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_data, fd, buf, bytes);

	ត្រឡប់ rc;
}

void ul_pty_wait_for_child(sa_nracanaa ul_pty *pty)
{
	ចំនួនគត់ status;
	pid_t pid;
	ចំនួនគត់ options = 0;

	ប្រសិនបើ (pty->child == (pid_t) -1)
		ត្រឡប់;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (ចំនួនគត់) pty->child));

	ប្រសិនបើ (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		សម្រាប់ (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (ចំនួនគត់) pid));
			ប្រសិនបើ (pid != (pid_t) - 1) {
				ប្រសិនបើ (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} បើមិនដូច្នេះ
				bha_nga;
		}
	} បើមិនដូច្នេះ {
		/* final wait */
		ខណៈពេល ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (ចំនួនគត់) pid));
			ប្រសិនបើ (pid == pty->child) {
				ប្រសិនបើ (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

static ចំនួនគត់ handle_signal(sa_nracanaa ul_pty *pty, ចំនួនគត់ fd)
{
	sa_nracanaa signalfd_siginfo info;
	ssize_t bytes;
	ចំនួនគត់ rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = read(fd, &info, parimaa_na(info));
	ប្រសិនបើ (bytes != parimaa_na(info)) {
		ប្រសិនបើ (bytes < 0 && (errno == EAGAIN || errno == EINTR))
			ត្រឡប់ 0;
		ត្រឡប់ -errno;
	}

	switch (info.ssi_signo) {
	pak_sa SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		ប្រសិនបើ (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			ប្រសិនបើ (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_data,
							  pty->child);
			បើមិនដូច្នេះ
				ul_pty_wait_for_child(pty);

		} បើមិនដូច្នេះ ប្រសិនបើ (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_data,
						     pty->child);
		}

		ប្រសិនបើ (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->next_callback_time);
		}
		ត្រឡប់ 0;
	pak_sa SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		ប្រសិនបើ (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (var_na *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (var_na *)&pty->win);

			ប្រសិនបើ (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_data,
							&info, (void *) &pty->win);
		}
		bha_nga;
	pak_sa SIGALRM:
		FALLTHROUGH;
	pak_sa SIGHUP:
		FALLTHROUGH;
	pak_sa SIGTERM:
		FALLTHROUGH;
	pak_sa SIGINT:
		FALLTHROUGH;
	pak_sa SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		ប្រសិនបើ (pty->child > 0)
			kill(pty->child, SIGTERM);

		ប្រសិនបើ (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_data,
					&info, (void *) &pty->win);
		bha_nga;
	pak_sa SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		ប្រសិនបើ (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_data);
		bha_nga;
	default:
		abort();
	}

	ត្រឡប់ rc;
}

/* loop in parent */
ចំនួនគត់ ul_pty_proxy_master(sa_nracanaa ul_pty *pty)
{
	ចំនួនគត់ rc = 0, ret, eof = 0;
	ga_nanaa {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	sa_nracanaa pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	assert(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	ខណៈពេល (!pty->delivered_signal) {
		size_t i;
		ចំនួនគត់ errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		ប្រសិនបើ (timerisset(&pty->next_callback_time)) {
			sa_nracanaa timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			ប្រសិនបើ (timercmp(&now, &pty->next_callback_time, >)) {
				rc = mainloop_callback(pty);
				ប្រសិនបើ (rc)
					bha_nga;
			}
		}

		/* set timeout */
		ប្រសិនបើ (timerisset(&pty->next_callback_time)) {
			sa_nracanaa timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->next_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} បើមិនដូច្នេះ
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		ប្រសិនបើ (pty->child_buffer_head)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		បើមិនដូច្នេះ
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_SIZE(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		ប្រសិនបើ (ret < 0) {
			ប្រសិនបើ (errsv == EAGAIN)
				continue;
			rc = -errno;
			bha_nga;
		}

		/* timeout */
		ប្រសិនបើ (ret == 0) {
			ប្រសិនបើ (timerisset(&pty->next_callback_time)) {
				rc = mainloop_callback(pty);
				ប្រសិនបើ (rc == 0)
					continue;
			} បើមិនដូច្នេះ {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			bha_nga;
		}
		/* event */
		សម្រាប់ (i = 0; i < ARRAY_SIZE(pfd); i++) {
			ប្រសិនបើ (pfd[i].revents == 0)
				continue;

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

			ប្រសិនបើ (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			បើមិនដូច្នេះ {
				ប្រសិនបើ (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /* data */
				ប្រសិនបើ (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, NULL);
			}

			ប្រសិនបើ (rc) {
				ចំនួនគត់ anything = 1;

				ul_pty_write_eof_to_child(pty);
				សម្រាប់ (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				bha_nga;
			}

			ប្រសិនបើ (i == POLLFD_SIGNAL)
				continue;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			ប្រសិនបើ ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				ប្រសិនបើ (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_to_child(pty);
				} បើមិនដូច្នេះ /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		ប្រសិនបើ (rc)
			bha_nga;
	}

	ប្រសិនបើ (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->child, SIGTERM);
		sleep(2);
		kill(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	ត្រឡប់ rc;
}

#ifdef TEST_PROGRAM_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
static void child_sigstop(void *data __attribute__((__unused__)), pid_t child)
{
	kill(getpid(), SIGSTOP);
	kill(child, SIGCONT);
}

ចំនួនគត់ main(ចំនួនគត់ argc, var_na *argv[])
{
	sa_nracanaa ul_pty_callbacks *cb;
	const var_na *shell, *command = NULL, *shname = NULL;
	ចំនួនគត់ caught_signal = 0;
	pid_t child;
	sa_nracanaa ul_pty *pty;

	shell = getenv("SHELL");
	ប្រសិនបើ (shell == NULL)
		shell = _PATH_BSHELL;
	ប្រសិនបើ (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	ប្រសិនបើ (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->child_sigstop = child_sigstop;

	ប្រសិនបើ (ul_pty_setup(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	ប្រសិនបើ (ul_pty_signals_setup(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	ប្រសិនបើ (ul_pty_terminal_setup(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	switch ((ចំនួនគត់) (child = fork())) {
	pak_sa -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		bha_nga;

	pak_sa 0: /* child */
		ul_pty_init_slave(pty);

		sa_nkeeta(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		ប្រសិនបើ (command)
			execl(shell, shname, "-c", command, (var_na *)NULL);
		បើមិនដូច្នេះ
			execl(shell, shname, "-i", (var_na *)NULL);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		bha_nga;

	default:
		bha_nga;
	}

	/* parent */
	ul_pty_set_child(pty, child);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	ប្រសិនបើ (!caught_signal && ul_pty_get_child(pty) != (pid_t)-1)
		ul_pty_wait_for_child(pty);	/* final wait */

	ប្រសិនបើ (caught_signal && ul_pty_get_child(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(child, SIGTERM);
		sleep(2);
		kill(child, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	ត្រឡប់ EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM */

