// Template, IGAD version 3
// Get the latest version from: https://github.com/jbikker/tmpl8
// IGAD/NHTV/BUAS/UU - Jacco Bikker - 2006-2023

#include "precomp.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#include "stb_image.h"

using namespace Tmpl8;

// Surface class implementation

Surface::Surface(int w, int h, uint* b) : pixels(b), width(w), height(h) {}

Surface::Surface(int w, int h) : width(w), height(h) {
	pixels = (uint*)MALLOC64(w * h * sizeof(uint));
	ownBuffer = true; // needs to be deleted in destructor
}
Surface::Surface(const char* file) : pixels(0), width(0), height(0) {
	// check if file exists; show an error if there is a problem
	FILE* f = fopen(file, "rb");
	if (!f) FatalError("File not found: %s", file);
	fclose(f);
	// load the file
	Surface::LoadFromFile(file);
}

void Surface::LoadFromFile(const char* file) {
	// use stb_image to load the image file
	int n;
	unsigned char* data = stbi_load(file, &width, &height, &n, 0);
	if (!data) return; // load failed
	pixels = (uint*)MALLOC64(width * height * sizeof(uint));
	ownBuffer = true; // needs to be deleted in destructor
	const int s = width * height;
	if (n == 1) /* greyscale */ for (int i = 0; i < s; i++) {
		const unsigned char p = data[i];
		pixels[i] = p + (p << 8) + (p << 16);
	} else {
		for (int i = 0; i < s; i++) pixels[i] = (data[i * n + 0] << 16) + (data[i * n + 1] << 8) + data[i * n + 2];
	}
	// free stb_image data
	stbi_image_free(data);
}

Surface::~Surface() {
	if (ownBuffer) FREE64(pixels); // free only if we allocated the buffer ourselves
}

void Surface::Clear(uint c) {
#if 1
	//slow
	// WARNING: not the fastest way to do this.
	const int s = width * height;
	for (int i = 0; i < s; i++) pixels[i] = c;
#else
	//fast
	memset(pixels, c, width * height * sizeof(uint));
#endif
}

void Surface::Clear(uint c, float alpha) {

	if (alpha == 1.0f) {
		Clear(c);
		return;
	}

	if (alpha == 0.0f) return;

	// WARNING: not the fastest way to do this.
	const int s = width * height;
	for (int i = 0; i < s; i++) {
		uint p = pixels[i];
		if (p == c) continue;
		uint r = (p & 0xff0000) >> 16;
		uint g = (p & 0x00ff00) >> 8;
		uint b = (p & 0x0000ff);
		r = (uint)(r * (1.0f - alpha) + ((c & 0xff0000) >> 16) * alpha);
		g = (uint)(g * (1.0f - alpha) + ((c & 0x00ff00) >> 8) * alpha);
		b = (uint)(b * (1.0f - alpha) + (c & 0x0000ff) * alpha);
		pixels[i] = (r << 16) + (g << 8) + b;
	}
}

void Surface::ClearSIMD(uint c, uint alpha) {//doesnt work
	const int s = width * height;
	const __m128i alphaVec = _mm_set1_epi16(alpha); // Duplicate alpha into each 16-bit part of the vector
	const __m128i oneMinusAlphaVec = _mm_set1_epi16(255 - alpha);
	const __m128i colorVec = _mm_set1_epi32(c | (c << 16)); // Duplicate color twice in the vector

	// Process four pixels per iteration
	for (int i = 0; i < s; i += 4) {
		__m128i pixelsVec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(pixels + i)); // Load 4 pixels
		__m128i srcVec = _mm_unpacklo_epi8(pixelsVec, _mm_setzero_si128()); // Unpack pixels to 16 bits per channel
		__m128i destVec = _mm_unpacklo_epi8(colorVec, _mm_setzero_si128()); // Unpack color to 16 bits per channel

		// Blend each channel
		__m128i srcAlpha = _mm_mullo_epi16(srcVec, oneMinusAlphaVec);
		__m128i destAlpha = _mm_mullo_epi16(destVec, alphaVec);
		__m128i blended = _mm_add_epi16(srcAlpha, destAlpha);
		blended = _mm_add_epi16(blended, _mm_srli_epi16(blended, 8)); // Add blended >> 8 to blended
		blended = _mm_srli_epi16(blended, 8); // Then divide by 256

		__m128i result = _mm_packus_epi16(blended, blended); // Pack back to 8 bits per channel
		_mm_storeu_si128(reinterpret_cast<__m128i*>(pixels + i), result); // Store result back to memory
	}
}

void Surface::Plot(const int x, const int y, const  uint c) {
	if (x < 0 || y < 0 || x >= width || y >= height) return;
	pixels[x + y * width] = c;
}

uint Tmpl8::Surface::GetPixel(int x, int y) {
	if (x < 0 || y < 0 || x >= width || y >= height) return 0;
	return pixels[x + y * width];
}

//fast circle plot
void Surface::Circle(const int x, const int y, const int r, const uint c) {
	int r2 = r * r;
	for (int dx = -r; dx <= r; dx++) {
		int h = (int)sqrtf((float)(r2 - dx * dx));
		for (int dy = -h; dy <= h; dy++) {
			Plot(x + dx, y + dy, c);
		}
	}
}

void Surface::Box(int x1, int y1, int x2, int y2, uint c) {
	Line((float)x1, (float)y1, (float)x2, (float)y1, c);
	Line((float)x2, (float)y1, (float)x2, (float)y2, c);
	Line((float)x1, (float)y2, (float)x2, (float)y2, c);
	Line((float)x1, (float)y1, (float)x1, (float)y2, c);
}

void Surface::Bar(int x1, int y1, int x2, int y2, uint c) {
	// clipping
	if (x1 < 0) x1 = 0;
	if (x2 >= width) x2 = width - 1;
	if (y1 < 0) y1 = 0;
	if (y2 >= height) y2 = width - 1;
	// draw clipped bar
	uint* a = x1 + y1 * width + pixels;
	for (int y = y1; y <= y2; y++) {
		for (int x = 0; x <= (x2 - x1); x++) a[x] = c;
		a += width;
	}
}

void Tmpl8::Surface::Rect(int x1, int y1, int x2, int y2, float rotation, uint color) {
	// calculate the center of the rectangle
	float centerX = (x1 + x2) / 2;
	float centerY = (y1 + y2) / 2;
	// calculate the width and height of the rectangle
	float width = x2 - x1;
	float height = y2 - y1;
	// calculate the sine and cosine of the rotation angle
	float sine = sin(rotation);
	float cosine = cos(rotation);
	// calculate the four corners of the rectangle
	float x1r = centerX + cosine * (x1 - centerX) - sine * (y1 - centerY);
	float y1r = centerY + sine * (x1 - centerX) + cosine * (y1 - centerY);
	float x2r = centerX + cosine * (x2 - centerX) - sine * (y1 - centerY);
	float y2r = centerY + sine * (x2 - centerX) + cosine * (y1 - centerY);
	float x3r = centerX + cosine * (x2 - centerX) - sine * (y2 - centerY);
	float y3r = centerY + sine * (x2 - centerX) + cosine * (y2 - centerY);
	float x4r = centerX + cosine * (x1 - centerX) - sine * (y2 - centerY);
	float y4r = centerY + sine * (x1 - centerX) + cosine * (y2 - centerY);
	// draw the four lines of the rectangle
	Line(x1r, y1r, x2r, y2r, color);
	Line(x2r, y2r, x3r, y3r, color);
	Line(x3r, y3r, x4r, y4r, color);
	Line(x4r, y4r, x1r, y1r, color);
}
// Surface::Print: Print some text with the hard-coded mini-font.
void Surface::Print(const char* s, int x1, int y1, uint c) {
	if (!fontInitialized) {
		// we will initialize the font on first use
		InitCharset();
		fontInitialized = true;
	}
	uint* t = pixels + x1 + y1 * width;
	for (int i = 0; i < (int)(strlen(s)); i++, t += 6) {
		int pos = 0;
		if ((s[i] >= 'A') && (s[i] <= 'Z')) pos = transl[(unsigned short)(s[i] - ('A' - 'a'))];
		else pos = transl[(unsigned short)s[i]];
		uint* a = t;
		const char* u = (const char*)font[pos];
		for (int v = 0; v < 5; v++, u++, a += width)
			for (int h = 0; h < 5; h++) if (*u++ == 'o') *(a + h) = c, * (a + h + width) = 0;
	}
}

// Surface::Line: Draw a line between the specified screen coordinates.
// Uses clipping for lines that are partially off-screen. Not efficient.
void Surface::Line(float x1, float y1, float x2, float y2, uint c) {
	// clip (Cohen-Sutherland, https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm)
	const float xmin = 0, ymin = 0, xmax = (float)width - 1, ymax = (float)height - 1;
	int c0 = OUTCODE(x1, y1), c1 = OUTCODE(x2, y2);
	bool accept = false;
	while (1) {
		if (!(c0 | c1)) { accept = true; break; } else if (c0 & c1) break; else {
			float x = 0, y = 0;
			const int co = c0 ? c0 : c1;
			if (co & 8) x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1), y = ymax;
			else if (co & 4) x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1), y = ymin;
			else if (co & 2) y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1), x = xmax;
			else if (co & 1) y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1), x = xmin;
			if (co == c0) x1 = x, y1 = y, c0 = OUTCODE(x1, y1);
			else x2 = x, y2 = y, c1 = OUTCODE(x2, y2);
		}
	}
	if (!accept) return;
	float b = x2 - x1, h = y2 - y1, l = fabsf(b);
	if (fabsf(h) > l) l = fabsf(h);
	int il = (int)l;
	float dx = b / (float)l, dy = h / (float)l;
	for (int i = 0; i <= il; i++, x1 += dx, y1 += dy)
		*(pixels + (int)x1 + (int)y1 * width) = c;
}

// Surface::CopyTo: Copy the contents of one Surface to another, at the specified
// location. With clipping.
void Surface::CopyTo(Surface* d, int x, int y) {
	uint* dst = d->pixels;
	uint* src = pixels;
	if ((src) && (dst)) {
		int srcwidth = width;
		int srcheight = height;
		int dstwidth = d->width;
		int dstheight = d->height;
		if ((srcwidth + x) > dstwidth) srcwidth = dstwidth - x;
		if ((srcheight + y) > dstheight) srcheight = dstheight - y;
		if (x < 0) src -= x, srcwidth += x, x = 0;
		if (y < 0) src -= y * srcwidth, srcheight += y, y = 0;
		if ((srcwidth > 0) && (srcheight > 0)) {
			dst += x + dstwidth * y;
			for (int i = 0; i < srcheight; i++) {
				memcpy(dst, src, srcwidth * 4);
				dst += dstwidth, src += width;
			}
		}
	}
}

void Surface::SetChar(int c, const char* c1, const char* c2, const char* c3, const char* c4, const char* c5) {
	strcpy(font[c][0], c1);
	strcpy(font[c][1], c2);
	strcpy(font[c][2], c3);
	strcpy(font[c][3], c4);
	strcpy(font[c][4], c5);
}

void Surface::InitCharset() {
	SetChar(0, ":ooo:", "o:::o", "ooooo", "o:::o", "o:::o");
	SetChar(1, "oooo:", "o:::o", "oooo:", "o:::o", "oooo:");
	SetChar(2, ":oooo", "o::::", "o::::", "o::::", ":oooo");
	SetChar(3, "oooo:", "o:::o", "o:::o", "o:::o", "oooo:");
	SetChar(4, "ooooo", "o::::", "oooo:", "o::::", "ooooo");
	SetChar(5, "ooooo", "o::::", "ooo::", "o::::", "o::::");
	SetChar(6, ":oooo", "o::::", "o:ooo", "o:::o", ":ooo:");
	SetChar(7, "o:::o", "o:::o", "ooooo", "o:::o", "o:::o");
	SetChar(8, "::o::", "::o::", "::o::", "::o::", "::o::");
	SetChar(9, ":::o:", ":::o:", ":::o:", ":::o:", "ooo::");
	SetChar(10, "o::o:", "o:o::", "oo:::", "o:o::", "o::o:");
	SetChar(11, "o::::", "o::::", "o::::", "o::::", "ooooo");
	SetChar(12, "oo:o:", "o:o:o", "o:o:o", "o:::o", "o:::o");
	SetChar(13, "o:::o", "oo::o", "o:o:o", "o::oo", "o:::o");
	SetChar(14, ":ooo:", "o:::o", "o:::o", "o:::o", ":ooo:");
	SetChar(15, "oooo:", "o:::o", "oooo:", "o::::", "o::::");
	SetChar(16, ":ooo:", "o:::o", "o:::o", "o::oo", ":oooo");
	SetChar(17, "oooo:", "o:::o", "oooo:", "o:o::", "o::o:");
	SetChar(18, ":oooo", "o::::", ":ooo:", "::::o", "oooo:");
	SetChar(19, "ooooo", "::o::", "::o::", "::o::", "::o::");
	SetChar(20, "o:::o", "o:::o", "o:::o", "o:::o", ":oooo");
	SetChar(21, "o:::o", "o:::o", ":o:o:", ":o:o:", "::o::");
	SetChar(22, "o:::o", "o:::o", "o:o:o", "o:o:o", ":o:o:");
	SetChar(23, "o:::o", ":o:o:", "::o::", ":o:o:", "o:::o");
	SetChar(24, "o:::o", "o:::o", ":oooo", "::::o", ":ooo:");
	SetChar(25, "ooooo", ":::o:", "::o::", ":o:::", "ooooo");
	SetChar(26, ":ooo:", "o::oo", "o:o:o", "oo::o", ":ooo:");
	SetChar(27, "::o::", ":oo::", "::o::", "::o::", ":ooo:");
	SetChar(28, ":ooo:", "o:::o", "::oo:", ":o:::", "ooooo");
	SetChar(29, "oooo:", "::::o", "::oo:", "::::o", "oooo:");
	SetChar(30, "o::::", "o::o:", "ooooo", ":::o:", ":::o:");
	SetChar(31, "ooooo", "o::::", "oooo:", "::::o", "oooo:");
	SetChar(32, ":oooo", "o::::", "oooo:", "o:::o", ":ooo:");
	SetChar(33, "ooooo", "::::o", ":::o:", "::o::", "::o::");
	SetChar(34, ":ooo:", "o:::o", ":ooo:", "o:::o", ":ooo:");
	SetChar(35, ":ooo:", "o:::o", ":oooo", "::::o", ":ooo:");
	SetChar(36, "::o::", "::o::", "::o::", ":::::", "::o::");
	SetChar(37, ":ooo:", "::::o", ":::o:", ":::::", "::o::");
	SetChar(38, ":::::", ":::::", "::o::", ":::::", "::o::");
	SetChar(39, ":::::", ":::::", ":ooo:", ":::::", ":ooo:");
	SetChar(40, ":::::", ":::::", ":::::", ":::o:", "::o::");
	SetChar(41, ":::::", ":::::", ":::::", ":::::", "::o::");
	SetChar(42, ":::::", ":::::", ":ooo:", ":::::", ":::::");
	SetChar(43, ":::o:", "::o::", "::o::", "::o::", ":::o:");
	SetChar(44, "::o::", ":::o:", ":::o:", ":::o:", "::o::");
	SetChar(45, ":::::", ":::::", ":::::", ":::::", ":::::");
	SetChar(46, "ooooo", "ooooo", "ooooo", "ooooo", "ooooo");
	SetChar(47, "::o::", "::o::", ":::::", ":::::", ":::::"); // Tnx Ferry
	SetChar(48, "o:o:o", ":ooo:", "ooooo", ":ooo:", "o:o:o");
	SetChar(49, "::::o", ":::o:", "::o::", ":o:::", "o::::");
	char c[] = "abcdefghijklmnopqrstuvwxyz0123456789!?:=,.-() #'*/";
	int i;
	for (i = 0; i < 256; i++) transl[i] = 45;
	for (i = 0; i < 50; i++) transl[(unsigned char)c[i]] = i;
}



Tmpl8::FLoatSurface::FLoatSurface(int w, int h, float4* buffer) {
	width = w;
	height = h;
	pixels = buffer;
	ownBuffer = false;
}

Tmpl8::FLoatSurface::FLoatSurface(int w, int h) {
	width = w;
	height = h;
	pixels = (float4*)MALLOC64(w * h * sizeof(float4));
	ownBuffer = true; // needs to be deleted in destructor
}

Tmpl8::FLoatSurface::FLoatSurface(const char* file) {
	LoadFromFile(file);
}

Tmpl8::FLoatSurface::~FLoatSurface() {
	if (ownBuffer) FREE64(pixels); // free only if we allocated the buffer ourselves
}

float4 Tmpl8::FLoatSurface::GetPixel(int x, int y) {
	if (x < 0 || y < 0 || x >= width || y >= height) return float4(0, 0, 0, 0);
	int index = x + y * width;
	float4 result = pixels[index];
	return result;
}

float4 Tmpl8::FLoatSurface::GetPixel(float u, float v) {

	int x = u * width;
	int y = v * height;

	int index = x + y * width;
	//clamp the index
	index = max(0, min(width * height - 1, index));
	float4 result = pixels[index];
	return result;
}

void Tmpl8::FLoatSurface::Clear(float4 c) {
	const int s = width * height;
	for (int i = 0; i < s; i++) pixels[i] = c;
}

void Tmpl8::FLoatSurface::Clear(float4 c, float alpha) {
	const int s = width * height;
	for (int i = 0; i < s; i++) {
		float4 p = pixels[i];
		p.x = p.x * (1.0f - alpha) + c.x * alpha;
		p.y = p.y * (1.0f - alpha) + c.y * alpha;
		p.z = p.z * (1.0f - alpha) + c.z * alpha;
		p.w = p.w * (1.0f - alpha) + c.w * alpha;
		pixels[i] = p;
	}
}

void Tmpl8::FLoatSurface::LoadFromFile(const char* file) {
	// use stb_image to load the image file
	int n;
	float* data = stbi_loadf(file, &width, &height, &n, 0);
	if (!data) return; // load failed
	pixels = (float4*)MALLOC64(width * height * sizeof(float4));
	ownBuffer = true; // needs to be deleted in destructor
	const int s = width * height;
	if (n == 1) {
		for (int i = 0; i < s; i++) {
			const unsigned char p = data[i];
			pixels[i] = float4(p, p, p, 1);
		}
	} else {
		for (int i = 0; i < s; i++) {
			pixels[i] = float4(data[i * n + 0], data[i * n + 1], data[i * n + 2], data[i * n + 3]);
		}
	}
	// free stb_image data
	stbi_image_free(data);
}

void Tmpl8::FLoatSurface::Line(float x1, float y1, float x2, float y2, float4 color) {
	// clip (Cohen-Sutherland, https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm)
	const float xmin = 0, ymin = 0, xmax = (float)width - 1, ymax = (float)height - 1;
	int c0 = OUTCODE(x1, y1), c1 = OUTCODE(x2, y2);
	bool accept = false;
	while (1) {
		if (!(c0 | c1)) { accept = true; break; } else if (c0 & c1) break; else {
			float x = 0, y = 0;
			const int co = c0 ? c0 : c1;
			if (co & 8) x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1), y = ymax;
			else if (co & 4) x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1), y = ymin;
			else if (co & 2) y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1), x = xmax;
			else if (co & 1) y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1), x = xmin;
			if (co == c0) x1 = x, y1 = y, c0 = OUTCODE(x1, y1);
			else x2 = x, y2 = y, c1 = OUTCODE(x2, y2);
		}
	}
	if (!accept) return;
	float b = x2 - x1, h = y2 - y1, l = fabsf(b);
	if (fabsf(h) > l) l = fabsf(h);
	int il = (int)l;
	float dx = b / (float)l, dy = h / (float)l;
	for (int i = 0; i <= il; i++, x1 += dx, y1 += dy)
		*(pixels + (int)x1 + (int)y1 * width) = color;
}

void Tmpl8::FLoatSurface::Plot(int x, int y, float4 color) {
	if (x < 0 || y < 0 || x >= width || y >= height) return;
	pixels[x + y * width] = color;
}

void Tmpl8::FLoatSurface::Box(int x1, int y1, int x2, int y2, float4 color) {
	Line((float)x1, (float)y1, (float)x2, (float)y1, color);
	Line((float)x2, (float)y1, (float)x2, (float)y2, color);
	Line((float)x1, (float)y2, (float)x2, (float)y2, color);
	Line((float)x1, (float)y1, (float)x1, (float)y2, color);
}

void Tmpl8::FLoatSurface::Bar(int x1, int y1, int x2, int y2, float4 color) {
	// clipping
	if (x1 < 0) x1 = 0;
	if (x2 >= width) x2 = width - 1;
	if (y1 < 0) y1 = 0;
	if (y2 >= height) y2 = width - 1;
	// draw clipped bar
	float4* a = x1 + y1 * width + pixels;
	for (int y = y1; y <= y2; y++) {
		for (int x = 0; x <= (x2 - x1); x++) a[x] = color;
		a += width;
	}
}

void Tmpl8::FLoatSurface::Circle(int x, int y, int r, float4 color) {
	int r2 = r * r;
	for (int dx = -r; dx <= r; dx++) {
		int h = (int)sqrtf((float)(r2 - dx * dx));
		for (int dy = -h; dy <= h; dy++) {
			Plot(x + dx, y + dy, color);
		}
	}
}

void Tmpl8::FLoatSurface::Rect(int x1, int y1, int x2, int y2, float rotation, float4 color) {
	// calculate the center of the rectangle
	float centerX = (x1 + x2) / 2;
	float centerY = (y1 + y2) / 2;
	// calculate the width and height of the rectangle
	float width = x2 - x1;
	float height = y2 - y1;
	// calculate the sine and cosine of the rotation angle
	float sine = sin(rotation);
	float cosine = cos(rotation);
	// calculate the four corners of the rectangle
	float x1r = centerX + cosine * (x1 - centerX) - sine * (y1 - centerY);
	float y1r = centerY + sine * (x1 - centerX) + cosine * (y1 - centerY);
	float x2r = centerX + cosine * (x2 - centerX) - sine * (y1 - centerY);
	float y2r = centerY + sine * (x2 - centerX) + cosine * (y1 - centerY);
	float x3r = centerX + cosine * (x2 - centerX) - sine * (y2 - centerY);
	float y3r = centerY + sine * (x2 - centerX) + cosine * (y2 - centerY);
	float x4r = centerX + cosine * (x1 - centerX) - sine * (y2 - centerY);
	float y4r = centerY + sine * (x1 - centerX) + cosine * (y2 - centerY);
	// draw the four lines of the rectangle
	Line(x1r, y1r, x2r, y2r, color);
	Line(x2r, y2r, x3r, y3r, color);
	Line(x3r, y3r, x4r, y4r, color);
	Line(x4r, y4r, x1r, y1r, color);
}

void Tmpl8::FLoatSurface::CopyTo(FLoatSurface* dst, int x, int y) {
	float4* dstPixels = dst->pixels;
	float4* srcPixels = pixels;
	if ((srcPixels) && (dstPixels)) {
		int srcwidth = width;
		int srcheight = height;
		int dstwidth = dst->width;
		int dstheight = dst->height;
		if ((srcwidth + x) > dstwidth) srcwidth = dstwidth - x;
		if ((srcheight + y) > dstheight) srcheight = dstheight - y;
		if (x < 0) srcPixels -= x, srcwidth += x, x = 0;
		if (y < 0) srcPixels -= y * srcwidth, srcheight += y, y = 0;
		if ((srcwidth > 0) && (srcheight > 0)) {
			dstPixels += x + dstwidth * y;
			for (int i = 0; i < srcheight; i++) {
				memcpy(dstPixels, srcPixels, srcwidth * 4);
				dstPixels += dstwidth, srcPixels += width;
			}
		}
	}
}
