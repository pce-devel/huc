// **************************************************************************
// **************************************************************************
//
// pngfile.c
//
// Simple interface for reading files in PNG format.
//
// Limitations ...
//
// Lots.
//
// Only the following file contents are currently understood ...
//
//   4 bits-per-pixel, 1 plane, 256 colour palette.
//   8 bits-per-pixel, 1 plane, 256 colour palette.
//  24 bits-per-pixel, 1 plane, no colour palette.
//  32 bits-per-pixel, 1 plane, no colour palette.
//
// Copyright John Brandwood 2000-2022.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

#include  "elmer.h"
#include  "errorcode.h"
#include  "pxlmap.h"
#include  "pngfile.h"
#include  "png.h"



// **************************************************************************
// **************************************************************************
//
// PngReadPxlmap ()
//
// Read the next data item from the file
//
// Inputs  void **     Ptr to address of input context
//
// Output  DATABLOCK_T * Ptr to data item, or NULL if an error or no data
//
// N.B.  The end of the file is signalled by a return of NULL with an
//     g_iErrorCode of ERROR_NONE
//

#ifdef PNG_READ_SUPPORTED

PXLMAP_T *PngReadPxlmap (
  const char *pFileName )

{
  // Local variables.

  PXLMAP_T *pPxlmap = NULL;

  png_structp pPngStruct = NULL;
  png_infop pPngInfo = NULL;
  png_byte **apImageRows = NULL;

  png_uint_32 uWidth;
  png_uint_32 uHeight;
  int iBitDepth;
  int iColorType;
  int iInterlaceType;
  int iCompressionType;
  int iFilterMethod;

  unsigned uNativeDepth;

  int iPngRGB = 0;
  png_color *pPngRGB = NULL;

  int iPngALPHA = 0;
  png_byte *pPngALPHA = NULL;

  jmp_buf jmpbuf;

  int i;

  FILE *pFile = NULL;

  //

  pFile = fopen(pFileName, "rb");

  // Rewind the file.

  if (fseek(pFile, 0L, SEEK_SET) == 0) {
    // Read the PNG file header.

    png_byte aPngSignature[8];

    i = fread(&aPngSignature, 1, 8, pFile);

    if ((ferror(pFile) == 0) && (i == 8)) {
      // Check for a PNG file header.

      if (!png_check_sig(aPngSignature, 8)) {
        g_iErrorCode = ERROR_PNG_MALFORMED;
        sprintf(g_aErrorMessage,
          "(PNG) \"%s\" is not a PNG file.\n", pFileName);
        goto errorCleanup;
      }
    }
  }

  // Leave the file at the beginning.

  fseek(pFile, 0, SEEK_SET);

  // Leave the file's error flag cleared.

  clearerr(pFile);

  // could also replace libpng warning-handler (final NULL), but no need:

  pPngStruct = png_create_read_struct(
    PNG_LIBPNG_VER_STRING,
    NULL, // (png_voidp) user_error_ptr,
    NULL, // user_error_fn,
    NULL);  // user_warning_fn);

  if (!pPngStruct) {
    g_iErrorCode = ERROR_NO_MEMORY;
    goto errorCleanup;
  }

  pPngInfo = png_create_info_struct(pPngStruct);

  if (!pPngInfo) {
    g_iErrorCode = ERROR_NO_MEMORY;
    goto errorCleanup;
  }

  // setjmp() must be called in every function that calls a PNG-writing
  // libpng function, unless an alternate error handler was installed--
  // but compatible error handlers must either use longjmp() themselves
  // (as in this program) or exit immediately, so here we go:

  if (setjmp(jmpbuf)) {
    g_iErrorCode = ERROR_IO_READ;
    goto errorCleanup;
  }

  // Set up the input code to use the file that we've already opened.

  png_init_io(pPngStruct, pFile);

  // Read all the file information up to the actual image data.

  png_read_info(pPngStruct, pPngInfo);

  // Find out what is in the image.

  png_get_IHDR(
    pPngStruct,
    pPngInfo,
    &uWidth,  // Width of the image in pixels (up to 2^31)
    &uHeight, // Height of the image in pixels (up to 2^31)
    &iBitDepth, // Bit depth per channel (1, 2, 4, 8, 16)
    &iColorType,  //  Describes which color/alpha channels are present.
    //  PNG_COLOR_TYPE_GRAY
    //     (bit depths 1, 2, 4, 8, 16)
    //  PNG_COLOR_TYPE_GRAY_ALPHA
    //     (bit depths 8, 16)
    //  PNG_COLOR_TYPE_PALETTE
    //     (bit depths 1, 2, 4, 8)
    //  PNG_COLOR_TYPE_RGB
    //     (bit_depths 8, 16)
    //  PNG_COLOR_TYPE_RGB_ALPHA
    //     (bit_depths 8, 16)
    //  PNG_COLOR_MASK_PALETTE
    //  PNG_COLOR_MASK_COLOR
    //  PNG_COLOR_MASK_ALPHA
    &iInterlaceType,  // PNG_INTERLACE_NONE or PNG_INTERLACE_ADAM7
    &iCompressionType,  // Must be PNG_COMPRESSION_TYPE_BASE for PNG 1.0
    &iFilterMethod);  // Must be PNG_FILTER_TYPE_BASE for PNG 1.0

  // Create a native bitmap of the required size.

  switch (iColorType) {
  case PNG_COLOR_TYPE_RGB:
    uNativeDepth = 24;
    break;
  case PNG_COLOR_TYPE_RGB_ALPHA:
    uNativeDepth = 32;
    break;
  default:
    uNativeDepth = 8;
    break;
  }

  pPxlmap = PxlmapAlloc(uWidth, uHeight, uNativeDepth, NO);

  if (pPxlmap == NULL)
    goto errorCleanup;

  // If this is a paletted image, then read in the palette data.

  if (iColorType == PNG_COLOR_TYPE_PALETTE) {
    // Fill in the PNG colour palette.

    if (png_get_valid(pPngStruct, pPngInfo, PNG_INFO_PLTE)) {
      png_get_PLTE(pPngStruct, pPngInfo, &pPngRGB, &iPngRGB);

      for (i = 0; i < iPngRGB; i += 1) {
        pPxlmap->m_aPxmC[i].m_uRgbB = pPngRGB[i].blue;
        pPxlmap->m_aPxmC[i].m_uRgbG = pPngRGB[i].green;
        pPxlmap->m_aPxmC[i].m_uRgbR = pPngRGB[i].red;
      }
    }

    // Fill in the PNG colour palette's transparency (0=solid, 255=transparent).

    if (png_get_valid(pPngStruct, pPngInfo, PNG_INFO_tRNS)) {
      png_get_tRNS(pPngStruct, pPngInfo, &pPngALPHA, &iPngALPHA, NULL);

      for (i = 0; i < iPngALPHA; i += 1)
        pPxlmap->m_aPxmC[i].m_uRgbA = pPngALPHA[i];

      pPxlmap->m_iPxmPaletteHasAlpha = 1;
    }
  }

  // After you've read the header information, you can set up the library
  // to handle any special transformations of the image data.

  // 16-bit RGB data will be returned RRGGBB RRGGBB, with the most significant
  // byte of the color value first, unless png_set_strip_16() is called to
  // transform it to regular RGB RGB triplets.

  if (iBitDepth == 16)
    png_set_strip_16(pPngStruct);

  // Data will be decoded into the supplied row buffers packed into bytes
  // unless the library has been told to transform it into another format.
  // For example, 4 bit/pixel paletted or grayscale data will be returned
  // 2 pixels/byte with the leftmost pixel in the high-order bits of the
  // byte, unless png_set_packing() is called.

  if (iBitDepth < 8) {
    png_set_packing(pPngStruct);
    if (iBitDepth == 4)
      pPxlmap->m_iPxmSignificantBits = 4;
  }

//  // In PNG files, the alpha channel in an image is the level of opacity.
//  // If you need the alpha channel in an image to be the level of transparency
//  // instead of opacity, you can invert the alpha channel (or the tRNS chunk
//  // data) after it's read, so that 0 is fully opaque and 255 (in 8-bit or
//  // paletted images) or 65535 (in 16-bit images) is fully transparent.
//
//  png_set_invert_alpha(pPngStruct);

  // PNG files store 3-color pixels in red, green, blue order.  This code
  // changes the storage of the pixels to blue, green, red:

  if (iColorType == PNG_COLOR_TYPE_RGB || iColorType == PNG_COLOR_TYPE_RGB_ALPHA)
    png_set_bgr(pPngStruct);

//  // PNG files store RGB pixels packed into 3 or 6 bytes. This code expands them
//  // into 4 or 8 bytes for windowing systems that need them in this format,
//  // where "filler" is the 8 or 16-bit number to fill with, and the location is
//  // either PNG_FILLER_BEFORE or PNG_FILLER_AFTER, depending upon whether
//  // you want the filler before the RGB or after.  This transformation
//  // does not affect images that already have full alpha channels.  To add an
//  // opaque alpha channel, use filler=0xff or 0xffff and PNG_FILLER_AFTER which
//  // will generate RGBA pixels.
//
//  if (iColorType == PNG_COLOR_TYPE_RGB)
//    {
//    png_set_filler(pPngStruct, 0xFF, PNG_FILLER_AFTER);
//    }

//  // If you are reading an image with an alpha channel, and you need the
//  // data as ARGB instead of the normal PNG format RGBA:
//
//  if (iColorType == PNG_COLOR_TYPE_RGB_ALPHA)
//    {
//    png_set_swap_alpha(pPngStruct);
//    }

//  // For some uses, you may want a grayscale image to be represented as
//  // RGB. This code will do that conversion:
//
//  if (iColorType == PNG_COLOR_TYPE_GRAY ||
//    iColorType == PNG_COLOR_TYPE_GRAY_ALPHA)
//    {
//    png_set_gray_to_rgb(pPngStruct);
//    }

//  // The png_set_background() function tells libpng to composite images
//  // with alpha or simple transparency against the supplied background
//  // color. If the PNG file contains a bKGD chunk (PNG_INFO_bKGD valid),
//
//  png_color_16  my_background;
//  png_color_16p image_background;
//
//  if (png_get_bKGD(pPngStruct, pPngInfo, &image_background))
//    png_set_background(png_ptr, image_background,
//      PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
//  else
//    png_set_background(pPngStruct, &my_background,
//      PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

  // After setting the transformations, libpng can update your png_info
  // structure to reflect any transformations you've requested with this
  // call.

  png_read_update_info(pPngStruct, pPngInfo);

  // Create a list of each row's starting point in memory.

  apImageRows = (png_byte **)malloc(pPxlmap->m_iPxmH * sizeof(png_byte *));

  {
    uint8_t *pRow = pPxlmap->m_pPxmPixels;

    for (i = 0; i < pPxlmap->m_iPxmH; i++) {
      apImageRows[i] = (png_byte *)pRow;
      pRow += pPxlmap->m_iPxmLineSize;
    }
  }

  // After you've allocated memory, you can read the image data.
  // The simplest way to do this is in one function call.

  png_read_image(pPngStruct, apImageRows);

  // After you are finished reading the image through either the high- or
  // low-level interfaces, you can finish reading the file.

  png_read_end(pPngStruct, NULL);

  // When you are done, you can free all memory allocated by libpng like this:

  png_destroy_read_struct(&pPngStruct, &pPngInfo, NULL);

  // Free up the list of each row's starting point in memory.

  free(apImageRows);

  if (pFile) fclose(pFile);

  // Return the bitmap.

  return (pPxlmap);

  // Error handlers (reached via the dreaded goto).

errorCleanup:

  if (pPngInfo)
    png_destroy_read_struct(&pPngStruct, &pPngInfo, NULL);
  else
  if (pPngStruct)
    png_destroy_read_struct(&pPngStruct, NULL, NULL);

  if (apImageRows)
    free(apImageRows);

  if (pFile) fclose(pFile);

  return (NULL);
}

#endif



// **************************************************************************
// **************************************************************************
//
// PngDumpPxlmap ()
//
// Writes out the bitmap (in PNG format) with the given filename
//
// Inputs  DATABITMAP_T *  Ptr to bitmap
//     char *      Ptr to filename
//
// Output  ERRORCODE     ERROR_NONE if OK
//

#ifdef PNG_WRITE_SUPPORTED

ERRORCODE PngDumpPxlmap (
  PXLMAP_T *pPxlmap, const char *pFilename )

{
  // Local variables.

  int         i;
  FILE *      f = NULL;

  bool        bImageHasAlpha = false;

  char        aString [256];

  png_structp pPngStruct    = NULL;
  png_infop   pPngInfo      = NULL;
  png_byte ** apImageRows   = NULL;

  int         iPngTYP;
  int         iPngBPP;
  int         iPngRGB = 0;
  png_color   aPngRGB   [256];
  png_byte    aPngALPHA [256];

  jmp_buf     jmpbuf;

  // could also replace libpng warning-handler (final NULL), but no need:

  pPngStruct = png_create_write_struct(
    PNG_LIBPNG_VER_STRING,
    NULL,
    NULL,
    NULL);

  if (!pPngStruct)
  {
    g_iErrorCode = ERROR_NO_MEMORY;
    goto errorCleanup;
  }

  pPngInfo = png_create_info_struct(pPngStruct);

  if (!pPngInfo)
  {
    g_iErrorCode = ERROR_NO_MEMORY;
    goto errorCleanup;
  }

  // setjmp() must be called in every function that calls a PNG-writing
  // libpng function, unless an alternate error handler was installed--
  // but compatible error handlers must either use longjmp() themselves
  // (as in this program) or exit immediately, so here we go:

  if (setjmp(jmpbuf))
  {
    g_iErrorCode = ERROR_IO_WRITE;
    goto errorClose;
  }

  // Create a list of each row's starting point in memory.

  apImageRows = (png_byte **) malloc(pPxlmap->m_iPxmH * sizeof(png_byte *));

  if (pPxlmap->m_iPxmH >= 0)
  {
    uint8_t * pRow = pPxlmap->m_pPxmPixels;

    for (i = 0; i < pPxlmap->m_iPxmH; i++)
    {
      apImageRows[i] = (png_byte *) pRow;
      pRow += pPxlmap->m_iPxmLineSize;
    }
  }
  else
  {
    uint8_t * pRow = pPxlmap->m_pPxmPixels + (pPxlmap->m_iPxmLineSize * pPxlmap->m_iPxmH);

    for (i = 0; i < pPxlmap->m_iPxmH; i++)
    {
      pRow -= pPxlmap->m_iPxmLineSize;
      apImageRows[i] = (png_byte *) pRow;
    }
  }

  // Fill in the PNG colour palette.

  if (pPxlmap->m_iPxmSignificantBits == 8)
  {
    iPngTYP = PNG_COLOR_TYPE_PALETTE;
    iPngBPP = 8;
    iPngRGB = 256;

    for (i = 0; i < iPngRGB; i += 1)
    {
      aPngRGB[i].blue  = pPxlmap->m_aPxmC[i].m_uRgbB;
      aPngRGB[i].green = pPxlmap->m_aPxmC[i].m_uRgbG;
      aPngRGB[i].red   = pPxlmap->m_aPxmC[i].m_uRgbR;

      if ((aPngALPHA[i] = pPxlmap->m_aPxmC[i].m_uRgbA) != 0) bImageHasAlpha = true;
    }
  }
  else
  if (pPxlmap->m_iPxmSignificantBits == 24)
  {
    iPngTYP = PNG_COLOR_TYPE_RGB;
    iPngBPP = 8;
  }
  else
  if (pPxlmap->m_iPxmSignificantBits == 32)
  {
    iPngTYP = PNG_COLOR_TYPE_RGBA;
    iPngBPP = 8;

    bImageHasAlpha = true;
  }
  else
  {
    sprintf(g_aErrorMessage,
      "PNG varient not handled : cannot write %d BPP image.\n",
      pPxlmap->m_iPxmB);
    goto errorExit;
  }

  // Open the file for output.

  strcpy(aString, pFilename);
//strcat(aString, ".png");

  printf("Writing \"%s\".\n", aString);

  f = fopen(aString, "wb");

  if (f == NULL)
  {
    goto errorWrite;
  }

  // make sure outfile is (re)opened in BINARY mode

  png_init_io(pPngStruct, f);

  // set the compression levels--in general, always want to leave filtering
  // turned on (except for palette images) and allow all of the filters,
  // which is the default; want 32K zlib window, unless entire image buffer
  // is 16K or smaller (unknown here)--also the default; usually want max
  // compression (NOT the default); and remaining compression flags should
  // be left alone

  // 02May05 JCB - Change compression from Z_BEST_COMPRESSION.

  // png_set_compression_level(pPngStruct, 6);

  // set the image parameters appropriately

  png_set_IHDR(
    pPngStruct,
    pPngInfo,
    pPxlmap->m_iPxmW,
    pPxlmap->m_iPxmH,
    iPngBPP,
    iPngTYP,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_DEFAULT,
    PNG_FILTER_TYPE_DEFAULT);

  // Setup the palette information.

  if (pPxlmap->m_iPxmB <= 8)
  {
    png_set_PLTE(pPngStruct, pPngInfo, aPngRGB, iPngRGB);

    if (bImageHasAlpha)
    {
      png_set_tRNS(pPngStruct, pPngInfo, aPngALPHA, iPngRGB, NULL);
    }
  }

  // Write all chunks up to (but not including) first IDAT.

  png_write_info(pPngStruct, pPngInfo);

  // PNG files pack pixels of bit depths 1, 2, and 4 into bytes as small
  // as they can, resulting in, for example, 8 pixels per byte for 1 bit
  // files. If the data is supplied at 1 pixel per byte, use this code,
  // which will correctly pack the pixels into a single byte:

  if ((pPxlmap->m_iPxmSignificantBits == 4) && (pPxlmap->m_iPxmB == 8))
  {
    png_set_packing(pPngStruct);
  }

  // PNG files store 3 color pixels in red, green, blue order.
  // This code is used if they are supplied as blue, green, red.
  //
  // BGRA is Windows-native format for BMP/DIB, so that's what we're using.

  png_set_bgr(pPngStruct);

  // and now we just write the whole image; libpng takes care of interlacing
  // for us

  png_write_image(pPngStruct, apImageRows);

  // Close the file and shutdown libpng.

  png_write_end(pPngStruct, NULL);

  // Closedown libpng.

  png_destroy_write_struct(&pPngStruct, &pPngInfo);

  // Close the file.

  if (fclose(f) != 0)
  {
    goto errorWrite;
  }

  // Return with success code.

  return (ERROR_NONE);

  // Error handlers (reached via the dreaded goto).

  errorClose:

    if (f != NULL)
    {
      fclose(f);
    }

  errorWrite:

    g_iErrorCode = ERROR_IO_WRITE;

  errorCleanup:

    if (pPngInfo)
    {
      png_destroy_write_struct(&pPngStruct, &pPngInfo);
    }
    else
    if (pPngStruct)
    {
      png_destroy_write_struct(&pPngStruct, NULL);
    }

    if (apImageRows)
    {
      free(apImageRows);
    }

  errorExit:

    return (g_iErrorCode);

}

#endif
