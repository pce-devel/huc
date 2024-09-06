// **************************************************************************
// **************************************************************************
//
// pcengine.h
//
// KickC definitions for PC Engine hardware and System Card vars/funcs.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#ifndef __PCE__
#error "Target platform must be PCE"
#endif



// **************************************************************************
//
// HuC6270 - Video Display Controller
//

byte * const	VDC_SR		= (byte *) 0x0200;	// Status Register
byte * const	VDC_AR		= (byte *) 0x0200;	// Address Register
byte * const	VDC_DL		= (byte *) 0x0202;	// Data (Read/Write) Low
byte * const	VDC_DH		= (byte *) 0x0203;	// Data (Read/Write) High
word * const	VDC_DW		= (word *) 0x0202;	// Data (Read/Write) Word

byte * const	SGX_SR		= (byte *) 0x0210;	// Status Register
byte * const	SGX_AR		= (byte *) 0x0210;	// Address Register
byte * const	SGX_DL		= (byte *) 0x0212;	// Data (Read/Write) Low
byte * const	SGX_DH		= (byte *) 0x0213;	// Data (Read/Write) High
word * const	SGX_DW		= (word *) 0x0202;	// Data (Read/Write) Word

const byte	VDC_MAWR	= 0;		// Memory Address Write
const byte	VDC_MARR	= 1;		// Memory Address Read
const byte	VDC_VWR		= 2;		// VRAM Data Write
const byte	VDC_VRR		= 2;		// VRAM Data Read
const byte	VDC_CR		= 5;		// Control
const byte	VDC_RCR		= 6;		// Raster Counter Register
const byte	VDC_BXR		= 7;		// BGX Scroll
const byte	VDC_BYR		= 8;		// BGY Scroll
const byte	VDC_MWR		= 9;		// Memory Access Width
const byte	VDC_HSR		= 10;		// Horizontal Sync
const byte	VDC_HDR		= 11;		// Horizontal Display
const byte	VDC_VPR		= 12;		// Vertical Sync
const byte	VDC_VDW		= 13;		// Vertical Display
const byte	VDC_VCR		= 14;		// Vertical Display End Position
const byte	VDC_DCR		= 15;		// Block Transfer Control
const byte	VDC_SOUR	= 16;		// Block Transfer Source
const byte	VDC_DESR	= 17;		// Block Transfer Destination
const byte	VDC_LENR	= 18;		// Block Transfer Length
const byte	VDC_DVSSR	= 19;		// VRAM-SATB Block Transfer Source



// **************************************************************************
//
// HuC6202 - Video Priority Controller (SGX-only)
//

word * const	VPC_CR		= (word *) 0x0208;
byte * const	VPC_CRL		= (byte *) 0x0208;
byte * const	VPC_CRH		= (byte *) 0x0209;

word * const	VPC_WIN1	= (word *) 0x020A;
byte * const	VPC_WIN1L	= (byte *) 0x020A;
byte * const	VPC_WIN1H	= (byte *) 0x020B;

word * const	VPC_WIN2	= (word *) 0x020C;
byte * const	VPC_WIN2L	= (byte *) 0x020C;
byte * const	VPC_WIN2H	= (byte *) 0x020D;

byte * const	VPC_VDC_REDIR	= (byte *) 0x020E;



// **************************************************************************
//
// HuC6260 - Video Color Encoder
//
// Palette Data is 0000000gggrrrbbb
//

word * const	VCE_CR		= (word *) 0x0400;	// Control Register
word * const	VCE_CTA		= (word *) 0x0402;	// Color Table Address
word * const	VCE_CTW		= (word *) 0x0404;	// Color Table Data Write
word * const	VCE_CTR		= (word *) 0x0404;	// Color Table Data Read



// **************************************************************************
//
// HuC6280 - Programmable Sound Generator
//

byte * const	PSG_R0		= (byte *) 0x0800;	// Channel Select
byte * const	PSG_R1		= (byte *) 0x0801;	// Main Amplitude Level
byte * const	PSG_R2		= (byte *) 0x0802;	// Frequency Low
byte * const	PSG_R3		= (byte *) 0x0803;	// Frequency High
byte * const	PSG_R4		= (byte *) 0x0804;	// Control & Channel Amplitude
byte * const	PSG_R5		= (byte *) 0x0805;	// L/R Amplitude Level
byte * const	PSG_R6		= (byte *) 0x0806;	// Waveform
byte * const	PSG_R7		= (byte *) 0x0807;	// Noise
byte * const	PSG_R8		= (byte *) 0x0808;	// LFO Frequency
byte * const	PSG_R9		= (byte *) 0x0809;	// LFO Control

byte * const	PSG_AR		= (byte *) 0x0800;	// Channel Select
byte * const	PSG_MAL		= (byte *) 0x0801;	// Main Amplitude Level
byte * const	PSG_FL		= (byte *) 0x0802;	// Frequency Low
byte * const	PSG_FH		= (byte *) 0x0803;	// Frequency High
byte * const	PSG_CR		= (byte *) 0x0804;	// Control & Channel Amplitude
byte * const	PSG_LRAL	= (byte *) 0x0805;	// L/R Amplitude Level
byte * const	PSG_WF		= (byte *) 0x0806;	// Waveform
byte * const	PSG_NZ		= (byte *) 0x0807;	// Noise
byte * const	PSG_LFO_FRQ	= (byte *) 0x0808;	// LFO Frequency
byte * const	PSG_LFO_CR	= (byte *) 0x0809;	// LFO Control



// **************************************************************************
//
// HuC6280 - Timer
//

byte * const	TIMER_DR	= (byte *) 0x0C00;	// Timer Data
byte * const	TIMER_CR	= (byte *) 0x0C01;	// Timer Control



// **************************************************************************
//
// HuC6280 - I/O Port
//

byte * const	IO_PORT		= (byte *) 0x1000;

byte const	JOY_L		= 0x80;
byte const	JOY_D		= 0x40;
byte const	JOY_R		= 0x20;
byte const	JOY_U		= 0x10;
byte const	JOY_RUN		= 0x08;
byte const	JOY_SEL		= 0x04;
byte const	JOY_B2		= 0x02;
byte const	JOY_B1		= 0x01;

byte const	PORT_CLR	= 0x02;
byte const	PORT_SEL	= 0x01;



// **************************************************************************
//
// HuC6280 - Interrupt Control
//

byte * const	IRQ_MSK		= (byte *) 0x1402;	// Interrupt Disable
byte * const	IRQ_REQ		= (byte *) 0x1403;	// Interrupt Request
byte * const	IRQ_ACK		= (byte *) 0x1403;	// Interrupt Acknowledge



// **************************************************************************
//
// CDROM/ADPCM hardware registers.
//

byte * const	IFU_SCSI_CTRL	= (byte *) 0x1800; // -W : SCSI control signals out.
byte * const	IFU_SCSI_FLGS	= (byte *) 0x1800; // R- : SCSI control signals in.
byte * const	IFU_SCSI_DATA	= (byte *) 0x1801; // RW : SCSI data bus.
byte * const	IFU_SCSI_ACK	= (byte *) 0x1802;

byte * const	IFU_IRQ_CTL	= (byte *) 0x1802;
byte * const	IFU_IRQ_FLG	= (byte *) 0x1803; // flags reporting CD/ADPCM info.

byte * const	IFU_HW_RESET	= (byte *) 0x1804;

byte * const	IFU_SCSI_AUTO	= (byte *) 0x1808; // RO - SCSI auto-handshake read.

byte * const	IFU_ADPCM_LSB	= (byte *) 0x1808;
byte * const	IFU_ADPCM_MSB	= (byte *) 0x1809;
byte * const	IFU_ADPCM_DAT	= (byte *) 0x180A;
byte * const	IFU_ADPCM_DMA	= (byte *) 0x180B;
byte * const	IFU_ADPCM_FLG	= (byte *) 0x180C;
byte * const	IFU_ADPCM_CTL	= (byte *) 0x180D;
byte * const	IFU_ADPCM_SPD	= (byte *) 0x180E;

byte * const	IFU_AUDIO_FADE	= (byte *) 0x180F;

byte * const	IFU_BRAM_LOCK	= (byte *) 0x1803;
byte * const	IFU_BRAM_UNLOCK	= (byte *) 0x1807;

// IFU interrupt bits in IFU_IRQ_CTL/IFU_IRQ_FLG

byte const	IFU_INT_HALF	= 0x04;	// ADPCM < 50% remaining.
byte const	IFU_INT_STOP	= 0x08;	// ADPCM finished.
byte const	IFU_INT_SUB	= 0x10;
byte const	IFU_INT_MSG_IN	= 0x20;	// SCSI MIN phase.
byte const	IFU_INT_DAT_IN	= 0x40;	// SCSI DIN phase.

// ADPCM control bits in IFU_ADPCM_CTL

byte const	ADPCM_WR_CLK	= 0x01;
byte const	ADPCM_SET_WR	= 0x02;
byte const	ADPCM_RD_CLK	= 0x04;
byte const	ADPCM_SET_RD	= 0x08;
byte const	ADPCM_SET_SZ	= 0x10;
byte const	ADPCM_PLAY	= 0x20;
byte const	ADPCM_INCR	= 0x40;
byte const	ADPCM_RESET	= 0x80;

// ADPCM status bits in IFU_ADPCM_FLG

byte const	ADPCM_AD_END	= 0x01;
byte const	ADPCM_WR_BSY	= 0x04;
byte const	ADPCM_AD_BSY	= 0x08;
byte const	ADPCM_RD_BSY	= 0x80;



// **************************************************************************
//
// Arcade Card
//

word * const	ACD_DAT0	= (word *) 0x1A00;	// 2
word * const	ACD_ADR0	= (word *) 0x1A02;	// 2
word * const	ACD_OFF0	= (word *) 0x1A05;	// 2
word * const	ACD_INC0	= (word *) 0x1A07;	// 2
byte * const	ACD_CTL0	= (byte *) 0x1A09;	// 1
byte * const	ACD_TRG0	= (byte *) 0x1A0A;	// 1

byte * const	ACD_D0L		= (byte *) 0x1A00;	// 1
byte * const	ACD_D0H		= (byte *) 0x1A01;	// 1
byte * const	ACD_A0L		= (byte *) 0x1A02;	// 1
byte * const	ACD_A0M		= (byte *) 0x1A03;	// 1
byte * const	ACD_A0H		= (byte *) 0x1A04;	// 1
byte * const	ACD_O0L		= (byte *) 0x1A05;	// 1
byte * const	ACD_O0H		= (byte *) 0x1A06;	// 1
byte * const	ACD_I0L		= (byte *) 0x1A07;	// 1
byte * const	ACD_I0H		= (byte *) 0x1A08;	// 1

word * const	ACD_DAT1	= (word *) 0x1A10;	// 2
byte * const	ACD_ADR1	= (byte *) 0x1A12;	// 3
word * const	ACD_OFF1	= (word *) 0x1A15;	// 2
word * const	ACD_INC1	= (word *) 0x1A17;	// 2
byte * const	ACD_CTL1	= (byte *) 0x1A19;	// 1
byte * const	ACD_TRG1	= (byte *) 0x1A1A;	// 1

byte * const	ACD_D1L		= (byte *) 0x1A10;	// 1
byte * const	ACD_D1H		= (byte *) 0x1A11;	// 1
byte * const	ACD_A1L		= (byte *) 0x1A12;	// 1
byte * const	ACD_A1M		= (byte *) 0x1A13;	// 1
byte * const	ACD_A1H		= (byte *) 0x1A14;	// 1
byte * const	ACD_O1L		= (byte *) 0x1A15;	// 1
byte * const	ACD_O1H		= (byte *) 0x1A16;	// 1
byte * const	ACD_I1L		= (byte *) 0x1A17;	// 1
byte * const	ACD_I1H		= (byte *) 0x1A18;	// 1

word * const	ACD_DAT2	= (word *) 0x1A20;	// 2
byte * const	ACD_ADR2	= (byte *) 0x1A22;	// 3
word * const	ACD_OFF2	= (word *) 0x1A25;	// 2
word * const	ACD_INC2	= (word *) 0x1A27;	// 2
byte * const	ACD_CTL2	= (byte *) 0x1A29;	// 1
byte * const	ACD_TRG2	= (byte *) 0x1A2A;	// 1

byte * const	ACD_D2L		= (byte *) 0x1A20;	// 1
byte * const	ACD_D2H		= (byte *) 0x1A21;	// 1
byte * const	ACD_A2L		= (byte *) 0x1A22;	// 1
byte * const	ACD_A2M		= (byte *) 0x1A23;	// 1
byte * const	ACD_A2H		= (byte *) 0x1A24;	// 1
byte * const	ACD_O2L		= (byte *) 0x1A25;	// 1
byte * const	ACD_O2H		= (byte *) 0x1A26;	// 1
byte * const	ACD_I2L		= (byte *) 0x1A27;	// 1
byte * const	ACD_I2H		= (byte *) 0x1A28;	// 1

word * const	ACD_DAT3	= (word *) 0x1A30;	// 2
byte * const	ACD_ADR3	= (byte *) 0x1A32;	// 3
word * const	ACD_OFF3	= (word *) 0x1A35;	// 2
word * const	ACD_INC3	= (word *) 0x1A37;	// 2
byte * const	ACD_CTL3	= (byte *) 0x1A39;	// 1
byte * const	ACD_TRG3	= (byte *) 0x1A3A;	// 1

byte * const	ACD_D3L		= (byte *) 0x1A30;	// 1
byte * const	ACD_D3H		= (byte *) 0x1A31;	// 1
byte * const	ACD_A3L		= (byte *) 0x1A32;	// 1
byte * const	ACD_A3M		= (byte *) 0x1A33;	// 1
byte * const	ACD_A3H		= (byte *) 0x1A34;	// 1
byte * const	ACD_O3L		= (byte *) 0x1A35;	// 1
byte * const	ACD_O3H		= (byte *) 0x1A36;	// 1
byte * const	ACD_I3L		= (byte *) 0x1A37;	// 1
byte * const	ACD_I3H		= (byte *) 0x1A38;	// 1

byte * const	ACD_SHIFTREG	= (byte *) 0x1AE0;	// 4 bytes
byte * const	ACD_ASL_CNT	= (byte *) 0x1AE4;	// positive = shift left
byte * const	ACD_ROL_CNT	= (byte *) 0x1AE5;	// positive = shift left

byte * const	ACD_VERL	= (byte *) 0x1AFD;
byte * const	ACD_VERH	= (byte *) 0x1AFE;
byte * const	ACD_FLAG	= (byte *) 0x1AFF;

byte const	ACD_ID		= 0x51;		// if ac_identflag = AC_IDENT, then AC in use



// **************************************************************************
//
// System Card's Zero Page Variables (real addresses).
//

byte * const	zpg_grp_top	= (byte *) 0x20DC;
byte * const	vi_bitpat	= (byte *) 0x20DC;
byte * const	vi_rvbitpat	= (byte *) 0x20DD;
byte * const	vi_ft_front	= (byte *) 0x20DE;
byte * const	vi_padrs	= (byte *) 0x20DE;
byte * const	vi_porg		= (byte *) 0x20E0;
byte * const	vi_ft_back	= (byte *) 0x20E1;
byte * const	vi_stack	= (byte *) 0x20E4;

byte * const	zpg_psg_top	= (byte *) 0x20E6;
byte * const	time_sw		= (byte *) 0x20E6;	// psg irq mutex (NZ == already running)
byte * const	main_sw		= (byte *) 0x20E7;	// psg driver mode (0x80 == disable)
word * const	psg_si		= (word *) 0x20E8;	//
byte * const	psg_si_l	= (byte *) 0x20E8;	//
byte * const	psg_si_h	= (byte *) 0x20E9;	//
word * const	psg_r0		= (word *) 0x20EA;	//
byte * const	psg_r0_l	= (byte *) 0x20EA;	//
byte * const	psg_r0_h	= (byte *) 0x20EB;	//

byte * const	zpg_sys_top	= (byte *) 0x20EC;
word * const	_bp		= (word *) 0x20EC;	// base pointer
word * const	_si		= (word *) 0x20EE;	// source address
word * const	_di		= (word *) 0x20F0;	// destination address
byte * const	cdi_b		= (byte *) 0x20F2;	// ???
byte * const	vdc_crl		= (byte *) 0x20F3;	// shadow of VDC control register (lo-byte)
byte * const	vdc_crh		= (byte *) 0x20F4;	// shadow of VDC control register (hi-byte)
byte * const	irq_vec		= (byte *) 0x20F5;	// interrupt vector control mask
byte * const	vdc_sr		= (byte *) 0x20F6;	// shadow of VDC status register
byte * const	vdc_reg		= (byte *) 0x20F7;	// shadow of VDC register index

word * const	_ax		= (word *) 0x20F8;
byte * const	_al		= (byte *) 0x20F8;
byte * const	_ah		= (byte *) 0x20F9;

word * const	_bx		= (word *) 0x20FA;
byte * const	_bl		= (byte *) 0x20FA;
byte * const	_bh		= (byte *) 0x20FB;

word * const	_cx		= (word *) 0x20FC;
byte * const	_cl		= (byte *) 0x20FC;
byte * const	_ch		= (byte *) 0x20FD;

word * const	_dx		= (word *) 0x20FE;
byte * const	_dl		= (byte *) 0x20FE;
byte * const	_dh		= (byte *) 0x20FF;

word * const	__temp		= (word *) 0x2000;	// CORE(not TM) library variable!
byte * const	_bank		= (byte *) 0x2002;	// CORE(not TM) library variable!



// **************************************************************************
//
// System Card's Zero Page Variables (6502-style zero-page addresses).
//

#if 1

word * const	__bp		= (word *) 0xEC;	// base pointer
word * const	__si		= (word *) 0xEE;	// source address
word * const	__di		= (word *) 0xF0;	// destination address
byte * const	__cdi_b		= (byte *) 0xF2;	// ???
byte * const	__vdc_crl	= (byte *) 0xF3;	// shadow of VDC control register (lo-byte)
byte * const	__vdc_crh	= (byte *) 0xF4;	// shadow of VDC control register (hi-byte)
byte * const	__irq_vec	= (byte *) 0xF5;	// interrupt vector control mask
byte * const	__vdc_sr	= (byte *) 0xF6;	// shadow of VDC status register
byte * const	__vdc_reg	= (byte *) 0xF7;	// shadow of VDC register index

word * const	__ax		= (word *) 0xF8;
byte * const	__al		= (byte *) 0xF8;
byte * const	__ah		= (byte *) 0xF9;

word * const	__bx		= (word *) 0xFA;
byte * const	__bl		= (byte *) 0xFA;
byte * const	__bh		= (byte *) 0xFB;

word * const	__cx		= (word *) 0xFC;
byte * const	__cl		= (byte *) 0xFC;
byte * const	__ch		= (byte *) 0xFD;

word * const	__dx		= (word *) 0xFE;
byte * const	__dl		= (byte *) 0xFE;
byte * const	__dh		= (byte *) 0xFF;

word * const	__temp		= (word *) 0x00;	// CORE(not TM) library variable!
byte * const	__bank		= (byte *) 0x02;	// CORE(not TM) library variable!

#else

__export __address(0xEC) volatile word __bp;		// base pointer
__export __address(0xEE) volatile word __si;		// source address
__export __address(0xF0) volatile word __di;		// destination address
__export __address(0xF2) volatile byte __cdi_b;		// ???
__export __address(0xF3) volatile byte __vdc_crl;	// shadow of VDC control register (lo-byte)
__export __address(0xF4) volatile byte __vdc_crh;	// shadow of VDC control register (hi-byte)
__export __address(0xF5) volatile byte __irq_vec;	// interrupt vector control mask
__export __address(0xF6) volatile byte __vdc_sr;	// shadow of VDC status register
__export __address(0xF7) volatile byte __vdc_reg;	// shadow of VDC register index

__export __address(0xF8) volatile word __ax;
__export __address(0xF8) volatile byte __al;
__export __address(0xF9) volatile byte __ah;

__export __address(0xFA) volatile word __bx;
__export __address(0xFA) volatile byte __bl;
__export __address(0xFB) volatile byte __bh;

__export __address(0xFC) volatile word __cx;
__export __address(0xFC) volatile byte __cl;
__export __address(0xFD) volatile byte __ch;

__export __address(0xFE) volatile word __dx;
__export __address(0xFE) volatile byte __dl;
__export __address(0xFF) volatile byte __dh;

__export __address(0x00) volatile word __temp;		// CORE(not TM) library variable!
__export __address(0x02) volatile byte __bank;		// CORE(not TM) library variable!

#endif



// **************************************************************************
//
// System Card's Main RAM Variables.
//

__export __address(0x2200) word irq2_hook;	// = (word *) 0x2200; // 2	officially called irq2_jmp

// word * const	irq2_hook	= (word *) 0x2200; // 2	officially called irq2_jmp
word * const	irq1_hook	= (word *) 0x2202; // 2	officially called irq_jmp
word * const	timer_hook	= (word *) 0x2204; // 2	officially called tim_jmp
word * const	nmi_hook	= (word *) 0x2206; // 2	officially called nmi_jmp
word * const	vsync_hook	= (word *) 0x2208; // 2	officially called sync_jmp
word * const	hsync_hook	= (word *) 0x220A; // 2	officially called rcr_jmp
word * const	bg_x1		= (word *) 0x220C; // 2	officially called bgx1
word * const	bg_x2		= (word *) 0x220E; // 2
word * const	bg_y1		= (word *) 0x2210; // 2
word * const	bg_y2		= (word *) 0x2212; // 2
word * const	sat_addr	= (word *) 0x2214; // 2	officially called sat_adr
byte * const	sprptr		= (byte *) 0x2216; // 1
byte * const	spryl		= (byte *) 0x2217; // 1
byte * const	spryh		= (byte *) 0x2218; // 1
byte * const	sprxl		= (byte *) 0x2219; // 1
byte * const	sprxh		= (byte *) 0x221A; // 1
byte * const	sprnl		= (byte *) 0x221B; // 1
byte * const	sprnh		= (byte *) 0x221C; // 1
byte * const	spral		= (byte *) 0x221D; // 1
byte * const	sprah		= (byte *) 0x221E; // 1
byte * const	color_cmd	= (byte *) 0x221F; // 1
word * const	bgc_ptr		= (word *) 0x2220; // 2
byte * const	bgc_len		= (byte *) 0x2222; // 1
word * const	sprc_ptr	= (word *) 0x2223; // 2
byte * const	sprc_len	= (byte *) 0x2225; // 1
byte * const	joyena		= (byte *) 0x2227; // 1
byte * const	joynow		= (byte *) 0x2228; // 5	officially called joy
byte * const	joytrg		= (byte *) 0x222D; // 5
byte * const	joyold		= (byte *) 0x2232; // 5
byte * const	irq_cnt		= (byte *) 0x2241; // 1
byte * const	vdc_mwr		= (byte *) 0x2242; // 1	officially called mwr_m
byte * const	vdc_dcr		= (byte *) 0x2243; // 1	officially called dcr_m
byte * const	rndseed		= (byte *) 0x2249; // 1
byte * const	rndl		= (byte *) 0x2249; // 1
byte * const	rndh		= (byte *) 0x224A; // 1
byte * const	rndm		= (byte *) 0x224B; // 1
byte * const	scsisend	= (byte *) 0x224C; // 10	*UNDOCUMENTED* buffer for SCSI cmd send
byte * const	scsirecv	= (byte *) 0x2256; // 10	*UNDOCUMENTED* buffer for SCSI cmd recv
byte * const	paramcpy	= (byte *) 0x2260; // 8	*UNDOCUMENTED* stored _ax,_bx,_cx,_dx
byte * const	initialmpr	= (byte *) 0x2268; // 1	*UNDOCUMENTED*
//?		= 0x2269 // ?
byte * const	tnomin		= (byte *) 0x226A; // 1
byte * const	tnomax		= (byte *) 0x226B; // 1
byte * const	outmin		= (byte *) 0x226C; // 1
byte * const	outsec		= (byte *) 0x226D; // 1
byte * const	outfrm		= (byte *) 0x226E; // 1
byte * const	endmin		= (byte *) 0x226F; // 1	*UNDOCUMENTED*
byte * const	endsec		= (byte *) 0x2270; // 1	*UNDOCUMENTED*
byte * const	endfrm		= (byte *) 0x2271; // 1	*UNDOCUMENTED*
byte * const	vdtin_flg	= (byte *) 0x2272; // 1
byte * const	recbase		= (byte *) 0x2273; // 1	*UNDOCUMENTED* active recbase (0 or 1)
byte * const	recbase0_h	= (byte *) 0x2274; // 1
byte * const	recbase0_m	= (byte *) 0x2275; // 1
byte * const	recbase0_l	= (byte *) 0x2276; // 1
byte * const	recbase1_h	= (byte *) 0x2277; // 1
byte * const	recbase1_m	= (byte *) 0x2278; // 1
byte * const	recbase1_l	= (byte *) 0x2279; // 1
byte * const	scsiphs		= (byte *) 0x227A; // 1	*UNDOCUMENTED* SCSI command phase
byte * const	scsists		= (byte *) 0x227B; // 1
byte * const	suberrc		= (byte *) 0x227C; // 1
byte * const	scsisns		= (byte *) 0x227D; // 1	*UNDOCUMENTED* SCSI "sense" return code
byte * const	subcode		= (byte *) 0x227E; // 1

word * const	reset_hook	= (word *) 0x2284; // 2	*UNDOCUMENTED* joypad soft-reset hook
byte * const	color_tia	= (byte *) 0x2286; // 8	*UNDOCUMENTED* self-mod TIA used by ex_colorcmd
//?				= 0x228E; // 3	*UNDOCUMENTED* self-mod TMA/TAM used by cd_read/ad_read/ad_write
//?				= 0x2291; // 10	*UNDOCUMENTED* copy of scsisend made by cd_read (never read)
//?				= 0x229B; // ?

//mprsav			= 0x22B5; // 8 backups of MPR when BIOS needs major bank swapping.

byte * const	ramend		= (byte *) 0x22D0; // 1

byte * const	psg_work_top	= (byte *) 0x22D0;
byte * const	psg_reg		= (byte *) 0x22ED; // 1 shadow for selected PSG register, like vdc_reg

// ...

void * const	graph_work_top	= (void *) 0x2616;
void * const	key_work_top	= (void *) 0x2646;
void * const	user_work_top	= (void *) 0x267C;



// **************************************************************************
//
// System Card's Function Vectors
//

// #include "bios.h"



// **************************************************************************
//
// Standard Display Timings.
//

// VDC constants for VRAM access speed.

word const	VDC_MWR_1CYCLE	= 0x0000;
word const	VDC_MWR_2CYCLE	= 0x000A;

// VDC constants for 224, 240 & 256 wide display (Set VDC_MWR_1CYCLE).

word const	VCE_CR_5MHz	= 0x00;	   // 43 chr (actually 42.8) -> 342 pixel slots.

word const	VDC_HSR_224	= 0x0402; // HDS HSW
word const	VDC_HDR_224	= 0x061B; // HDE HDW

word const	VDC_HSR_240	= 0x0302; // HDS HSW (Reduced)
word const	VDC_HDR_240	= 0x051D; // HDE HDW

word const	VDC_HSR_256	= 0x0202; // HDS HSW (Normal)
word const	VDC_HDR_256	= 0x041F; // HDE HDW

// VDC constants for 320, 336, 344 & 352 wide display (Set VDC_MWR_1CYCLE).

word const	VCE_CR_7MHz	= 0x01;	   // 57 chr (actually 57.0) -> 456 pixel slots.

word const	VDC_HSR_320	= 0x0503; // HDS HSW
word const	VDC_HDR_320	= 0x0627; // HDE HDW

word const	VDC_HSR_336	= 0x0403; // HDS HSW (Reduced)
word const	VDC_HDR_336	= 0x0529; // HDE HDW

word const	VDC_HSR_352	= 0x0303; // HDS HSW (Normal)
word const	VDC_HDR_352	= 0x042B; // HDE HDW

// VDC constants for 480 & 512 wide display (Set VDC_MWR_2CYCLE).

word const	VCE_CR_10MHz	= 0x02;	   // 86 chr (actually 85.5) -> 684 pixel slots.

word const	VDC_HSR_480	= 0x0D05; // HDS HSW (Reduced)
word const	VDC_HDR_480	= 0x053B; // HDE HDW

word const	VDC_HSR_512	= 0x0B05; // HDS HSW (Normal)
word const	VDC_HDR_512	= 0x033F; // HDE HDW

// VDC constants for 200, 208, 224 & 240 high display.
//
// N.B. 2 lines higher on the screen than the System Card.

word const	VDC_VPR_200	= 0x2102;
word const	VDC_VDW_200	= 0x00C7;
word const	VDC_VCR_200	= 0x0016; // bios sets 0x00E2

word const	VDC_VPR_208	= 0x1E02;
word const	VDC_VDW_208	= 0x00CF;
word const	VDC_VCR_208	= 0x0013; // bios sets 0x00EE

word const	VDC_VPR_224	= 0x1502;
word const	VDC_VDW_224	= 0x00DF;
word const	VDC_VCR_224	= 0x000B; // bios sets 0x00EE

word const	VDC_VPR_240	= 0x0D02;
word const	VDC_VDW_240	= 0x00EF;
word const	VDC_VCR_240	= 0x0003; // bios sets 0x00F6

// VDC constants for different BAT screen sizes.

word const	VDC_MWR_32x32	= 0x0000;
word const	VDC_MWR_32x64	= 0x0040;

word const	VDC_MWR_64x32	= 0x0010;
word const	VDC_MWR_64x64	= 0x0050;

word const	VDC_MWR_128x32	= 0x0020;
word const	VDC_MWR_128x64	= 0x0060;
