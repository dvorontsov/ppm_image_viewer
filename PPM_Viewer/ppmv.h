#ifndef PPMV_H
#define PPMV_H

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <string>

#define TOP_DOWN -1
#define BOTTOM_UP 1

typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
} pixel;

typedef struct
{
	void *memory;
	int width;
	int height;
	std::string format;
	pixel *pixels;
} ppm_file;

typedef struct
{
	BITMAPINFO bitMapInfo;
	void* memory;
	int width;
	int height;
	int pitch;
} video_buffer;

enum LINE_TYPE
{
	DIMENTIONS,
	PIXEL
};


#endif