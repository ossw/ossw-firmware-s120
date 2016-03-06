#include <stdint.h>
#include <math.h>
#include "graph.h"
#include "mlcd.h"
#include "mlcd_draw.h"
#include "ext_ram.h"

// Additional graphics primitives for monochrome LCD
// Author: Pavel Vasilyev

// each line is 18 bytes long
#define LINE_OFFSET(y) (y << 1) + (y << 4)
#define PLOT8(x0, y0, x, y) {\
		pixel( x + x0,  y + y0);\
		pixel( y + x0,  x + y0);\
		pixel(-x + x0,  y + y0);\
		pixel(-y + x0,  x + y0);\
    pixel(-x + x0, -y + y0);\
    pixel(-y + x0, -x + y0);\
    pixel( x + x0, -y + y0);\
    pixel( y + x0, -x + y0); }

// plot a XOR pixel
void pixel(uint_fast8_t x, uint_fast8_t y) {
    if (x > MLCD_XRES || y > MLCD_YRES)
			return;
		x = MLCD_XRES - x - 1;
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
		x1 = MLCD_XRES - x1 - 1;
		x2 = MLCD_XRES - x2 - 1;
		if (x1 > x2)
				SWAP(x1, x2);
		mlcd_set_line_changed(y);
		uint_fast8_t leftByte = x1 >> 3;
		uint_fast8_t rightByte = x2 >> 3;
		uint8_t sizeByte = rightByte - leftByte + 1;
		uint8_t buff[sizeByte];
    uint16_t ext_ram_address = EXT_RAM_DATA_FRAME_BUFFER + leftByte + LINE_OFFSET(y);
    ext_ram_read_data(ext_ram_address, buff, sizeByte);
		
		uint_fast8_t fullFirst = 0, fullLast = sizeByte;
		uint_fast8_t maskFirst = 0, maskLast = 0;
		
		uint_fast8_t leftBits = x1 & 0x7;
		if (leftBits > 0) {
				maskFirst = 0xFF >> leftBits;
				fullFirst++;
		}
		uint_fast8_t rightBits = x2 & 0x7;
		if (rightBits < 7) {
				maskLast = 0xFF << (7 - rightBits);
				fullLast--;
		}
		if (sizeByte == 1 && leftBits > 0 && rightBits < 7) {
				maskFirst &= maskLast;
				buff[0] ^=  maskFirst;
		} else {
				if (leftBits > 0) {
						buff[0] ^=  maskFirst;
				}
				if (rightBits < 7) {
						buff[sizeByte-1] ^= maskLast;
				}
		}
		for (uint_fast8_t b = fullFirst; b < fullLast; b++) {
				buff[b] ^=  0xFF;
		}
		
		ext_ram_write_data(ext_ram_address, buff, sizeByte);
}

void lineBresenham(uint_fast8_t x1, uint_fast8_t y1, uint_fast8_t x2, uint_fast8_t y2) {
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

static void fillBorder(uint8_t border[], uint_fast8_t x1, uint_fast8_t y1, uint_fast8_t x2, uint_fast8_t y2, bool right) {
		if (x2 < x1) {
				SWAP(x1, x2);
				SWAP(y1, y2);
		}
    int dy = y2 - y1;
    int dx = x2 - x1;
    int8_t stepy;

    if (dy < 0) { dy = -dy;  stepy = -1; }
		else { stepy = 1; }
    
    dy <<= 1;
    dx <<= 1;

    border[y1] = x1;
    if (dx > dy) {
        int fraction = dy - (dx >> 1);
        while (x1 != x2) {
						x1++;
						if (fraction >= 0) {
								if (right) {
										border[y1] = x1;
										y1 += stepy;
								} else {
										y1 += stepy;
										border[y1] = x1;
								}
								fraction -= dx;
						}
						fraction += dy;
        }
				if (right)
						border[y1] = x1;
    } else {
        int fraction = dx - (dy >> 1);
        while (y1 != y2) {
						if (fraction >= 0) {
								x1++;
								fraction -= dy;
						}
						y1 += stepy;
						fraction += dx;
						border[y1] = x1;
        }
		}
}

void fillConvex(int_fast8_t size, int16_t x[], int16_t y[]) {
		if (size < 3)
				return;
		int8_t last = size - 1;
		int16_t dir = SIGN(y[0] - y[last]);
		int16_t newDir;
		uint8_t l1, l2, r1, r2;
		for (int_fast8_t i = 0; i < last; i++) {
				newDir = SIGN(y[i+1] - y[i]);
				if (dir == newDir)
						continue;

				if (dir == 1)
						r2 = i;
				else if (dir == -1)
						l2 = i;
				if (newDir == 1)
						r1 = i;
				else if (newDir == -1)
						l1 = i;

				dir = newDir;
		}
		newDir = SIGN(y[0] - y[last]);
		if (dir != newDir) {
				if (dir == 1)
						r2 = last;
				else if (dir == -1)
						l2 = last;
				if (newDir == 1)
						r1 = last;
				else if (newDir == -1)
						l1 = last;
		}
		uint8_t l[MLCD_YRES], r[MLCD_YRES];
		uint8_t curr = l1, next;
		bool clockwize = (x[1]-x[0])*(y[2]-y[1]) - (x[2]-x[1])*(y[1]-y[0]) > 0;
		do {
				next = curr + 1;
				if (next >= size)
						next = 0;
				fillBorder(l, x[curr], y[curr], x[next], y[next], !clockwize);
				curr = next;
		} while (next != l2);
		curr = r1;
		do {
				next = curr + 1;
				if (next >= size)
						next = 0;
				fillBorder(r, x[curr], y[curr], x[next], y[next], clockwize);
				curr = next;
		} while (next != r2);

		for (uint_fast8_t line = y[l2]; line <= y[l1]; line++) {
				hLine(line, l[line], r[line]);
		}
}

void circle(uint_fast8_t xc, uint_fast8_t yc, uint_fast8_t r) {
    uint8_t x = r, y = 0;
    int cd2 = 0;  //current distance squared - radius squared
    if (!r)
			return; 
		pixel(xc,  yc + r);
		pixel(xc,  yc - r);
		pixel(xc + r,  yc);
		pixel(xc - r,  yc);
    while (x > y + 2) {
        cd2 -= (--x) - (++y);
        if (cd2 < 0)
						cd2 += x++;
				PLOT8(xc, yc, x, y);
    } 
    if (--x == ++y) {
				pixel(xc + x,  yc + y);
				pixel(xc - x,  yc + y);
				pixel(xc + x,  yc - y);
				pixel(xc - x,  yc - y);
		}
}

void fillCircle(uint_fast8_t xc, uint_fast8_t yc, uint_fast8_t r) {
    uint8_t x = r, y = 0;
    int cd2 = 0;
    if (!r)
			return; 
		hLine(yc,  xc - r, xc + r);
    while (x > y + 2) {
        cd2 -= (--x) - (++y);
        if (cd2 < 0) {
						cd2 += x++;
				} else {
						hLine(yc - x - 1, xc - y + 1, xc + y - 1);
						hLine(yc + x + 1, xc - y + 1, xc + y - 1);
				}
				hLine(yc - y, xc - x, xc + x);
				hLine(yc + y, xc - x, xc + x);
    } 
		hLine(yc - x, xc - y, xc + y);
		hLine(yc + x, xc - y, xc + y);
    if (--x == ++y) {
				hLine(yc - y, xc - x, xc + x);
				hLine(yc + y, xc - x, xc + x);
		}
}

void fillRectangle(uint_fast8_t x, uint_fast8_t y, uint_fast8_t w, uint_fast8_t h) {
		uint_fast8_t x1 = x + w;
		for (uint_fast8_t i = 0; i < h; i++) {
				hLine(y++, x, x1);
		}
}

void radialLine(int16_t cx, int16_t cy, int16_t deg, int16_t r1, int16_t r2) {
		float angle = DEG_TO_RAD(deg);
		float sa = sin(angle);
		float ca = cos(angle);
		int16_t x0 = cx + r1 * sa + 0.5;
		int16_t y0 = cy - r1 * ca + 0.5;
		int16_t x1 = cx + r2 * sa + 0.5;
		int16_t y1 = cy - r2 * ca + 0.5;
		lineBresenham(x0, y0, x1, y1);
}

void radialTriangle(int16_t cx, int16_t cy, int16_t deg, int16_t r1, int16_t r2, uint_fast8_t thickness) {
		float angle = DEG_TO_RAD(deg);
		float sa = sin(angle);
		float ca = cos(angle);
		thickness--;
		float shiftX = 0.5 * thickness * ca;
		float shiftY = 0.5 * thickness * sa;
		int16_t x0 = cx + r1 * sa + 0.5;
		int16_t y0 = cy - r1 * ca + 0.5;
		int16_t x1 = cx + r2 * sa + shiftX + 0.5;
		int16_t y1 = cy - r2 * ca + shiftY + 0.5;
		int16_t x2 = cx + r2 * sa - shiftX + 0.5;
		int16_t y2 = cy - r2 * ca - shiftY + 0.5;
		int16_t x[3] = {x0, x1, x2};
		int16_t y[3] = {y0, y1, y2};
		fillConvex(3, x, y);
//		triangle(x0, y0, x1, y1, x2, y2);
}

void radialRect(int16_t cx, int16_t cy, int16_t deg, int16_t r1, int16_t r2, uint_fast8_t thickness) {
		float angle = DEG_TO_RAD(deg);
		float sa = sin(angle);
		float ca = cos(angle);
		thickness--;
		float shiftX = 0.5 * thickness * ca;
		float shiftY = 0.5 * thickness * sa;
		int16_t x0 = cx + r1 * sa - shiftX + 0.5;
		int16_t y0 = cy - r1 * ca - shiftY + 0.5;
		int16_t x1 = cx + r1 * sa + shiftX + 0.5;
		int16_t y1 = cy - r1 * ca + shiftY + 0.5;
		int16_t x2 = cx + r2 * sa + shiftX + 0.5;
		int16_t y2 = cy - r2 * ca + shiftY + 0.5;
		int16_t x3 = cx + r2 * sa - shiftX + 0.5;
		int16_t y3 = cy - r2 * ca - shiftY + 0.5;
		int16_t x[4] = {x0, x1, x2, x3};
		int16_t y[4] = {y0, y1, y2, y3};
		fillConvex(4, x, y);
//		tetragon(x0, y0, x1, y1, x2, y2, x3, y3);
}

void draw_switch(uint8_t x, uint8_t y, bool active) {
		mlcd_clear_rect(x, y, 30, 15);
		if (active) {
				fillCircle(x+22, y+7, 7);
				mlcd_draw_rect(x+4, y+4, 10, 7);
		} else {
				circle(x+7, y+7, 7);
				mlcd_draw_rect(x+16, y+4, 10, 7);
		}
}

