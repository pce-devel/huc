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
; The VDC is mirrored every 32-bytes in the first 1KB of the hardware bank.
;
; Traditionally, PCE developers use the first mirrored address at $0000, but
; I'm using the mirror at $0200 so that the address doesn't confuse folk who
; are used to $00xx being the zero-page location, and $01xx being the stack.
;
; Avoiding $00xx is also important for KickC, which follows 6502 conventions
; and assumes that equates in the $00xx region are zero-page.
;

VDC_SR		= $FF:0200	; Status Register
VDC_AR		= $FF:0200	; Address Register
VDC_DL		= $FF:0202	; Data (Read/Write) Low
VDC_DH		= $FF:0203	; Data (Read/Write) High

SGX_SR		= $FF:0210	; Status Register
SGX_AR		= $FF:0210	; Address Register
SGX_DL		= $FF:0212	; Data (Read/Write) Low
SGX_DH		= $FF:0213	; Data (Read/Write) High

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

VPC_CR		= $FF:0208	; Defaults to $1111
VPC_WINDOW1	= $FF:020A	; Defaults to $0000
VPC_WINDOW2	= $FF:020C	; Defaults to $0000
VPC_STMODE	= $FF:020E	; Defaults to $00=VDC ($01=SGX)
VPC_UNKNOWN	= $FF:020F	; Defaults to $00



; ***************************************************************************
;
; HuC6260 - Video Color Encoder
;
; Palette Data is 0000000gggrrrbbb
;

VCE_CR		= $FF:0400	; Control Register
VCE_CTA		= $FF:0402	; Color Table Address
VCE_CTW		= $FF:0404	; Color Table Data Write
VCE_CTR		= $FF:0404	; Color Table Data Read



; ***************************************************************************
;
; HuC6280 - Programmable Sound Generator
;

PSG_R0		= $FF:0800	; Channel Select
PSG_R1		= $FF:0801	; Main Amplitude Level
PSG_R2		= $FF:0802	; Frequency Low
PSG_R3		= $FF:0803	; Frequency High
PSG_R4		= $FF:0804	; Control & Channel Amplitude
PSG_R5		= $FF:0805	; L/R Amplitude Level
PSG_R6		= $FF:0806	; Waveform
PSG_R7		= $FF:0807	; Noise
PSG_R8		= $FF:0808	; LFO Frequency
PSG_R9		= $FF:0809	; LFO Control

PSG_AR		= $FF:0800	; Channel Select
PSG_MAL		= $FF:0801	; Main Amplitude Level
PSG_FL		= $FF:0802	; Frequency Low
PSG_FH		= $FF:0803	; Frequency High
PSG_CR		= $FF:0804	; Control & Channel Amplitude
PSG_LRAL	= $FF:0805	; L/R Amplitude Level
PSG_WF		= $FF:0806	; Waveform
PSG_NZ		= $FF:0807	; Noise
PSG_LFO_FRQ	= $FF:0808	; LFO Frequency
PSG_LFO_CR	= $FF:0809	; LFO Control



; ***************************************************************************
;
; HuC6280 - Timer
;

TIMER_DR	= $FF:0C00	; Timer Data
TIMER_CR	= $FF:0C01	; Timer Control



; ***************************************************************************
;
; HuC6280 - I/O Port
;

IO_PORT		= $FF:1000

JOY_L		= $0080
JOY_D		= $0040
JOY_R		= $0020
JOY_U		= $0010
JOY_RUN		= $0008
JOY_SEL		= $0004
JOY_B2		= $0002
JOY_B1		= $0001
JOY_B3		= $0100
JOY_B4		= $0200
JOY_B5		= $0400
JOY_B6		= $0800

PORT_CLR	= $02
PORT_SEL	= $01



; ***************************************************************************
;
; HuC6280 - Interrupt Control
;

IRQ_MSK		= $FF:1402	; Interrupt Disable
IRQ_REQ		= $FF:1403	; Interrupt Request
IRQ_ACK		= $FF:1403	; Interrupt Acknowledge



; ***************************************************************************
;
; CDROM/ADPCM hardware registers.
;

IFU_SCSI_CTRL	= $FF:1800	; -W : SCSI control signals out.
IFU_SCSI_FLGS	= $FF:1800	; R- : SCSI control signals in.
IFU_SCSI_DATA	= $FF:1801	; RW : SCSI data bus.
IFU_SCSI_ACK	= $FF:1802

IFU_IRQ_CTL	= $FF:1802
IFU_IRQ_FLG	= $FF:1803	; flags reporting CD/ADPCM info.

IFU_HW_RESET	= $FF:1804

IFU_SCSI_AUTO	= $FF:1808	; RO - SCSI auto-handshake read.

IFU_ADPCM_LSB	= $FF:1808
IFU_ADPCM_MSB	= $FF:1809
IFU_ADPCM_DAT	= $FF:180A
IFU_ADPCM_DMA	= $FF:180B
IFU_ADPCM_FLG	= $FF:180C
IFU_ADPCM_CTL	= $FF:180D
IFU_ADPCM_SPD	= $FF:180E

IFU_AUDIO_FADE	= $FF:180F

IFU_BRAM_LOCK	= $FF:1803
IFU_BRAM_UNLOCK	= $FF:1807

; IFU interrupt bits in IFU_IRQ_CTL/IFU_IRQ_FLG

IFU_INT_HALF	= $04		; ADPCM < 50% remaining.
IFU_INT_STOP	= $08		; ADPCM finished.
IFU_INT_SUB	= $10
IFU_INT_MSG_IN	= $20		; SCSI MIN phase.
IFU_INT_DAT_IN	= $40		; SCSI DIN phase.

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

ACD_DAT0	= $FF:1A00	; 2
ACD_ADR0	= $FF:1A02	; 2
ACD_OFF0	= $FF:1A05	; 2
ACD_INC0	= $FF:1A07	; 2
ACD_CTL0	= $FF:1A09	; 1
ACD_TRG0	= $FF:1A0A	; 1

ACD_D0L		= $FF:1A00	; 1
ACD_D0H		= $FF:1A01	; 1
ACD_A0L		= $FF:1A02	; 1
ACD_A0M		= $FF:1A03	; 1
ACD_A0H		= $FF:1A04	; 1
ACD_O0L		= $FF:1A05	; 1
ACD_O0H		= $FF:1A06	; 1
ACD_I0L		= $FF:1A07	; 1
ACD_I0H		= $FF:1A08	; 1

ACD_DAT1	= $FF:1A10	; 2
ACD_ADR1	= $FF:1A12	; 3
ACD_OFF1	= $FF:1A15	; 2
ACD_INC1	= $FF:1A17	; 2
ACD_CTL1	= $FF:1A19	; 1
ACD_TRG1	= $FF:1A1A	; 1

ACD_D1L		= $FF:1A10	; 1
ACD_D1H		= $FF:1A11	; 1
ACD_A1L		= $FF:1A12	; 1
ACD_A1M		= $FF:1A13	; 1
ACD_A1H		= $FF:1A14	; 1
ACD_O1L		= $FF:1A15	; 1
ACD_O1H		= $FF:1A16	; 1
ACD_I1L		= $FF:1A17	; 1
ACD_I1H		= $FF:1A18	; 1

ACD_DAT2	= $FF:1A20	; 2
ACD_ADR2	= $FF:1A22	; 3
ACD_OFF2	= $FF:1A25	; 2
ACD_INC2	= $FF:1A27	; 2
ACD_CTL2	= $FF:1A29	; 1
ACD_TRG2	= $FF:1A2A	; 1

ACD_D2L		= $FF:1A20	; 1
ACD_D2H		= $FF:1A21	; 1
ACD_A2L		= $FF:1A22	; 1
ACD_A2M		= $FF:1A23	; 1
ACD_A2H		= $FF:1A24	; 1
ACD_O2L		= $FF:1A25	; 1
ACD_O2H		= $FF:1A26	; 1
ACD_I2L		= $FF:1A27	; 1
ACD_I2H		= $FF:1A28	; 1

ACD_DAT3	= $FF:1A30	; 2
ACD_ADR3	= $FF:1A32	; 3
ACD_OFF3	= $FF:1A35	; 2
ACD_INC3	= $FF:1A37	; 2
ACD_CTL3	= $FF:1A39	; 1
ACD_TRG3	= $FF:1A3A	; 1

ACD_D3L		= $FF:1A30	; 1
ACD_D3H		= $FF:1A31	; 1
ACD_A3L		= $FF:1A32	; 1
ACD_A3M		= $FF:1A33	; 1
ACD_A3H		= $FF:1A34	; 1
ACD_O3L		= $FF:1A35	; 1
ACD_O3H		= $FF:1A36	; 1
ACD_I3L		= $FF:1A37	; 1
ACD_I3H		= $FF:1A38	; 1

ACD_SHIFTREG	= $FF:1AE0	; 4 bytes
ACD_ASL_CNT	= $FF:1AE4	; positive = shift left
ACD_ROL_CNT	= $FF:1AE5	; positive = shift left

ACD_VERL	= $FF:1AFD
ACD_VERH	= $FF:1AFE
ACD_FLAG	= $FF:1AFF

ACD_ID		= $51		; if ac_identflag = AC_IDENT, then AC in use



; ***************************************************************************
;
; System Card's Zero Page Variables.
;

zpg_grp_top	= $F8:20DC
vi_bitpat	= $F8:20DC
vi_rvbitpat	= $F8:20DD
vi_ft_front	= $F8:20DE
vi_padrs	= $F8:20DE
vi_porg		= $F8:20E0
vi_ft_back	= $F8:20E1
vi_stack	= $F8:20E4

zpg_psg_top	= $F8:20E6
time_sw		= $F8:20E6	; psg irq mutex (NZ == already running)
main_sw		= $F8:20E7	; psg driver mode ($80 == disable)
psg_si		= $F8:20E8	;
psg_si_l	= $F8:20E8	;
psg_si_h	= $F8:20E9	;
psg_r0		= $F8:20EA	;
psg_r0_l	= $F8:20EA	;
psg_r0_h	= $F8:20EB	;

zpg_sys_top	= $F8:20EC
_bp		= $F8:20EC	; base pointer
_si		= $F8:20EE	; source address
_di		= $F8:20F0	; destination address
cdi_b		= $F8:20F2	; ???
vdc_crl		= $F8:20F3	; shadow of VDC control register (lo-byte)
vdc_crh		= $F8:20F4	; shadow of VDC control register (hi-byte)
irq_vec		= $F8:20F5	; interrupt vector control mask
vdc_sr		= $F8:20F6	; shadow of VDC status register
vdc_reg		= $F8:20F7	; shadow of VDC register index

_ax		= $F8:20F8
_al		= $F8:20F8
_ah		= $F8:20F9

_bx		= $F8:20FA
_bl		= $F8:20FA
_bh		= $F8:20FB

_cx		= $F8:20FC
_cl		= $F8:20FC
_ch		= $F8:20FD

_dx		= $F8:20FE
_dl		= $F8:20FE
_dh		= $F8:20FF



; ***************************************************************************
;
; System Card's Main RAM Variables.
;

irq2_hook	= $F8:2200 ; 2	officially called irq2_jmp
irq1_hook	= $F8:2202 ; 2	officially called irq_jmp
timer_hook	= $F8:2204 ; 2	officially called tim_jmp
nmi_hook	= $F8:2206 ; 2	officially called nmi_jmp
vsync_hook	= $F8:2208 ; 2	officially called sync_jmp
hsync_hook	= $F8:220A ; 2	officially called rcr_jmp
bg_x1		= $F8:220C ; 2	officially called bgx1
bg_x2		= $F8:220E ; 2
bg_y1		= $F8:2210 ; 2
bg_y2		= $F8:2212 ; 2
satb_addr	= $F8:2214 ; 2	officially called sat_adr
sprptr		= $F8:2216 ; 1
spryl		= $F8:2217 ; 1
spryh		= $F8:2218 ; 1
sprxl		= $F8:2219 ; 1
sprxh		= $F8:221A ; 1
sprnl		= $F8:221B ; 1
sprnh		= $F8:221C ; 1
spral		= $F8:221D ; 1
sprah		= $F8:221E ; 1
color_cmd	= $F8:221F ; 1
bgc_ptr		= $F8:2220 ; 2
bgc_len		= $F8:2222 ; 1
sprc_ptr	= $F8:2223 ; 2
sprc_len	= $F8:2225 ; 1
joyena		= $F8:2227 ; 1
joynow		= $F8:2228 ; 5	officially called joy
joytrg		= $F8:222D ; 5
joyold		= $F8:2232 ; 5
irq_cnt		= $F8:2241 ; 1
vdc_mwr		= $F8:2242 ; 1	officially called mwr_m
vdc_dcr		= $F8:2243 ; 1	officially called dcr_m
rndseed		= $F8:2249 ; 1
rndl		= $F8:2249 ; 1
rndh		= $F8:224A ; 1
rndm		= $F8:224B ; 1
rndm		= $F8:224B ; 1
scsisend	= $F8:224C ; 10	*UNDOCUMENTED* buffer for SCSI cmd send
scsirecv	= $F8:2256 ; 10	*UNDOCUMENTED* buffer for SCSI cmd recv
paramcpy	= $F8:2260 ; 8	*UNDOCUMENTED* stored _ax,_bx,_cx,_dx
initialmpr	= $F8:2268 ; 1	*UNDOCUMENTED*
;?		= $F8:2269 ; ?
tnomin		= $F8:226A ; 1
tnomax		= $F8:226B ; 1
outmin		= $F8:226C ; 1
outsec		= $F8:226D ; 1
outfrm		= $F8:226E ; 1
endmin		= $F8:226F ; 1	*UNDOCUMENTED*
endsec		= $F8:2270 ; 1	*UNDOCUMENTED*
endfrm		= $F8:2271 ; 1	*UNDOCUMENTED*
vdtin_flg	= $F8:2272 ; 1
recbase		= $F8:2273 ; 1	*UNDOCUMENTED* active recbase (0 or 1)
recbase0_h	= $F8:2274 ; 1
recbase0_m	= $F8:2275 ; 1
recbase0_l	= $F8:2276 ; 1
recbase1_h	= $F8:2277 ; 1
recbase1_m	= $F8:2278 ; 1
recbase1_l	= $F8:2279 ; 1
scsiphs		= $F8:227A ; 1	*UNDOCUMENTED* SCSI command phase
scsists		= $F8:227B ; 1
suberrc		= $F8:227C ; 1
scsisns		= $F8:227D ; 1	*UNDOCUMENTED* SCSI "sense" return code
subcode		= $F8:227E ; 1

reset_hook	= $F8:2284 ; 2	*UNDOCUMENTED* joypad soft-reset hook
color_tia	= $F8:2286 ; 8	*UNDOCUMENTED* self-mod TIA used by ex_colorcmd
;?		= $F8:228E ; 3	*UNDOCUMENTED* self-mod TMA/TAM used by cd_read/ad_read/ad_write
;?		= $F8:2291 ; 10	*UNDOCUMENTED* copy of scsisend made by cd_read (never read)
;?		= $F8:229B ; ?

;mprsav		= $F8:22B5 ; 8 backups of MPR when BIOS needs major bank swapping.

ramend		= $F8:22D0 ; 1

psg_work_top	= $F8:22D0
psg_reg		= $F8:22ED ; 1 shadow for selected PSG register, like vdc_reg
			; ...

graph_work_top	= $F8:2616
key_work_top	= $F8:2646
user_work_top	= $F8:267C



; ***************************************************************************
;
; System Card's Function Vectors
;

cd_boot		= $00:E000
cd_reset	= $00:E003
cd_base		= $00:E006
cd_read		= $00:E009
cd_seek		= $00:E00C
cd_exec		= $00:E00F
cd_play		= $00:E012
cd_search	= $00:E015
cd_pause	= $00:E018
cd_stat		= $00:E01B
cd_subq		= $00:E01E
cd_dinfo	= $00:E021
cd_contnts	= $00:E024
cd_subrd	= $00:E027
cd_pcmrd	= $00:E02A
cd_fade		= $00:E02D

ad_reset	= $00:E030
ad_trans	= $00:E033
ad_read		= $00:E036
ad_write	= $00:E039
ad_play		= $00:E03C
ad_cplay	= $00:E03F
ad_stop		= $00:E042
ad_stat		= $00:E045

bm_format	= $00:E048
bm_free		= $00:E04B
bm_read		= $00:E04E
bm_write	= $00:E051
bm_delete	= $00:E054
bm_files	= $00:E057

ex_getver	= $00:E05A
ex_setvec	= $00:E05D
ex_getfnt	= $00:E060
ex_joysns	= $00:E063
ex_joyrep	= $00:E066
ex_scrsiz	= $00:E069
ex_dotmod	= $00:E06C
ex_scrmod	= $00:E06F
ex_imode	= $00:E072
ex_vmode	= $00:E075
ex_hmode	= $00:E078
ex_vsync	= $00:E07B
ex_rcron	= $00:E07E
ex_rcroff	= $00:E081
ex_irqon	= $00:E084
ex_irqoff	= $00:E087
ex_bgon		= $00:E08A
ex_bgoff	= $00:E08D
ex_spron	= $00:E090
ex_sproff	= $00:E093
ex_dspon	= $00:E096
ex_dspoff	= $00:E099
ex_dmamod	= $00:E09C
ex_sprdma	= $00:E09F
ex_satclr	= $00:E0A2
ex_sprput	= $00:E0A5
ex_setrcr	= $00:E0A8
ex_setred	= $00:E0AB
ex_setwrt	= $00:E0AE
ex_setdma	= $00:E0B1
ex_binbcd	= $00:E0B4
ex_bcdbin	= $00:E0B7
ex_rnd		= $00:E0BA

ma_mul8u	= $00:E0BD
ma_mul8s	= $00:E0C0
ma_mul16u	= $00:E0C3
ma_div16s	= $00:E0C6
ma_div16u	= $00:E0C9
ma_sqrt		= $00:E0CC
ma_sin		= $00:E0CF
ma_cos		= $00:E0D2
ma_atni		= $00:E0D5

psg_bios	= $00:E0D8
grp_bios	= $00:E0DB

ex_memopen	= $00:E0DE

psg_driver	= $00:E0E1

ex_colorcmd	= $00:E0E4

max_mapping	= $00:FFF5



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

; VDC constants for 192, 208, 224 & 240 high display.

VDC_VPR_192	= $2702
VDC_VDW_192	= $00BF
VDC_VCR_192	= $00FF ; Minimum of $1B for 263 lines.

VDC_VPR_208	= $1F02
VDC_VDW_208	= $00CF
VDC_VCR_208	= $00FF ; Minimum of $13 for 263 lines.

VDC_VPR_224	= $1702
VDC_VDW_224	= $00DF
VDC_VCR_224	= $00FF ; Minimum of $0B for 263 lines.

VDC_VPR_240	= $0F02
VDC_VDW_240	= $00EF
VDC_VCR_240	= $00FF ; Minimum of $03 for 263 lines.

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
