# mdb

Minimal debugger

## ptrace

在执行系统调用之前, 内核会先检查当前进程是否处于被 "跟踪" (traced) 的状态.
如果是的话， 内核暂停当前进程并将控制权交给跟踪进程， 使跟踪进程得以察看或者修改被跟踪进程的寄存器.
Whenever traced process trigger a system call,
control is passed to the debugger.
Debugger can use `ptrace(PTRACE_CONT, trace_pid, nullptr, nullptr)`
to let traced process continue to system call.
Meanwhile, debugger can use:

- `ptrace(PTRACE_PEEKUSER)`
- `PEEKDATA`: memory data
- `PEEKTEXT`
- `PEEKSIGINFO`
- `GETREGS`: registers info

to get data of traced process,
`ptrace(PTRACE_POKEUSER)` or `POKEDATA` or `POKETEXT`
to set data of traced process.

```c
// Source: arch/i386/kernel/ptrace.c
if (request == PTRACE_TRACEME) {
    // are we already being traced?
    if (current->ptrace & PT_PTRACED) {
        goto out;
    }

    // set the ptrace bit in the process flags.
    current->ptrace |= PT_PTRACED;
    ret = 0;
    goto out;
}
```

## Breakpoints

When the processor executes the `int 3` instruction,
control is passed to the breakpoint interrupt handler,
which signals the process with a `SIGTRAP`.

Use `waitpid` to listen for signals which are sent to the debugee.
Set the breakpoint, continue the program,
call `waitpid` and wait until the `SIGTRAP` occurs.

Replace the instruction which is currently
at the given address with an int 3 instruction,
which is encoded as 0xcc to implement software breakpoints.

## Registers

- defination in `<sys/user.h>`
- ptrace api: `ptrace(PTRACE_GETREGS, pid, nullptr, &regs);`

## Reference

- [minidbg](https://github.com/sabertazimi/mdb)
