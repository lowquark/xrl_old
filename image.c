
#include "image.h"

#include <stdio.h>
#include <png.h>
#include <string.h>
#include <stdlib.h>

extern bool image_load_png(const char * filename, uint8_t ** pixels, size_t * w, size_t * h)
{
	FILE * fp = fopen(filename, "rb");

	if(!fp)
	{
		printf("PNG ERROR: Could not open PNG file for reading at %s\n", filename);

		return false;
	}

	unsigned char header[8];

	fread(header, 1, 8, fp);

	if(png_sig_cmp(header, 0, 8))
	{
		printf("PNG ERROR: %s is not a PNG file.\n", filename);

		return false;
	}

	// Header is good so far, get ready to read
	png_structp png_ptr;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if(!png_ptr)
	{
		printf("PNG ERROR: png_create_read_struct() failed.\n");

		return false;
	}

	png_infop info_ptr;

	info_ptr = png_create_info_struct(png_ptr);

	if(!info_ptr)
	{
		printf("PNG ERROR: png_create_info_struct() failed.\n");

		return false;
	}

	// Read the png info
	if(setjmp(png_jmpbuf(png_ptr)))
	{
		printf("PNG ERROR: info read error (longjmp).\n");

		return false;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	int width = png_get_image_width(png_ptr, info_ptr);
	int height = png_get_image_height(png_ptr, info_ptr);
	int color_type = png_get_color_type(png_ptr, info_ptr);
	//int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	if(!(color_type == PNG_COLOR_TYPE_RGB || 
		 color_type == PNG_COLOR_TYPE_RGB_ALPHA ||
		 color_type == PNG_COLOR_TYPE_GRAY ||
		 color_type == PNG_COLOR_TYPE_GRAY_ALPHA))
	{
		printf("PNG ERROR: Unsupported color type. (%d)\n", color_type);

		return false;
	}

	//int number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);
	int row_bytes = png_get_rowbytes(png_ptr, info_ptr);


	// Read the actual image
	png_byte * volatile data = 0;

	if(setjmp(png_jmpbuf(png_ptr)))
	{
		printf("PNG ERROR: image read error (longjmp).\n");

		if(data)
			free(data);

		return false;
	}

	data = malloc(row_bytes * height);

	//cout << "row_bytes: " << row_bytes << endl;
	for(int i = 0;i < height;i ++)
	{
		png_read_row(png_ptr, data + i*row_bytes, NULL);
	}

	printf("Read PNG %s: width=%u, height=%u, color_type=%u\n", filename, width, height, color_type);

	uint8_t * p = malloc(width*height*4);

	*pixels = p;
	*w = width;
	*h = height;

	// Depending on the format, load the data into the Image struct
	if(color_type == PNG_COLOR_TYPE_RGB)
	{
		for(int i = 0;i < width*height;i ++)
		{
			p[i*4 + 0] = data[i*3];
			p[i*4 + 1] = data[i*3 + 1];
			p[i*4 + 2] = data[i*3 + 2];
			p[i*4 + 3] = 255;
		}
	}
	else if(color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		for(int i = 0;i < width*height;i ++)
		{
			p[i*4 + 0] = data[i*4];
			p[i*4 + 1] = data[i*4 + 1];
			p[i*4 + 2] = data[i*4 + 2];
			p[i*4 + 3] = data[i*4 + 3];
		}
	}
	else if(color_type == PNG_COLOR_TYPE_GRAY)
	{
		for(int i = 0;i < width*height;i ++)
		{
			p[i*4 + 0] = data[i];
			p[i*4 + 1] = data[i];
			p[i*4 + 2] = data[i];
			p[i*4 + 3] = 255;
		}
	}
	else if(color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		for(int i = 0;i < width*height;i ++)
		{
			p[i*4 + 0] = data[i*2];
			p[i*4 + 1] = data[i*2];
			p[i*4 + 2] = data[i*2];
			p[i*4 + 3] = data[i*2 + 1];
		}
	}

	free(data);

	fclose(fp);

	return true;
}

