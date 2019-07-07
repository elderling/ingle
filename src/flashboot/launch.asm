.export launch
.exportzp tmp2
.importzp s, d, c, tmp
.import popax
.import ultimem_offset2bank

bstart  = $2b       ; start of BASIC program text
bend    = $2d       ; end of basic program text
membot  = $282      ; start page of BASIC RAM
memtop  = $284      ; end page of BASIC RAM
screen  = $288      ; start page of text matrix

warmstt = $c7ae     ; BASIC warm start

.zeropage

tmp2:   .res 1

.code

.proc launch
    lda d
    sta tmp
    lda d+1
    sta tmp2

    ; Don't get interrupted.
    sei
    lda #$7f
    sta $911d
    sta $911e

    lda #0      ; Blank screen.
    sta $9002

    jsr copy_loaded_to_ram

    lda #$20
    sta $c2         ; I/O start addresses high byte.

    lda tmp2
    cmp #>$1000
    bne l7

    lda #>$1e00     ; screen
    ldx #>$1000     ; BASIC
    ldy #>$1e00     ; BASIC end
    bne l6

l7: bcs l5

    ; +3K
    lda #>$1e00     ; screen
    ldx #>$0400     ; BASIC
    ldy #>$1e00     ; BASIC end
    bne l6

    ; +24/32/35
l5: lda #$80
    sta $c2         ; I/O start addresses high byte.
    lda #>$1000     ; screen
    ldx #>$1200     ; BASIC
    ldy #>$8000     ; BASIC end

l6: sta screen
    stx membot
    sty memtop

    jsr $ff8a       ; initialize the KERNAL jump vectors
    jsr $fdf9       ; initialize the I/O chips
    jsr $e518       ; initialize the screen
    jsr $e45b       ; initialize jump vectors for BASIC
    jsr $e3a4       ; initialize zero page for BASIC
    lda bstart
    ldy bstart+1
    jsr $c408       ; check memory overlap
    jsr $c659       ; CLR

    ; Create trampoline in tape buffer.
    ldx #trampoline_end-trampoline-1
l8: lda trampoline,x
    sta $33c,x
    dex
    bpl l8

    ; Set up BLK1-BLK5.
    ldx #$01
    ldy #$00
    stx $9ff8
    sty $9ff9
    inx
    stx $9ffa
    sty $9ffb
    inx
    stx $9ffc
    sty $9ffd
    inx

    jmp $33c
trampoline:
    stx $9ffe
    sty $9fff

    ; Map RAM banks.
    lda #%11111111
    sta $9ff2

    lda #%10000000  ; Hide registers, LED off.
    sta $9ff0

    lda #>warmstt
    pha
    lda #<warmstt
    pha
    lda #0
    pha
    rti
trampoline_end:
.endproc

    ; Copy loaded data starting at bank 12 to RAM via BLK5.
.proc copy_loaded_to_ram
    ; Setup banks.
    lda #%00111111
    sta $9ff1
    lda #%01110111
    sta $9ff2
    ldy #0
    sty $9ff4   ; RAM1,2,3
    sty $9ff5
    sty $9ff6   ; IO2/IO3, so it does not have to happen in the trampoline.
    sty $9ff7
    lda #1
    sta $9ff8
    sty $9ff9
    ldy #$0a
    ldx #s
    jsr ultimem_offset2bank
    lda s+1
    ora #$40
    sta s+1

    ldy #0
    ldx c
    inx
    inc c+1
    jmp l

l4: inc $9ffa
    lda #>$4000
    sta s+1
l:  lda (s),y
    sta (d),y
    inc d
    beq d1
l2: inc s
    beq d2
l3: dex
    bne l
    dec c+1
    bne l

    rts

d1: inc d+1
    lda d+1
    cmp #>$4000
    bne l2
    inc $9ff8
    lda #>$2000
    sta d+1
    bne l2  ; (jmp)

d2: inc s+1
    lda s+1
    cmp #>$6000
    beq l4
    bne l3  ; (jmp)
.endproc
