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
dylib,dll}`. 
If libtecla is installed, the `cs-s7` REPL will use it for enhanced command editing.

## Running

To run the interpreter from the build directory,

```
./cs-s7
cs-s7: Csound S7 scheme interpreter
cs-s7>
```

### Functions

```
(make-csound)
```

creates a new Csound engine object.


```
(csound-compile csound-obj csdfile)
```

compiles Csound code from CSD file.

```
(csound-options csound-obj options-str)
```

sets engine options from a string before engine startup, non-op on a
running engine.

```
(csound-start csound-obj (sync 0))
```

starts the Csound engine and performance thread, optionally
synchronously `:sync 1`.

```
(csound-stop csound-obj)
```

stops a Csound performance thread and resets the engine.

```
(csound-pause csound-obj)
```

toggles-pause/play a Csound performance.

```
(csound-is-asynchronous csound-obj)
```

returns the status of asynchronous performance.

```
(csound-compile-string csound-obj code-string)
```

compiles Csound code from a string before or after engine startup.


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

sets the value of a bus channel

```
(csound-get-channel csound-obj channel)
```

gets the value of a bus channel

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
(define code "instr 1
               sig:a = oscili(p4, p5)
               out(linen(sig,0.1,p3,0.1))
              endin
              schedule(1,0,1,0.5,440)
             ")
(csound-compile-string cs code)
```

## Opcodes

The cs-s7 interface also adds opcodes for accessing the interpreter
from Csound code.

```
res:i = s7eval(code:S)
res:k = s7eval(code:S)
```

evaluates a code string, returning the result (real or integer results
only), at init and perf-times. Note that perf-time code executes at
every k-cycle, so it may need to be protected with flow-control.

```
s7definevar(var:s, value:i)
s7definevar(var:s, value:k)
```

defines a variable with a given value, similar comments apply here.

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
#t
cs-s7> instr 2:	#i0 = 6.000
```

These opcodes can also be used in other Csound applications (beyond
cs-s7) by loading the plugin library `libcss7.{so,dylib,dll}`
(e.g. with `--opcode-lib=`). In this case, the library module starts
an internal s7 interpreter, which is used by the opcodes.

Other opcodes will follow...

## Embedding

The cs-s7 interface can be embedded in other s7 applications. In the
original spirit of s7, this is provided as a single source/header
pair, cs-s7.c and cs-s7.h. Applications still need to be linked to the
Csound library. With the interface, the Csound functions described
above are all made available to the interpreter.

VL, April 2025.
