(define help "To see the gojira scheme tutorial, visit http://example.com. To see the currently defined variables, try (stacktrace).")

; Recursive factorial function
(define fact
  (lambda (x)
    (if (> x 0)
      (* x (fact (- x 1)))
      1)))

; Sequence function
(define seq
  (lambda (x)
    (+ x 1)))

(define not
  (lambda (x)
    (if (eq? x #f)
      #t
      #f)))

(define or
  (lambda (a b)
	(if a
	  #t
	  (if b
		#t
		#f))))

(define and
  (lambda (a b)
	(if a
	  (if b
		#t
		#f)
	  #f)))

(define <=
  (lambda (a b)
	(or
	  (< a b)
	  (eq? a b))))

(define caar
  (lambda (x)
	(car (car x))))

(define caaar
  (lambda (x)
	(car (caar x))))

(define print
  (lambda (x)
	(display x)
	(newline)))

; recursively counts down from a given number 
(define countdown
  (lambda (x)
    (display "T minus ")
	(print x)
    (if (eq? x 0)
      x
      (countdown (- x 1)))))

; repeatedly perform a function for "times", using recursion
(define for
  (lambda (times f)
    (if (> times 0)
	  (begin
        (for (- times 1) f)
		(f times))
	  times)))

; repeatedly perform a function for "times", using iteration
(define for-iter
  (lambda (times f)
    ((lambda (iter)
      (iter iter 1))

     (lambda (self count)
       (if (<= count times)
         (begin
           (f count)
           (self self (seq count)))
         count)))))

; Square a number
(define square
  (lambda (x)
    (* x x)))

; Sort of clear the terminal
(define clear
  (lambda ()
    (for
      100
      (lambda (x) (newline)))))

; print "x" number of squares
(define psquares
  (lambda (x)
    (for-iter
      x
      (lambda (y)
        (display y)
        (display ": ")
        (print (square y))))))

; print "x" number of factorials
(define pfacts
  (lambda (x)
    (for
      x
      (lambda (y)
		(print (fact y))))))

(define wut (fact 6))

; print every element in a list
(define asdf
  (lambda (x)
	(if (null? x)
	  x
	  (begin
		(print (car x))
		(asdf (cdr x))))))

; Calculate the sum of a function with inputs from 1 to n.
(define sum
  (lambda (n f)
    (if (> n 0)
      (+ (f n)
         (sum (- n 1) f))
      0)))

; The main function, used as the entry point
(define main
  (lambda ()
    ;(clear)
	(print "Hello, world!")

    (print "-== Factorial of 6:")
	(print wut)

    (print "-== Squares of numbers from 0 to 30:")
    (psquares 30)))

(main)