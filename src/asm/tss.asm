global tss_flush
tss_flush:
	MOV AX, 0x2B
	LTR AX
	RET

global enter_usermode
	
enter_usermode:
	CLI
	MOV AX, 0x23
	MOV DS, AX
	MOV ES, AX
	MOV FS, AX
	MOV GS, AX

	MOV EAX, ESP
	PUSH 0x23
	PUSH EAX
	PUSHF
	POP EAX
	OR EAX, 0x200
	PUSH EAX
	PUSH 0x1B
	PUSH int_enable
	IRET
int_enable:
