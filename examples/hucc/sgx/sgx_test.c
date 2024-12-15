#include <huc.h>
#include <hucc-scroll.h>

/* HuCC enables and disables *both* the PC Engine and SuperGRAFX VDCs
   when disp_on() and disp_off() are called.
   Disabling them seperately is not normally done in games, but it is
   easy to accomplish with a bit of inline asm!
   Note that HuC's scroll(), HuCC's scroll_split() and sgx_scroll_split()
   *all* override the current disp_on() or disp_off()! */
void __fastcall __macro pce_disp_on( void );
void __fastcall __macro pce_disp_off( void );
void __fastcall __macro sgx_disp_on( void );
void __fastcall __macro sgx_disp_off( void );
#asm
	.macro	_pce_disp_on
	lda	#$C0
	tsb	<vdc_crl
	.endm

	.macro	_pce_disp_off
	lda	#$C0
	trb	<vdc_crl
	.endm

	.macro	_sgx_disp_on
	lda	#$C0
	tsb	<sgx_crl
	.endm

	.macro	_sgx_disp_off
	lda	#$C0
	trb	<sgx_crl
	.endm
#endasm

#incbin(map,"pce_bat1.bin");
#incbin(tile,"pce_tile1.bin");
#incbin(pal,"pce_pal1.bin");


main()
{
	unsigned char joyt;
	unsigned char d_on, sd_on;
	char i,j,k,l,m,n;
	
	disp_off();
	vsync();

	load_default_font();
	set_screen_size(SCR_SIZE_32x32);
	disp_on();

	if(!sgx_detect())
	{  put_string("Halt: SGX hardware not found", 2, 13); for(;;){} }

	/* The VPC is automatically initialized when a SuperGRAFX is detected in HuCC */
	
	set_font_pal(4);
	set_font_color(14,0);
	put_string("SGX hardware found", 2, 3);
	
	sgx_set_screen_size(SCR_SIZE_32x32);
	sgx_load_vram(0x0000,map, 0x400);
	sgx_load_vram(0x1000,tile, 0x4000);
	load_palette(0, pal,16);

	sgx_disp_on();
				
	put_string("Scrolling SGX layer ", 2, 6);
	put_string("I  = Toggle normal display", 2, 8);
	put_string("II = Toggle SGX display", 2, 9);
	d_on = sd_on = 1;
	for(;;)
	{
		for( j=0; j<0xff; j++)
		{
			vsync();

			sgx_scroll_split( 0, 0, j, j, sd_on ? 0xC0 : 0x00 );

			joyt = joytrg(0);
			if (joyt & JOY_I) {
				d_on = !d_on;
				if (d_on)
					pce_disp_on();
				else
					pce_disp_off();
			}
			if (joyt & JOY_II) {
				sd_on = !sd_on;
			}
		}
	}		
}
