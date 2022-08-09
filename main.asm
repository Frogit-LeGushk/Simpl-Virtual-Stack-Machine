;
; - program on low level language for virtual machine
; for calculation factorials
; - stack item == 32 bit
;

label begin
    push 10      ; n! == initial value
    call fact
    jmp end
label end
    pop         ; read last value from stack into return value in virtual machine
    exit        ; exit program

label fact      ; wrapper factorial function
    load $-2
    push 2
    load $-2
    jl _close

label _fact     ; recursive factorial inner function
    push 1
    load $-2
    sub
    store $-2 $-1
    pop
    load $-3
    load $-2
    mul
    store $-4 $-1
    pop
    push 1
    load $-2
    jg _fact

label _close    ; close from wrapper factorial function
    pop
    ret





