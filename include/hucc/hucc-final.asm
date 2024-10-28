; ***************************************************************************
; ***************************************************************************
;
; hucc-final.asm
;
; PCEAS auto-includes this file at the end of every pass in HuCC or SDCC.
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
; This is used to select which assembly-language library files to include in
; a HuCC project, using labels defined in the compiler's header files.
;
; ***************************************************************************
; ***************************************************************************

		; Allow projects to customize what is included at the end
		; of a pass without replacing the entire "hucc-final.asm".

		.code
		.bank	CORE_BANK
		.page	CORE_PAGE

		include	"hucc-final-extra.asm"

		; Libraries required for basic functionality.

		.code
		.bank	CORE_BANK
		.page	CORE_PAGE

		include	"common.asm"		; Common helpers.
		include	"vce.asm"		; Useful VCE routines.
		include	"vdc.asm"		; Useful VDC routines.

		include	"hucc-math.asm"		; HuCC multiply and divide.

		; Define in hucc-config.inc to remove this.

	.ifndef	HUCC_NO_DEFAULT_RANDOM
		include	"random.asm"		; Random number generator.
	.endif

		; Optional libraries that get used when their header files
		; are included in a HuCC project.
		;
		; When the HuCC compiler is invoked with the "--legacy"
		; option to compile old projects, then the "huc.h" file
		; is automatically included, which then includes a list
		; of specific headers corresponding to HuC's library.

	.ifdef	HUCC_USES_GFX
		include	"hucc-gfx.asm"		; Set in hucc_gfx.h
	.endif

	.ifdef	HUCC_USES_STRING		; Set in hucc_string.h
		include	"hucc-string.asm"
	.endif

	.ifdef	HUCC_USES_NEW_SCROLL		; Set in hucc_scroll.h
		include	"hucc-scroll.asm"
	.else
	.ifdef	HUCC_USES_OLD_SCROLL		; Set in hucc_old_scroll.h
		include	"hucc-old-scroll.asm"
	.endif	HUCC_USES_OLD_SCROLL
	.endif	HUCC_USES_NEW_SCROLL

	.ifdef	HUCC_USES_OLD_SPR		; Set in hucc_old_spr.h
		include	"hucc-old-spr.asm"
	.endif

	.ifdef	HUCC_USES_OLD_MAP		; Set in hucc_old_map.h
		include	"hucc-old-map.asm"
	.endif

	.ifdef	HUCC_USES_OLD_LINE		; Set in hucc_old_line.h
		include	"hucc-old-line.asm"
	.endif

	.ifdef	HUCC_USES_ZX0			; Set in hucc_zx0.h
		include	"unpack-zx0.asm"
	.endif
