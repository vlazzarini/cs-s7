(define cs (make-csound))
(csound-options cs "--0dbfs=1")
(csound-start cs async: #f)
(define code "instr s7test
 s7definevar("x", 1)
 res:i = s7eval("(+ x 2)")
 print(res)
endin
schedule(s7test,0,0)
event_i("e",0,1)
             ")
(csound-compile-string cs code)
(define (run-cs p) (cond ((not (= 0 p)) p)
                         (else
                          (run-cs
                           (csound-perform-ksmps cs)
                           ))))
(run-cs 0)
(exit)
   
