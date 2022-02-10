
# Lab1
- student name:Xiaoran Xie
- student number:7884702
## Lab1-pe
- how to compile: 'make'
- how to run: './pe ' + filename
- (such as './pe putty.exe', i have put one 64bit-x86 putty.exe in this folder, you can put other putty.exe here)
- how to clean: 'make clean'
## Lab1-process
- how to compile: 'make'
- how to run: './process ' + number of processes you want
- (such as './process 8')
- how to clean: 'make clean'
#### What system calls have you written into your program?
- exit, fork, getpid, setpgid, getppid, kill
#### What system calls does your program actually make? 
- execve, brk, mmap, access, open, stat, fstat, close, read, mprotect, arch_prctl, rt_sigaction, clone, getpid, setpgid, getppid, write, nanosleep, restart_syscall, wait4, exit_group
#### where do these system calls come from? Your code isn’t making those system calls, so who is?
- System calls provide interface to operating system, they come from kernel of operating system. I think that GNU c library makes system calls，so i can call c functions to use system calls.
## Lab1-threads
- how to compile: 'make'
- how to run: './threads' 
- how to clean: 'make clean'
-