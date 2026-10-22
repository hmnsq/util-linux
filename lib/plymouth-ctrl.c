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

#अन्तर्भाव <errno.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <poll.h>
#अन्तर्भाव <signal.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <sys/mman.h>
#अन्तर्भाव <sys/socket.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/un.h>
#अन्तर्भाव <unistd.h>

#अन्तर्भाव "all-io.h"
#अन्तर्भाव "c.h"
#अन्तर्भाव "nls.h"
#अन्तर्भाव "plymouth-ctrl.h"

स्थिर पूर्णाङ्क can_read(पूर्णाङ्क fd, ध्रुव long timeout)
{
	संरचना pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	पूर्णाङ्क ret;

	करेहि {
		ret = poll(&fds, 1, timeout);
	} जाव ((ret < 0) && (errno == EINTR));

	प्रत्यागम (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

स्थिर पूर्णाङ्क open_un_socket_and_connect(शून्य)
{
	/* The abstract UNIX socket of plymouth */
	संरचना sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_PATH,
	};
	ध्रुव पूर्णाङ्क one = 1;
	पूर्णाङ्क fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	जइ (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		गच्छ err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, परिमाण(one));
	जइ (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		close(fd);
		fd = -1;
		गच्छ err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (संरचना sockaddr *) &su,
		offsetof(संरचना sockaddr_un, sun_path) + 1 + strlen(su.sun_path+1));
	जइ (ret < 0) {
		जइ (errno != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		close(fd);
		fd = -1;
		गच्छ err;
	}
err:
	प्रत्यागम fd;
}

पूर्णाङ्क plymouth_command(पूर्णाङ्क cmd, ...)
{
	uint8_t answer[2], command[2];
	संरचना sigaction sp, op;
	पूर्णाङ्क fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = SIG_IGN;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	विकल्प (cmd) {
	पक्ष MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		जइ (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, परिमाण(command));
		}
		भङ्ग;
	पक्ष MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		जइ (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, परिमाण(command));
		}
		भङ्ग;
	स्वभाव:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	पक्ष '?':
		गच्छ err;
	}

	answer[0] = '\0';
	जइ (fdsock >= 0) {
		जइ (can_read(fdsock, 1000))
			ul_read_all(fdsock, (वर्ण *) &answer[0], परिमाण(answer));
		close(fdsock);
	}
	sigaction(SIGPIPE, &op, रिक्त);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	प्रत्यागम ret;
}

