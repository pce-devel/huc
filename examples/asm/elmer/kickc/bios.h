// **************************************************************************
// **************************************************************************
//
// bios.h
//
// KickC definitions for PC Engine hardware and System Card vars/funcs.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#ifndef _PCE_
#error "Target platform must be PCE"
#endif



// **************************************************************************
//
// System Card's Function Vectors
//

void * const	cd_boot		= (void *) 0xE000;
void * const	cd_reset	= (void *) 0xE003;
void * const	cd_base		= (void *) 0xE006;
void * const	cd_read		= (void *) 0xE009;
void * const	cd_seek		= (void *) 0xE00C;
void * const	cd_exec		= (void *) 0xE00F;
void * const	cd_play		= (void *) 0xE012;
void * const	cd_search	= (void *) 0xE015;
void * const	cd_pause	= (void *) 0xE018;
void * const	cd_stat		= (void *) 0xE01B;
void * const	cd_subq		= (void *) 0xE01E;
void * const	cd_dinfo	= (void *) 0xE021;
void * const	cd_contnts	= (void *) 0xE024;
void * const	cd_subrd	= (void *) 0xE027;
void * const	cd_pcmrd	= (void *) 0xE02A;
void * const	cd_fade		= (void *) 0xE02D;

void * const	ad_reset	= (void *) 0xE030;
void * const	ad_trans	= (void *) 0xE033;
void * const	ad_read		= (void *) 0xE036;
void * const	ad_write	= (void *) 0xE039;
void * const	ad_play		= (void *) 0xE03C;
void * const	ad_cplay	= (void *) 0xE03F;
void * const	ad_stop		= (void *) 0xE042;
void * const	ad_stat		= (void *) 0xE045;

void * const	bm_format	= (void *) 0xE048;
void * const	bm_free		= (void *) 0xE04B;
void * const	bm_read		= (void *) 0xE04E;
void * const	bm_write	= (void *) 0xE051;
void * const	bm_delete	= (void *) 0xE054;
void * const	bm_files	= (void *) 0xE057;

void * const	ex_getver	= (void *) 0xE05A;
void * const	ex_setvec	= (void *) 0xE05D;
void * const	ex_getfnt	= (void *) 0xE060;
void * const	ex_joysns	= (void *) 0xE063;
void * const	ex_joyrep	= (void *) 0xE066;
void * const	ex_scrsiz	= (void *) 0xE069;
void * const	ex_dotmod	= (void *) 0xE06C;
void * const	ex_scrmod	= (void *) 0xE06F;
void * const	ex_imode	= (void *) 0xE072;
void * const	ex_vmode	= (void *) 0xE075;
void * const	ex_hmode	= (void *) 0xE078;
void * const	ex_vsync	= (void *) 0xE07B;
void * const	ex_rcron	= (void *) 0xE07E;
void * const	ex_rcroff	= (void *) 0xE081;
void * const	ex_irqon	= (void *) 0xE084;
void * const	ex_irqoff	= (void *) 0xE087;
void * const	ex_bgon		= (void *) 0xE08A;
void * const	ex_bgoff	= (void *) 0xE08D;
void * const	ex_spron	= (void *) 0xE090;
void * const	ex_sproff	= (void *) 0xE093;
void * const	ex_dspon	= (void *) 0xE096;
void * const	ex_dspoff	= (void *) 0xE099;
void * const	ex_dmamod	= (void *) 0xE09C;
void * const	ex_sprdma	= (void *) 0xE09F;
void * const	ex_satclr	= (void *) 0xE0A2;
void * const	ex_sprput	= (void *) 0xE0A5;
void * const	ex_setrcr	= (void *) 0xE0A8;
void * const	ex_setred	= (void *) 0xE0AB;
void * const	ex_setwrt	= (void *) 0xE0AE;
void * const	ex_setdma	= (void *) 0xE0B1;
void * const	ex_binbcd	= (void *) 0xE0B4;
void * const	ex_bcdbin	= (void *) 0xE0B7;
void * const	ex_rnd		= (void *) 0xE0BA;

void * const	ma_mul8u	= (void *) 0xE0BD;
void * const	ma_mul8s	= (void *) 0xE0C0;
void * const	ma_mul16u	= (void *) 0xE0C3;
void * const	ma_div16s	= (void *) 0xE0C6;
void * const	ma_div16u	= (void *) 0xE0C9;
void * const	ma_sqrt		= (void *) 0xE0CC;
void * const	ma_sin		= (void *) 0xE0CF;
void * const	ma_cos		= (void *) 0xE0D2;
void * const	ma_atni		= (void *) 0xE0D5;

void * const	psg_bios	= (void *) 0xE0D8;
void * const	grp_bios	= (void *) 0xE0DB;

void * const	ex_memopen	= (void *) 0xE0DE;

void * const	psg_driver	= (void *) 0xE0E1;

void * const	ex_colorcmd	= (void *) 0xE0E4;

byte * const	max_mapping	= (byte *) 0xFFF5;



// **************************************************************************
//
// System Card's PSG BIOS call functions.
//

byte const	PSG_ON		= 0;
byte const	PSG_OFF		= 1;
byte const	PSG_INIT	= 2;
byte const	PSG_BANK	= 3;
byte const	PSG_TRACK	= 4;
byte const	PSG_WAVE	= 5;
byte const	PSG_ENV		= 6;
byte const	PSG_FM		= 7;
byte const	PSG_PE		= 8;
byte const	PSG_PC		= 9;
byte const	PSG_TEMPO	= 10;
byte const	PSG_PLAY	= 11;
byte const	PSG_MSTAT	= 12;
byte const	PSG_SSTAT	= 13;
byte const	PSG_MSTOP	= 14;
byte const	PSG_SSTOP	= 15;
byte const	PSG_ASTOP	= 16;
byte const	PSG_MVOFF	= 17;
byte const	PSG_CONT	= 18;
byte const	PSG_FDOUT	= 19;
byte const	PSG_DCNT	= 20;



// **************************************************************************
//
// System Card's GRP BIOS call functions.
//

byte const	VI_GINIT	= 0;
byte const	VI_CASHCLR	= 1;
byte const	VI_STRTADR	= 2;
byte const	VI_GETADRS	= 3;
byte const	VI_CLS		= 4;
byte const	VI_PSET		= 5;
byte const	VI_POINT	= 6;
byte const	VI_LINE		= 7;
byte const	VI_BOX		= 8;
byte const	VI_BOXF		= 9;
byte const	VI_FLOOD	= 10;
byte const	VI_PAINT	= 11;
byte const	VI_GWINDOW	= 12;
byte const	VI_GFONT	= 13;
byte const	VI_PUTFONT	= 14;
byte const	VI_SYMBOL	= 15;



// **************************************************************************
//
// KickC wrappers for the System Card's Functions
//
// N.B. This is a very early work-in-progress!
//
// N.B. This will have to be done differently to work with the CORE library.
//

//
//
//

inline byte _ex_getfnt (word sjis, word addr, byte type) {
	byte ret;
	*__ax = sjis;
	*__bx = addr;
	*__dh = type;
	asm( clobbers "AXY" ) {
	jsr ex_getfnt
	sta ret
	}
	return ret;
}

//
//
//

inline word _ma_mul8u (byte multiplicand, byte multiplier) {
	*__al = multiplicand;
	*__bl = multiplier;
	asm( clobbers "AXY" ) {
	jsr ma_mul8u
	}
	return *__cx;
}

//
//
//

inline int _ma_mul8s (signed char multiplicand, signed char multiplier) {
	*__al = (byte) multiplicand;
	*__bl = (byte) multiplier;
	asm( clobbers "AXY" ) {
	jsr ma_mul8u
	}
	return (int) *__cx;
}
