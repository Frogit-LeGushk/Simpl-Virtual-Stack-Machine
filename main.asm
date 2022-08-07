label begin
    push 10
    call fact
    jmp end
label end
    pop
    exit
label fact
    load $-2
    push 2
    load $-2
    jl _close
label _fact
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
label _close
    pop
    ret





