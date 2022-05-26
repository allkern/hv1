.def MY_NEG -1

.org 0x8000

._start:
    li      %gp0, 0x0002

.loop:
    adds    %gp0, .MY_NEG
    bne     .loop
    b       ._start