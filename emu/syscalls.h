#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <signal.h>
#include <syscall.h>

long syscall0(long no);
long syscall1(long no, long a0);
long syscall2(long no, long a0, long a1);
long syscall3(long no, long a0, long a1, long a2);
long syscall4(long no, long a0, long a1, long a2, long a3);
long syscall5(long no, long a0, long a1, long a2, long a3, long a4);
long syscall6(long no, long a0, long a1, long a2, long a3, long a4, long a5);

long syscall_emu(long call, long arg1, long arg2, long arg3,
                            long arg4, long arg5, long arg6);

/* does not go through if a signal arrived before the call */
long syscall_intr(long call, long arg1, long arg2, long arg3,
                             long arg4, long arg5, long arg6);

#define sys_mmap2(a, b, c, d, e, f) \
	syscall6(SYS_mmap2, (long)(a), (long)(b), (long)(c), (long)(d), (long)(e), (long)(f))

#define sys_munmap(a, b) \
	syscall2(SYS_munmap, (long)(a), (long)(b))

#define sys_mprotect(a, b, c) \
	syscall3(SYS_mprotect, (long)(a), (long)(b), (long)(c))

#define sys_mremap(a, b, c, d, e) \
	syscall5(SYS_mremap, (long)(a), (long)(b), (long)(c), (long)(d), (long)e)

#define sys_brk(a) \
	syscall1(SYS_brk, (long)(a))

#define sys_open(a, b, c) \
	syscall3(SYS_open, (long)(a), (long)(b), (long)(c))

#define sys_read(a, b, c) \
	syscall3(SYS_read, (long)(a), (long)(b), (long)(c))

#define sys_write(a, b, c) \
	syscall3(SYS_write, (long)(a), (long)(b), (long)(c))

#define sys_lseek(a, b, c) \
	syscall3(SYS_lseek, (long)(a), (long)(b), (long)(c))

#define sys_access(a, b) \
	syscall2(SYS_access, (long)(a), (long)(b))

#define sys_close(a) \
	syscall1(SYS_close, (long)(a))

#define sys_exit(a) \
	syscall1(SYS_exit, (long)(a))

#define sys_gettid() \
	syscall0(SYS_gettid)

#define sys_tgkill(a, b, c) \
	syscall3(SYS_tgkill, (long)(a), (long)(b), (long)(c))

#define exit_group(a) \
	syscall1(a)

#define abort() \
	raise(SIGABRT)

#define raise(a) \
	sys_tgkill(sys_gettid(), sys_gettid(), a)

#define sys_personality(a) \
	syscall1(SYS_personality, (long)(a))

#define sys_execve(a, b, c) \
	syscall3(SYS_execve, (long)(a), (long)(b), (long)(c))

#define sys_ptrace(a, b, c, d) \
	syscall4(SYS_ptrace, (long)(a), (long)(b), (long)(c), (long)(d))

#define sys_waitpid(a, b, c) \
	syscall3(SYS_waitpid, (long)(a), (long)(b), (long)(c))

#define sys_sigaltstack(ss, oss) \
	syscall2(SYS_sigaltstack, (long)(ss), (long)(oss))

#define sys_rt_sigaction(sig, act, oact, sigsetsize) \
	syscall4(SYS_rt_sigaction, (long)(sig), (long)(act), (long)(oact), (long)(sigsetsize))

#define sys_fork() \
	syscall2(SYS_clone, SIGCHLD, 0)

#endif /* SYSCALLS_H */
