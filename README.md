# mdb

Minimal debugger

## Breakpoints

When the processor executes the `int 3` instruction,
control is passed to the breakpoint interrupt handler,
which signals the process with a `SIGTRAP`.

Use `waitpid` to listen for signals which are sent to the debugee.
Set the breakpoint, continue the program,
call `waitpid` and wait until the `SIGTRAP` occurs.

## Reference

- [minidbg](https://github.com/sabertazimi/mdb)
