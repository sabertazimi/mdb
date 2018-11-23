# mdb

[![Build Status](https://travis-ci.org/sabertazimi/mdb.svg?branch=master)](https://travis-ci.org/sabertazimi/mdb)

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

- register address/value in `<sys/user.h>`: `struct user_regs_struct`
- ptrace api: `ptrace(PTRACE_GETREGS, pid, nullptr, &regs);`

## DWARF Debugging Format

- [ELF Format](http://www.skyfree.org/linux/references/ELF_Format.pdf)
- [DWARF Format](http://www.dwarfstd.org/doc/Debugging%20using%20DWARF-2012.pdf)

DWARF is a widely used, standardized debugging data format.
DWARF was originally designed along with Executable and Linkable Format (ELF),
although it is independent of object file formats.

- .debug_abbrev Abbreviations used in the .debug_info section
- .debug_aranges A mapping between memory address and compilation
- .debug_frame Call Frame Information
- .debug_info The core DWARF data containing DWARF Information Entries (DIEs)
- .debug_line Line Number Program
- .debug_loc Location descriptions
- .debug_macinfo Macro descriptions
- .debug_pubnames A lookup table for global objects and functions
- .debug_pubtypes A lookup table for global types
- .debug_ranges Address ranges referenced by DIEs
- .debug_str String table used by .debug_info
- .debug_types Type descriptions

```bash
# compile with `-g` flag
dwarfdump a.out
```

## Reference

- [minidbg](https://github.com/sabertazimi/mdb)
