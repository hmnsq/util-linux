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

स्थिर पूर्णांक can_read(पूर्णांक fd, स्थिरांक दीर्घ timeout)
{
	संरचना pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	पूर्णांक ret;

	करा {
		ret = poll(&fds, 1, timeout);
	} पर्यंत ((ret < 0) && (त्रुटि_सं == EINTR));

	परत (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

स्थिर पूर्णांक open_un_socket_and_connect(शून्य)
{
	/* The abstract UNIX socket of plymouth */
	संरचना sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_PATH,
	};
	स्थिरांक पूर्णांक one = 1;
	पूर्णांक fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	जर (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		जा err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, आकार(one));
	जर (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		बंद(fd);
		fd = -1;
		जा err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (संरचना sockaddr *) &su,
		दुरत्व(संरचना sockaddr_un, sun_path) + 1 + म_माप(su.sun_path+1));
	जर (ret < 0) {
		जर (त्रुटि_सं != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		बंद(fd);
		fd = -1;
		जा err;
	}
err:
	परत fd;
}

पूर्णांक plymouth_command(पूर्णांक cmd, ...)
{
	uint8_t answer[2], command[2];
	संरचना sigaction sp, op;
	पूर्णांक fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = संक_छोड़ो;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	निवड (cmd) {
	स्थिती MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		जर (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, आकार(command));
		}
		थांबा;
	स्थिती MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		जर (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, आकार(command));
		}
		थांबा;
	पूर्वनिर्धारित:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	स्थिती '?':
		जा err;
	}

	answer[0] = '\0';
	जर (fdsock >= 0) {
		जर (can_read(fdsock, 1000))
			ul_read_all(fdsock, (अक्षर *) &answer[0], आकार(answer));
		बंद(fdsock);
	}
	sigaction(SIGPIPE, &op, रिक्त);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	परत ret;
}

