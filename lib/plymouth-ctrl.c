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

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <poll.h>
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <sys/mman.h>
#অন্তর্ভাব <sys/socket.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/un.h>
#অন্তর্ভাব <unistd.h>

#অন্তর্ভাব "all-io.h"
#অন্তর্ভাব "c.h"
#অন্তর্ভাব "nls.h"
#অন্তর্ভাব "plymouth-ctrl.h"

স্থির পূর্ণসংখ্যা can_read(পূর্ণসংখ্যা fd, ধ্রুবক long timeout)
{
	কাঠামো pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	পূর্ণসংখ্যা ret;

	করো {
		ret = poll(&fds, 1, timeout);
	} যখন ((ret < 0) && (errno == EINTR));

	ফেরত (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

স্থির পূর্ণসংখ্যা open_un_socket_and_connect(শূন্য)
{
	/* The abstract UNIX socket of plymouth */
	কাঠামো sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_PATH,
	};
	ধ্রুবক পূর্ণসংখ্যা one = 1;
	পূর্ণসংখ্যা fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	যদি (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		যাও err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, আকার(one));
	যদি (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		close(fd);
		fd = -1;
		যাও err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (কাঠামো sockaddr *) &su,
		offsetof(কাঠামো sockaddr_un, sun_path) + 1 + strlen(su.sun_path+1));
	যদি (ret < 0) {
		যদি (errno != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		close(fd);
		fd = -1;
		যাও err;
	}
err:
	ফেরত fd;
}

পূর্ণসংখ্যা plymouth_command(পূর্ণসংখ্যা cmd, ...)
{
	uint8_t answer[2], command[2];
	কাঠামো sigaction sp, op;
	পূর্ণসংখ্যা fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = SIG_IGN;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	নির্বাচন (cmd) {
	ক্ষেত্র MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		যদি (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, আকার(command));
		}
		বিরতি;
	ক্ষেত্র MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		যদি (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, আকার(command));
		}
		বিরতি;
	পূর্বনির্ধারিত:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	ক্ষেত্র '?':
		যাও err;
	}

	answer[0] = '\0';
	যদি (fdsock >= 0) {
		যদি (can_read(fdsock, 1000))
			ul_read_all(fdsock, (অক্ষর *) &answer[0], আকার(answer));
		close(fdsock);
	}
	sigaction(SIGPIPE, &op, ফাঁকা);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	ফেরত ret;
}

