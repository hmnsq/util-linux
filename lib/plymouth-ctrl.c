/*
 * plymouth-ctrl.c	Simply communications with plymouthd
 *			to avoid forked sub processes and/or
 *			missed plymouth send commands tool
 *			due a plymouthd replacement.
 *
 * Copyright (c) 2016 SUSE Linux GmbH, All rights reserved.
 * Copyright (c) 2016 Werner Fink <werner@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://gnu.org/licenses/>.
 *
 * Author: Werner Fink <werner@suse.de>
 */

#ضمن <errno.h>
#ضمن <limits.h>
#ضمن <poll.h>
#ضمن <signal.h>
#ضمن <stdarg.h>
#ضمن <stdlib.h>
#ضمن <sys/mman.h>
#ضمن <sys/socket.h>
#ضمن <sys/types.h>
#ضمن <sys/un.h>
#ضمن <unistd.h>

#ضمن "all-io.h"
#ضمن "c.h"
#ضمن "nls.h"
#ضمن "plymouth-ctrl.h"

ساكن صحيح can_read(صحيح fd, ثابت long timeout)
{
	بنية pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	صحيح ret;

	افعل {
		ret = poll(&fds, 1, timeout);
	} بينما ((ret < 0) && (errno == EINTR));

	إرجاع (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

ساكن صحيح open_un_socket_and_connect(فراغ)
{
	/* The abstract UNIX socket of plymouth */
	بنية sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_PATH,
	};
	ثابت صحيح one = 1;
	صحيح fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	إذا (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		اذهب err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, حجم(one));
	إذا (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		close(fd);
		fd = -1;
		اذهب err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (بنية sockaddr *) &su,
		offsetof(بنية sockaddr_un, sun_path) + 1 + strlen(su.sun_path+1));
	إذا (ret < 0) {
		إذا (errno != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		close(fd);
		fd = -1;
		اذهب err;
	}
err:
	إرجاع fd;
}

صحيح plymouth_command(صحيح cmd, ...)
{
	uint8_t answer[2], command[2];
	بنية sigaction sp, op;
	صحيح fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = SIG_IGN;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	بدل (cmd) {
	حالة MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		إذا (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, حجم(command));
		}
		اكسر;
	حالة MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		إذا (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, حجم(command));
		}
		اكسر;
	افتراضي:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	حالة '?':
		اذهب err;
	}

	answer[0] = '\0';
	إذا (fdsock >= 0) {
		إذا (can_read(fdsock, 1000))
			ul_read_all(fdsock, (حرف *) &answer[0], حجم(answer));
		close(fdsock);
	}
	sigaction(SIGPIPE, &op, عدم);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	إرجاع ret;
}

