# Csound S7 

This is the repository for the Csound S7 project, which combines the S7 scheme
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
(compile-csound csound-obj string)
```

compiles Csound code from file `string`.

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

... more on the way!
