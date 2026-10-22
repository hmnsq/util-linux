/*
 * exec_shell() - launch a shell, else exit!
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://gnu.org/licenses/>.
 */

#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <unistd.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <libgen.h>

#অন্তর্ভাৱ "nls.h"
#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "xalloc.h"

#অন্তর্ভাৱ "exec_shell.h"
#অন্তর্ভাৱ "default_shell.h"

শূন্য __attribute__((__noreturn__)) exec_shell(শূন্য)
{
	ধ্রুৱ ৱর্ণ *shell = ul_default_shell(0, রিক্ত);
	ৱর্ণ *shellc;
	ধ্রুৱ ৱর্ণ *shell_basename;
	ৱর্ণ *arg0;

	shellc = xstrdup(shell);
	shell_basename = basename(shellc);
	xasprintf(&arg0, "-%s", shell_basename);

	execl(shell, arg0, (ৱর্ণ *)রিক্ত);
	errexec(shell);
}
