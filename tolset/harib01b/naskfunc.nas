; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; the format for making objective file and like .out 
[INSTRSET "i486p"]				; tell nask that the program is for 486, so nask will regards EAX a register rather than a label
[BITS 32]						; make machine language used for 32-bit mode
[FILE "naskfunc.nas"]			; the file name of source code

		GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt                                      ;_write_mem8
		GLOBAL	_io_in8, _io_in16, _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags
		
[SECTION .text]

_io_hlt:	; void io_hlt(void);
		HLT
		RET

_io_cli:	; void io_cli(void);		
		CLI
		RET
		
_io_sti:	; void io_sti(void);
		STI
		RET

_io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET
	
_io_in8:	; int io_in8(int port);
		MOV		EDX, [ESP+4]	; port
		MOV		EAX, 0
		IN		AL, DX			;  <*_*>
		RET
		
_io_in16:	; int io_in16(int port);
		MOV		EDX, [ESP+4]	;port
		MOV 	EAX, 0
		IN		AX, DX
		RET
		
_io_in32:	; int io_in32(int port);
		MOV		EDX, [ESP+4]	; port	
		IN		EAX, DX			; port is 16-bit(0x03c8), so we can use DX
		RET
		
_io_out8:	; void io_out8(int port, int data);
		MOV		EDX, [ESP+4]	; port
		MOV 	AL, [ESP+8]		; port
		OUT		DX, AL
		RET
		
_io_out16:	; void io_out16(int port, int data);
		MOV		EDX, [ESP+4]	; port
		MOV 	EAX, [ESP+8]	; data
		OUT		DX, AX
		RET
		
_io_out32:	; void io_out32(int port, int data);
		MOV 	EDX, [ESP+4]	; port
		MOV 	EAX, [ESP+8] 	; data
		OUT		DX, EAX
		RET
		
_io_load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS		load EFLAGS into stack
		POP		EAX ; pop data from stack, and then load it into EAX
		RET	
		
_io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX, [ESP+4]
		PUSH	EAX
		POPFD		; pop data(from EAX) into EFLAGS
		RET
		
		
		
		
		
;_write_mem8:	; void write_mem8(int addr, int data);
;		MOV		ECX,[ESP+4]		; [ESP+4]saves address and read it into ECX
;		MOV		AL,[ESP+8]		; [ESP+8]saves data and read it into AL
;		MOV		[ECX],AL
;		RET
