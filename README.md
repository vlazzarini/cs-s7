# Csound s7 

This is the repository for the Csound s7 project, which combines the s7 scheme
interpreter (https://ccrma.stanford.edu/software/snd/snd/s7.html) and the
Csound Sound and Music Computing System (https://csound.com). 

A simple performance run example:

```
(define cs (make-csound))
(compile-csound "code.csd")
(start-csound cs)
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

are used to build the `cs-s7` program.

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
(compile-csound csound-obj filename)
```

compiles Csound code from file.

```
(start-csound csound-obj)
```

starts the Csound engine and performance.

```
(stop-csound csound-obj)
```

stops a Csound performance.

```
(pause-csound csound-obj)
```

toggles-pause a Csound performance.

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

sets the value of a bus channel


```
(csound-get-channel csound-obj channel)
```

gets the value of a bus channel

```
(csound-time cs)
```

returns a list with the current performance time in seconds and in sample frames.

## Embedding

The cs-s7 interface can be embedded in other s7 applications. In the
original spirit of s7, this is provided as a single source/header
pair, cs-s7.c and cs-s7.h. Applications still need to be linked to the
Csound library. With the interface, the Csound functions described
above are all made available to the interpreter.

VL, April 2025.
