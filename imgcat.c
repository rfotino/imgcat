#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int endswith(char* str, char* suffix) {
  int i, start, suflen = strlen(suffix);
  start = strlen(str) - suflen;
  if (start < 0) {
    return 0;
  }
  for (i = 0; i < suflen; i++) {
    if (str[start+i] != suffix[i]) {
      return 0;
    }
  }
  return 1;
}

typedef struct pixel_t {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} pixel_t;

typedef struct raster_t {
  pixel_t **pixels;
  int width;
  int height;
} raster_t;

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
  int buflen = 65535;
  int c;
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

raster_t* convert_byte_array(unsigned char *bytes, const int length) {
  raster_t *raster;
  int width, height, row_padded, x, y;
  unsigned int offset;

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

  //Create the raster of the specified size
  raster = create_raster(width, height);

  //Each row of pixels is padded in byte multiples of 4
  row_padded = ((width * 3) + 3) & (~3);
  for (y = height - 1; y >= 0; --y) {
    for (x = 0; x < width; ++x) {
      raster->pixels[height-y-1][x].b = bytes[offset+(row_padded*y)+(x*3)];
      raster->pixels[height-y-1][x].g = bytes[offset+(row_padded*y)+(x*3)+1];
      raster->pixels[height-y-1][x].r = bytes[offset+(row_padded*y)+(x*3)+2];
    }
  }

  return raster;
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
  int i, print_width = 80, raw_bytes_length = 0;
  char *filename = 0;
  FILE *imgfile;
  unsigned char *raw_bytes;
  raster_t *raster;

  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-w") == 0) {
      if (i+1 < argc) {
	print_width = atoi(argv[i+1]);
	if (print_width < 1) {
	  fprintf(stderr, "Usage: -w NUMCHARSWIDE\n");
	  exit(1);
	}
      } else {
	fprintf(stderr, "Usage: -w NUMCHARSWIDE\n");
	exit(1);
      }
    } else if (endswith(argv[i], ".bmp")) {
      filename = argv[i];
    }
  }

  if (filename) {
    imgfile = fopen(filename, "r");
    if (!imgfile) {
      fprintf(stderr, "Failed to open image file %s.\n", filename);
      exit(1);
    }
  } else {
    imgfile = stdin;
  }

  raw_bytes = read_byte_array(imgfile, &raw_bytes_length);
  if (!raw_bytes) {
    fprintf(stderr, "Failed to read image data.\n");
    exit(1);
  }

  raster = convert_byte_array(raw_bytes, raw_bytes_length);
  if (!raster) {
    fprintf(stderr, "Failed to convert raw bytes to pixel data.\n");
    exit(1);
  }

  print_raster(raster, print_width);

  free(raw_bytes);
  free_raster(raster);

  return 0;
}

