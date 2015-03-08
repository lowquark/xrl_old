#ifndef IMAGE_H
#define IMAGE_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool image_load_png(const char * filename, uint8_t ** pixels, size_t * w, size_t * h);

#endif
