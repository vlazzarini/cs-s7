# Csound s7 

This is the repository for the Csound s7 project, which combines the s7 scheme
interpreter (https://ccrma.stanford.edu/software/snd/snd/s7.html) and the
Csound Sound and Music Computing System (https://csound.com). 

A simple performance run example:

```
(define cs (make-csound))
(csound-compile "code.csd")
(csound-start cs)
```

In this example, Csound runs on a separate thread, making the above
ideal for REPL use.

## Building

CMake is used for building the cs-s7 interpreter. With Csound installed,
the commands

```
mkdir build
cd build
cmake ..
make
```

are used to build the `cs-s7` program. If libtecla is installed, the
interpreter will use it for enhanced command editing.

## Running

To run the interpreter from the build directory,

```
./cs-s7
cs-s7: Csound S7 scheme interpreter
cs-s7>
```

## Functions

```
(make-csound)
```

creates a new Csound engine object.


```
(csound-compile csound-obj filename)
```

compiles Csound code from file before engine startup, non-op on a
running engine.

```
(csound-options csound-obj options-str)
```

sets engine options from a string before engine startup, non-op on a
running engine.

```
(csound-start csound-obj)
```

starts the Csound engine and performance.

```
(csound-stop csound-obj)
```

stops a Csound performance and resets the engine.

```
(csound-pause csound-obj)
```

toggles-pause a Csound performance.

```
(csound-play csound-obj)
```

re-starts a Csound performance from pause.

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
(csound-time cs)
```

returns a list with the current performance time in seconds and in
sample frames.

## Opcodes

The cs-s7 interface also adds opcodes for accessing the interpreter
from Csound code.

```
res:i = s7eval(code:S)
```

evaluates a code string, returning the result (real or integer results only)

```
s7definevar(var:s, value:i)
```

defines a variable with a given value.

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
