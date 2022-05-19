
#ifndef _FILEIO_H_
#define _FILEIO_H_

/* Global data */
extern char game_name[0x100];

/* Function prototypes */
int load_rom(char *filename, int split, int flip);
int file_exist(char *filename);
int load_file(char *filename, char *buf, int size);
int save_file(char *filename, char *buf, int size);

#endif /* _FILEIO_H_ */
