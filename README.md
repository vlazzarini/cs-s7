# Csound s7 

This is the repository for the Csound s7 project, which combines the s7 scheme
interpreter (https://ccrma.stanford.edu/software/snd/snd/s7.html) and the
Csound Sound and Music Computing System (https://csound.com). 

The project consists of two components

- The `cs-s7` interpreter program - provides a REPL with functions and
  opcodes to interface Csound with s7 (and vice-versa).
- The `libcss7` plugin module - provides an s7 interpreter accessible
  by Csound through a library of opcodes.

## Building

CMake is used for building the project. With Csound installed,
the commands

```
mkdir build
cd build
cmake ..
make
```

are used to build the `cs-s7` program and the `libcss7.{so,
dylib,dll}`. The REPL is built by default with notcurses if
installed (to turn this option off, pass `-DUSE_NOTCURSES=OFF`
to CMake). Otherwise, lf libtecla is installed, the REPL will use it for enhanced
command editing. The build falls back on the vanilla REPL with no
special editing capabilities.

## Running

To run the interpreter from the build directory,

```
./cs-s7
cs-s7: Csound S7 scheme interpreter
cs-s7>
```

The command takes arguments in the form of files to be loaded
or scheme expressions to be evaluated,

```
cs-s7 [file.scm] [-e "scheme-expression"] [-q]
```

any number of arguments in these forms are accepted. The `-q` flag
makes the interpreter quit after running the command-line. Without it,
the REPL is launched.

## Functions

```
(make-csound)
```

creates a new Csound engine object. Realtime audio is enabled by
default, but this can be changed by options set before the engine is
started. Multiple Csound objects may be created.

```
(csound-compile csound-obj csdfile)
```

compiles Csound code from CSD file. The <CsOptions> section is only 
relevant before the engine is started.

```
(csound-options csound-obj options-str)
```

sets engine options from a string before engine startup, non-op on a
running engine.

```
(csound-start csound-obj (async #t))
```

starts the Csound engine, asynchronously by default. 
In synchronous performance, processing needs to be run by calling
`(csound-perform-ksmps csound-obj)` (see below); 
in asynchronous performance this
is run in a separate thread loop, which starts immediately.

```
(csound-stop csound-obj)
```

stops a Csound and resets the engine. Also re-enables realtime audio
as default. The performance may be restarted from this point.

```
(csound-pause csound-obj)
```

toggles-pause/play a Csound performance (asynchronous mode)

```
(csound-paused? csound-obj)
```

returns the performance status.

```
(csound-async? csound-obj)
```

returns whether performance is asynchronous or not.

```
(csound-compile-string csound-obj code-string)
```

compiles Csound code from a string.

```
(csound-event csound-obj type p1 p2 p3 ...)
```

sends event of type (0  = instr, 1 = ftable, 2 = end) with pfields p1,
p2, p3 ...

```
(csound-event-string csound-obj evt-string)
```

sends an event defined as a string.


```
(csound-set-channel csound-obj channel val)
```

sets the value of a (control) bus channel.

```
(csound-get-channel csound-obj channel)
```

gets the value of a (control) bus channel.

```
(csound-perform-ksmps cs)
```

performs one ksmps-full of sample frames, synchronously. Non-op on
asynchronous performance.

```
(csound-time cs)
```

returns a list with the current performance time in seconds and in
sample frames.

### Example

A typical set of REPL commands to play a CSD from a file should be

```
cs-s7>(define cs (make-csound))
cs-s7>(csound-compile cs "test.csd")
cs-s7>(csound-start cs)
```

Realtime output is enforced by default (`-odac`) but this can be
overriden by options set in the CSD or with the appropriate function.
Processing is done by default in a separate thread (asynchronous) and so the REPL is
continuously available to accept input. Csound messages are
sent to stderr, unless suppressed, or redirected. If stderr is the terminal, then
they will be shown together with the REPL printouts. One possibility
is to redirect the stderr to a log file or console. On MacOS, we can
start the interpreter with the command

```
cs-s7 2> ~/tmp/cs.log
```

and open the log file in the console (e.g. using another terminal window)

```
open /System/Applications/Utilities/Console.app ~/tmp/cs.log
```

this will log all Csound messages, which will not get in the way of
the REPL. Of course we can always load s7 code from files, which
can be formatted more conveniently etc. This is particularlly useful
for multiline Csound code strings. For example, the following
program starts csound and then compiles code from a string,

```
(define cs (make-csound))
(csound-options cs "--0dbfs=1")
(csound-start cs)
(define code "
   instr 1
    sig:a = oscili(p4, p5)
    out(linen(sig,0.1,p3,0.1))
   endin
   schedule(1,0,1,0.5,440)
             ")
(csound-compile-string cs code)
```

Unfortunately, due to line continuation, this code cannot be run in
the basic `cs-s7` REPL, as each command needs to terminate on a
line end. However, if built using notcurses, there is 
multiline support.

Alternatively, s7 provides a handy scheme-based 
REPL in repl.scm. With this and some other s7 files, we can run a 
more fully-featured REPL by loading the repl.scm in the command 
line and starting it

```
./cs-s7 ./cs-s7 repl.scm  -e "((*repl* (quote run)))"
loading libc_s7.so
<1> 
```

The `cs-s7` command in this case needs to be run from the s7 sources directory
so all the relevant files can be found. The REPL will then allow 
multine strings etc as well as other facilities such as command
completion. See the s7 manual for more information.

## Opcodes

The cs-s7 interface also adds opcodes for accessing the interpreter
from Csound code.

```
res:i = s7eval(code:S)
res:k = s7eval(code:S)
```

evaluates a code string, returning the result (real or integer results
only).  Perf-time code executes at every k-cycle , so it may need to be protected with flow-control.
Also note that k-var opcodes do not execute at i-time.

```
s7definevar(var:s, value:i)
s7definevar(var:s, value:k)
```

defines a variable with a given value, similar comments apply here.

The opcode module also defines a new type for s7 objects, `S7obj` (the
convention is that new types should start with a capital letter). To
manipulate this type we have

```
s7definevar(var:S, obj:S7obj)
obj:S7obj = s7real(value:i)
obj:S7obj = s7real(value:k)
value:i = s7real(obj:S7obj)
value:k = s7real(obj:S7obj)
obj:S7obj = s7eval(code:S)
obj:S7obj = s7car(obj:S7obj)
obj:S7obj = s7cdr(obj:S7obj)
```

I-time and k-rate opcodes execute at i- and perf-time respectively.
S7obj opcodes run at both i-pass and every k-cycle. If for some
reason, the obj is null, then the opcode does not call the
respective s7 function and either the result is a null (in case of
S7obj) or just 0 (if it is an i or k-var).

### Example

Here is a trivial example

```
instr s7test
 s7definevar("x", 1)
 res:i = s7eval("(+ x 2)")
 print(res)
endin
schedule(s7test,0,0)
```

which should print

```
new alloc (instance 1) for instr s7test:
cs-s7> instr 1:	res = 3.000
```

These opcodes also allow a direct connection between the interpreter
and Csound code. For example, if we compile this instrument

```
instr 2
 res:i = s7eval("(+ x 2)")
 print(res)
endin
```

we can use the value of a variable `x` defined in the interpreter,

```
cs-s7> (define x 4)
4
cs-s7> (csound-event cs 0 2 0 0)
(0 2 0 0)
cs-s7> instr 2:	#i0 = 6.000
```

## Plugin Library

The cs-s7 opcodes can also be used in other Csound applications (beyond
the `cs-s7` program) by loading the plugin library `libcss7.{so,dylib,dll}`
(e.g. with `--opcode-lib=`). In this case, the library module starts
an internal s7 interpreter, which is used by the opcodes. The opcode
library returns a non-fatal error if attempted to be loaded in a `cs-s7`
REPL session as the opcodes are already present in that case.

## Embedding

The cs-s7 interface can be embedded in other s7 applications. In the
original spirit of s7, this is provided as a single source/header
pair, cs-s7.c and cs-s7.h. Applications still need to be linked to the
Csound library. With the interface, the Csound functions described
above are all made available to the interpreter.

VL, April 2025.
