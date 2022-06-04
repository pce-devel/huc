/*
 * OVERLAY.H  -  Important addresses and data for CDROM overlay usage
 *               References startup.asm, but information is used by
 *               MagicKit 'AS' and 'ISOLINK' for preparation of chaining
 */

#define	OVL_ENTRY_POINT		0xC000	/* overlay 're-entry' point in startup */
#define	BOOT_ENTRY_POINT	0x4070	/* initial entry point in startup */
#define	OVL_DATA_SECTOR		0x4030	/* Bank offset (from 0) to hack the directory */
#define	CDERR_LENGTH		0x4031	/* CDROM overlay length# instead of err text */
#define	CDERR_SECTOR		0x4032	/* CDROM overlay sector# instead of err text */
