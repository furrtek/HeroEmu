#define __SFR_OFFSET 0
#include <avr/io.h>
 
;extern void output_grb(u8 * ptr, u16 count, u8 outbit)
;
; r18 = data byte
; r19 = 7-bit count
; r20 = 1 output
; r21 = 0 output
; r22 = SREG save
; r24:25 = 16-bit count
; r26:27 (X) = data pointer
 
.equ      OUTBIT,   0
 
 
.global output_grb
output_grb:
         movw   r26, r24      ;r26:27 = X = p_buf
         movw   r24, r22      ;r24:25 = count
         in     r22, SREG     ;save SREG (global int state)
         cli                  ;no interrupts from here on, we're cycle-counting
		 mov    r30, r20
         in     r18, PORTB
         or     r20, r18	  ;our '1' output
		 ldi    r21, 255
		 eor    r30, r21
         and    r30, r18      ;our '0' output
		 mov    r21, r30
         ldi    r19, 7        ;7 bit counter (8th bit is different)
         ld     r18,X+        ;get first data byte
loop1:
         out    PORTB, r20    ; 1   +0 start off a bit pulse
         lsl    r18           ; 1   +1 next bit into C, MSB first
         brcs   L1            ; 1/2 +2 branch if 1
         out    PORTB, r21    ; 1   +3 end hi for '0' bit (3 clocks hi)
         nop                  ; 1   +4
         bst    r18, 7        ; 1   +5 save last bit of data for fast branching
         subi   r19, 1        ; 1   +6 how many more bits for this byte?
         breq   bit8          ; 1/2 +7 last bit, do differently
         rjmp   loop1         ; 2   +8, 10 total for 0 bit
L1:
         nop                  ; 1   +4
         bst    r18, 7        ; 1   +5 save last bit of data for fast branching
         subi   r19, 1        ; 1   +6 how many more bits for this byte
         out    PORTB, r21    ; 1   +7 end hi for '1' bit (7 clocks hi)
         brne   loop1         ; 2/1 +8 10 total for 1 bit (fall thru if last bit)
bit8:
         ldi    r19, 7        ; 1   +9 bit count for next byte
         out    PORTB, r20    ; 1   +0 start of a bit pulse
         brts   L2            ; 1/2 +1 branch if last bit is a 1
         nop                  ; 1   +2
         out    PORTB, r21    ; 1   +3 end hi for '0' bit (3 clocks hi)
         ld     r18, X+       ; 2   +4 fetch next byte
         sbiw   r24, 1        ; 2   +6 dec byte counter
         brne   loop1         ; 2   +8 loop back or return
         out    SREG, r22     ; restore global int flag
         ret
L2:
         ld     r18, X+       ; 2   +3 fetch next byte
         sbiw   r24, 1        ; 2   +5 dec byte counter
         out    PORTB, r21    ; 1   +7 end hi for '1' bit (7 clocks hi)
         brne   loop1         ; 2   +8 loop back or return
         out    SREG, r22     ; restore global int flag
         ret
