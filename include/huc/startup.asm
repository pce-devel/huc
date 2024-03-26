;
; STARTUP.ASM  -  MagicKit standard startup code
;
;
; Note: lib_exclude.asm is empty and can be overridden with a local version
;       which is how compile-time variables should be defined in a HuC program
;
; Example: You may wish to implement your own joyport device driver, in which
;          case you will need to disable the automatic Mouse/Joypad reading
;          by setting "DISABLEJOYSCAN .equ 1"
;

		.list

		.include "lib_exclude.asm"

	.ifdef	_SGX
HAVE_LIB3	=	1
	.endif

	.ifdef	_AC
HAVE_LIB3	=	1
	.endif

; first, set MOUSE to default on:
;
SUPPORT_MOUSE	.equ	1

		.list

	.ifdef	HUC
		.include "huc.inc"
		.include "huc_opt.inc"
	.endif	HUC

		.include "standard.inc" ; HUCARD

; ----
; if FONT_VADDR is not specified, then specify it
; (VRAM address to load font into)
;
	.ifndef	FONT_VADDR
FONT_VADDR	.equ	$0800
	.endif

; ----
; system variables
;
		.zp
zp_ptr1:	.ds 2


		.bss

	.if	(CDROM)	; CDROM def's in system.inc

		.include "system.inc"

	.else		; ie HuCard

		.org	$2200
user_jmptbl:		; user interrupt vectors
irq2_jmp:	.ds 2	; IRQ2 (BRK instruction and external IRQ)
irq1_jmp:	.ds 2	; IRQ1 (VDC interrupt)
timer_jmp:	.ds 2	; TIMER
nmi_jmp:	.ds 2	; NMI (unused)
vsync_hook:	.ds 2	; VDC vertical sync routine
hsync_hook:	.ds 2	; VDC horizontal sync rountine

bg_x1:		.ds 2
bg_x2:		.ds 2
bg_y1:		.ds 2
bg_y2:		.ds 2

		.org	$2227
joyena:		.ds 1	; soft reset enable (bit 0/pad 1, bit 1/pad2, etc.)
joy:		.ds 5	; 'current' pad values (pad #1-5)
joytrg:		.ds 5	; 'delta' pad values (new keys hit)
joyold:		.ds 5	; 'previous' pad values

		.org	$2241
irq_cnt:	.ds 1	; VDC interrupt counter; increased 60 times per second
			; reset to zero when vsync() function called
vdc_mwr:	.ds 1
vdc_dcr:	.ds 1

	.endif	(CDROM)

		.org	$2244
scr_mode:	.ds 1	; screen mode and dimensions - set by <ex_scrmod>
scr_w:		.ds 1
scr_h:		.ds 1

		.org	$2284
soft_reset:	.ds 2	; soft reset jump loc (run+select)

; include sound driver variables

		.include  "sound.inc"

; sound.inc sets the starting location for these HuC variables
;
; this is normally $2680, but can be lower if a custom
; sound driver is used that doesn't need all of the
; standard System Card allocation of sound RAM.

vsync_cnt:	.ds 1		; counter for 'wait_vsync' routine

joybuf:		.ds 5		; 'delta' pad values collector
joyhook:	.ds 2		; 'read_joypad' routine hook
joycallback:	.ds 6		; joypad enhanced callback support
disp_cr:	.ds 1		; display control
				; HuCard: 1 = on, 0 = off
				; CD-ROM: 2 = on, 1 = off, 0 = no change
clock_hh	.ds 1		; system clock, hours since startup (0-255)
clock_mm	.ds 1		; system clock, minutes since startup (0-59)
clock_ss	.ds 1		; system clock, seconds since startup (0-59)
clock_tt	.ds 1		; system clock, ticks (1/60th sec) since startup (0-59)

joy6:		.ds 5		; second byte for 6-button joysticks
joytrg6:	.ds 5
joyold6:	.ds 5
joybuf6:	.ds 5

joytmp:		.ds 5
joytmp6:	.ds 5

color_queue_r:	.ds 1		; ring buffer index for read
color_queue_w:	.ds 1		; ring buffer index for write
color_index:	.ds 8		; ring buffer - palette index
color_count:	.ds 8		; ring buffer - palette count
color_bank:	.ds 8		; ring buffer - data bank
color_addr_l:	.ds 8		; ring buffer - data addr lo
color_addr_h:	.ds 8		; ring buffer - data addr hi
color_tia:	.ds 8		; self-modifying RAM tia function

	.if	(CDROM)

ovl_running	.ds	1	; overlay # that is currently running
cd_super	.ds	1	; Major CDROM version #
irq_storea	.ds	1	; CDROM IRQ-specific handling stuff
irq_storeb	.ds	1
ram_vsync_hndl	.ds	25
ram_hsync_hndl	.ds	25

	.endif	(CDROM)

; ----
; setup flexible boundaries for startup code
; and user program's "main".
;
		.rsset	0
START_BANK	.rs	0
LIB1_BANK	.rs	1
LIB2_BANK	.rs	1

	.ifdef	HAVE_LIB3
LIB3_BANK	.rs	1
	.endif

	.if	(NEED_SOUND_BANK)
SOUND_BANK	.rs	1
	.endif ; defined in sound.inc if needed

	.ifdef HUC
FONT_BANK	.equ	LIB2_BANK
CONST_BANK	.rs	1

	.ifdef	HUC_RESERVE_BANKS
HUC_USER_RESERVED .rs HUC_RESERVE_BANKS
	.endif

DATA_BANK	.rs	1
	.else	; HUC
		; HuC (because of .proc/.endp) does not use MAIN_BANK
MAIN_BANK	.rs	1
	.endif	HUC

; [ STARTUP CODE ]

; Let's prepare the secondary library banks first, for use later.
; The reason, as you will see, is because code for a given function
; which sits together in a file, may have things in zero-page,
; bss, LIB1_BANK (ie. START_BANK), and LIB2_BANK.
;
; The assembler must know beforehand what address etc. to use as a basis.
;

	.ifdef	_PAD
		.data
		.bank	$7e
		.org	$a000
	.endif

		.data
		.bank	LIB2_BANK,"Base Library 2/Font"
		.org	$6000
		.include "font.inc"
		.code
		.bank	LIB2_BANK
		.org	$A300

	.ifdef HAVE_LIB3
		.code
		.bank	LIB3_BANK,"Base Library 3"
		.org	$A000
	.endif

		.data
		.bank CONST_BANK,"Constants"
		.org  $4000

.ifdef HUC_RESERVE_BANKS
		.code
		.bank HUC_USER_RESERVED,"Huc User Reserved Banks"
		.org  $4000
.endif

		.data
		.bank DATA_BANK,"User Program"
		.org  $6000

;
; place overlay array here
; 100 entries, stored as 100
; lo-bytes of the starting
; sector num, followed by 100
; hi-bytes for a 128MB range
; all files are contiguous, so
; the # sectors is calculated
;

ovlarray:	.ds	200


	.code
	.bank START_BANK,"Base Library 1"

;
; A little introduction to the boot sequence:
;
; A HuCard has its origin at bank 0, and is mapped at $E000
; It needs to grab the interrupt vectors at $FFF6 and implement
; implement handlers for them
;
; A CDROM will load at bank $80 ($68 for SCD), and the initial
; loader will be mapped at $4000.  The MagicKit startup sequence
; also maps $C000 to this same bank.  However, the initial boot
; sequence will execute at $4000, proceeding to load additional
; code and data, and then jump to a post-boot section called
; 'init_go'.  This is the point at which the loader explicitly
; relinquishes the $4000 segment.  It should be noted that there
; are library subroutines loaded as part of this initial segment,
; and those routines are located in the $C000 range as well.
;
; A second entry point is defined for overlays that are not
; being booted (ie. they are loaded and executed from another
; overlay).  This entry point is at $C006, after the segments
; have all found their natural loading spots (ie. segment $68
; for Super CDROMs).  This entry point maps the necessary
; segments and resets the stack, without clearing memory or
; performing other setup chores, and then maps and executes
; _main() to run the module.  The user has no choice regarding
; this function, although he can pass values through the global
; variables which main() can use to decide what to do next.
;
; An additional "Hook" area has now been defined at $4038,
; which is used at initial load time, in case a SCD overlay
; program is run on plain CDROM hardware, and the author
; wishes to override the default text error message by
; loading and executing a plain CDROM program instead
;
	 ; ----
	 ; interrupt vectors

.if	!(CDROM)
		.org	$FFF6

		.dw	irq2
		.dw	irq1
		.dw	timer
		.dw	nmi
		.dw	reset
.endif	!(CDROM)

	 ; ----
	 ; develo startup code

.if	(DEVELO)
		.org	$6000

		sei
		map	reset
		jmp	reset

restart:	cla		; map the CD-ROM system bank
		tam	#7
		jmp	$4000	; back to the Develo shell
.endif	(DEVELO)


; ----
; reset
; ----
; things start here
; ----

; ----
; CDROM re-map library bank
;

	.if	(CDROM)

;
; entry point when the system card boots this overlay
;
; the pc engine's memory map is set to ...
;
;  mpr0 = bank $FF : pce hardware
;  mpr1 = bank $F8 : zp & stack in ram
;  mpr2 = bank $80 : cd ram
;  mpr3 = bank $81 : cd ram
;  mpr4 = bank $82 : cd ram
;  mpr5 = bank $83 : cd ram
;  mpr6 = bank $84 : cd ram
;  mpr7 = bank $00 : system card
;

		.org	$4000

boot_vector:	jmp	startup_boot

		db	"HuC"		; IDENTIFY OVERLAY TO ISOLINK!

;
; overlay entry point
;
; assume MMR0, MMR1, MMR6, MMR7 are set.
; set others & reset stack pointer
;

		.org	$C006

		; current overlay number that is running
		; this is overwritten by the isolink prgram; the load
		; statement must be the first in the block

ovlentry:	lda	#1
		sta	ovl_running

		lda	#CONST_BANK+_bank_base
		tam	#2
		lda	#DATA_BANK+_bank_base
		tam	#3
		lda	#_call_bank
		tam	#4

	.ifndef SMALL
		stw	#$4000,<__sp
	.else
		stw	#$3fff,<__sp
	.endif	SMALL
		ldx	#$ff
		txs

	.ifdef HAVE_INIT
		tii	huc_rodata, huc_data, huc_rodata_end-huc_rodata
	.endif

	.ifdef LINK_malloc
		call	___malloc_init2
	.endif

		lda	#bank(_main)
		tam	#page(_main)
		jsr	_main
		bra	*

		ds	$C038 - *, $EA	; Fail if this gets too big!

;
; CDROM error message alternate load entry point
;
		.org	$4038

overlay_data:	db	bank( ovlarray ) - _bank_base ; - NEEDED BY ISOLINK!

cdrom_err_load:	lda	#0		; <cderr_length - WRITTEN BY ISOLINK!
		ldx	#0		; <cderr_sector - WRITTEN BY ISOLINK!
		ldy	#0		; >cderr_sector - WRITTEN BY ISOLINK!

		stx	<__dl		; sector (offset from base of track)
		sty	<__ch
		stz	<__cl
		sta	<__al		; # sectors
		lda	#$80
		sta	<__bl		; bank #
		lda	#3
		sta	<__dh		; MPR #
		jsr	cd_read
		cmp	#0
		beq	boot_vector
		jmp	cd_boot		; Can't load - reboot CDROM system card

;
; Proper Boot-time entry point
;

startup_boot:	stz	$2680		; clear program RAM
		tii	$2680,$2681,$197F

		tma	#2		; map LIB1_BANK into MPR6
		tam	#6
		inc	a		; map LIB2_BANK into MPR5
		tam	#5

		;
		; Note: All CDROM boot loaders will load into MMR $80 region
		;	regardless of whether they are CD or SCD.
		;	Here, we will move the information to occupy
		;	base memory at MMR $68 if appropriate
		;

	.if	(CDROM = SUPER_CD)
		tax			; don't copy if SCD RAM already mapped
		bmi	.copy		; & disable loading, somebody else has
		stz	ovlentry+1	; already loaded us completely
		bra	.copied

.copy:		jsr	ex_getver	; check if SCD program running on SCD
		cpx	#3		; hardware
		bcc	.copied		; don't copy to _bank_base if memory
		stx	cd_super	; doesn't exist there

		sei
		lda	#_bank_base	; copy LIB1_BANK into SuperCD MPR6
		tam	#6
		inc	a		; copy LIB2_BANK into SuperCD MPR5
		tam	#5
		tii	$4000,$C000,$2000
		tii	$6000,$A000,$2000
		bit	video_reg	; purge overdue vblank irq
		cli

	.endif	(CDROM = SUPER_CD)

.copied:	jmp	.init_hw

	.else	;HuCARD

		.org  $E010

reset:		sei			; disable interrupts
		csh			; select the 7.16 MHz clock
		cld			; clear the decimal flag
		ldx	#$FF		; initialize the stack pointer
		txs
		txa			; map the I/O bank in the first page
		tam	#0
		lda	#$F8		; and the RAM bank in the second page
		tam	#1
		stz	$2000		; clear all the RAM
		tii	$2000,$2001,$1FFF

		tma	#7		; get LIB1_BANK
		tam	#6		; map LIB1_BANK into MPR6
		inc	a		; map LIB2_BANK into MPR5
		tam	#5

	.endif	(CDROM)

		; ----
		; initialize the hardware

	.if	(CDROM)

		; This is the point in the CDROM loader where the code no longer
		; executes in the $4000 segment, in favour of using the $C000
		; segment (also used for the library subroutines)

		.page	6

.init_hw:	jsr	ex_dspoff
		jsr	ex_rcroff
		jsr	ex_irqoff
		jsr	ad_reset

	.else	;HuCARD
		stz	timer_ctrl	; init timer
	.endif	(CDROM)

		jsr	lib2_init_psg	; init sound
		jsr	lib2_init_vdc	; init video
	.ifdef	_SGX
		jsr	lib2_init_sgx_vdc
	.endif	_SGX

		lda	#$1F		; init joypad
		sta	joyena

		; ----
		; initialize the sound driver

		__sound_init

		; ----
		; initialize interrupt vectors

	.if	(CDROM)
		jsr	ex_dspon
		jsr	ex_rcron
		jsr	ex_irqon
		stz	disp_cr		; "no change"

	.else	;HuCARD
		ldx	#4		; user vector table
		cly
.l2:		lda	#LOW(rti)
		sta	user_jmptbl,Y
		iny
		lda	#HIGH(rti)
		sta	user_jmptbl,Y
		iny
		dex
		bne	.l2

		stw	#reset,soft_reset
		stw	#rts,vsync_hook	; user vsync routine
		stw	#rts,hsync_hook	; user hsync routine

		lda	#$01		; enable interrupts
		sta	irq_disable
		stz	irq_status
		cli

		; ----
		; enable display and VSYNC interrupt

		vreg	#5
		lda	#$C8
		sta	<vdc_crl
		sta	video_data_l
		st2	#$00
		lda	#$01
		sta	disp_cr

	.endif	(CDROM)

		; ----
		; init TIA instruction in RAM (fast BLiTter to hardware)

		lda	#$E3		; TIA instruction opcode
		sta	ram_hdwr_tia
		lda	#$60		; RTS instruction opcode
		sta	ram_hdwr_tia_rts

		; ----
		; init random number generator

		lda	#1
		jsr	wait_vsync	; wait for one frame & randomize rndseed2
		stw	#$03E7,<__cx	; set random seed
		stw	rndseed2,<__dx
		jsr	srand

	.if	(CDROM)

	.if	(CDROM = SUPER_CD)
		lda	cd_super		; don't load the program if SCD
		bne	loadprog		; program not running on SCD hrdware

		lda	cdrom_err_load + 1	; is there an error overlay?
		beq	init_go
		jmp	cdrom_err_load
	.endif	(CDROM = SUPER_CD)

		; ----
		; load program
		; ----
		; CL/CH/DL = sector address
		; DH = load mode - bank mode ($6000-$7FFF)
		; BL = bank index
		; AL = number of sectors

loadprog:	lda	ovlentry + 1	; is this the initial overlay ?
		cmp	#1		; if not then somebody already loaded
		bne	init_go		; us completely, do not load the rest

		stz	<__cl		; initial boot doesn't load complete program;
		stz	<__ch		; prepare to load remainder
		lda	#10		; 10th sector (0-1 are boot;
		sta	<__dl		; 2-9 are this library...)
		lda	#3		; load mode (consecutive banks; use MPR 3)
		sta	<__dh
		lda	#(_bank_base+2)	; 2 banks are boot/base library
		sta	<__bl
		stz	<__bh
		lda	#(_nb_bank-2)*4	; 512k maximum
		sta	<__al
		stz	<__ah
		jsr	cd_read
		tax
		beq	init_go
		; ----
		jmp	cd_boot		; reset

init_go:

; These routines will be run from RAM @ $2000 so we
; need to count bytes to determine how much to xfer
; (The total is 24 bytes, but we copy 25)

		.bank	LIB2_BANK

vsync_irq_ramhndlr:
		php			; 1 byte
		pha			; 1
		tma	#6		; 2
		sta	irq_storea	; 3
		lda	#BANK(vsync_hndl) ; 2
		tam	#6		; 2
		pla			; 1
		pha			; 1
		jsr	vsync_hndl	; 3
		lda	irq_storea	; 3
		tam	#6		; 2
		pla			; 1
		plp			; 1
		rts			; 1 = 24 bytes

hsync_irq_ramhndlr:
		php			; 1 byte
		pha			; 1
		tma	#6		; 2
		sta	irq_storeb	; 3
		lda	#BANK(hsync_hndl) ; 2
		tam	#6		; 2
		pla			; 1
		pha			; 1
		jsr	hsync_hndl	; 3
		lda	irq_storeb	; 3
		tam	#6		; 2
		pla			; 1
		plp			; 1
		rts			; 1 = 24 bytes

		.bank	LIB1_BANK

	.endif	(CDROM)

		; ----
		; jump to main routine

.ifdef HUC

	.ifndef	SMALL
		stw	#$4000,<__sp	; init stack ptr first
	.else
		stw	#$3FFF,<__sp
	.endif	SMALL

		; ----
		; load font
		;
		; this section of font loading was stolen
		; from _load_default_font because the default
		; FONT segment number is not yet guaranteed
		; if the SCD is being run on a plain CDROM system
		; so we need to trick the segment pointer
		; with a reliable one

		ldx.l	#FONT_VADDR	; Load Font @ VRAM addr
		lda.h	#FONT_VADDR
		stx.l	<__di
		sta.h	<__di
		jsr	_set_font_addr	; set font addr (in LIB1_BANK)

		lda	#96
		sta	<__cl
		lda	#1
		sta	<__al		; font foreground color
		stz	<__ah		; font background color
		clx
		lda.l	#font_1
		sta.l	<__si
		lda.h	#font_1
		sta.h	<__si
		tma	#5		; guarantee LIB2_BANK even if
		sta	<__bl		; SCD on regular CDROM system
		jsr	lib2_load_font

		lda	#1
		jsr	wait_vsync ; cure screen flash
		jsr	_cls

		stz	color_reg	; set color #0 = 0/0/0 rgb
		stz	color_reg+1	; set color #1 = 7/7/7 rgb
		tia	pal_fnt,color_data,4

	.if (CDROM)
	.if (CDROM == SUPER_CD)
		; ----
		; Super CDROM error message
		; ----

		lda	cd_super	; SCD running on Super system?
		bne	scd_ok

		ldx.l	#scd_msg
		stx.l	<__si
		lda.h	#scd_msg
		sta.h	<__si
		ldx.l	#$0341		; X=1, Y=13 on a 64-wide screen
		lda.h	#$0341
		jsr	_put_string.2

		bra	*		; otherwise loop on blank screen

		.bank	LIB2_BANK

scd_msg:	db	"Super CD-ROM2 System required!",0

		.bank	LIB1_BANK

scd_ok:
	.endif	(CDROM == SUPER_CD)
	.endif	(CDROM)

	.endif	HUC

	.ifndef	DISABLEJOYSCAN
	.ifdef	SUPPORT_MOUSE
		jsr  mousinit		; check existence of mouse
	.endif	SUPPORT_MOUSE
	.endif	DISABLEJOYSCAN

	.if	(CDROM)
		; Now, install the RAM-based version of the
		; interrupt-handlers and activate them

		tma	#page(vsync_irq_ramhndlr)
		pha
		lda	#bank(vsync_irq_ramhndlr)
		tam	#page(vsync_irq_ramhndlr)
		tii	vsync_irq_ramhndlr,ram_vsync_hndl,25
		tii	hsync_irq_ramhndlr,ram_hsync_hndl,25
		pla
		tam	#page(vsync_irq_ramhndlr)

		; set VSYNC handler

		stw	#ram_vsync_hndl,vsync_hook
		smb	#4,<irq_m	; enable new code
		smb	#5,<irq_m	; disable system card code

		; set HSYNC handler

		stw	#ram_hsync_hndl,hsync_hook
		smb	#6,<irq_m	; enable new code
		smb	#7,<irq_m	; disable system card code

	.endif	(CDROM)

	.ifdef	HUC
		; ----
		; Map the final stuff before executing main()
		; ----

		lda	#CONST_BANK+_bank_base	; map string constants bank
		tam	#2		; (ie. $4000-$5FFF)
		lda	#_call_bank	; map call bank
		tam	#4		; (ie. $8000-$9FFF)
		; ---
		.if	(CDROM)
		lda	#1		; first overlay to run at boot time
		sta	ovl_running	; store for later use
		.endif
		; ---
		stz	clock_hh		; clear clock
		stz	clock_mm
		stz	clock_ss
		stz	clock_tt

.ifdef HAVE_INIT
		tii  huc_rodata, huc_data, huc_rodata_end-huc_rodata
.endif

.ifdef LINK_malloc
		call	___malloc_init2
.endif

		map	_main
		jsr	_main		; go!
		bra	*

pal_fnt:	dw	$0000,$01FF	; Black and White font palette.

	.else	HUC
		map	main
		jmp	main
	.endif	HUC

	; XXX: if LINK_malloc or HAVE_INIT are defined the interrupt handlers start
	; a couple of bytes later.  When an overlay is loaded the handlers are not
	; relocated, causing a crash.  This hack makes sure the handlers are in the
	; same place in all cases.

	.if	(CDROM)

	.ifndef	HAVE_INIT
		nop
		nop
		nop
		nop
		nop
		nop
		nop
	.endif	HAVE_INIT

	.ifndef	LINK_malloc
		nop
		nop
		nop
	.endif	LINK_malloc

	.endif	(CDROM)

; ----
; system
; ----
; give back control to the Develo system
; ----

	.if	(DEVELO)

system:		sei
		csh
		cld
		ldx	#$FF	; stack
		txs
		lda	#$FF	; I/O bank
		tam	#0
		lda	#$F8	; RAM bank
		tam	#1
		lda	#$80	; Develo Bank
		tam	#2
		tma	#7	; startup code bank
		tam	#3

		; ----
		; re-initialize the machine
		;

		stz	$2000		; clear RAM
		tii	$2000,$2001,$1FFF
		stz	timer_ctrl	; init timer
		jsr	init_psg	; init sound
		jsr	init_vdc	; init video
		lda	#$1F		; init joypad
		sta	joyena
		lda	#$07		; set interrupt mask
		sta	irq_disable
		stz	irq_status	; reset timer interrupt

		st0	#5		; enable display and vsync interrupt
		lda	#$C8
		sta	<vdc_crl
		sta	video_data_l
		jmp	restart		; restart

	.endif	(DEVELO)

; [INTERRUPT CODE]

rts:		rts
rti:		rti

; ----
; irq2
; ----
; IRQ2 interrupt handler
; ----

	.if	!(CDROM)
irq2:		bbs0 <irq_m,.user
		rti
.user:
	jmp	[irq2_jmp]
	.endif	!(CDROM)

; ----
; irq1
; ----
; VDC interrupt handler
; ----

	.if !(CDROM)

irq1:		bbs1	<irq_m,user_irq1; jump to the user irq1 vector if bit set
		; --
		pha			; save registers
		phx
		phy
		; --
		lda	video_reg	; get VDC status register
		sta	<vdc_sr		; save a copy

		 ; ----
		 ; vsync interrupt
		 ;
.vsync:		bbr5	<vdc_sr,.hsync
		; --
		inc	irq_cnt		; increment IRQ counter
		; --
		st0	#5		; update display control (bg/sp)
		lda	<vdc_crl
		sta	video_data_l
		; --
		bbs5	<irq_m,.hsync
		; --
		jsr	vsync_hndl
		; --
		; ----
		; hsync interrupt
		;

.hsync:		bbr2	<vdc_sr,.exit
		bbs7	<irq_m,.exit
		; --
		jsr	hsync_hndl

		; ----
		; exit interrupt
		;
.exit:		lda	<vdc_reg	; restore VDC register index
		sta	video_reg
		; --
		ply
		plx
		pla
		rti

	.endif	!(CDROM)

		; ----
		; user routine hooks
		;

user_irq1:	jmp	[irq1_jmp]


; ----
; vsync_hndl
; ----
; Handle VSYNC interrupts
; ----
vsync_hndl:

	.if  !(CDROM)
		ldx	disp_cr		; check display state (on/off)
		bne	.l1
		and	#$3F		; disable display
		st0	#5		; update display control (bg/sp)
		sta	video_data_l
		bra	.l2
		; --
	.else
		; The CD-ROM version only acts if the display state has changed
		; (disp_cr != 0).
		ldx	disp_cr
		beq	.l1
		stz	disp_cr
		dex
		beq	.m1
		jsr	ex_dspon
		bra	.l1
.m1:		jsr	ex_dspoff
	.endif	!(CDROM)

.l1:		jsr	xfer_palette	; transfer queued palettes

		jsr	rcr_init	; init display list

.l2:		st0	#7		; scrolling
		stw	bg_x1,video_data
		st0	#8
		stw	bg_y1,video_data

		; --
		lda	clock_tt	; keep track of time
		inc	A
		cmp	#60
		bne	.lcltt
		lda	clock_ss
		inc	A
		cmp	#60
		bne	.lclss
		lda	clock_mm
		inc	A
		cmp	#60
		bne	.lclmm
		inc	clock_hh
		cla
.lclmm:		sta	clock_mm
		cla
.lclss:		sta	clock_ss
		cla
.lcltt:		sta	clock_tt
		; --

	.if	(CDROM)
		jsr	ex_colorcmd
		; XXX: Why call this every vsync, when it's called from rand()
		; as needed anyway?
		inc	rndseed
		jsr	randomize
	.endif	(CDROM)

		; invoke the sound driver's vsync irq code

		__sound_vsync

	.ifdef	SUPPORT_MOUSE
		lda	msflag		; if mouse supported, and exists
		beq	.l3		; then read mouse instead of pad

	.ifndef	DISABLEJOYSCAN
		jsr	mousread
	.endif	DISABLEJOYSCAN

		bra	.out
	.endif	SUPPORT_MOUSE


	.ifdef	DISABLEJOYSCAN
.l3:
	.else
.l3:		jsr	read_joypad	; else read joypad
	.endif
.out:		rts


; ----
; hsync_hndl
; ----
; Handle HSYNC interrupts
; ----
		; ----
		; hsync scrolling handler
		;

hsync_hndl:	ldy	s_idx
		bpl	.r1
		; --
		lda	<vdc_crl
		and	#$3F
		sta	<vdc_crl
		stz	s_idx
		ldx	s_list
		lda	s_top,X
		jsr	rcr5
		rts
		; --
.r1:		ldx	s_list,Y
		lda	<vdc_crl
		and	#$3F
		ora	s_cr,X
		sta	<vdc_crl
		; --
		jsr	rcr_set
		; --
		lda	s_top,X
		cmp	#$FF
		beq	.out
		; --
		st0	#7
		lda	s_xl,X
		ldy	s_xh,X
		sta	video_data_l
		sty	video_data_h
		st0	#8
		lda	s_yl,X
		ldy	s_yh,X
		sub	#1
		bcs	.r2
		dey
.r2:		sta	video_data_l
		sty	video_data_h
.out:		rts

		; ----
		; init display list
		;
rcr_init:
		maplibfunc	build_disp_list
		bcs  .r3
		rts
		; --
.r3:		smb	#7,<vdc_crl
		lda	#$FF
		sta	s_idx
		ldx	s_list
		ldy	s_top,X
		cpy	#$FF
		bne	rcr5
		; --
		ldy	s_xl,X
		sty	bg_x1
		ldy	s_xh,X
		sty	bg_x1+1
		ldy	s_yl,X
		sty	bg_y1
		ldy	s_yh,X
		sty	bg_y1+1
		stz	s_idx
		bra	rcr5

		; ----
		; program scanline interrupt
		;

rcr_set:	iny
		sty	s_idx
		lda	s_list,Y
		tay
		lda	s_top,Y
		cmp	scr_height
		bhs	rcr6
		cmp	s_bottom,X
		blo	rcr5
		; --
		lda	s_bottom,X
rcr4:		dec	A
		pha
		lda	#$F0
		sta	s_bottom,X
		stz	s_cr,X
		dec	s_idx
		pla
		; --
rcr5:		st0	#6		; set scanline counter
		add	#64
		sta	video_data_l
		cla
		adc	#0
		sta	video_data_h
		bra	__rcr_on
		;--
rcr6:		lda	s_bottom,X
		cmp	scr_height
		blo	rcr4
		bra	__rcr_off

; ----
; rcr_on
; ----
; enable scanline interrupt
; ----

_rcr_on:	lda	#5
		sta	<vdc_reg

__rcr_on:	st0	#5
		lda	<vdc_crl
		ora	#$04
		sta	<vdc_crl
		sta	video_data_l
		rts

; ----
; rcr_off
; ----
; disable scanline interrupt
; ----

_rcr_off:	lda	#5
		sta	<vdc_reg

__rcr_off:	st0	#5
		lda	<vdc_crl
		and	#$FB
		sta	<vdc_crl
		sta	video_data_l
		rts



; ----
; timer
; ----
; timer interrupt handler
; ----

	.if	!(CDROM)

timer_user:	jmp	[timer_jmp]

timer:		bbs2	<irq_m,timer_user
		pha
		phx
		phy

		sta	irq_status	; acknowledge interrupt

		; invoke the sound driver's timer irq code

		__sound_timer

		ply
		plx
		pla
		rti

	.endif	!(CDROM)

; ----
; nmi
; ----
; NMI interrupt handler
; ----

	.if	!(CDROM)
nmi:		bbs3	<irq_m,.user
		rti

.user:		jmp	[nmi_jmp]
	.endif	!(CDROM)


; [LIBRARY]

; ----
; standard library
; ----

		.include "library.asm"
		.include "scroll.asm"
		.include "math.asm"

	.ifdef	HUC
		.include "huc.asm"
		.include "huc_gfx.asm"

	.ifdef	_AC
		.include "ac_lib.asm"
	.endif	_AC

	.ifdef	_SGX
		.include "sgx_gfx.asm"
	.endif	_SGX

		.include "huc_math.asm"
		.include "huc_bram.asm"
		.include "huc_misc.asm"

	.endif	HUC

	.ifdef SUPPORT_MOUSE
		.include "mouse.asm"
	.endif	; SUPPORT_MOUSE

	.if	(CDROM)
		.include "cdrom.asm"
	.endif	(CDROM)

	.if	(NEED_SOUND_CODE)
		.include "sound.asm"
	.endif	; defined in sound.inc if needed

;
;
;

; ----
; disp_on
; ----
; enable display
; ----

	.ifdef	HUC

_disp_on:	ldx	disp_cr
	.if	(CDROM)
		lda	#2
	.else
		lda	#1
	.endif	(CDROM)
		sta	disp_cr
		cla
		rts

	.else	; HUC

disp_on:	lda	#1
		sta	disp_cr
		rts

	.endif	HUC

; ----
; disp_off
; ----
; disable display
; ----

	.ifdef HUC

_disp_off:	ldx	disp_cr
	.if	(CDROM)
		lda	#1
		sta	disp_cr
	.else
		stz	disp_cr
	.endif	(CDROM)
		cla
		rts

	.else	; HUC

disp_off:	stz	disp_cr
		rts

	.endif	HUC

; ----
; set_intvec
; ----
; set interrupt vector
; ----
; IN : A = vector number
;		 0 IRQ2
;		 1 IRQ1 (VDC)
;		 2 TIMER
;		 3 NMI
;		 4 VSYNC
;		 5 HSYNC
;		 6 SOFT RESET (RUN + SELECT)
;		X = vector address low byte
;		Y =	"		"	 high byte
; ----

	.if	!(CDROM)

set_intvec:	php
		sei
		cmp	#6
		blo	.vector
		bne	.exit

.reset:		stx	soft_reset
		sty	soft_reset+1
		bra	.exit

.vector:	pha
		asl	A
		sax
		sta	user_jmptbl,X
		inx
		tya
		sta	user_jmptbl,X
		pla

.exit:		plp
		rts

	.endif	!(CDROM)

; ----
; wait_vsync
; ----
; wait the next vsync
; ----
; IN :	A = number of frames to be sync'ed on
; ----
; OUT:	A = number of elapsed frames since last call
; ----

wait_vsync:	bbr1	<irq_m,.l1
		cla			; return immediately if IRQ1 is redirected
	.ifdef	HUC
		clx
	.endif	HUC
		rts

.l1:		sei			; disable interrupts
		cmp	irq_cnt		; calculate how many frames to wait
		beq	.l2
		bhs	.l3
		lda	irq_cnt
.l2:		inc	A
.l3:		sub	irq_cnt
		sta	vsync_cnt
		cli			; re-enable interrupts

.l4:		lda	irq_cnt		; wait loop
.l5:		incw  rndseed2
		cmp	irq_cnt
		beq	.l5
		dec	vsync_cnt
		bne	.l4

		stz	irq_cnt		; reset system interrupt counter
		inc	A		; return number of elapsed frames

	.ifndef HUC

		rts

	.else	; !HUC

		; ----
		; callback support

		pha
		lda	joycallback	; callback valid?
		bpl	.t3
		bit	#$01
		bne	.t3

		lda	joycallback+1	; get events for all the
		beq	.t3		; selected joypads
		sta	<__al
		cly
		cla
.t1:		lsr	<__al
		bcc	.t2
		ora	joybuf,Y
.t2:		iny
		cpy	#5
		blo	.t1

		and	joycallback+2	; compare with requested state
		beq	.t3

		inc	joycallback	; lock callback feature
		tax			; call user routine
		tma	#5
		pha
		lda	joycallback+3
		tam	#5
		cla
		jsr	.callback
		pla
		tam	#5
		dec	joycallback	; unlock
		; --
.t3:		plx
		cla
		rts

		; ----
		; user routine callback
		;
.callback:
		jmp	[joycallback+4]

	.endif	HUC

		.include  "joypad.asm"	; read joypad values
