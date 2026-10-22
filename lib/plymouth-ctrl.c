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

#समावेश <errno.h>
#समावेश <limits.h>
#समावेश <poll.h>
#समावेश <signal.h>
#समावेश <stdarg.h>
#समावेश <stdlib.h>
#समावेश <sys/mman.h>
#समावेश <sys/socket.h>
#समावेश <sys/types.h>
#समावेश <sys/un.h>
#समावेश <unistd.h>

#समावेश "all-io.h"
#समावेश "c.h"
#समावेश "nls.h"
#समावेश "plymouth-ctrl.h"

अटल पूर्णांक can_read(पूर्णांक fd, स्थिर दीर्घ timeout)
{
	काष्ठा pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	पूर्णांक ret;

	करो {
		ret = poll(&fds, 1, timeout);
	} जबतक ((ret < 0) && (त्रुटि_सं == EINTR));

	वापस (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

अटल पूर्णांक open_un_socket_and_connect(व्योम)
{
	/* The abstract UNIX socket of plymouth */
	काष्ठा sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_PATH,
	};
	स्थिर पूर्णांक one = 1;
	पूर्णांक fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	अगर (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		जाओ err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, माप(one));
	अगर (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		बंद(fd);
		fd = -1;
		जाओ err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (काष्ठा sockaddr *) &su,
		दुरत्व(काष्ठा sockaddr_un, sun_path) + 1 + म_माप(su.sun_path+1));
	अगर (ret < 0) {
		अगर (त्रुटि_सं != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		बंद(fd);
		fd = -1;
		जाओ err;
	}
err:
	वापस fd;
}

पूर्णांक plymouth_command(पूर्णांक cmd, ...)
{
	uint8_t answer[2], command[2];
	काष्ठा sigaction sp, op;
	पूर्णांक fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = संक_छोड़ो;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	चयन (cmd) {
	हाल MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		अगर (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, माप(command));
		}
		अवरोध;
	हाल MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		अगर (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, माप(command));
		}
		अवरोध;
	शेष:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	हाल '?':
		जाओ err;
	}

	answer[0] = '\0';
	अगर (fdsock >= 0) {
		अगर (can_read(fdsock, 1000))
			ul_read_all(fdsock, (अक्षर *) &answer[0], माप(answer));
		बंद(fdsock);
	}
	sigaction(SIGPIPE, &op, शून्य);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	वापस ret;
}

