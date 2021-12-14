; ***************************************************************************
; ***************************************************************************
;
; pcengine.inc
;
; Definitions for PC Engine hardware and System Card variables/functions.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************



; ***************************************************************************
;
; HuC6270 - Video Display Controller
;

VDC_SR		= $0200		; Status Register
VDC_AR		= $0200		; Address Register
VDC_DL		= $0202		; Data (Read/Write) Low
VDC_DH		= $0203		; Data (Read/Write) High

SGX_SR		= $0210		; Status Register
SGX_AR		= $0210		; Address Register
SGX_DL		= $0212		; Data (Read/Write) Low
SGX_DH		= $0213		; Data (Read/Write) High

VDC_MAWR	= 0		; Memory Address Write
VDC_MARR	= 1		; Memory Address Read
VDC_VWR		= 2		; VRAM Data Write
VDC_VRR		= 2		; VRAM Data Read
VDC_CR		= 5		; Control
VDC_RCR		= 6		; Raster Counter Register
VDC_BXR		= 7		; BGX Scroll
VDC_BYR		= 8		; BGY Scroll
VDC_MWR		= 9		; Memory Access Width
VDC_HSR		= 10		; Horizontal Sync
VDC_HDR		= 11		; Horizontal Display
VDC_VPR		= 12		; Vertical Sync
VDC_VDW		= 13		; Vertical Display
VDC_VCR		= 14		; Vertical Display End Position
VDC_DCR		= 15		; Block Transfer Control
VDC_SOUR	= 16		; Block Transfer Source
VDC_DESR	= 17		; Block Transfer Destination
VDC_LENR	= 18		; Block Transfer Length
VDC_DVSSR	= 19		; VRAM-SATB Block Transfer Source



; ***************************************************************************
;
; HuC6202 - Video Priority Controller (SGX-only)
;

VPC_CR		= $0208
VPC_CRL		= $0208
VPC_CRH		= $0209

VPC_WIN1	= $020A
VPC_WIN1L	= $020A
VPC_WIN1H	= $020B

VPC_WIN2	= $020C
VPC_WIN2L	= $020C
VPC_WIN2H	= $020D

VPC_VDC_REDIR	= $020E



; ***************************************************************************
;
; HuC6260 - Video Color Encoder
;
; Palette Data is 0000000gggrrrbbb
;

VCE_CR		= $0400		; Control Register
VCE_CTA		= $0402		; Color Table Address
VCE_CTW		= $0404		; Color Table Data Write
VCE_CTR		= $0404		; Color Table Data Read



; ***************************************************************************
;
; HuC6280 - Programmable Sound Generator
;

PSG_R0		= $0800		; Channel Select
PSG_R1		= $0801		; Main Amplitude Level
PSG_R2		= $0802		; Frequency Low
PSG_R3		= $0803		; Frequency High
PSG_R4		= $0804		; Control & Channel Amplitude
PSG_R5		= $0805		; L/R Amplitude Level
PSG_R6		= $0806		; Waveform
PSG_R7		= $0807		; Noise
PSG_R8		= $0808		; LFO Frequency
PSG_R9		= $0809		; LFO Control

PSG_AR		= $0800		; Channel Select
PSG_MAL		= $0801		; Main Amplitude Level
PSG_FL		= $0802		; Frequency Low
PSG_FH		= $0803		; Frequency High
PSG_CR		= $0804		; Control & Channel Amplitude
PSG_LRAL	= $0805		; L/R Amplitude Level
PSG_WF		= $0806		; Waveform
PSG_NZ		= $0807		; Noise
PSG_LFO_FRQ	= $0808		; LFO Frequency
PSG_LFO_CR	= $0809		; LFO Control



; ***************************************************************************
;
; HuC6280 - Timer
;

TIMER_DR	= $0C00		; Timer Data
TIMER_CR	= $0C01		; Timer Control



; ***************************************************************************
;
; HuC6280 - I/O Port
;

IO_PORT		= $1000

JOY_L		= $80
JOY_D		= $40
JOY_R		= $20
JOY_U		= $10
JOY_RUN		= $08
JOY_SEL		= $04
JOY_B2		= $02
JOY_B1		= $01

PORT_CLR	= $02
PORT_SEL	= $01



; ***************************************************************************
;
; HuC6280 - Interrupt Control
;

IRQ_MSK		= $1402		; Interrupt Disable
IRQ_REQ		= $1403		; Interrupt Request
IRQ_ACK		= $1403		; Interrupt Acknowledge



; ***************************************************************************
;
; CDROM/ADPCM hardware registers.
;

IFU_SCSI_CTRL	= $1800 ; -W : SCSI control signals out.
IFU_SCSI_FLGS	= $1800 ; R- : SCSI control signals in.
IFU_SCSI_DATA	= $1801 ; RW : SCSI data bus.
IFU_SCSI_ACK	= $1802

IFU_IRQ_CTL	= $1802
IFU_IRQ_FLG	= $1803 ; flags reporting CD/ADPCM info.

IFU_HW_RESET	= $1804

IFU_SCSI_AUTO	= $1808 ; RO - SCSI auto-handshake read.

IFU_ADPCM_LSB	= $1808
IFU_ADPCM_MSB	= $1809
IFU_ADPCM_DAT	= $180A
IFU_ADPCM_DMA	= $180B
IFU_ADPCM_FLG	= $180C
IFU_ADPCM_CTL	= $180D
IFU_ADPCM_SPD	= $180E

IFU_AUDIO_FADE	= $180F

IFU_BRAM_LOCK	= $1803
IFU_BRAM_UNLOCK	= $1807

; IFU interrupt bits in IFU_IRQ_CTL/IFU_IRQ_FLG

IFU_INT_HALF	= $04	; ADPCM < 50% remaining.
IFU_INT_STOP	= $08	; ADPCM finished.
IFU_INT_SUB	= $10
IFU_INT_MSG_IN	= $20	; SCSI MIN phase.
IFU_INT_DAT_IN	= $40	; SCSI DIN phase.

; ADPCM control bits in IFU_ADPCM_CTL

ADPCM_WR_CLK	= $01
ADPCM_SET_WR	= $02
ADPCM_RD_CLK	= $04
ADPCM_SET_RD	= $08
ADPCM_SET_SZ	= $10
ADPCM_PLAY	= $20
ADPCM_INCR	= $40
ADPCM_RESET	= $80

; ADPCM status bits in IFU_ADPCM_FLG

ADPCM_AD_END	= $01
ADPCM_WR_BSY	= $04
ADPCM_AD_BSY	= $08
ADPCM_RD_BSY	= $80



; ***************************************************************************
;
; Arcade Card
;

ACD_DAT0	= $1A00		; 2
ACD_ADR0	= $1A02		; 2
ACD_OFF0	= $1A05		; 2
ACD_INC0	= $1A07		; 2
ACD_CTL0	= $1A09		; 1
ACD_TRG0	= $1A0A		; 1

ACD_D0L		= $1A00		; 1
ACD_D0H		= $1A01		; 1
ACD_A0L		= $1A02		; 1
ACD_A0M		= $1A03		; 1
ACD_A0H		= $1A04		; 1
ACD_O0L		= $1A05		; 1
ACD_O0H		= $1A06		; 1
ACD_I0L		= $1A07		; 1
ACD_I0H		= $1A08		; 1

ACD_DAT1	= $1A10		; 2
ACD_ADR1	= $1A12		; 3
ACD_OFF1	= $1A15		; 2
ACD_INC1	= $1A17		; 2
ACD_CTL1	= $1A19		; 1
ACD_TRG1	= $1A1A		; 1

ACD_D1L		= $1A10		; 1
ACD_D1H		= $1A11		; 1
ACD_A1L		= $1A12		; 1
ACD_A1M		= $1A13		; 1
ACD_A1H		= $1A14		; 1
ACD_O1L		= $1A15		; 1
ACD_O1H		= $1A16		; 1
ACD_I1L		= $1A17		; 1
ACD_I1H		= $1A18		; 1

ACD_DAT2	= $1A20		; 2
ACD_ADR2	= $1A22		; 3
ACD_OFF2	= $1A25		; 2
ACD_INC2	= $1A27		; 2
ACD_CTL2	= $1A29		; 1
ACD_TRG2	= $1A2A		; 1

ACD_D2L		= $1A20		; 1
ACD_D2H		= $1A21		; 1
ACD_A2L		= $1A22		; 1
ACD_A2M		= $1A23		; 1
ACD_A2H		= $1A24		; 1
ACD_O2L		= $1A25		; 1
ACD_O2H		= $1A26		; 1
ACD_I2L		= $1A27		; 1
ACD_I2H		= $1A28		; 1

ACD_DAT3	= $1A30		; 2
ACD_ADR3	= $1A32		; 3
ACD_OFF3	= $1A35		; 2
ACD_INC3	= $1A37		; 2
ACD_CTL3	= $1A39		; 1
ACD_TRG3	= $1A3A		; 1

ACD_D3L		= $1A30		; 1
ACD_D3H		= $1A31		; 1
ACD_A3L		= $1A32		; 1
ACD_A3M		= $1A33		; 1
ACD_A3H		= $1A34		; 1
ACD_O3L		= $1A35		; 1
ACD_O3H		= $1A36		; 1
ACD_I3L		= $1A37		; 1
ACD_I3H		= $1A38		; 1

ACD_SHIFTREG	= $1AE0		; 4 bytes
ACD_ASL_CNT	= $1AE4		; positive = shift left
ACD_ROL_CNT	= $1AE5		; positive = shift left

ACD_VERL	= $1AFD
ACD_VERH	= $1AFE
ACD_FLAG	= $1AFF

ACD_ID		= $51		; if ac_identflag = AC_IDENT, then AC in use



; ***************************************************************************
;
; System Card's Zero Page Variables.
;

zpg_grp_top	= $20DC
vi_bitpat	= $20DC
vi_rvbitpat	= $20DD
vi_ft_front	= $20DE
vi_padrs	= $20DE
vi_porg		= $20E0
vi_ft_back	= $20E1
vi_stack	= $20E4

zpg_psg_top	= $20E6
time_sw		= $20E6		; psg irq mutex (NZ == already running)
main_sw		= $20E7		; psg driver mode ($80 == disable)
psg_si		= $20E8		;
psg_si_l	= $20E8		;
psg_si_h	= $20E9		;
psg_r0		= $20EA		;
psg_r0_l	= $20EA		;
psg_r0_h	= $20EB		;

zpg_sys_top	= $20EC
__bp		= $20EC		; base pointer
__si		= $20EE		; source address
__di		= $20F0		; destination address
cdi_b		= $20F2		; ???
vdc_crl		= $20F3		; shadow of VDC control register (lo-byte)
vdc_crh		= $20F4		; shadow of VDC control register (hi-byte)
irq_vec		= $20F5		; interrupt vector control mask
vdc_sr		= $20F6		; shadow of VDC status register
vdc_reg		= $20F7		; shadow of VDC register index

__ax		= $20F8
__al		= $20F8
__ah		= $20F9

__bx		= $20FA
__bl		= $20FA
__bh		= $20FB

__cx		= $20FC
__cl		= $20FC
__ch		= $20FD

__dx		= $20FE
__dl		= $20FE
__dh		= $20FF



; ***************************************************************************
;
; System Card's Main RAM Variables.
;

irq2_hook	= $2200 ; 2	officially called irq2_jmp
irq1_hook	= $2202 ; 2	officially called irq_jmp
timer_hook	= $2204 ; 2	officially called tim_jmp
nmi_hook	= $2206 ; 2	officially called nmi_jmp
vsync_hook	= $2208 ; 2	officially called sync_jmp
hsync_hook	= $220A ; 2	officially called rcr_jmp
bg_x1		= $220C ; 2	officially called bgx1
bg_x2		= $220E ; 2
bg_y1		= $2210 ; 2
bg_y2		= $2212 ; 2
satb_addr	= $2214 ; 2	officially called sat_adr
sprptr		= $2216 ; 1
spryl		= $2217 ; 1
spryh		= $2218 ; 1
sprxl		= $2219 ; 1
sprxh		= $221A ; 1
sprnl		= $221B ; 1
sprnh		= $221C ; 1
spral		= $221D ; 1
sprah		= $221E ; 1
color_cmd	= $221F ; 1
bgc_ptr		= $2220 ; 2
bgc_len		= $2222 ; 1
sprc_ptr	= $2223 ; 2
sprc_len	= $2225 ; 1
joyena		= $2227 ; 1
joynow		= $2228 ; 5	officially called joy
joytrg		= $222D ; 5
joyold		= $2232 ; 5
irq_cnt		= $2241 ; 1
vdc_mwr		= $2242 ; 1	officially called mwr_m
vdc_dcr		= $2243 ; 1	officially called dcr_m
rndseed		= $2249 ; 1
rndl		= $2249 ; 1
rndh		= $224A ; 1
rndm		= $224B ; 1
rndm		= $224B ; 1
scsisend	= $224C ; 10	*UNDOCUMENTED* buffer for SCSI cmd send
scsirecv	= $2256 ; 10	*UNDOCUMENTED* buffer for SCSI cmd recv
paramcpy	= $2260 ; 8	*UNDOCUMENTED* stored __ax,__bx,__cx,__dx
initialmpr	= $2268 ; 1	*UNDOCUMENTED*
;?		= $2269 ; ?
tnomin		= $226A ; 1
tnomax		= $226B ; 1
outmin		= $226C ; 1
outsec		= $226D ; 1
outfrm		= $226E ; 1
endmin		= $226F ; 1	*UNDOCUMENTED*
endsec		= $2270 ; 1	*UNDOCUMENTED*
endfrm		= $2271 ; 1	*UNDOCUMENTED*
vdtin_flg	= $2272 ; 1
recbase		= $2273 ; 1	*UNDOCUMENTED* active recbase (0 or 1)
recbase0_h	= $2274 ; 1
recbase0_m	= $2275 ; 1
recbase0_l	= $2276 ; 1
recbase1_h	= $2277 ; 1
recbase1_m	= $2278 ; 1
recbase1_l	= $2279 ; 1
scsiphs		= $227A ; 1	*UNDOCUMENTED* SCSI command phase
scsists		= $227B ; 1
suberrc		= $227C ; 1
scsisns		= $227D ; 1	*UNDOCUMENTED* SCSI "sense" return code
subcode		= $227E ; 1

reset_hook	= $2284 ; 2	*UNDOCUMENTED* joypad soft-reset hook
color_tia	= $2286 ; 8	*UNDOCUMENTED* self-mod TIA used by ex_colorcmd
;?		= $228E ; 3	*UNDOCUMENTED* self-mod TMA/TAM used by cd_read/ad_read/ad_write
;?		= $2291 ; 10	*UNDOCUMENTED* copy of scsisend made by cd_read (never read)
;?		= $229B ; ?

;mprsav		= $22B5 ; 8 backups of MPR when BIOS needs major bank swapping.

ramend		= $22D0 ; 1

psg_work_top	= $22D0
psg_reg		= $22ED ; 1 shadow for selected PSG register, like vdc_reg
			; ...

graph_work_top	= $2616
key_work_top	= $2646
user_work_top	= $267C



; ***************************************************************************
;
; System Card's Function Vectors
;

cd_boot		= $E000
cd_reset	= $E003
cd_base		= $E006
cd_read		= $E009
cd_seek		= $E00C
cd_exec		= $E00F
cd_play		= $E012
cd_search	= $E015
cd_pause	= $E018
cd_stat		= $E01B
cd_subq		= $E01E
cd_dinfo	= $E021
cd_contnts	= $E024
cd_subrd	= $E027
cd_pcmrd	= $E02A
cd_fade		= $E02D

ad_reset	= $E030
ad_trans	= $E033
ad_read		= $E036
ad_write	= $E039
ad_play		= $E03C
ad_cplay	= $E03F
ad_stop		= $E042
ad_stat		= $E045

bm_format	= $E048
bm_free		= $E04B
bm_read		= $E04E
bm_write	= $E051
bm_delete	= $E054
bm_files	= $E057

ex_getver	= $E05A
ex_setvec	= $E05D
ex_getfnt	= $E060
ex_joysns	= $E063
ex_joyrep	= $E066
ex_scrsiz	= $E069
ex_dotmod	= $E06C
ex_scrmod	= $E06F
ex_imode	= $E072
ex_vmode	= $E075
ex_hmode	= $E078
ex_vsync	= $E07B
ex_rcron	= $E07E
ex_rcroff	= $E081
ex_irqon	= $E084
ex_irqoff	= $E087
ex_bgon		= $E08A
ex_bgoff	= $E08D
ex_spron	= $E090
ex_sproff	= $E093
ex_dspon	= $E096
ex_dspoff	= $E099
ex_dmamod	= $E09C
ex_sprdma	= $E09F
ex_satclr	= $E0A2
ex_sprput	= $E0A5
ex_setrcr	= $E0A8
ex_setred	= $E0AB
ex_setwrt	= $E0AE
ex_setdma	= $E0B1
ex_binbcd	= $E0B4
ex_bcdbin	= $E0B7
ex_rnd		= $E0BA

ma_mul8u	= $E0BD
ma_mul8s	= $E0C0
ma_mul16u	= $E0C3
ma_div16s	= $E0C6
ma_div16u	= $E0C9
ma_sqrt		= $E0CC
ma_sin		= $E0CF
ma_cos		= $E0D2
ma_atni		= $E0D5

psg_bios	= $E0D8
grp_bios	= $E0DB

ex_memopen	= $E0DE

psg_driver	= $E0E1

ex_colorcmd	= $E0E4

max_mapping	= $FFF5



; ***************************************************************************
;
; System Card's PSG BIOS call functions.
;

PSG_ON		= 0
PSG_OFF		= 1
PSG_INIT	= 2
PSG_BANK	= 3
PSG_TRACK	= 4
PSG_WAVE	= 5
PSG_ENV		= 6
PSG_FM		= 7
PSG_PE		= 8
PSG_PC		= 9
PSG_TEMPO	= 10
PSG_PLAY	= 11
PSG_MSTAT	= 12
PSG_SSTAT	= 13
PSG_MSTOP	= 14
PSG_SSTOP	= 15
PSG_ASTOP	= 16
PSG_MVOFF	= 17
PSG_CONT	= 18
PSG_FDOUT	= 19
PSG_DCNT	= 20



; ***************************************************************************
;
; System Card's GRP BIOS call functions.
;

VI_GINIT	= 0
VI_CASHCLR	= 1
VI_STRTADR	= 2
VI_GETADRS	= 3
VI_CLS		= 4
VI_PSET		= 5
VI_POINT	= 6
VI_LINE		= 7
VI_BOX		= 8
VI_BOXF		= 9
VI_FLOOD	= 10
VI_PAINT	= 11
VI_GWINDOW	= 12
VI_GFONT	= 13
VI_PUTFONT	= 14
VI_SYMBOL	= 15



; ***************************************************************************
;
; Standard Display Timings.
;

; VDC constants for VRAM access speed.

VDC_MWR_1CYCLE	= $0000
VDC_MWR_2CYCLE	= $000A

; VDC constants for 224, 240 & 256 wide display (Set VDC_MWR_1CYCLE).

VCE_CR_5MHz	= $00	   ; 43 chr (actually 42.8) -> 342 pixel slots.

VDC_HSR_224	= $0402 ; HDS HSW
VDC_HDR_224	= $061B ; HDE HDW

VDC_HSR_240	= $0302 ; HDS HSW (Reduced)
VDC_HDR_240	= $051D ; HDE HDW

VDC_HSR_256	= $0202 ; HDS HSW (Normal)
VDC_HDR_256	= $041F ; HDE HDW

; VDC constants for 320, 336, 344 & 352 wide display (Set VDC_MWR_1CYCLE).

VCE_CR_7MHz	= $01	   ; 57 chr (actually 57.0) -> 456 pixel slots.

VDC_HSR_320	= $0503 ; HDS HSW
VDC_HDR_320	= $0627 ; HDE HDW

VDC_HSR_336	= $0403 ; HDS HSW (Reduced)
VDC_HDR_336	= $0529 ; HDE HDW

VDC_HSR_352	= $0303 ; HDS HSW (Normal)
VDC_HDR_352	= $042B ; HDE HDW

; VDC constants for 480 & 512 wide display (Set VDC_MWR_2CYCLE).

VCE_CR_10MHz	= $02	   ; 86 chr (actually 85.5) -> 684 pixel slots.

VDC_HSR_480	= $0D05 ; HDS HSW (Reduced)
VDC_HDR_480	= $053B ; HDE HDW

VDC_HSR_512	= $0B05 ; HDS HSW (Normal)
VDC_HDR_512	= $033F ; HDE HDW

; VDC constants for 200, 208, 224 & 240 high display.
;
; N.B. 2 lines higher on the screen than the System Card.

VDC_VPR_200	= $2102
VDC_VDW_200	= $00C7
VDC_VCR_200	= $0016 ; bios sets $00E2

VDC_VPR_208	= $1E02
VDC_VDW_208	= $00CF
VDC_VCR_208	= $0013 ; bios sets $00EE

VDC_VPR_224	= $1502
VDC_VDW_224	= $00DF
VDC_VCR_224	= $000B ; bios sets $00EE

VDC_VPR_240	= $0D02
VDC_VDW_240	= $00EF
VDC_VCR_240	= $0003 ; bios sets $00F6

; VDC constants for different BAT screen sizes.

VDC_MWR_32x32	= $0000
VDC_MWR_32x64	= $0040

VDC_MWR_64x32	= $0010
VDC_MWR_64x64	= $0050

VDC_MWR_128x32	= $0020
VDC_MWR_128x64	= $0060

;
;
;
