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
