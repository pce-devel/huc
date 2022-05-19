/* ATARI.C (for Atari 8-bit cartridges) */
void fuji_write_header(FILE *f, int banks);
int  fuji_pack_8x8_tile(unsigned char *buffer, void *data, int line_offset, int format);
void fuji_carttype(int *ip);

/* ATARI specific pseudos */
/* *INDENT-OFF* */
struct t_opcode fuji_pseudo[3] = {
	{NULL,  "CART",  fuji_carttype, PSEUDO, P_CARTRIDGE, 0},

	{NULL,  ".CART", fuji_carttype, PSEUDO, P_CARTRIDGE, 0},
	{NULL, NULL, NULL, 0, 0, 0}
};
/* *INDENT-ON* */

const char defdirs_fuji[] =
#ifdef WIN32
	"c:\\huc\\include\\atari"
#else
	"/usr/local/lib/huc/include/atari;" \
	"/usr/local/huc/include/atari;" \
	"/usr/local/share/huc/include/atari;" \
	"/usr/local/include/atari;" \
	"/usr/lib/huc/include/atari;" \
	"/usr/share/huc/include/atari;" \
	"/usr/include/atari"
#endif
;


extern struct t_opcode m6502_inst[];
extern struct t_opcode undoc_inst[];

/* Atari 8-bit machine description */
struct t_machine fuji = {
	MACHINE_FUJI,		/* type */
	"FUJIAS",		/* asm_name */
	FUJI_ASM_VERSION,	/* asm_title */
	".car",			/* rom_ext */
	"FUJI_INCLUDE",		/* include_env */
	defdirs_fuji,		/* default_dirs */
	0x0100,			/* zp_limit */
	0xFFF6,			/* ram_limit */
	0,			/* ram_base */
	0,			/* ram_page */
	RESERVED_BANK,		/* ram_bank */
	m6502_inst,		/* base_inst */
	undoc_inst,		/* plus_inst */
	fuji_pseudo,		/* pseudo_inst */
	fuji_pack_8x8_tile,	/* pack_8x8_tile */
	NULL,			/* pack_16x16_tile */
	NULL,			/* pack_16x16_sprite */
	fuji_write_header	/* write_header */
};

