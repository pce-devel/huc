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

VDC_SR		= $FF:0200, 1	; Status Register
VDC_AR		= $FF:0200, 1	; Address Register
VDC_DL		= $FF:0202, 1	; Data (Read/Write) Low
VDC_DH		= $FF:0203, 1	; Data (Read/Write) High

SGX_SR		= $FF:0210, 1	; Status Register
SGX_AR		= $FF:0210, 1	; Address Register
SGX_DL		= $FF:0212, 1	; Data (Read/Write) Low
SGX_DH		= $FF:0213, 1	; Data (Read/Write) High

SGX_DETECT	= $FF:0218, 2	; $FF if SGX or VDC_SR mirror (i.e. < $80)

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

VPC_CR		= $FF:0208, 2	; Defaults to $1111
VPC_WINDOW1	= $FF:020A, 2	; Defaults to $0000
VPC_WINDOW2	= $FF:020C, 2	; Defaults to $0000
VPC_STMODE	= $FF:020E, 1	; Defaults to $00=VDC ($01=SGX)
VPC_UNKNOWN	= $FF:020F, 1	; Defaults to $00



; ***************************************************************************
;
; HuC6260 - Video Color Encoder
;
; Palette Data is 0000000gggrrrbbb
;

VCE_CR		= $FF:0400, 2	; Control Register
VCE_CTA		= $FF:0402, 2	; Color Table Address
VCE_CTW		= $FF:0404, 2	; Color Table Data Write
VCE_CTR		= $FF:0404, 2	; Color Table Data Read

XRES_SHARP	= $00
XRES_SOFT	= $04
XRES_BW		= $80



; ***************************************************************************
;
; HuC6280 - Programmable Sound Generator
;

PSG_R0		= $FF:0800, 1	; Channel Select
PSG_R1		= $FF:0801, 1	; Main Amplitude Level
PSG_R2		= $FF:0802, 1	; Frequency Low
PSG_R3		= $FF:0803, 1	; Frequency High
PSG_R4		= $FF:0804, 1	; Control & Channel Amplitude
PSG_R5		= $FF:0805, 1	; L/R Amplitude Level
PSG_R6		= $FF:0806, 1	; Waveform
PSG_R7		= $FF:0807, 1	; Noise
PSG_R8		= $FF:0808, 1	; LFO Frequency
PSG_R9		= $FF:0809, 1	; LFO Control

PSG_AR		= $FF:0800, 1	; Channel Select
PSG_VOL		= $FF:0801, 1	; Main Amplitude Level
PSG_FREQ	= $FF:0802, 1	; Frequency Low
PSG_FREQ_LO	= $FF:0802, 1	; Frequency Low
PSG_FREQ_HI	= $FF:0803, 1	; Frequency High
PSG_CTRL	= $FF:0804, 1	; Control & Channel Amplitude
PSG_PAN		= $FF:0805, 1	; L/R Amplitude Level
PSG_WAVEFORM	= $FF:0806, 1	; Waveform
PSG_NOISE	= $FF:0807, 1	; Noise
PSG_LFO_FREQ	= $FF:0808, 1	; LFO Frequency
PSG_LFO_CTRL	= $FF:0809, 1	; LFO Control



; ***************************************************************************
;
; HuC6280 - Timer
;

TIMER_DR	= $FF:0C00, 1	; Timer Data
TIMER_CR	= $FF:0C01, 1	; Timer Control



; ***************************************************************************
;
; HuC6280 - I/O Port
;

IO_PORT		= $FF:1000, 1

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

IRQ_MSK		= $FF:1402, 1	; Interrupt Disable
IRQ_REQ		= $FF:1403, 1	; Interrupt Request
IRQ_ACK		= $FF:1403, 1	; Interrupt Acknowledge



; ***************************************************************************
;
; CDROM/ADPCM hardware registers.
;

IFU_SCSI_CTL	= $FF:1800, 1	; _W : SCSI control signals out.
IFU_SCSI_FLG	= $FF:1800, 1	; R_ : SCSI control signals in.
IFU_SCSI_DAT	= $FF:1801, 1	; RW : SCSI data bus.

IFU_SCSI_ACK	= $FF:1802, 1	; RW : Top bit of IFU_IRQ_MSK!
IFU_IRQ_MSK	= $FF:1802, 1	; RW : Interrupt mask, 0=disabled.

IFU_IRQ_FLG	= $FF:1803, 1	; R_ : Interrupt flags.

IFU_HW_RESET	= $FF:1804, 1	; _W : Set bit 1 to reset.

IFU_AUDIO_LSB	= $FF:1805, 1	; R_ : Current CD audio.
IFU_AUDIO_MSB	= $FF:1806, 1	; R_ : Current CD audio.
IFU_SUBCODE	= $FF:1807, 1	; R_ : Current subcode.

IFU_SCSI_AUTO	= $FF:1808, 1	; R_ : SCSI auto-handshake read.

IFU_ADPCM_LSB	= $FF:1808, 1	; _W : Temporary address latch.
IFU_ADPCM_MSB	= $FF:1809, 1	; _W : Temporary address latch.
IFU_ADPCM_DAT	= $FF:180A, 1	; RW : ADPCM RAM data.
IFU_ADPCM_DMA	= $FF:180B, 1	; RW : ADPCM DMA control (bits 0 & 1).
IFU_ADPCM_FLG	= $FF:180C, 1	; R- : ADPCM status.
IFU_ADPCM_CTL	= $FF:180D, 1	; RW : ADPCM control.
IFU_ADPCM_SPD	= $FF:180E, 1	; RW : ADPCM playback clock.

IFU_AUDIO_FADE	= $FF:180F, 1	; RW : IFU audio fade control/status.

IFU_BRAM_LOCK	= $FF:1803, 1	; _W :
IFU_BRAM_UNLOCK	= $FF:1807, 1	; _W :

; IFU interrupt bits in IFU_IRQ_MSK/IFU_IRQ_FLG

IFU_INT_HALF	= $04		; Set if ADPCM LENGTH <= $7FFF.
IFU_INT_END	= $08		; Set if ADPCM LENGTH == 0 (when ADPCM_AUTO).
IFU_INT_SUBC	= $10		; SUBCODE read.
IFU_INT_MSG_IN	= $20		; SCSI MIN phase.
IFU_INT_DAT_IN	= $40		; SCSI DIN phase.

; IFU audio fade bits in IFU_AUDIO_FADE.

IFU_FADE_MUTE	= $01
IFU_FADE_TYPE	= $02		; ADPCM or CDAUDIO
IFU_FADE_SELN	= $04		; ?
IFU_FADE_TRIG	= $08		; Trigger a fade-out.
IFU_FADE_TEST	= $80		; ?

; ADPCM control bits in IFU_ADPCM_CTL

ADPCM_WR_CLK	= $01
ADPCM_SET_WR	= $02
ADPCM_RD_CLK	= $04
ADPCM_SET_RD	= $08
ADPCM_SET_SZ	= $10
ADPCM_PLAY	= $20		; Start playback.
ADPCM_AUTO	= $40		; Automatically stop when ADPCM LENGTH == 0.
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

ACD0_DATA	= $FF:1A00, 2	; RW
ACD0_BASE	= $FF:1A02, 3	; RW
ACD0_INDX	= $FF:1A05, 2	; RW
ACD0_INCR	= $FF:1A07, 2	; RW
ACD0_CTRL	= $FF:1A09, 1	; RW
ACD0_TRIG	= $FF:1A0A, 1	; _W

ACD1_DATA	= $FF:1A10, 2	; RW
ACD1_BASE	= $FF:1A12, 3	; RW
ACD1_INDX	= $FF:1A15, 2	; RW
ACD1_INCR	= $FF:1A17, 2	; RW
ACD1_CTRL	= $FF:1A19, 1	; RW
ACD1_TRIG	= $FF:1A1A, 1	; _W

ACD2_DATA	= $FF:1A20, 2	; RW
ACD2_BASE	= $FF:1A22, 3	; RW
ACD2_INDX	= $FF:1A25, 2	; RW
ACD2_INCR	= $FF:1A27, 2	; RW
ACD2_CTRL	= $FF:1A29, 1	; RW
ACD2_TRIG	= $FF:1A2A, 1	; _W

ACD3_DATA	= $FF:1A30, 2	; RW
ACD3_BASE	= $FF:1A32, 3	; RW
ACD3_INDX	= $FF:1A35, 2	; RW
ACD3_INCR	= $FF:1A37, 2	; RW
ACD3_CTRL	= $FF:1A39, 1	; RW (the bottom 7-bits are readable).
ACD3_TRIG	= $FF:1A3A, 1	; _W

ACD_SHIFT	= $FF:1AE0, 4	; RW : 4 bytes
ACD_ASL		= $FF:1AE4, 1	; RW : 4-bit signed shift (positive is left).
ACD_ROL		= $FF:1AE5, 1	; RW : 4-bit signed shift (positive is left).

ACD_VERL	= $FF:1AFD, 1	; R_ :
ACD_VERH	= $FF:1AFE, 1	; R_ :
ACD_FLAG	= $FF:1AFF, 1	; R_ :

ACD_ID		= $51		; If ACD_FLAG == ACD_ID, then ACD exists.



; ***************************************************************************
;
; System Card's Zero Page Variables.
;

zpg_grp_top	= $F8:20DC
vi_bitpat	= $F8:20DC, 1
vi_rvbitpat	= $F8:20DD, 1
vi_ft_front	= $F8:20DE, 2
vi_padrs	= $F8:20DE, 2
vi_porg		= $F8:20E0, 1
vi_ft_back	= $F8:20E1, 3
vi_stack	= $F8:20E4, 2

zpg_psg_top	= $F8:20E6
time_sw		= $F8:20E6, 1	; psg irq mutex (NZ == already running)
main_sw		= $F8:20E7, 1	; psg driver mode ($80 == disable)
psg_si		= $F8:20E8, 2	;
psg_si_l	= $F8:20E8, 1	;
psg_si_h	= $F8:20E9, 1	;
psg_r0		= $F8:20EA, 2	;
psg_r0_l	= $F8:20EA, 1	;
psg_r0_h	= $F8:20EB, 1	;

zpg_sys_top	= $F8:20EC
_bp		= $F8:20EC, 2	; base pointer
_bpl		= $F8:20EC, 1
_bph		= $F8:20ED, 1
_si		= $F8:20EE, 2	; source address
_sil		= $F8:20EE, 1
_sih		= $F8:20EF, 1
_di		= $F8:20F0, 2	; destination address
_dil		= $F8:20F0, 1
_dih		= $F8:20F1, 1
cdi_b		= $F8:20F2, 1	; IRQ2 interrupt status (like vdc_sr)
vdc_crl		= $F8:20F3, 1	; shadow of VDC control register (lo-byte)
vdc_crh		= $F8:20F4, 1	; shadow of VDC control register (hi-byte)
irq_vec		= $F8:20F5, 1	; interrupt vector control mask
vdc_sr		= $F8:20F6, 1	; shadow of VDC status register
vdc_reg		= $F8:20F7, 1	; shadow of VDC register index

_ax		= $F8:20F8, 2
_al		= $F8:20F8, 1
_ah		= $F8:20F9, 1

_bx		= $F8:20FA, 2
_bl		= $F8:20FA, 1
_bh		= $F8:20FB, 1

_cx		= $F8:20FC, 2
_cl		= $F8:20FC, 1
_ch		= $F8:20FD, 1

_dx		= $F8:20FE, 2
_dl		= $F8:20FE, 1
_dh		= $F8:20FF, 1



; ***************************************************************************
;
; System Card's Main RAM Variables.
;

irq2_hook	= $F8:2200, 2		; officially called irq2_jmp
irq1_hook	= $F8:2202, 2		; officially called irq_jmp
timer_hook	= $F8:2204, 2		; officially called tim_jmp
nmi_hook	= $F8:2206, 2		; officially called nmi_jmp
vsync_hook	= $F8:2208, 2		; officially called sync_jmp
hsync_hook	= $F8:220A, 2		; officially called rcr_jmp
bg_x1		= $F8:220C, 2		; officially called bgx1
bg_x2		= $F8:220E, 2
bg_y1		= $F8:2210, 2
bg_y2		= $F8:2212, 2
satb_addr	= $F8:2214, 2		; officially called sat_adr
sprptr		= $F8:2216, 1
spryl		= $F8:2217, 1
spryh		= $F8:2218, 1
sprxl		= $F8:2219, 1
sprxh		= $F8:221A, 1
sprnl		= $F8:221B, 1
sprnh		= $F8:221C, 1
spral		= $F8:221D, 1
sprah		= $F8:221E, 1
color_cmd	= $F8:221F, 1
bgc_ptr		= $F8:2220, 2
bgc_len		= $F8:2222, 1
sprc_ptr	= $F8:2223, 2
sprc_len	= $F8:2225, 1
joyena		= $F8:2227, 1
joynow		= $F8:2228, 5		; officially called joy
joytrg		= $F8:222D, 5
joyold		= $F8:2232, 5
irq_cnt		= $F8:2241, 1
vdc_mwr		= $F8:2242, 1		; officially called mwr_m
vdc_dcr		= $F8:2243, 1		; officially called dcr_m
vce_cr		= $F8:2244, 1		; *UNDOCUMENTED* set by ex_scrmod (not 10MHz!)
vdc_hdw		= $F8:2245, 1		; *UNDOCUMENTED* set by ex_scrmod
vdc_vdw		= $F8:2246, 1		; *UNDOCUMENTED* set by ex_scrmod
cd_not_rdy	= $F8:2247, 1		; *UNDOCUMENTED* set if CDERR_NOT_READY from scsi_req_sense()
cd_searching	= $F8:2248, 1		; *UNDOCUMENTED* 1 when background cd_search() requested
rndseed		= $F8:2249, 1
rndl		= $F8:2249, 1
rndh		= $F8:224A, 1
rndm		= $F8:224B, 1
scsi_send_buf	= $F8:224C, 10		; *UNDOCUMENTED* buffer for SCSI cmd send
scsi_recv_buf	= $F8:2256, 10		; *UNDOCUMENTED* buffer for SCSI cmd recv
parameter_buf	= $F8:2260, 8		; *UNDOCUMENTED* stored _ax,_bx,_cx,_dx
initialmpr	= $F8:2268, 1		; *UNDOCUMENTED*
;?		= $F8:2269, ?		; *UNDOCUMENTED* subcode
tnomin		= $F8:226A, 1
tnomax		= $F8:226B, 1
outmin		= $F8:226C, 1
outsec		= $F8:226D, 1
outfrm		= $F8:226E, 1
endmin		= $F8:226F, 1		; *UNDOCUMENTED*
endsec		= $F8:2270, 1		; *UNDOCUMENTED*
endfrm		= $F8:2271, 1		; *UNDOCUMENTED*
vdtin_flg	= $F8:2272, 1
recbase		= $F8:2273, 1		; *UNDOCUMENTED* active recbase (0 or 1)
recbase0_h	= $F8:2274, 1
recbase0_m	= $F8:2275, 1
recbase0_l	= $F8:2276, 1
recbase1_h	= $F8:2277, 1
recbase1_m	= $F8:2278, 1
recbase1_l	= $F8:2279, 1
scsiphs		= $F8:227A, 1		; *UNDOCUMENTED* SCSI command phase
scsists		= $F8:227B, 1
suberrc		= $F8:227C, 1
scsisns		= $F8:227D, 1		; *UNDOCUMENTED* unused SCSI "sense" return code
subcode		= $F8:227E, 1		;  WRITE-ONLY!
;?		= $F8:227F, 1		; *UNDOCUMENTED* unused, never accessed!

reset_hook	= $F8:2284, 2		; *UNDOCUMENTED* joypad soft-reset hook
color_tia	= $F8:2286, 8		; *UNDOCUMENTED* self-mod TIA used by ex_colorcmd
;?		= $F8:228E, 3		; *UNDOCUMENTED* self-mod TMA/TAM used by cd_read/ad_read/ad_write
;?		= $F8:2291, 10		; *UNDOCUMENTED* copy of scsisend made by cd_read (never read)
reset_cdrom	= $F8:229B, 1		; *UNDOCUMENTED* NZ if CD reset is needed
cdplay_mode	= $F8:229C, 1		; *UNDOCUMENTED*

cplay_siz_l	= $F8:229D, 1		; *UNDOCUMENTED* ad_cplay streaming sector load size (low)
cplay_len_l	= $F8:229E, 1		; *UNDOCUMENTED* ad_cplay streaming length (low/mid/high)
cplay_len_m	= $F8:229F, 1		; *UNDOCUMENTED* ad_cplay streaming length (low/mid/high)
cplay_len_h	= $F8:22A0, 1		; *UNDOCUMENTED* ad_cplay streaming length (low/mid/high)
cplay_lba_h	= $F8:22A1, 1		; *UNDOCUMENTED* ad_cplay streaming sector (high/mid/low)
cplay_lba_m	= $F8:22A2, 1		; *UNDOCUMENTED* ad_cplay streaming sector (high/mid/low)
cplay_lba_l	= $F8:22A3, 1		; *UNDOCUMENTED* ad_cplay streaming sector (high/mid/low)
bios_cd_mutex	= $F8:22A4, 1		; *UNDOCUMENTED* NZ if BIOS is using the CD in the background
ad_play_mode	= $F8:22A5, 1		; *UNDOCUMENTED*
ad_play_size	= $F8:22A6, 2		; *UNDOCUMENTED*
ad_play_addr	= $F8:22A8, 2		; *UNDOCUMENTED*
ad_play_rate	= $F8:22AA, 1		; *UNDOCUMENTED*

;bram_count	= $F8:22B3, 2		; *UNDOCUMENTED*
;mprsav		= $F8:22B5, 8		; *UNDOCUMENTED* backups of MPR when BIOS needs major bank swapping.
;?		= $F8:22BD, 1		; *UNDOCUMENTED*
;?		= $F8:22BE, 1		; *UNDOCUMENTED*

;?		= $F8:22C1, 1		; *UNDOCUMENTED*
;?		= $F8:22C2, 1		; *UNDOCUMENTED*
;?		= $F8:22C3, 1		; *UNDOCUMENTED*
;?		= $F8:22C4, 1		; *UNDOCUMENTED*

ramend		= $F8:22D0, 1

psg_work_top	= $F8:22D0
psg_reg		= $F8:22ED, 1		; shadow for selected PSG register, like vdc_reg

graph_work_top	= $F8:2616
key_work_top	= $F8:2646
user_work_top	= $F8:267C



; ***************************************************************************
;
; System Card's Function Vectors
;

	.if	CDROM
cd_boot		= $00:E000, FUNC
cd_reset	= $00:E003, FUNC
cd_base		= $00:E006, FUNC
cd_read		= $00:E009, FUNC
cd_seek		= $00:E00C, FUNC
cd_exec		= $00:E00F, FUNC
cd_play		= $00:E012, FUNC
cd_search	= $00:E015, FUNC
cd_pause	= $00:E018, FUNC
cd_stat		= $00:E01B, FUNC
cd_subq		= $00:E01E, FUNC
cd_dinfo	= $00:E021, FUNC
cd_contnts	= $00:E024, FUNC
cd_subrd	= $00:E027, FUNC
cd_pcmrd	= $00:E02A, FUNC
cd_fade		= $00:E02D, FUNC

ad_reset	= $00:E030, FUNC
ad_trans	= $00:E033, FUNC
ad_read		= $00:E036, FUNC
ad_write	= $00:E039, FUNC
ad_play		= $00:E03C, FUNC
ad_cplay	= $00:E03F, FUNC
ad_stop		= $00:E042, FUNC
ad_stat		= $00:E045, FUNC

bm_format	= $00:E048, FUNC
bm_free		= $00:E04B, FUNC
bm_read		= $00:E04E, FUNC
bm_write	= $00:E051, FUNC
bm_delete	= $00:E054, FUNC
bm_files	= $00:E057, FUNC

ex_getver	= $00:E05A, FUNC
ex_setvec	= $00:E05D, FUNC
ex_getfnt	= $00:E060, FUNC
ex_joysns	= $00:E063, FUNC
ex_joyrep	= $00:E066, FUNC
ex_scrsiz	= $00:E069, FUNC
ex_dotmod	= $00:E06C, FUNC
ex_scrmod	= $00:E06F, FUNC
ex_imode	= $00:E072, FUNC
ex_vmode	= $00:E075, FUNC
ex_hmode	= $00:E078, FUNC
ex_vsync	= $00:E07B, FUNC
ex_rcron	= $00:E07E, FUNC
ex_rcroff	= $00:E081, FUNC
ex_irqon	= $00:E084, FUNC
ex_irqoff	= $00:E087, FUNC
ex_bgon		= $00:E08A, FUNC
ex_bgoff	= $00:E08D, FUNC
ex_spron	= $00:E090, FUNC
ex_sproff	= $00:E093, FUNC
ex_dspon	= $00:E096, FUNC
ex_dspoff	= $00:E099, FUNC
ex_dmamod	= $00:E09C, FUNC
ex_sprdma	= $00:E09F, FUNC
ex_satclr	= $00:E0A2, FUNC
ex_sprput	= $00:E0A5, FUNC
ex_setrcr	= $00:E0A8, FUNC
ex_setred	= $00:E0AB, FUNC
ex_setwrt	= $00:E0AE, FUNC
ex_setdma	= $00:E0B1, FUNC
ex_binbcd	= $00:E0B4, FUNC
ex_bcdbin	= $00:E0B7, FUNC
ex_rnd		= $00:E0BA, FUNC

ma_mul8u	= $00:E0BD, FUNC
ma_mul8s	= $00:E0C0, FUNC
ma_mul16u	= $00:E0C3, FUNC
ma_div16s	= $00:E0C6, FUNC
ma_div16u	= $00:E0C9, FUNC
ma_sqrt		= $00:E0CC, FUNC
ma_sin		= $00:E0CF, FUNC
ma_cos		= $00:E0D2, FUNC
ma_atni		= $00:E0D5, FUNC

psg_bios	= $00:E0D8, FUNC
grp_bios	= $00:E0DB, FUNC

ex_memopen	= $00:E0DE, FUNC

psg_driver	= $00:E0E1, FUNC

ex_colorcmd	= $00:E0E4, FUNC

max_mapping	= $00:FFF5, 1
	.endif	CDROM



; ***************************************************************************
;
; System Card's PSG BIOS call functions.
;

	.if	CDROM
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
	.endif	CDROM



; ***************************************************************************
;
; System Card's GRP BIOS call functions.
;

	.if	CDROM
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
	.endif	CDROM



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

; VDC constants for 480, 496 & 512 wide display (Set VDC_MWR_2CYCLE).

VCE_CR_10MHz	= $02	   ; 86 chr (actually 85.5) -> 684 pixel slots.

VDC_HSR_480	= $0D05 ; HDS HSW (Reduced)
VDC_HDR_480	= $053B ; HDE HDW

VDC_HSR_496	= $0C05 ; HDS HSW (Reduced)
VDC_HDR_496	= $043D ; HDE HDW

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
