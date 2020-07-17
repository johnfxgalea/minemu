
/* This file is part of minemu
 *
 * Copyright 2010-2011 Erik Bosman <erik@minemu.org>
 * Copyright 2011 Vrije Universiteit Amsterdam
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/mman.h>
#include <sys/prctl.h>
#include <linux/personality.h>
#include <string.h>
#include <errno.h>

#include "syscalls.h"
#include "error.h"
#include "exec.h"
#include "lib.h"
#include "mm.h"
#include "runtime.h"
#include "jit.h"
#include "codemap.h"
#include "sigwrap.h"
#include "options.h"
#include "opcodes.h"
#include "threads.h"
#include "jit_cache.h"

/* not called main() to avoid warnings about extra parameters :-(  */
int minemu_main(int argc, char *orig_argv[], char *envp[], long auxv[])
{
    debug("Hello there!");
	unsigned long pers = sys_personality(0xffffffff);
	char **argv = orig_argv;

	if (ADDR_NO_RANDOMIZE & ~pers)
	{
		sys_personality(ADDR_NO_RANDOMIZE | pers);
		sys_execve("/proc/self/exe", argv, envp);
	}

	init_threads();

	argv = parse_options(argv);

	if ( (progname == NULL) && (argv[0][0] == '/') )
		progname = argv[0];

    debug("Program name is NULL", progname == NULL);

	init_minemu_mem(auxv, envp);
	init_shield(TAINT_END);
	sigwrap_init();
	unblock_signals();
	jit_init();

	elf_prog_t prog =
	{
		.argv = argv,
		.envp = envp,
		.auxv = auxv,
		.task_size = USER_END,
		.stack_size = USER_STACK_SIZE,
	};


    debug ("Attempting to load binary.");

	int ret;
	if (progname)
	{
        debug("Progname not null, trying %s", progname);
		prog.filename = progname;
		ret = load_binary(&prog);
	}
	else
	{
		ret = -ENOENT;
		char *path;
		if ( strchr(argv[0], '/') )
			path = getenve("PWD", envp);
		else
			path = getenve("PATH", envp);

        /* Iterate over path in attempt to find the executable binary. */
		while ( (ret < 0) && (path != NULL) )
		{
			long len;
			char *next = strchr(path, ':');

			if (next)
			{
				len = (long)next-(long)path;
				next += 1;
			}
			else
				len = strlen(path);

			{
				char progname_buf[len + 1 + strlen(argv[0]) + 1];
				strncpy(progname_buf, path, len);
				progname_buf[len] = '/';
				progname_buf[len+1] = '\0';
				strcat(progname_buf, argv[0]);
				prog.filename = progname_buf;
				int ret_tmp = load_binary(&prog);
                debug("Progname is null, trying %s %d", progname_buf, ret_tmp);
				if ( (ret != -EACCES) || (ret_tmp >= 0) )
					ret = ret_tmp;
			}
			path = next;
		}
	}

	if (ret < 0)
	{
		debug("Minemu: cannot run binary %s", argv[0]);
		sys_exit(1);
	}

	/* hide minemu command line for pretty ps output */
	copy_cmdline(orig_argv, prog.argv);
	char *process_name = strrchr(prog.filename, '/');
	if (process_name)
		process_name++;
	else
		process_name = prog.filename;

    debug("Process name: %s", process_name);
	sys_prctl(PR_SET_NAME, process_name, 0,0,0);

	stack_bottom = (unsigned long)prog.sp;

	set_aux(prog.auxv, AT_HWCAP, get_aux(prog.auxv, AT_HWCAP) & CPUID_FEATURE_INFO_EDX_MASK);
	set_aux(prog.auxv, AT_SYSINFO_EHDR, vdso);

	long sysinfo = get_aux(prog.auxv, AT_SYSINFO);
	if (sysinfo)
		set_aux(prog.auxv, AT_SYSINFO, (sysinfo & 0xfff) + vdso);

    debug("Emu starting!");
	emu_start(prog.entry, prog.sp);

    debug("I shouldn't be here!");

	sys_exit(1);
	return 1;
}
