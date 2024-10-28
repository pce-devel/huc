; ***************************************************************************
; ***************************************************************************
;
; Turbo Everdrive V2 Programming Notes (2019-03-24)
;
; Definitions and macros for accessing the TED2's hardware.
;
; Copyright John Brandwood 2019-2022.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; The TED v2 contains 4MB of RAM, which it maps into the PCE's 1MB of HuCard
; address space (banks 0-127).
;
; The TED v2 maps either RAM or its FPGA into bank 0 of HuCard address space.
;
; When the TED is reset, TED_REG_CFG is set to 0 and the FPGA is mapped
; into bank 0, which the CPU reset maps into $E000-$FFFF ... and so the
; TED boots into it's firmware using the normal reset vector contained
; in the FPGA firmware ROM.
;
; In order to read/write the TED's registers you've got to map out the
; entire bank 0 of HuCard address space ... which means that you can't
; do it if your code is located in HuCard RAM bank 0, and you usually
; can't do it with interrupts enabled, either.
;
; The TED v1 only mapped 256 bytes of registers into HuCard address space,
; but the TED v2 maps the entire 8KB (you can read the TED boot code).
;
; The TED v2 hardware registers are mirrored every 256 bytes in the first
; bottom 4KB of the bank.
;
; The TED v2 boot code is 512 bytes long, and is mirrored every 512 bytes
; in the top 4KB of the bank.
;
; The TED's registers are normally left "locked" to prevent problems.
;
; In order to make an changes to the registers, you must first unlock
; them by writing $A5 to TED_REG_KEY1 (and $56 to TED_REG_KEY2 if a TED v1).
;
; The registers can then be "locked" again when you're finished by
; setting a bit in the TED_REG_CFG register.
;
; It is technically possible to leave them "writable" while the RAM bank
; is selected ... but I can't think of a time that this would actually
; be useful.
;
; ***************************************************************************
; ***************************************************************************



; ***************************************************************************
;
; TED_REG_CFG Bit Settings - (N.B. The register is read-write)
;
;   Bit 0 : cfg_regs_oe_off : Bank 0 mapping (0=FPGA, 1=RAM)
;   Bit 1 : cfg_regs_we_off : FPGA Registers (0=access, 1=locked)
;   Bit 2 : cfg_usb_boot    : unknown (leave zero)
;   Bit 3 : cfg_skip_eep    : Normally set by KRIKzz : unknown (set to 1)
;   Bit 4 : cfg_ram_we_off  : RAM Read-Only? (0=writable, 1=read-only)
;   Bit 5 :		    : SF2 mapper $1FF0-$1FF3 (0=disabled, 1=enabled)
;   Bit 6 :		    : unknown (leave zero)
;   Bit 7 :		    : unknown (leave zero)
;


; ***************************************************************************
;
; TED_REG_STATE Bit Settings - (N.B. The register is read-only)
;
;   Bit 0 : st_spi	    : SPI shift register busy
;   Bit 1 :		    : unused (leave zero)
;   Bit 2 : st_usb_wr	    : USB write FIFO
;   Bit 3 : st_usb_rd	    : USB read FIFO
;   Bit 4 :		    : unused (leave zero)
;   Bit 5 :		    : unused (leave zero)
;   Bit 6 :		    : unused (leave zero)
;   Bit 7 :		    : unused (leave zero)
;

TED_FLG_SPI	=	0
TED_FLG_USB_WR	=	2
TED_FLG_USB_RD	=	3


; ***************************************************************************
;
; TED1_REG_SPI_CFG Bit Settings - (N.B. The register is read-write)
; TED2_REG_SPI_CFG Bit Settings - (N.B. The register is read-write)
;

TED_SPI_CS	=	0		; Card Select	   (0=selected)
TED_SPI_SPEED	=	1		; Card Speed	   (0=high)
TED_SPI_AREAD	=	2		; Accelerated Read (0=off)


; ***************************************************************************
;
; TED_REG_MAP Bit Settings - (N.B. The register is write-only)
;
;   Bit 0  : Low 512KB bank bit 0
;   Bit 1  : Low 512KB bank bit 1
;   Bit 2  : Low 512KB bank bit 2
;   Bit 3  : Low 512KB bank bit 3 (ignored)
;   Bit 4  : Top 512KB bank bit 0
;   Bit 5  : Top 512KB bank bit 1
;   Bit 6  : Top 512KB bank bit 2
;   Bit 7  : Top 512KB bank bit 3 (ignored)
;
;   The 4MB RAM in the TED2 is split into 8x512KB banks ...
;
;   Bank 0 : Used as Top 512KB : Used for SF2 mapper $1FF0
;   Bank 1 :		       : Used for SF2 mapper $1FF1
;   Bank 2 :		       : Used for SF2 mapper $1FF2
;   Bank 3 :		       : Used for SF2 mapper $1FF3
;   Bank 4 : Used as Low 512KB
;   Bank 5 :
;   Bank 6 :
;   Bank 7 : Used for TED OS (cannot be trashed if cfg_skip_eep is set!)
;
;   The TED2 runs a HuCard image with TED_REG_MAP set to $04!
;
;   !!! WARNING !!!
;
;   Writing to the SF2 mapper registers only changes the bottom 2 bits
;   of the 512KB bank number, so you should set the mapper to bank 0
;   before enabling/using the SF2 mapper, or you'll get weird results.
;

TED_MAP_BANK0	=	$04
TED_MAP_BANK1	=	$14
TED_MAP_BANK2	=	$24
TED_MAP_BANK3	=	$34
TED_MAP_BANK4	=	$44
TED_MAP_BANK5	=	$54
TED_MAP_BANK6	=	$64
TED_MAP_BANK7	=	$74


; ***************************************************************************
;
; Map the TED hardware into commonly-banked CPU space at $8000-$9FFF (MPR4).
;

TED_BASE_ADDR	=	$8000

;
; TED registers that are the same for TED v1 and TED v2.
;

TED_REG_SPI	=	0
TED_REG_SPI2	=	1
TED_REG_FIFO	=	3
TED_REG_STATE	=	4
TED_REG_KEY1	=	5
TED_REG_CFG	=	6
TED_REG_MAP	=	7

;
; TED v1 (From KRIKzz's edio.c sample code)
;
	.if	0
TED_REG_SPI_CFG	=	8
TED_REG_VERSION	=	9
TED_REG_KEY2	=	10
	.endif

;
; TED v2 (From KRIKzz's bios.s source code)
;

TED_REG_SPI_CFG	=	2
TED_REG_VERSION	=	8
TED_REG_I2C	=	9

TED_STR_KRIKZZ	=	TED_BASE_ADDR + $1FE5



; ***************************************************************************
;
; Example macros in PCEAS format.
;

		; This works on both TED1 and TED2.
		;
		; N.B. The 2nd write is only needed on the TED1.

TED_UNLOCK	.macro
		lda	#$A5
		sta	TED_BASE_ADDR + TED_REG_KEY1
;		lda	#$56
;		sta	TED_BASE_ADDR + TED_REG_KEY2
		.endm

		; This works on both TED1 and TED2.

TED_CFG_FPGA	.macro
		TED_UNLOCK
		stz	TED_BASE_ADDR + TED_REG_CFG
		.endm

		;
		; TED2-only macros.
		;

TED_CFG_1MB_RAM	.macro
		lda	#%00001011
		sta	TED_BASE_ADDR + TED_REG_CFG
		.endm

TED_CFG_SF2_RAM	.macro
		lda	#%00101011
		sta	TED_BASE_ADDR + TED_REG_CFG
		.endm

TED_CFG_1MB_ROM	.macro
		lda	#%00011011
		sta	TED_BASE_ADDR + TED_REG_CFG
		.endm

TED_CFG_SF2_ROM	.macro
		lda	#%00111011
		sta	TED_BASE_ADDR + TED_REG_CFG
		.endm

		;
		; These USB macros should work on both TED1 and TED2.
		;

TED_USB_RD_TEST	.macro
		tst	#(1 << TED_FLG_USB_RD), TED_BASE_ADDR + TED_REG_STATE
		.endm

TED_USB_WR_TEST	.macro
		tst	#(1 << TED_FLG_USB_WR), TED_BASE_ADDR + TED_REG_STATE
		.endm

TED_USB_RD_WAIT	.macro
.wait\@:	TED_USB_RD_TEST
		beq	.wait\@
		.endm

TED_USB_WR_WAIT	.macro
.wait\@:	TED_USB_WR_TEST
		beq	.wait\@
		.endm

TED_USB_RD_BYTE	.macro
		TED_USB_RD_WAIT
		lda	TED_BASE_ADDR + TED_REG_FIFO
		.endm

TED_USB_WR_BYTE	.macro
		TED_USB_WR_WAIT
		sta	TED_BASE_ADDR + TED_REG_FIFO
		.endm

		;
		; These SPI macros would work on both TED1 and TED2,
		; but the addresses of the registers are different.
		;

TED_SPI_CS_OFF	.macro
		lda	#(1 << TED_SPI_CS)
		tsb	TED_BASE_ADDR + TED_REG_SPI_CFG
		.endm

TED_SPI_CS_ON	.macro
		lda	#(1 << TED_SPI_CS)
		trb	TED_BASE_ADDR + TED_REG_SPI_CFG
		.endm

TED_SPI_SPD_LO	.macro
		lda	#(1 << TED_SPI_SPEED)
		tsb	TED_BASE_ADDR + TED_REG_SPI_CFG
		.endm

TED_SPI_SPD_HI	.macro
		lda	#(1 << TED_SPI_SPEED)
		trb	TED_BASE_ADDR + TED_REG_SPI_CFG
		.endm

TED_SPI_ARD_ON	.macro
		lda	#(1 << TED_SPI_AREAD)
		tsb	TED_BASE_ADDR + TED_REG_SPI_CFG
		.endm

TED_SPI_ARD_OFF	.macro
		lda	#(1 << TED_SPI_AREAD)
		trb	TED_BASE_ADDR + TED_REG_SPI_CFG
		.endm

TED_SPI_WAIT	.macro
.wait\@:	tst	#(1 << TED_FLG_SPI), TED_BASE_ADDR + TED_REG_STATE
		beq	.wait\@
		.endm
