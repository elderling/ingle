# g – an operating system for the Commodore VIC–20 with Ultimem expansion.

This is a work in progress aiming for

* 28Kb directly addressable RAM for programs
* shared libraries
* pre-emptive multitasking

without requiring special tools to make applications.

A compiled version can be downloaded from
http://hugbox.org/pixel/external/denial/g.zip
A version of the cc65 C compiler with "vic20g" target is to be found at
https://github.com/SvenMichaelKlose/cc65g

## Building g

```
make
```

## Status

Development of g is currently limited to src/flashboot and src/flashmenu.

## How it works.

First, let's take a look at the memory map:

0000    Core zero page
0020    Application
0090    KERNAL zero page
0100    Stack
0200    KERNAL area
0400    "load" system call entry point
04xx    Core data per application
1000    Screen
1100    Character set
2000    Application
8000    Character ROM
9000    VIC area
9800    Application
a000    Application
c000    BASIC
e000    KERNAL

g's core is the kernel.  It resides at memory block 5 ($a000-$bfff) and
cannot be occupied by anything else.  Also, the +3K area is reserved for
process-related data and jump tables generated by the linker.  RAM bank 0
contains global data like process lists. It is also mapped to the +3K area
when needed.

When g switches to another process, it saves the stack, zero page, registers
and bank configuration to the process' core data in the +3K area and does
the reverse for the process being switched to.

## Program format

g automatically loads "sh" which you can replace by your own application
to fool around with.

```
load address (2 bytes)
program size (2 bytes)
data size (2 bytes)
program code
```

The load address does NOT include itself or sizes.

## Library format

Libraries are programs that export funtions to other programs by name.
addresses.  Libraries have the following format:

```
symbol index size (2 bytes)
symbol index
load address (2 bytes)
library size (2 bytes)
library code
```

The symbol index is trivial.  Keep in mind that the function names
are ASCIIZ strings.

```
    .word index_size    ; Does NOT include these two bytes.
    .asciiz "call1"
    .word call1
    .asciiz "call2"
    .word call2
    .byte 0             ; End of index.
```

## System calls

Programs need to request jump tables via the function at address $0400.

### Errors

System calls that fail return with the carry flag set and an error
code in the A register.

### Core zero page locations

#### $0000: s – Source pointer
#### $0002: d – Destination pointer
#### $0004: c – Counter/extra pointer
#### $0006–$0010: Reserved

### Process management
#### $0400 – Link process to core or a library.
```
s: ASCIIZ library path followed zero–terminated list of ASCIIZ system call names.   Pass "/g" to link to core functions.
d: Jump table address.
```

"link" loads the library and creates a jump table to it.

This function also creates trampline procedures in the +3K core
bank of the process which must be switched in before making a
call.

A dedicated +3K bank will be allocated.


#### "launch" – Launch program on file system.
```
A: If not 0 the parent process is halted until the launched process is killed.
s: Program name (ASCIIZ string).

Returns:
A: Process ID.
```

Loads a program and runs it independently of the invoking task.


#### "fork" – Create child process.

#### "halt" – Halt a process.
```
A: Process ID.
```

Can be resumed with "resume".

#### "resume" – Resumes a halted process.
```
A: Process ID.
```

#### "kill" – Kill a process and its libraries
```
A: Process ID.
```

Does not return if process killed itself.

### Strings
#### "inc_s" – Increment pointer s.

Increments zero page pointer "s".

#### "inc_d" – Increment pointer d.

Increments zero page pointer "d".

#### "strcmp" – Compare zero–terminated strings at s and d.
```
s:  String A.
d:  String A.

Returns:
Zero flag clear if strings match.
```

### Memory
#### "alloc_bank" – Allocate memory bank.
```
Returns:
tmp: Allocated bank number.
```

#### "free_bank" – Free memory bank.
```
A: Bank number
```

#### "moveram" – Move memory block.
```
A:  0 = forward, 1 = backward
s: Source address
d: Destination address
c: Number of bytes
```

#### "clrram" – Clear memory block.
```
d: Destination address
c: Number of bytes
```

### File I/O
#### "create" – Create file.
#### "mount" – Mount process to directory.
#### "mknode" – Mount process to virtual file.
#### "remove" – Remove file or node.
#### "open" – Open file.
#### "setin" – Set input channel.
#### "setout" – Set output channel.
#### "chkin" – Check input channel data availability.
#### "read" – Read byte.
#### "readw" – Read word.
#### "readn" – Read multiple bytes.
#### "readm" – Read block of unknown size.
#### "load" – Like "readm" but also allocating memory banks.
#### "write" – Write byte.
#### "writew" – Write word.
#### "writen" – Write multiple bytes.
#### "writem" – Write block starting with size.
#### "close" – Close file.

### Directories
#### "cd" – Change working directory.
#### "mkdir" – Make directory.
#### "rmdir" – Remove directory.

### Timer
#### "settimer" – Set timer.
#### "cleartimer" – Clear timer.

### Multitasking control
#### "take_over" – Stop multitasking.

Turns off multitasking and restores the NMI vector.  Use this before doing
time–critical operations or calling KERNAL functions.

Calls to "take_over" may happen multiple times.

#### "release" – Continue multitasking.

Turns on multitasking and diverts the NMI vector to the task switcher.
Flags and registers are not affected.

"release" has to be called as often as "take_over" before multitasking is
actually turned back on.

When multitasking has been switched back on and the system tried to switch
while it was turned off "release" will do a task switch immediately, so
processes cannot block the system by accident.

## File system

To support multitasking, g maintains a set of file descriptors for each
process.  Each file descriptor points to a unique vfile that can be
shared by processes and which hides file system specific implementations.

vfiles are created when a file is opened, and it is removed when it has
been closed by all processes that reference it.  Only the vfile for the
root directory is never removed.

### Directories

Directories are always read in completely and saved as lists of dirent
structures in a dedicated memory bank.