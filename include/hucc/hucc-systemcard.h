#ifndef _hucc_systemcard_h
#define _hucc_systemcard_h

/****************************************************************************
; ***************************************************************************
;
; hucc-systemcard.h
;
; Macros and library functions for using the System Card.
;
; Copyright John Brandwood 2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; Because these are mainly macros, and so must be included before being used
; in compiled code, the actual functions here are written to avoid using any
; BSS memory so that HuCC's overlay global-shared-variables are not effected.
;
; ***************************************************************************
; **************************************************************************/

// *************
// Backup RAM defines ...
// *************

#define  BM_OK             0
#define  BM_NOT_FOUND      1
#define  BM_BAD_CHECKSUM   2
#define  BM_DIR_CORRUPTED  3
#define  BM_FILE_EMPTY     4
#define  BM_FULL           5
#define  BM_NOT_FORMATED   0xFF			// HuC incorrect spelling.
#define  BM_NOT_FORMATTED  0xFF			// HuCC can use a dictionary!

#define  BRAM_STARTPTR     0x8010

// *************
// CD defines ...
// *************

#define	CDPLAY_MUTE		0
#define	CDPLAY_REPEAT		1
#define	CDPLAY_NORMAL		2
#define	CDPLAY_ENDOFDISC	0

#define	CDFADE_CANCEL	0
#define	CDFADE_PCM6	8
#define	CDFADE_ADPCM6	10
#define	CDFADE_PCM2	12
#define	CDFADE_ADPCM2	14

#define CDTRK_AUDIO	0
#define CDTRK_DATA	4

// *************
// ADPCM defines ...
// *************

#define	ADPLAY_AUTOSTOP		0
#define	ADPLAY_REPEAT		0x80

#define ADPLAY_FREQ_16KHZ	0xE
#define ADPLAY_FREQ_10KHZ	0xD
#define ADPLAY_FREQ_8KHZ	0xC
#define ADPLAY_FREQ_6KHZ	0xB
#define ADPLAY_FREQ_5KHZ	0xA

#define ADREAD_RAM	0
#define ADREAD_VRAM	0xFF

#define ADWRITE_RAM	0
#define ADWRITE_VRAM	0xFF

// *************
// Functions in hucc-systemcard.asm ...
// *************

#ifdef __HUCC__

#asmdef	HUCC_USES_SYSTEMCARD 1

extern void __fastcall __macro cd_boot( void );
extern unsigned int __fastcall __macro cd_getver( void );
extern void __fastcall __macro cd_reset( void );
extern unsigned char __fastcall __macro cd_pause( void );
extern unsigned char __fastcall cd_unpause( void );
extern void __fastcall __macro cd_fade( unsigned char type<acc> );
extern unsigned char __fastcall cd_playtrk( unsigned char start_track<_bx>, unsigned char end_track<_cx>, unsigned char mode<_dh> );
extern unsigned char __fastcall cd_playmsf( unsigned char start_minute<_al>,  unsigned char start_second<_ah>,  unsigned char start_frame<_bl>, unsigned char end_minute<_cl>,  unsigned char end_second<_ch>,  unsigned char end_frame<_dl>,  unsigned char mode<_dh> );
extern unsigned char __fastcall cd_loadvram( unsigned char ovl_index<_cl>, unsigned int sect_offset<_si>, unsigned int vramaddr<_bx>, unsigned int bytes<_ax> );
extern unsigned char __fastcall cd_loaddata( unsigned char ovl_index<_cl>, unsigned int sect_offset<_si>, unsigned char __far *buffer<_bp_bank:_bp>, unsigned int bytes<__ptr> );
extern unsigned char __fastcall cd_loadbank( unsigned char ovl_index<_cl>, unsigned int sect_offset<_si>, unsigned char bank<_bl>, unsigned int sectors<_al> );
extern unsigned char __fastcall __macro cd_status( unsigned char mode<acc> );

extern void __fastcall __macro ad_reset( void );
extern unsigned char __fastcall __macro ad_trans( unsigned char ovl_index<_cl>, unsigned int sect_offset<_si>, unsigned char nb_sectors<_dh>, unsigned int ad_addr<_bx> );
extern void __fastcall __macro ad_read( unsigned int ad_addr<_cx>, unsigned char mode<_dh>, unsigned int buf<_bx>, unsigned int bytes<_ax> );
extern void __fastcall __macro ad_write( unsigned int ad_addr<_cx>, unsigned char mode<_dh>, unsigned int buf<_bx>, unsigned int bytes<_ax> );
extern unsigned char __fastcall __macro ad_play( unsigned int ad_addr<_bx>, unsigned int bytes<_ax>, unsigned char freq<_dh>, unsigned char mode<_dl> );
extern unsigned char __fastcall __macro ad_cplay( unsigned char ovl_index<_cl>, unsigned int sect_offset<_si>, unsigned int nb_sectors<_bx>, unsigned char freq<_dh> );
extern void __fastcall __macro ad_stop( void );
extern unsigned char __fastcall __macro ad_stat( void );

extern unsigned char __fastcall bm_check( void );
extern unsigned char __fastcall bm_format( void );
extern unsigned int __fastcall __macro bm_free( void );
extern unsigned char __fastcall __macro bm_read( unsigned char *buffer<_bx>, unsigned char *name<_ax>, unsigned int offset<_dx>, unsigned int length<_cx> );
extern unsigned char __fastcall __macro bm_write( unsigned char *buffer<_bx>, unsigned char *name<_ax>, unsigned int offset<_dx>, unsigned int length<_cx> );
extern unsigned char __fastcall __macro bm_delete( unsigned char *name<_ax> );

// Deprecated functions ...

extern unsigned char __fastcall __macro bm_exist( unsigned char *name<_ax> );
extern unsigned char __fastcall __macro bm_create( unsigned char *name<_ax>, unsigned int length<_cx> );

// void __fastcall _xsafe add_sectors( unsigned int sector_offset<acc> );

#endif // __HUCC__

#endif // _hucc_systemcard_h
