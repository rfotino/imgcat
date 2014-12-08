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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "environment.h"
#include "convert.h"

#define IMAGE_TYPE_INVALID 0
#define IMAGE_TYPE_BITMAP 1
#define IMAGE_TYPE_JPEG 2
#define IMAGE_TYPE_PNG 4

int endswith(const char* str, const char* suffix) {
  int i, start, suflen = strlen(suffix);
  start = strlen(str) - suflen;
  if (start < 0) {
    return 0;
  }
  for (i = 0; i < suflen; i++) {
    if (tolower(str[start+i]) != tolower(suffix[i])) {
      return 0;
    }
  }
  return 1;
}

int get_image_type(const char *filename) {
  if (endswith(filename, ".bmp")) {
    return IMAGE_TYPE_BITMAP;
  } else if (endswith(filename, ".jpg") || endswith(filename, ".jpeg")) {
    return IMAGE_TYPE_JPEG;
  } else if (endswith(filename, ".png")) {
    return IMAGE_TYPE_PNG;
  }
  return IMAGE_TYPE_INVALID;
}

void print_raster(raster_t *raster, const int print_width) {
  char *ascii_chars = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
  int intensity_count, i, j, x, y, num_ascii_chars = strlen(ascii_chars);
  float intensity, ratio_x = (float)raster->width / print_width, ratio_y = ratio_x * 2.5;
  int print_height = (int)(raster->height / ratio_y);
  pixel_t *pixel;

  for (y = 0; y < print_height; ++y) {
    for (x = 0; x < print_width; ++x) {
      intensity = 0;
      intensity_count = 0;
      for (j = (int)(ratio_y * y); j < ratio_y * (y+1); ++j) {
	for (i = (int)(ratio_x * x); i < ratio_x * (x+1); ++i) {
	  pixel = &raster->pixels[j][i];
	  intensity += (0.21 * pixel->r) + (0.72 * pixel->g) + (0.07 * pixel->b);
	  intensity_count++;
	}
      }
      intensity /= intensity_count;
      printf("%c", ascii_chars[(int)(num_ascii_chars * intensity / 256)]);
    }
    printf("\n");
  }

  return;
}

int main(int argc, char **argv) {
  int i, print_width = 0, image_type = IMAGE_TYPE_INVALID;
  char *filename = 0;
  FILE *imgfile;
  raster_t *raster;

  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-w") == 0) {
      if (i+1 < argc) {
	print_width = atoi(argv[i+1]);
	if (print_width < 1) {
	  fprintf(stderr, "Usage: -w NUMCHARSWIDE\n");
	  exit(1);
	}
	++i;
      } else {
	fprintf(stderr, "Usage: -w NUMCHARSWIDE\n");
	exit(1);
      }
    } else {
      filename = argv[i];
      image_type = get_image_type(filename);
    }
  }

  if (print_width < 1) {
    print_width = get_terminal_width();
  }

  if (filename) {
    imgfile = fopen(filename, "rb");
    if (!imgfile) {
      fprintf(stderr, "Failed to open image file %s.\n", filename);
      exit(1);
    }
  } else {
    image_type = IMAGE_TYPE_BITMAP;
    imgfile = stdin;
  }

  switch (image_type) {
    case IMAGE_TYPE_BITMAP:
      raster = create_raster_from_bmp(imgfile);
      break;
    case IMAGE_TYPE_JPEG:
      raster = create_raster_from_jpg(imgfile);
      break;
    case IMAGE_TYPE_PNG:
      raster = create_raster_from_png(imgfile);
      break;
    case IMAGE_TYPE_INVALID:
    default:
      fprintf(stderr, "Unable to detect valid image format.\n");
      exit(5);
  }

  fclose(imgfile);

  if (!raster) {
    fprintf(stderr, "Failed to get pixel data from file.\n");
    exit(1);
  }

  print_raster(raster, print_width);

  free_raster(raster);
  return 0;
}

