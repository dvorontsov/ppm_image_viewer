#include "ppmv.h"

using namespace std;

bool IsRunning = false;

void FillVideoBufferMemory(video_buffer *buffer, ppm_file *file)
{
	int arrayLength = file->width * file->height;
	int32_t *pixel = (int32_t *)buffer->memory;
	for (int i = 0; i < arrayLength; i++)
	{
		// 0xAARRGGBB
		*pixel = file->pixels[i].r << 16 | *pixel;
		*pixel = file->pixels[i].g << 8 | *pixel;
		*pixel = file->pixels[i].b | *pixel;

		pixel++;
	}
}

video_buffer InitVideoBuffer(int width, int height)
{
	video_buffer buffer = {};

	if (buffer.memory)
	{
		VirtualFree(buffer.memory, 0, MEM_RELEASE);
	}

	buffer.width = width;
	buffer.height = height;

	buffer.bitMapInfo.bmiHeader.biSize = sizeof(buffer.bitMapInfo.bmiHeader);
	buffer.bitMapInfo.bmiHeader.biWidth = buffer.width;
	buffer.bitMapInfo.bmiHeader.biHeight = buffer.height * TOP_DOWN;
	buffer.bitMapInfo.bmiHeader.biPlanes = 1;
	buffer.bitMapInfo.bmiHeader.biBitCount = 32;
	buffer.bitMapInfo.bmiHeader.biCompression = BI_RGB;

	int bytesPerPixel = 4;
	int bitmapMemorySize = buffer.width * buffer.height * bytesPerPixel;

	buffer.memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	buffer.pitch = width * bytesPerPixel;

	return buffer;
}

void ReadLine(ppm_file* result, LINE_TYPE type, string line, int recordSequence)
{
	const char* c_line = line.c_str();
	char* next;

	int idx = 0;
	unsigned long value;

	while (idx != -1)
	{
		// read number
		value = strtoul(c_line, &next, 10);
		c_line = next;

		if (type == LINE_TYPE::DIMENTIONS)
		{
			// save number
			switch (idx)
			{
			case 0:
				result->width = value;
				break;
			case 1:
				result->height = value;
				break;
			default:
				break;
			}
		} 
		else if (type == PIXEL)
		{
			// save number
			switch (idx)
			{
			case 0:
				result->pixels[recordSequence].r = value;
				break;
			case 1:
				result->pixels[recordSequence].g = value;
				break;
			case 2:
				result->pixels[recordSequence].b = value;
				break;
			default:
				break;
			}
		}
		
		idx++;
		if (*next == '\0')
		{
			idx = -1;
		}
	}
}

ppm_file LoadPPMFile(string fileName) 
{
	ppm_file result = {};

	bool readFormat = false;
	bool readDimensions = false;
	bool readColorRange = false;

	bool isPixlesInitialized = false;
	int recordSequence = 0;

	ifstream myfile;
	myfile.open(fileName);

	string line;
	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			if (line[0] == '#')
			{
				// Skip comment line
				continue;
			}
			
			if (!readFormat)
			{
				result.format = line;
				readFormat = true;
				continue;
			}

			if (!readDimensions)
			{
				ReadLine(&result, LINE_TYPE::DIMENTIONS, line, recordSequence);
				readDimensions = true;
				continue;
			}

			if (!readColorRange)
			{
				readColorRange = true;
				continue;
			}

			if (!isPixlesInitialized)
			{
				result.pixels = new pixel[result.width * result.height];
				isPixlesInitialized = true;
			}						

			ReadLine(&result, LINE_TYPE::PIXEL, line, recordSequence);
			
			recordSequence++;
		}
		myfile.close();
	} 
	return result;
}

int DisplayBufferInWindow(video_buffer *buffer, HWND window)
{
	HDC deviceCtx = GetDC(window);
	return StretchDIBits(deviceCtx,
		0, 0, buffer->width, buffer->height,
		0, 0, buffer->width, buffer->height,
		buffer->memory, &buffer->bitMapInfo, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK WindowProc(HWND hwnd,
							UINT msg,
							WPARAM wParam,
							LPARAM lParam)
{
	LRESULT result = 0;

	switch (msg)
	{
		case WM_CLOSE:
		{
			IsRunning = false;
			PostQuitMessage(0);
			return 0;
		} break;

		default:
		{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}

	return result;
}


int CALLBACK WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR lpCmdLine,
					 int nCmdShow)
{
	string fileName = lpCmdLine;
	ppm_file file = LoadPPMFile(fileName);
	if (file.format != "P3")
	{
		MessageBox(NULL, "Bad input file.", NULL, MB_OK);
		return -1;
	}
	video_buffer buffer = InitVideoBuffer(file.width, file.height);
	FillVideoBufferMemory(&buffer, &file);

	WNDCLASS wc = {};

	wc.style = CS_HREDRAW | CS_VREDRAW;  
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "PPM Viewer";

	RegisterClass(&wc);

	// Create the window.
	HWND window = CreateWindowEx(
			0,                              // Optional window styles.
			"PPM Viewer",                   // Window class
			"PPM Viewer",                   // Window text
			WS_OVERLAPPEDWINDOW,            // Window style

			// Size and position
			CW_USEDEFAULT, CW_USEDEFAULT, file.width + 100, file.height + 100,

			NULL,       // Parent window    
			NULL,       // Menu
			hInstance,  // Instance handle
			NULL        // Additional application data
		);

	if (window)
	{
		IsRunning = true;
		ShowWindow(window, nCmdShow);

		while (IsRunning)
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			DisplayBufferInWindow(&buffer, window);
		}
	}
	delete[] file.pixels;

	return 0;
}