;defun-test

; canlisp: consp がない.
(defun my-consp (x)
  (if (not (null x)) t))

(defun add-element (x y)
  (if (and (my-consp x) (my-consp y))
    (cons (+ (car x) (car y))
          (add-element (cdr x) (cdr y)))))
