#include <huc.h>
#include <hucc-scroll.h>

/*
 * DISPLAY CONTROL MACROS
 * ------------------------
 * These macros directly control the PC Engine (PCE) and SuperGrafx (SGX) displays
 * by setting or clearing bits in their respective VDC control registers.
 * They are implemented using inline assembly.
 */
void __fastcall __macro pce_disp_on(void);
void __fastcall __macro pce_disp_off(void);
void __fastcall __macro sgx_disp_on(void);
void __fastcall __macro sgx_disp_off(void);

#asm
    .macro _pce_disp_on
        lda    #$C0
        tsb    <vdc_crl
    .endm

    .macro _pce_disp_off
        lda    #$C0
        trb    <vdc_crl
    .endm

    .macro _sgx_disp_on
        lda    #$C0
        tsb    <sgx_crl
    .endm

    .macro _sgx_disp_off
        lda    #$C0
        trb    <sgx_crl
    .endm
#endasm

/*
 * GRAPHICS ASSET LOADING
 * -----------------------
 * The following directives load the foreground (PC Engine) and background (SuperGrafx)
 * assets from bitmap files. These include character (tile) data, palette data, and tile maps.
 */

/* Foreground graphics (PC Engine) */
#incchr(f_chr, "f.bmp")
#incpal(f_pal, "f.bmp")
#incbat(f_bat, "f.bmp", 0x1000, 32, 32)

/* Background graphics (SuperGrafx) */
#incchr(b_chr, "b.bmp")
#incpal(b_pal, "b.bmp")
#incbat(b_bat, "b.bmp", 0x1000, 32, 32)

main()
{
    unsigned char joyt;      /* Joystick trigger input */
    unsigned char pce_on, sgx_on; /* Display state toggles: d_on for PCE, sd_on for SGX */
    int scroll_x, scroll_y;  /* Fixed-point scroll positions (8-bit fractional part) */
    int j;                   /* Loop counter (declared separately for C89 compliance) */

    /* Temporary variables to extract integer scroll values */
    unsigned char j_x, j_y;
    int temp_x, temp_y;
    
    /* Check for SGX hardware; if not present, display error and halt */
    if (!sgx_detect())
    {
        put_string("ERROR: SGX hardware not found", 2, 13);
        for (;;){}
    }
    /* If we pass we're on a SGX */
    /* put_string("SGX hardware found", 12, 1); */

    /* INITIALIZATION */
    /* --------------- */
    disp_off(); /* Turn off display while setting up */
    sgx_disp_off(); /* Turn off display while setting up */
    vsync();     /* Wait for vertical sync */
    
    /* Setup PCE/SuperGrafx display, do this before loading anything or you will get a 64x32 BAT as a default for VDC1  */
    set_screen_size(SCR_SIZE_32x32);
    sgx_set_screen_size(SCR_SIZE_32x32);

    /* Set font colors with a transparent background for text */
    set_font_color(2, 0);
    load_default_font();

    /* Load the background scene (using foreground assets for demo purposes) */
    load_background(f_chr, f_pal, f_bat, 32, 32);

    /* Load VRAM for the background (SGX) layer and set the palette */
    sgx_load_vram(0x0000, b_bat, 0x400);
    sgx_load_vram(0x1000, b_chr, 0x5000);
    //load_palette(0, b_pal, 16);

    disp_on();
    sgx_disp_on();  /* Enable the SGX display */

    /* Display on-screen instructions and bakes them into the SGX layer by default */
    put_string("VDC1 & VDC2 scrolling on SGX", 2, 1);
    put_string(" I=Toggle VDC1 PCE layer", 6, 28);
    put_string("II=Toggle VDC2 SGX layer", 6, 29);

    /* Initialize display states and scroll positions */
    pce_on = 1;
    sgx_on = 1;
    scroll_x = 0;
    scroll_y = 0;

    /* MAIN LOOP shows Sub-pixel scrolling and input handling */
    for (;;)
    {
        /* Inner loop to gradually update the scroll position over 255 frames */
        for (j = 0; j < 0xFF; j++)
        {
            vsync();  /* Synchronize with vertical refresh for smooth animation */

            /*
             * UPDATE SCROLL POSITION:
             * Increment the fixed-point values by 4, which represents 1/8 of a pixel per frame.
             * This allows for smooth sub-pixel scrolling.
             */
            scroll_x += 4;
            scroll_y += 4;

            /* Extract the integer part of the fixed-point scroll values */
            temp_x = scroll_x >> 3;  /* Divide by 8 to remove the fractional part */
            temp_y = scroll_y >> 3;
            j_x = (unsigned char)temp_x;
            j_y = (unsigned char)temp_y;

            /*
             * APPLY SCROLLING:
             * - sgx_scroll_split() scrolls the SuperGrafx background layer.
             * - scroll_split() scrolls the PC Engine layer, with a negative X offset for a parallax effect.
             */
            /* Background layer */
            sgx_scroll_split(0, 0, j_x, j_y, sgx_on ? 0xC0 : 0x00);
            /* Foreground layer */
            scroll_split(0, 0, -j_x, -j_y, pce_on ? 0xC0 : 0x00);

            /* HANDLE INPUT: Toggle display outputs when joystick buttons are pressed */
            joyt = joytrg(0);
            if (joyt & JOY_I)
            {
                pce_on = !pce_on;  /* Toggle PCE display flag */
                if (pce_on)
                    pce_disp_on();
                else
                    pce_disp_off();
            }
            if (joyt & JOY_II)
            {
                sgx_on = !sgx_on;  /* Toggle SGX display flag */
                if (sgx_on)
                    sgx_disp_on();
                else
                    sgx_disp_off();
            }
        }
    }
}
