; naskfunc
; TAB=4

[FORMAT "WCOFF"]				;	
[INSTRSET "i486p"]				; tell nask that the program is for 486, so nask will regards EAX a register rather than a label
[BITS 32]						; 
[FILE "naskfunc.nas"]			; 

		GLOBAL	_io_hlt,_write_mem8

[SECTION .text]

_io_hlt:	; void io_hlt(void);
		HLT
		RET

_write_mem8:	; void write_mem8(int addr, int data);
		MOV		ECX,[ESP+4]		; [ESP+4]saves address��and read it into ECX
		MOV		AL,[ESP+8]		; [ESP+8]saves data��and read it into AL
		MOV		[ECX],AL
		RET
