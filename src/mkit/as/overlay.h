/*
 * OVERLAY.H  -  Important addresses and data for CDROM overlay usage
 *               References startup.asm, but information is used by
 *               MagicKit 'AS' and 'ISOLINK' for preparation of chaining
 */

#define	OVL_ENTRY_POINT		0xC000	/* overlay 're-entry' point in startup */
#define	BOOT_ENTRY_POINT	0x4070	/* initial entry point in startup */
#define	CDERR_OVERRIDE		0x402F	/* flag to use CDROM prog instead of err text */
#define	CDERR_OVERLAY_NUM	0x4030	/* CDROM overlay # to use instead of err text */
#define	OVL_DATA_SECTOR		0x4031	/* Bank offset (from 0) to hack the directory */

