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
#ifndef CONVERT_H
#define CONVERT_H

/**
 * Defines an RGB pixel.
 */
typedef struct pixel_t {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} pixel_t;

/**
 * Defines a raster image as a rectangular array of RGB pixels.
 */
typedef struct raster_t {
  pixel_t **pixels;
  int width;
  int height;
} raster_t;

/**
 * Returns a pointer to a raster_t of the given width and height.
 * Should be deallocated with free_raster().
 */
raster_t* create_raster(int width, int height);

/**
 * Deallocates a raster_t.
 */
void free_raster(raster_t *raster);

/**
 * Loads a BMP image from the given file to a raster_t.
 */
raster_t* create_raster_from_bmp(FILE *file);

/**
 * Loads a JPEG image from the given file to a raster_t.
 */
raster_t* create_raster_from_jpg(FILE *file);

/**
 * Loads a PNG image from the given file to a raster_t.
 */
raster_t* create_raster_from_png(FILE *file);

#endif

