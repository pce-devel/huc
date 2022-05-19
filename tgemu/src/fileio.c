#include <stdlib.h>
#include <stdio.h>

#include "shared.h"
#include "pcecrc.h"
#include <fcntl.h>
#include <sys/stat.h>

/* Name of the loaded file */
char game_name[0x100];

/* split : 1= Split image (only needed for 512k versions of 384k images)
   flip  : 1= Bit-flip image (only for some TurboGrafx-16 images) */
int load_rom(char *filename, int split, int flip)
{
    #include "bitflip.h"
    uint8 header[0x200];
    uint8 *ptr = NULL, *buf = NULL;
    int size;

    /* Default */
    strcpy(game_name, filename);

    {
        FILE* gd;

        /* Open file */
        if ((gd = fopen(filename, "rb")) == NULL)
          return (2);

        size = fseek(gd, 0, SEEK_END);
        rewind(gd);

        /* Allocate file data buffer */
        buf = malloc(size);
        if(!buf) {
            fclose(gd);
            return (0);
        }

        /* Read file data */
        int numBytes = 0;
        numBytes = fread(buf, 1, size, gd);

        if (numBytes != size) {
            return 42;
        }

        /* Close file */
        fclose(gd);
    }

    /* Check for 512-byte header */
    ptr = buf;
    if((size / 512) & 1)
    {
        memcpy(header, buf, 0x200);
        size -= 0x200;
        buf += 0x200;
    }

#if 0
    /* Generate CRC and print information */
    crc = crc32(0, buf, size);

    /* Look up game CRC in the CRC database, and set up flip and
       split options accordingly */
    int found = 0;
    for(n = 0; n < (sizeof(pcecrc_list) / sizeof(t_pcecrc)); n += 1)
    {
        if(crc == pcecrc_list[n].crc)
        {
            found = 1;
            return (int)crc;
            if(pcecrc_list[n].flag & FLAG_BITFLIP) flip = 1;
            if(pcecrc_list[n].flag & FLAG_SPLIT) split = 1;
        }
    }
    if (!found)
        return 23;
#endif
#if 0
    int fd, res;
    fd = open("wback.bin", O_CREAT|O_WRONLY|O_TRUNC);
    write(fd, buf, size);
    close(fd);
#endif

    /* Bit-flip image */
    if(flip)
    {
        uint8 temp;
        int count;

        for(count = 0; count < size; count += 1)
        {
            temp = buf[count];
            buf[count] = bitflip[temp];
        }
    }

    /* Always split 384K images */
    if(size == 0x60000)
    {
        memcpy(rom + 0x00000, buf + 0x00000, 0x40000);
        memcpy(rom + 0x80000, buf + 0x40000, 0x20000);
    }
    else /* Split 512K images if requested */
    if(split && (size == 0x80000))
    {
        memcpy(rom + 0x00000, buf + 0x00000, 0x40000);
        memcpy(rom + 0x80000, buf + 0x40000, 0x40000);
    }
    else
    {
        memcpy(rom, buf, (size > 0x100000) ? 0x100000 : size);
    }

    /* Free allocated memory and exit */
    free(ptr);

#ifdef DOS
    /* I need Allegro to handle this... */
    strcpy(game_name, get_filename(game_name));
#endif

    return (1);
}


int load_file(char *filename, char *buf, int size)
{
    int fd = 0;
    fd = open(filename, O_RDONLY);
    if(!fd) return (0);
    read(fd, buf, size);
    close(fd);
    return (1);
}


int save_file(char *filename, char *buf, int size)
{
    int fd = 0;
    fd = open(filename, O_WRONLY);
    if(!fd) return (0);
    write(fd, buf, size);
    close(fd);
    return (1);
}
