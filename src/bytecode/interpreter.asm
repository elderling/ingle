num_args:   0

inc_sp:
    inc sp
    bne +n
    inc @(++ sp)
n:  rts

inc_sa:
    inc sa
    bne +n
    inc @(++ sa)
n:  rts

exec_script:
    pla ; Y
    pla ; X
    sta srx
    pla ; A
    plp ; flags
    pla
    sta sp
    pla
    sta @(++ sp)
    lda sp
    sec
    sbc #1
    sta sp
    bcs +n
    dec @(++ sp)
n:

next_bytecode:
    ; Get opcode.
    ldy #0
    lda (sp),y
    beq +done   ; Return to native code…

    ; Get pointer to argument info.
    tax
    dex
    lda syscall_vectors_l,x
    sta @(+ 1 +mod_call)
    lda syscall_vectors_h,x
    sta @(+ 2 +mod_call)
    lda syscall_args_l,x
    sta sa
    lda syscall_args_h,x
    sta @(++ sa)

    ; Get number of arguments.
    lda (sa),y
    sta num_args

    ; Copy arguments to zero page.
next_arg:
    jsr inc_sp
    dec num_args
    bmi script_call

    jsr inc_sa
    lda (sa),y      ; Get zero page address from argument info.
    tax
    lda (sp),y      ; Copy in argument value.
    sta 0,x

    jmp next_arg

script_call:
mod_call:
    jsr $ffff
    jmp next_bytecode

    ; Return to native code.
done:
    lda @(++ sp)
    pha
    lda sp
    pha
    ldx srx
    rts