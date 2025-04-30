;;; simple example
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




