/*
 * Copyright (C) 2014 Robert Fotino
 * Copyright (C) 2014 Garrett Brown
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
#include <png.h>

#include "convert.h"

#define BUFSIZE 65536

raster_t* create_raster(int width, int height) {
  int i;
  raster_t *raster = malloc(sizeof(raster_t));
  if (!raster) {
    fprintf(stderr, "Memory allocation failure.\n");
    exit(2);
  }

  raster->pixels = malloc(height * sizeof(pixel_t*));
  if (!raster->pixels) {
    fprintf(stderr, "Memory allocation failure.\n");
    exit(2);
  }

  for (i = 0; i < height; i++) {
    raster->pixels[i] = malloc(width * sizeof(pixel_t));
    if (!raster->pixels[i]) {
      fprintf(stderr, "Memory allocation failure.\n");
      exit(2);
    }
  }

  raster->width = width;
  raster->height = height;

  return raster;
}

void free_raster(raster_t *raster) {
  int i;
  for (i = 0; i < raster->height; i++) {
    free(raster->pixels[i]);
  }
  free(raster->pixels);
  free(raster);
}

unsigned char* read_byte_array(FILE *file, int *length) {
  int buflen = BUFSIZE, c;
  unsigned char *new_bytes, *bytes = malloc(buflen * sizeof(unsigned char));
  if (!bytes) {
    fprintf(stderr, "Memory allocation failure.\n");
    exit(2);
  }
  *length = 0;
  while ((c = fgetc(file)) != EOF) {
    if (*length > buflen) {
      buflen *= 1.5;
      new_bytes = realloc(bytes, buflen * sizeof(unsigned char));
      if (!new_bytes) {
	free(bytes);
	fprintf(stderr, "Memory allocation failure.\n");
	exit(2);
      }
      bytes = new_bytes;
    }
    bytes[*length] = (unsigned char)c;
    (*length)++;
  }
  return bytes;
}

raster_t* create_raster_from_bmp(FILE *file) {
  raster_t *raster;
  int length, width, height, row_padded, x, y;
  unsigned int offset;
  unsigned short bit_depth;
  unsigned char *bytes;

  //First get the raw byte array
  bytes = read_byte_array(file, &length);

  //The combined byte length of the headers should be at least 54 bytes,
  //and the first two bytes of the file should be 'B' and 'M'.
  if (length < 54 || bytes[0] != 'B' || bytes[1] != 'M') {
    fprintf(stderr, "The input was not recognized as a BMP file.\n");
    exit(3);
  }

  //Read in relevant header info
  offset = *(unsigned int *)(&bytes[10]);
  width = *(int *)(&bytes[18]);
  height = *(int *)(&bytes[22]);
  bit_depth = *(unsigned short *)(&bytes[28]);

  //We only support 24-bit BMPs right now
  if (bit_depth != 24) {
    fprintf(stderr, "BMP must be a 24-bit image.\n");
    exit(4);
  }

  //Each row of pixels is padded in byte multiples of 4
  row_padded = ((width * 3) + 3) & (~3);

  //Make sure width and height are positive and the byte array is at least
  //long enough to contain all of the pixels.
  if (width < 0 || height < 0 || (offset + (row_padded * height)) > length) {
    fprintf(stderr, "Invalid BMP file.\n");
    exit(3);
  }

  //Create the raster of the specified size
  raster = create_raster(width, height);
  for (y = height - 1; y >= 0; --y) {
    for (x = 0; x < width; ++x) {
      raster->pixels[height-y-1][x].b = bytes[offset+(row_padded*y)+(x*3)];
      raster->pixels[height-y-1][x].g = bytes[offset+(row_padded*y)+(x*3)+1];
      raster->pixels[height-y-1][x].r = bytes[offset+(row_padded*y)+(x*3)+2];
    }
  }
  free(bytes);
  return raster;
}
void do_stuff(void) { }
raster_t* create_raster_from_jpg(FILE *file) {
  raster_t *raster = 0;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPARRAY buffer;
  int x, y;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, file);
  jpeg_read_header(&cinfo, TRUE);
  jpeg_start_decompress(&cinfo);

  raster = create_raster(cinfo.image_width, cinfo.image_height);

  buffer = (*cinfo.mem->alloc_sarray)
    ((j_common_ptr) &cinfo, JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, 1);

  y = 0;
  while (cinfo.output_scanline < cinfo.output_height) {
    jpeg_read_scanlines(&cinfo, buffer, 1);
    for (x = 0; x < cinfo.output_width; x++) {
      raster->pixels[y][x].r = buffer[0][x*cinfo.output_components];
      raster->pixels[y][x].g = buffer[0][x*cinfo.output_components + 1];
      raster->pixels[y][x].b = buffer[0][x*cinfo.output_components + 2];
    }
    y++;
  }
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  return raster;
}

raster_t* create_raster_from_png(FILE *file) {
  raster_t *raster = 0;
  unsigned char sig[8], *image_data;
  png_struct *png_ptr;
  png_info *info_ptr;
  png_uint_32 width, height, rowbytes;
  png_bytep *row_pointers;
  int bit_depth, color_type, i, x, y;
  float alpha;

  fread(sig, 1, 8, file);
  if (!png_check_sig(sig, 8)) {
    fprintf(stderr, "Invalid PNG file.\n");
    exit(5);
  }

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    fprintf(stderr, "Memory allocation failure.\n");
    exit(2);
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    fprintf(stderr, "Memory allocation failure.\n");
    exit(2);
  }

  png_init_io(png_ptr, file);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

  if (color_type == PNG_COLOR_TYPE_PALETTE ||
      (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) ||
      png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
    png_set_expand(png_ptr);
  }
  if (bit_depth == 16) {
    png_set_strip_16(png_ptr);
  }
  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
    png_set_gray_to_rgb(png_ptr);
  }

  row_pointers = malloc(height * sizeof(row_pointers));
  if (!row_pointers) {
    fprintf(stderr, "Memory allocation failure.\n");
    exit(2);
  }

  png_read_update_info(png_ptr, info_ptr);
  rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  if ((image_data = (unsigned char *)malloc(rowbytes * height)) == NULL) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fprintf(stderr, "Memory allocation failure.\n");
    exit(2);
  }

  for (i = 0; i < height; ++i) {
    row_pointers[i] = image_data + i*rowbytes;
  }

  png_read_image(png_ptr, row_pointers);
  png_read_end(png_ptr, NULL);

  raster = create_raster(width, height);
  for (y = 0; y < height; ++y) {
    for (x = 0; x < width; ++x) {
      //Do alpha correction so that higher alpha values make the image more white
      alpha = row_pointers[y][x*4 + 3] / 256.0;
      raster->pixels[y][x].r = (alpha * row_pointers[y][x*4]) + ((1 - alpha) * 255);
      raster->pixels[y][x].g = (alpha * row_pointers[y][x*4 + 1]) + ((1 - alpha) * 255);
      raster->pixels[y][x].b = (alpha * row_pointers[y][x*4 + 2]) + ((1 - alpha) * 255);
    }
  }

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  free(image_data);
  free(row_pointers);

  return raster;
}

