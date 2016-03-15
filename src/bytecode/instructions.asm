; Call system function without argument mapping.
apply:
    lda (sp),y
    tax
    dex
    jsr inc_sp
    lda syscall_vectors_l,x
    sta @(+ 1 +mod_call)
    lda syscall_vectors_h,x
    sta @(+ 2 +mod_call)
mod_call:
    jmp $ffff

addx:
    lda tmp
    clc
    adc xpos
    sta xpos
    rts

addy:
    lda tmp
    clc
    adc ypos
    sta ypos
    rts

setzb:
    ldx tmp
    lda tmp2
    sta 0,x
    rts

setzw:
    ldx tmp
    lda tmp2
    sta 0,x
    lda tmp3
    sta 1,x
    rts

addzb:
    ldx tmp2
    ldy tmp3
    lda 0,x
    clc
    adc 0,y
    ldx tmp
    sta 0,x
    rts

sbczb:
    ldx tmp2
    ldy tmp3
    lda 0,x
    sec
    sbc 0,y
    ldx tmp
    sta 0,x
    rts

sbczbi:
    ldx tmp
    lda 0,x
    sec
    sbc tmp2
    sta 0,x
    rts

setpattern:
    rts

setzs:
    sty @(++ d)
l:  lda (sp),y
    sta (d),y
    jsr inc_sp
    inc d
    bne +n
    inc @(++ d)
n:  dec tmp
    bne -l
    rts

pushz:
    pla
    pla
    ldx tmp
    ldy tmp2
l:  lda 0,x
    inx
    pha
    dey
    bne -l
    jmp next_bytecode

popz:
    pla
    pla
    ldy tmp2
    tya
    clc
    adc tmp
    tax
l:  pla
    dex
    sta 0,x
    dey
    bne -l
    jmp next_bytecode