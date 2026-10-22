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

void ul_pty_init_debug(целое mask)
{
	если (ulpty_debug_mask)
		вернуть;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

struct ul_pty *ul_new_pty(целое is_stdin_tty)
{
	struct ul_pty *pty = calloc(1, sizeof(*pty));

	если (!pty)
		вернуть NULL;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	вернуть pty;
}

void ul_free_pty(struct ul_pty *pty)
{
	struct ul_pty_child_buffer *hd;

	пока ((hd = pty->child_buffer_head)) {
		pty->child_buffer_head = hd->next;
		free(hd);
	}

	пока ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->next;
		free(hd);
	}
	free(pty);
}

void ul_pty_slave_echo(struct ul_pty *pty, целое enable)
{
	assert(pty);
	pty->slave_echo = enable ? 1 : 0;
}

целое ul_pty_get_delivered_signal(struct ul_pty *pty)
{
	assert(pty);
	вернуть pty->delivered_signal;
}

struct ul_pty_callbacks *ul_pty_get_callbacks(struct ul_pty *pty)
{
	assert(pty);
	вернуть &pty->callbacks;
}

void ul_pty_set_callback_data(struct ul_pty *pty, void *data)
{
	assert(pty);
	pty->callback_data = data;
}

void ul_pty_set_child(struct ul_pty *pty, pid_t child)
{
	assert(pty);
	pty->child = child;
}

целое ul_pty_get_childfd(struct ul_pty *pty)
{
	assert(pty);
	вернуть pty->master;
}

pid_t ul_pty_get_child(struct ul_pty *pty)
{
	assert(pty);
	вернуть pty->child;
}

/* it's active when signals are redirected to sigfd */
целое ul_pty_is_running(struct ul_pty *pty)
{
	assert(pty);
	вернуть pty->sigfd >= 0;
}

void ul_pty_set_mainloop_time(struct ul_pty *pty, struct timeval *tv)
{
	assert(pty);
	если (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->next_callback_time);
	} иначе {
		pty->next_callback_time.tv_sec = tv->tv_sec;
		pty->next_callback_time.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

static void pty_signals_cleanup(struct ul_pty *pty)
{
	если (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, NULL);
}

/* call me before fork() */
целое ul_pty_setup(struct ul_pty *pty)
{
	struct termios attrs;
	целое rc = 0;

	assert(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, NULL, &pty->orgsig);

	если (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		если (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			goto done;
		}

		attrs = pty->stdin_attrs;
		если (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		иначе
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (char *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, NULL, &attrs, &pty->win);
		если (rc)
			goto done;

	} иначе {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, NULL, NULL, NULL);
		если (rc)
			goto done;

		tcgetattr(pty->slave, &attrs);

		если (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		иначе
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

done:
	если (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	вернуть rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
целое ul_pty_terminal_setup(struct ul_pty *pty)
{
	struct termios attrs;

	если (!pty->isterm)
		вернуть 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	если (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		вернуть -errno;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	вернуть 0;
}

/* call me before fork() */
целое ul_pty_signals_setup(struct ul_pty *pty)
{
	sigset_t ourset;
	целое rc = 0;

	assert(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, NULL, &pty->orgsig);

	sigfillset(&ourset);
	если (sigprocmask(SIG_BLOCK, &ourset, NULL)) {
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

	если (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	если ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
done:
	если (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	вернуть rc;
}

/* cleanup in parent process */
void ul_pty_cleanup(struct ul_pty *pty)
{
	struct termios rtt;

	pty_signals_cleanup(pty);

	если (pty->master == -1 || !pty->isterm)
		вернуть;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

целое ul_pty_chownmod_slave(struct ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	если (fchown(pty->slave, uid, gid))
		вернуть -errno;
	если (fchmod(pty->slave, mode))
		вернуть -errno;
	вернуть 0;
}

/* call me in child process */
void ul_pty_init_slave(struct ul_pty *pty)
{
	DBG_OBJ(SETUP, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	если (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, NULL);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

static целое write_output(char *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	если (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		вернуть -errno;
	}

	вернуть 0;
}

static целое schedule_child_write(struct ul_pty *pty, char *buf, size_t bufsz, целое final)
{
	struct ul_pty_child_buffer *stash;

	если (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->next;
		memset(stash, 0, sizeof(*stash));
	} иначе
		stash = calloc(1, sizeof(*stash));
	если (!stash)
		вернуть -1;

	assert(bufsz <= sizeof(stash->buf));

	memcpy(stash->buf, buf, bufsz);
	stash->size = bufsz;
	stash->final_input = final ? 1 : 0;

	если (pty->child_buffer_head)
		pty->child_buffer_tail = pty->child_buffer_tail->next = stash;
	иначе
		pty->child_buffer_head = pty->child_buffer_tail = stash;
	вернуть 0;
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
static void drain_child_buffers(struct ul_pty *pty)
{
	unsigned целое tries = 0;
	struct pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	пока (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	если (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

static целое flush_child_buffers(struct ul_pty *pty, целое *anything)
{
	целое rc = 0, any = 0;

	пока (pty->child_buffer_head) {
		struct ul_pty_child_buffer *hd = pty->child_buffer_head;
		ssize_t ret;

		если (hd->final_input)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->size - hd->cursor));

		ret = write(pty->master, hd->buf + hd->cursor, hd->size - hd->cursor);
		если (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			если (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			goto out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		если (hd->cursor == hd->size) {
			pty->child_buffer_head = hd->next;
			если (!hd->next)
				pty->child_buffer_tail = NULL;

			hd->next = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
out:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	если (any)
		fdatasync(pty->master);

	если (anything)
		*anything = any;
	вернуть rc;
}

void ul_pty_write_eof_to_child(struct ul_pty *pty)
{
	char c = DEF_EOF;
	schedule_child_write(pty, &c, sizeof(char), 1);
}

static целое mainloop_callback(struct ul_pty *pty)
{
	целое rc;

	если (!pty->callbacks.mainloop)
		вернуть 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_data);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	вернуть rc;
}

static целое handle_io(struct ul_pty *pty, целое fd, целое *eof)
{
	char buf[BUFSIZ];
	ssize_t bytes;
	целое rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
	/* read from active FD */
	bytes = read(fd, buf, sizeof(buf));
	sigprocmask(SIG_BLOCK, &set, NULL);
	если (bytes == -1) {
		если (errno == EAGAIN || errno == EINTR)
			вернуть 0;
		вернуть -errno;
	}

	если (bytes == 0) {
		*eof = 1;
		вернуть 0;
	}

	/* from stdin (user) to command */
	если (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		если (schedule_child_write(pty, buf, bytes, 0))
			вернуть -errno;

	/* from command (master) to stdout */
	} иначе если (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	если (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_data, fd, buf, bytes);

	вернуть rc;
}

void ul_pty_wait_for_child(struct ul_pty *pty)
{
	целое status;
	pid_t pid;
	целое options = 0;

	если (pty->child == (pid_t) -1)
		вернуть;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (целое) pty->child));

	если (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		для (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (целое) pid));
			если (pid != (pid_t) - 1) {
				если (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} иначе
				break;
		}
	} иначе {
		/* final wait */
		пока ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (целое) pid));
			если (pid == pty->child) {
				если (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

static целое handle_signal(struct ul_pty *pty, целое fd)
{
	struct signalfd_siginfo info;
	ssize_t bytes;
	целое rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = read(fd, &info, sizeof(info));
	если (bytes != sizeof(info)) {
		если (bytes < 0 && (errno == EAGAIN || errno == EINTR))
			вернуть 0;
		вернуть -errno;
	}

	switch (info.ssi_signo) {
	case SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		если (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			если (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_data,
							  pty->child);
			иначе
				ul_pty_wait_for_child(pty);

		} иначе если (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_data,
						     pty->child);
		}

		если (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->next_callback_time);
		}
		вернуть 0;
	case SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		если (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (char *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (char *)&pty->win);

			если (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_data,
							&info, (void *) &pty->win);
		}
		break;
	case SIGALRM:
		FALLTHROUGH;
	case SIGHUP:
		FALLTHROUGH;
	case SIGTERM:
		FALLTHROUGH;
	case SIGINT:
		FALLTHROUGH;
	case SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		если (pty->child > 0)
			kill(pty->child, SIGTERM);

		если (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_data,
					&info, (void *) &pty->win);
		break;
	case SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		если (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_data);
		break;
	default:
		abort();
	}

	вернуть rc;
}

/* loop in parent */
целое ul_pty_proxy_master(struct ul_pty *pty)
{
	целое rc = 0, ret, eof = 0;
	enum {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	struct pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	assert(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	пока (!pty->delivered_signal) {
		size_t i;
		целое errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		если (timerisset(&pty->next_callback_time)) {
			struct timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			если (timercmp(&now, &pty->next_callback_time, >)) {
				rc = mainloop_callback(pty);
				если (rc)
					break;
			}
		}

		/* set timeout */
		если (timerisset(&pty->next_callback_time)) {
			struct timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->next_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} иначе
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		если (pty->child_buffer_head)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		иначе
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_SIZE(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		если (ret < 0) {
			если (errsv == EAGAIN)
				continue;
			rc = -errno;
			break;
		}

		/* timeout */
		если (ret == 0) {
			если (timerisset(&pty->next_callback_time)) {
				rc = mainloop_callback(pty);
				если (rc == 0)
					continue;
			} иначе {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			break;
		}
		/* event */
		для (i = 0; i < ARRAY_SIZE(pfd); i++) {
			если (pfd[i].revents == 0)
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

			если (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			иначе {
				если (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /* data */
				если (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, NULL);
			}

			если (rc) {
				целое anything = 1;

				ul_pty_write_eof_to_child(pty);
				для (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				break;
			}

			если (i == POLLFD_SIGNAL)
				continue;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			если ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				если (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_to_child(pty);
				} иначе /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		если (rc)
			break;
	}

	если (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->child, SIGTERM);
		sleep(2);
		kill(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	вернуть rc;
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

целое основной(целое argc, char *argv[])
{
	struct ul_pty_callbacks *cb;
	const char *shell, *command = NULL, *shname = NULL;
	целое caught_signal = 0;
	pid_t child;
	struct ul_pty *pty;

	shell = getenv("SHELL");
	если (shell == NULL)
		shell = _PATH_BSHELL;
	если (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	если (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->child_sigstop = child_sigstop;

	если (ul_pty_setup(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	если (ul_pty_signals_setup(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	если (ul_pty_terminal_setup(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	switch ((целое) (child = fork())) {
	case -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		break;

	case 0: /* child */
		ul_pty_init_slave(pty);

		signal(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		если (command)
			execl(shell, shname, "-c", command, (char *)NULL);
		иначе
			execl(shell, shname, "-i", (char *)NULL);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		break;

	default:
		break;
	}

	/* parent */
	ul_pty_set_child(pty, child);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	если (!caught_signal && ul_pty_get_child(pty) != (pid_t)-1)
		ul_pty_wait_for_child(pty);	/* final wait */

	если (caught_signal && ul_pty_get_child(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(child, SIGTERM);
		sleep(2);
		kill(child, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	вернуть EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM */

