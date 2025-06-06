unsigned char rom[MAX_BANKS][8192];
unsigned char map[MAX_BANKS][8192];
uint32_t dbg_info[MAX_BANKS][8192];
uint8_t dbg_column[MAX_BANKS][8192];
char bank_name[MAX_BANKS][64];
int bank_loccnt[MAX_S][MAX_BANKS];
int bank_page[MAX_S][MAX_BANKS];
int bank_maxloc[MAX_BANKS];                     /* record max location in bank */

int discontiguous;                              /* NZ signals a warp in loccnt */
int max_zp;                                     /* higher used address in zero page */
int max_bss;                                    /* higher used address in ram */
int max_bank;                                   /* last bank used */
int data_loccnt;                                /* data location counter */
int data_size;                                  /* size of binary output (in bytes) */
int data_level;                                 /* data output level, must be <= listlevel to be outputed */
int phase_offset;                               /* location counter offset for .phase */
int phase_bank;                                 /* location counter bank for .phase */
int loccnt;                                     /* location counter */
int bank;                                       /* current bank */
int bank_base;                                  /* bank base index */
int rom_limit;                                  /* bank limit */
int bank_limit;                                 /* rom max. size in bytes */
int page;                                       /* page */
int rs_base;                                    /* .rs counter */
int rs_mprbank;                                 /* .rs counter */
int rs_overlay;                                 /* .rs counter */
unsigned char section;                          /* current section: S_ZP, S_BSS, S_CODE or S_DATA */
int section_bank[MAX_S];                        /* current bank for each section */
int section_phase[MAX_S];                       /* current phase offset for each section */
int stop_pass;                                  /* stop the program; set by fatal_error() */
int errcnt;                                     /* error counter */
int no_rom_file;                                /* NZ if only assembling data file(s) */
int kickc_mode;                                 /* NZ if assembling KickC code */
int sdcc_mode;                                  /* NZ if assembling SDCC code */
int hucc_mode;                                  /* NZ if assembling HuCC code */
int kickc_final;                                /* auto-include "kickc-final.asm" */
int hucc_final;                                 /* auto-include "hucc-final.asm" */
int in_final;                                   /* set when in xxxx-final.asm include */
int preproc_inblock;                            /* C-style comment: within block comment */
int preproc_sfield;                             /* C-style comment: SFIELD as a variable */
int preproc_modidx;                             /* C-style comment: offset to modified char */

t_machine *machine;
t_opcode *inst_tbl[HASH_COUNT];                 /* instructions hash table */
t_symbol *hash_tbl[HASH_COUNT];                 /* label hash table */
t_symbol *lablptr;                              /* label pointer into symbol table */
t_symbol *glablptr;                             /* pointer to the latest defined global label */
t_symbol *scopeptr;                             /* pointer to the latest defined scope label */
t_symbol *lastlabl;                             /* last label we have seen */
t_symbol *bank_glabl[MAX_S][MAX_BANKS];         /* latest global symbol for each bank */
t_symbol *unaliased;                            /* unaliased version of last symbol lookup */
t_branch *branchlst;                            /* first branch instruction assembled */
t_branch *branchptr;                            /* last branch instruction assembled */

int branches_changed;                           /* count of branches changed in pass */
char need_another_pass;                         /* NZ if another pass if required */
char hex[5];                                    /* hexadecimal character buffer */
void (*opproc)(int *);                          /* instruction gen proc */
int opflg;                                      /* instruction flags */
int opval;                                      /* instruction value */
int optype;                                     /* instruction type */
char opext;                                     /* instruction extension (.l or .h) */
int pass;                                       /* pass type (FIRST_PASS, EXTRA_PASS, LAST_PASS */
int pass_count;                                 /* pass counter */
char prlnbuf[LAST_CH_POS + 4];                  /* input line buffer */
char tmplnbuf[LAST_CH_POS + 4];                 /* temporary line buffer */
int slnum;                                      /* source line number counter */
char symbol[SBOLSZ];                            /* temporary symbol storage */
int undef;                                      /* undefined symbol in expression flg  */
int notyetdef;                                  /* undefined-in-current-pass symbol in expr */
unsigned int value;                             /* operand field value */

int opvaltab[6][16] = {
  {0x08, 0x08, 0x04, 0x14, 0x14, 0x11, 0x00, 0x10, // ADC AND CMP EOR LDA ORA SBC STA
   0x0C, 0x1C, 0x18, 0x2C, 0x3C, 0x00, 0x00, 0x00},

  {0x00, 0x00, 0x04, 0x14, 0x14, 0x00, 0x00, 0x10, // CPX CPY LDX LDY LAX ST0 ST1 ST2
   0x0C, 0x1C, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00},

  {0x00, 0x89, 0x24, 0x34, 0x00, 0x00, 0x00, 0x00, // BIT
   0x2C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

  {0x3A, 0x00, 0xC6, 0xD6, 0x00, 0x00, 0x00, 0x00, // DEC
   0xCE, 0xDE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

  {0x1A, 0x00, 0xE6, 0xF6, 0x00, 0x00, 0x00, 0x00, // INC
   0xEE, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

  {0x00, 0x00, 0x64, 0x74, 0x00, 0x00, 0x00, 0x00, // STZ
   0x9C, 0x9E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};
