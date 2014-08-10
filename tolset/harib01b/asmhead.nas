; haribote-os boot asm
; TAB=4

BOTPAK	EQU		0x00280000		; EQU pseudoinstruction, like #define
DSKCAC	EQU		0x00100000		; 
DSKCAC0	EQU		0x00008000		; 

; 
CYLS	EQU		0x0ff0			; 
LEDS	EQU		0x0ff1
VMODE	EQU		0x0ff2			; 
SCRNX	EQU		0x0ff4			; 
SCRNY	EQU		0x0ff6			;
VRAM	EQU		0x0ff8			; 

		ORG		0xc200			; 

; setting the mode of screen

		MOV		BX,0x4101		; VBE 640x480x8bit
		MOV		AX,0x4f02
		INT		0x10
		MOV		BYTE [VMODE],8	; record screen mode
		MOV		WORD [SCRNX],640
		MOV		WORD [SCRNY],480
		MOV		DWORD [VRAM],0xe0000000
		


; 

		MOV		AH,0x02
		INT		0x16 			; keyboard BIOS
		MOV		[LEDS],AL

; PIC disables all interrupts 
;	io_out(PIC0_IMR, 0xff); disable all interrupts of master PIC
;	io_out(PIC1_IMR, 0xff); disable all interrupts of slave PIC
;	io_cli(); disable the interrupt of CPU level

		MOV		AL,0xff
		OUT		0x21,AL         ; master PIC
		NOP						; rest for a clock, do nothing, avoid executing NOP continuously
		OUT		0xa1,AL			; slave PIC

		CLI						; disable the interrupt of CPU level 

; in order to let CPU access more than 1MB memory, setting A20 GATE

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20, A20's open let all memory accessed
		OUT		0x60,AL
		CALL	waitkbdout

; switch to protected mode

[INSTRSET "i486p"]				; 

		LGDT	[GDTR0]			;
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; set bit31 to 0, for page
		OR		EAX,0x00000001	; set bit0 to 1, for switching to protected mode
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			; 
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

;

		MOV		ESI,bootpack	; 
		MOV		EDI,BOTPAK		; 
		MOV		ECX,512*1024/4
		CALL	memcpy

; 

;

		MOV		ESI,0x7c00		; 
		MOV		EDI,DSKCAC		; 
		MOV		ECX,512/4
		CALL	memcpy

; 

		MOV		ESI,DSKCAC0+512	; 
		MOV		EDI,DSKCAC+512	; 
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; 
		SUB		ECX,512/4		; 
		CALL	memcpy

; 
;	

; start bootpack

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 
		MOV		ESI,[EBX+20]	; 
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		; 
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 
		RET
; 

		ALIGNB	16
GDT0:
		RESB	8				; 
		DW		0xffff,0x0000,0x9200,0x00cf	; 
		DW		0xffff,0x0000,0x9a28,0x0047	; 

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack:
