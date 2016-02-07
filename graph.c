#include <stdint.h>
#include <math.h>
#include "graph.h"
#include "mlcd.h"
#include "ext_ram.h"

// Graphics primitives
// Author: Pavel Vasilyev

// each line is 18 bytes long
#define LINE_OFFSET(y) (y << 1) + (y << 4)

// plot a XOR pixel
void pixel(uint_fast8_t x, uint_fast8_t y) {
    if (x > MLCD_XRES || y > MLCD_YRES)
			return;
		x = MLCD_XRES - x;
		mlcd_set_line_changed(y);
    uint16_t ext_ram_address = EXT_RAM_DATA_FRAME_BUFFER + (x >> 3) + LINE_OFFSET(y);
    uint8_t old_val = 0;
    ext_ram_read_data(ext_ram_address, &old_val, 1);
		uint8_t new_val = old_val ^ (1 << (7 - x & 0x7));
		ext_ram_write_data(ext_ram_address, &new_val, 1);
}

// plot a horizontal line
void hLine(uint_fast8_t y, uint_fast8_t x1, uint_fast8_t x2) {
    if ((x1 > MLCD_XRES && x2 > MLCD_XRES) || y > MLCD_YRES)
				return;
		x1 = MLCD_XRES - x1;
		x2 = MLCD_XRES - x2;
		if (x1 > x2)
				SWAP(x1, x2);
		mlcd_set_line_changed(y);
		uint_fast8_t leftByte = x1 >> 3;
		uint_fast8_t rightByte = x2 >> 3;
		uint8_t sizeByte = rightByte - leftByte + 1;
		uint8_t buff[sizeByte];
    uint16_t ext_ram_address = EXT_RAM_DATA_FRAME_BUFFER + leftByte + LINE_OFFSET(y);
    ext_ram_read_data(ext_ram_address, buff, sizeByte);
		
		uint_fast8_t leftBits = x1 & 0x7;
		if (leftBits > 0) {
				buff[0] ^=  0xFF >> leftBits;
				leftByte = 1;
		} else
				leftByte = 0;
		uint_fast8_t rightBits = x2 & 0x7;
		if (rightBits > 0) {
				buff[sizeByte-1] ^=  0xFF << (8 - rightBits);
				rightByte = sizeByte-1;
		} else
				rightByte = sizeByte;
		for (uint_fast8_t b = leftByte; b < rightByte; b++) {
				buff[b] ^=  0xFF;
		}
		
		ext_ram_write_data(ext_ram_address, buff, sizeByte);
}

void lineBresenham(uint_fast8_t x1, uint_fast8_t y1, uint_fast8_t x2, uint_fast8_t y2)
{
    int dy = y2 - y1;
    int dx = x2 - x1;
    int_fast8_t stepx, stepy;

    if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    dy <<= 1;        // dy is now 2*dy
    dx <<= 1;        // dx is now 2*dx

    pixel(x1,y1);
    if (dx > dy) {
        int fraction = dy - (dx >> 1);  // same as 2*dy - dx
        while (x1 != x2) {
           if (fraction >= 0) {
               y1 += stepy;
               fraction -= dx;          // same as fraction -= 2*dx
           }
           x1 += stepx;
           fraction += dy;              // same as fraction -= 2*dy
           pixel(x1, y1);
        }
    } else {
        int fraction = dx - (dy >> 1);
        while (y1 != y2) {
           if (fraction >= 0) {
               x1 += stepx;
               fraction -= dy;
           }
           y1 += stepy;
           fraction += dx;
           pixel(x1, y1);
        }
    }
}

// THE EXTREMELY FAST LINE ALGORITHM Variation E (Addition Fixed Point PreCalc Small Display)
// Small Display (256x256) resolution.
void lineEFLA(uint_fast8_t x, uint_fast8_t y, uint_fast8_t x2, uint_fast8_t y2) {
  bool yLonger = false;
	int shortLen = y2-y;
	int longLen = x2-x;
	if (abs(shortLen)>abs(longLen)) {
		int swap = shortLen;
		shortLen = longLen;
		longLen = swap;				
		yLonger = true;
	}
	int decInc;
	if (longLen == 0) decInc = 0;
	else decInc = (shortLen << 8) / longLen;

	if (yLonger) {
		if (longLen > 0) {
			longLen += y;
			for (int j = 0x80+(x<<8); y<=longLen; ++y) {
				pixel(j >> 8,y);	
				j += decInc;
			}
			return;
		}
		longLen += y;
		for (int j = 0x80+(x<<8); y>=longLen; --y) {
			pixel(j >> 8,y);	
			j -= decInc;
		}
		return;	
	}

	if (longLen>0) {
		longLen+=x;
		for (int j=0x80+(y<<8);x<=longLen;++x) {
			pixel(x,j >> 8);
			j+=decInc;
		}
		return;
	}
	longLen+=x;
	for (int j=0x80+(y<<8);x>=longLen;--x) {
		pixel(x,j >> 8);
		j-=decInc;
	}
}

void triangle(uint_fast8_t x0, uint_fast8_t y0, uint_fast8_t x1, uint_fast8_t y1, uint_fast8_t x2, uint_fast8_t y2) {
		lineBresenham(x0, y0, x1, y1);
		lineBresenham(x1, y1, x2, y2);
		lineBresenham(x2, y2, x0, y0);
}

void tetragon(uint_fast8_t x0, uint_fast8_t y0, uint_fast8_t x1, uint_fast8_t y1, uint_fast8_t x2, uint_fast8_t y2, uint_fast8_t x3, uint_fast8_t y3) {
		lineBresenham(x0, y0, x1, y1);
		lineBresenham(x1, y1, x2, y2);
		lineBresenham(x2, y2, x3, y3);
		lineBresenham(x3, y3, x0, y0);
}

void polygon(int_fast8_t size, uint_fast8_t x[], uint_fast8_t y[]) {
		--size;
		for (int_fast8_t i = 0; i < size; i++) {
				lineBresenham(x[i], y[i], x[i+1], y[i+1]);
		}
		lineBresenham(x[size], y[size], x[0], y[0]);
}

static void fillBorder(uint_fast8_t border[], uint_fast8_t x1, uint_fast8_t y1, uint_fast8_t x2, uint_fast8_t y2)
{
    int dy = y2 - y1;
    int dx = x2 - x1;
    int_fast8_t stepx, stepy;

    if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    dy <<= 1;
    dx <<= 1;

    border[y1] = x1;
    if (dx > dy) {
        int fraction = dy - (dx >> 1);
        while (x1 != x2) {
						if (fraction >= 0) {
								y1 += stepy;
								fraction -= dx;
						}
						x1 += stepx;
						fraction += dy;
						border[y1] = x1;
        }
    } else {
        int fraction = dx - (dy >> 1);
        while (y1 != y2) {
						if (fraction >= 0) {
								x1 += stepx;
								fraction -= dy;
						}
						y1 += stepy;
						fraction += dx;
						border[y1] = x1;
        }
		}
}

void fillConvex(int_fast8_t size, int_fast16_t x[], int_fast16_t y[]) {
		if (size < 3)
				return;
		int_fast8_t last = size - 1;
		int_fast16_t dir = SIGN(y[0] - y[last]);
		int_fast16_t newDir;
		uint_fast8_t l1, l2, r1, r2;
		for (int_fast8_t i = 0; i < last; i++) {
				newDir = SIGN(y[i+1] - y[i]);
				if (dir == newDir)
						continue;
				if (dir == 1) {
						r2 = i;
						if (newDir == -1)
								l1 = i;
				} else if (dir == -1) {
						l2 = i;
						if (newDir == 1)
								r1 = i;
				} else
						r1 = i;
				dir = newDir;
		}
		newDir = SIGN(y[0] - y[last]);
		if (dir != newDir) {
				if (dir == 1) {
						r2 = last;
						if (newDir == -1)
								l1 = last;
				} else if (dir == -1) {
						l2 = last;
						if (newDir == 1)
								r1 = last;
				} else
						r1 = last;
		}
		uint_fast8_t l[MLCD_YRES], r[MLCD_YRES];
		uint_fast8_t curr = l1, next;
		do {
				next = curr + 1;
				if (next >= size)
						next = 0;
				fillBorder(l, x[curr], y[curr], x[next], y[next]);
				curr = next;
		} while (next != l2);
		curr = r1;
		do {
				next = curr + 1;
				if (next >= size)
						next = 0;
				fillBorder(r, x[curr], y[curr], x[next], y[next]);
				curr = next;
		} while (next != r2);

		for (uint_fast8_t line = y[l2]; line < y[l1]; line++) {
				hLine(line, l[line], r[line]);
		}
}

void lineHand(uint_fast8_t tick, uint_fast8_t length, uint_fast8_t tail) {
		float angle = tick * PI_2 / 60;
		float sa = sin(angle);
		float ca = cos(angle);
		uint_fast8_t x0 = CENTER_X + length * sa;
		uint_fast8_t y0 = CENTER_Y - length * ca;
		uint_fast8_t x1 = CENTER_X - tail * sa;
		uint_fast8_t y1 = CENTER_Y + tail * ca;
		lineBresenham(x0, y0, x1, y1);
}

void triangleHand(uint_fast8_t tick, uint_fast8_t length, uint_fast8_t tail, uint_fast8_t thickness) {
		float angle = tick * PI_2 / 60;
		float sa = sin(angle);
		float ca = cos(angle);
		float shiftX = thickness * ca;
		float shiftY = thickness * sa;
		uint_fast8_t x0 = CENTER_X + length * sa ;
		uint_fast8_t y0 = CENTER_Y - length * ca;
		uint_fast8_t x1 = CENTER_X - tail * sa + 0.5 * shiftX;
		uint_fast8_t y1 = CENTER_Y + tail * ca + 0.5 * shiftY;
		uint_fast8_t x2 = x1 - shiftX;
		uint_fast8_t y2 = y1 - shiftY;
		triangle(x0, y0, x1, y1, x2, y2);
}

void rectHand(uint_fast8_t tick, uint_fast8_t length, uint_fast8_t tail, uint_fast8_t thickness) {
		float angle = tick * PI_2 / 60;
		float sa = sin(angle);
		float ca = cos(angle);
		float shiftX = thickness * ca;
		float shiftY = thickness * sa;
		uint_fast8_t x0 = CENTER_X + length * sa - 0.5 * shiftX;
		uint_fast8_t y0 = CENTER_Y - length * ca - 0.5 * shiftY;
		uint_fast8_t x1 = x0 + shiftX;
		uint_fast8_t y1 = y0 + shiftY;
		uint_fast8_t x2 = CENTER_X - tail * sa + 0.5 * shiftX;
		uint_fast8_t y2 = CENTER_Y + tail * ca + 0.5 * shiftY;
		uint_fast8_t x3 = x2 - shiftX;
		uint_fast8_t y3 = y2 - shiftY;
		tetragon(x0, y0, x1, y1, x2, y2, x3, y3);
}
