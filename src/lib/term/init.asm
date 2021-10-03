.export _term_init, _term_put, _term_puts

.import init_bitmap_mode, gfx_init
.import clear_screen
.import box
.import putchar_fixed
.import _pattern_solid
.import init_region_stack
.import reset_region
.import moveram
.import charset, charset_size

.importzp s, d, c, scrbase
.importzp font, pencil_mode, pattern
.importzp xpos, ypos, width, height
.importzp screen_width, screen_height, screen_columns

    .zeropage

tmp:            .res 1
tmp2:           .res 1
p:              .res 2
cursor_x:       .res 1
cursor_y:       .res 1
has_cursor:     .res 1
visible_cursor: .res 1

    .code

.proc _term_init
    jsr clear_screen
    lda #1
    ldx #0
    ldy #0
    jsr init_bitmap_mode
    lda #$00
    sta scrbase
    lda #$11
    sta scrbase+1

    jsr init_region_stack
    jsr reset_region

    lda #<our_charset
    sta font
    sta p
    lda #>our_charset
    sta font+1
    sta p+1

    ; Double each char's half.
    ldy #0
l2: lda (p),y
    sta tmp
    asl
    asl
    asl
    asl
    ora tmp
    sta (p),y
    iny
    bne l2
    inc p+1
    lda p+1
    cmp #>our_charset + 8
    bne l2

    ldy #0
    sty cursor_x
    sty cursor_y
    sty visible_cursor
    iny
    sty has_cursor
    jmp cursor_enable
.endproc

.proc cursor_draw
    lda has_cursor
    beq r

    lda cursor_x
    sta xpos
    lda cursor_y
    sta ypos
    lda #<_pattern_solid
    sta pattern
    lda #>_pattern_solid
    sta pattern+1
    lda #2
    sta pencil_mode
    asl
    sta width
    asl
    sta height
    jmp box

r:  rts
.endproc

.proc cursor_enable
    pha
    lda visible_cursor
    inc visible_cursor
    ora #0
    bne r
    jsr cursor_draw
r:  pla
    rts
.endproc

.proc cursor_disable
    pha
    dec visible_cursor
    lda visible_cursor
    bne r
    jsr cursor_draw
r:  pla
    rts
.endproc

.proc cursor_step
    lda cursor_x
    clc
    adc #4
    sta cursor_x

    cmp #160
    bne n

    lda #0
    sta cursor_x
    jmp cursor_down

n:  rts
.endproc

.proc cursor_down
    lda cursor_y
    cmp #screen_height-8
    bne n

    jsr scroll_up
    rts

n:  clc
    adc #8
    sta cursor_y
    
    rts
.endproc

.proc line_break
    lda #0
    sta cursor_x
    jmp cursor_down
.endproc

.proc set_cursor_pos
    pha
    lda cursor_x
    sta xpos
    lda cursor_y
    sta ypos
    pla
    rts
.endproc

.proc _term_put
    jsr cursor_disable
    cmp #13
    bne n
    jsr line_break
    jmp r

n:  pha
    pha
    jsr set_cursor_pos
    pla
    jsr putchar_fixed
    jsr cursor_step
    pla
r:  jmp cursor_enable
.endproc

.proc putstring_fixed
    jsr cursor_disable

l:  ldy #0
    lda (p),y
    beq r

    jsr _term_put

next:
    inc p
    bne l
    inc p+1
    jmp l   ; (bne)

r:  jmp cursor_enable
.endproc

.proc _term_puts
    sta p
    stx p+1
    jmp putstring_fixed
.endproc

.proc scroll_up
    lda #<charset
    sta s
    lda #>charset
    sta s+1

    ldx #screen_columns
m:  ldy #7
    lda #0
l:  sta (s),y
    dey
    bpl l

    lda s
    clc
    adc #screen_height
    sta s
    bcc n
    inc s+1
n:  dex
    bne m

    lda #<(charset + 8)
    sta s
    lda #<charset
    sta d
    lda #>charset
    sta s+1
    sta d+1
    lda #<(charset_size - 8)
    sta c
    lda #>(charset_size - 8)
    sta c+1
    lda #0
    jsr moveram

    ldx #7
    lda #0
l2: sta charset + (screen_height * screen_columns) - 8,x
    dex
    bpl l2

    rts
.endproc

    .data
    .align 256

our_charset:
    .include "charset-4x8.asm"
