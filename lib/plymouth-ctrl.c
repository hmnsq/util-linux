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

#شامل <errno.h>
#شامل <limits.h>
#شامل <poll.h>
#شامل <signal.h>
#شامل <stdarg.h>
#شامل <stdlib.h>
#شامل <sys/mman.h>
#شامل <sys/socket.h>
#شامل <sys/types.h>
#شامل <sys/un.h>
#شامل <unistd.h>

#شامل "all-io.h"
#شامل "c.h"
#شامل "nls.h"
#شامل "plymouth-ctrl.h"

ساکن صحیح can_read(صحیح fd, ثابت طویل timeout)
{
	ساخت pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	صحیح ret;

	کرو {
		ret = poll(&fds, 1, timeout);
	} جب ((ret < 0) && (errno == EINTR));

	واپس (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

ساکن صحیح open_un_socket_and_connect(خالی)
{
	/* The abstract UNIX socket of plymouth */
	ساخت sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_راستہ,
	};
	ثابت صحیح one = 1;
	صحیح fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	اگر (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		جاؤ err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, حجم(one));
	اگر (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		close(fd);
		fd = -1;
		جاؤ err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (ساخت sockaddr *) &su,
		offsetof(ساخت sockaddr_un, sun_path) + 1 + لڑی_لمبائی(su.sun_path+1));
	اگر (ret < 0) {
		اگر (errno != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		close(fd);
		fd = -1;
		جاؤ err;
	}
err:
	واپس fd;
}

صحیح plymouth_command(صحیح cmd, ...)
{
	uint8_t answer[2], command[2];
	ساخت sigaction sp, op;
	صحیح fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = SIG_IGN;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	انتخاب (cmd) {
	حالت MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		اگر (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, حجم(command));
		}
		توڑو;
	حالت MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		اگر (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, حجم(command));
		}
		توڑو;
	طےشدہ:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	حالت '?':
		جاؤ err;
	}

	answer[0] = '\0';
	اگر (fdsock >= 0) {
		اگر (can_read(fdsock, 1000))
			ul_read_all(fdsock, (حرف *) &answer[0], حجم(answer));
		close(fdsock);
	}
	sigaction(SIGPIPE, &op, تہی);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	واپس ret;
}

