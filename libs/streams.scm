(define (memo-proc proc)
  (define :mut already-run? #f)
  (define :mut result #f)

  (lambda ()
    (if (not already-run?)
      (begin
        (set! result (proc))
        (set! already-run? #t)
        result)
      result)))

(define-syntax delay
  (syntax-rules ()
    ((_ expr)
     (memo-proc
       (lambda () expr)))))

(define (force delayed)
  (delayed))

(define-syntax cons-stream
  (syntax-rules ()
    ((_ a b)
     (list a (delay b)))))

(define stream-cons cons-stream)

(define stream-car car)
(define (stream-cdr str)
  (force (cadr str)))

(define (list->stream xs)
  (if (null? xs)
    '()
    (cons-stream (car xs)
                 (list->stream (cdr xs)))))

(define (stream :rest xs)
  (list->stream xs))

(define (stream-length str)
  (if (stream-null? str)
    0
    (+ 1
       (stream-length (stream-cdr str)))))

(define (stream-head str k)
  (if (stream-null? str)
    '()
    (if (> k 0)
      (cons (stream-car str)
            (stream-head (stream-cdr str) (- k 1)))
      '())))

(define (stream-tail str k)
  (if (stream-null? str)
    '()
    (if (> k 0)
      (stream-tail (stream-cdr str) (- k 1))
      str)))

(define (stream-ref str k)
  (stream-car (stream-tail str k)))

; TODO: support variable length args
(define (stream-map proc args)
  (if (stream-null? args)
    '()
    (cons-stream (proc (stream-car args))
                 (stream-map proc (stream-cdr args)))))

(define (stream-for-each proc str)
  (if (stream-null? str)
    '()
    (begin
      (proc (stream-car str))
      (stream-for-each proc (stream-cdr str)))))

(define empty-stream? null?)
(define stream-null? null?)

(define (integers n)
  (cons-stream n (integers (+ n 1))))

(define (string->stream-at str k)
  (if (< k (string-length str))
    (cons-stream (string-ref str k) (string->stream-at str (+ k 1)))
    '()))

(define (string->stream str)
  (string->stream-at str 0))
