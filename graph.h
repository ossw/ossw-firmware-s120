#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>
#include <stdbool.h>

#define DRAW_XOR		0
#define DRAW_WHITE	1
#define DRAW_BLACK	2

#define PI 3.1415926535
#define PI_2 2*PI
#define DEG_TO_RAD(deg) deg * PI / 180.0
#define CENTER_X (MLCD_XRES >> 1)
#define CENTER_Y (MLCD_YRES >> 1)
#define SWAP(a, b) {a ^= b; b ^= a; a ^= b;}
#define SIGN(a) (a > 0 ? 1 : (a < 0 ? -1 : 0))

void pixel(uint_fast8_t x_pos, uint_fast8_t y_pos, uint8_t mode);
void vLine(uint_fast8_t x, uint_fast8_t y1, uint_fast8_t y2, uint8_t mode);
void hLine(uint_fast8_t y, uint_fast8_t x1, uint_fast8_t x2, uint8_t mode);
void lineBresenham(uint_fast8_t x1, uint_fast8_t y1, uint_fast8_t x2, uint_fast8_t y2, uint8_t mode);
void lineEFLA(uint_fast8_t x, uint_fast8_t y, uint_fast8_t x2, uint_fast8_t y2);
void triangle(uint_fast8_t x0, uint_fast8_t y0, uint_fast8_t x1, uint_fast8_t y1, uint_fast8_t x2, uint_fast8_t y2, uint8_t mode);
void tetragon(uint_fast8_t x0, uint_fast8_t y0, uint_fast8_t x1, uint_fast8_t y1, uint_fast8_t x2, uint_fast8_t y2, uint_fast8_t x3, uint_fast8_t y3, uint8_t mode);
void fillConvex(int_fast8_t size, int16_t x[], int16_t y[], uint8_t mode);
void circle(uint_fast8_t xc, uint_fast8_t yc, uint_fast8_t r, uint8_t mode);
void fillCircle(uint_fast8_t xc, uint_fast8_t yc, uint_fast8_t r, uint8_t mode);
void rectangle(uint_fast8_t x, uint_fast8_t y, uint_fast8_t w, uint_fast8_t h, uint8_t mode);
void fillRectangle(uint_fast8_t x, uint_fast8_t y, uint_fast8_t w, uint_fast8_t h, uint8_t mode);
void fillUp(uint_fast8_t x, uint_fast8_t y, uint_fast8_t h, uint8_t mode);
void fillDown(uint_fast8_t x, uint_fast8_t y, uint_fast8_t h, uint8_t mode);

void radialLine(int16_t cx, int16_t cy, int16_t deg, int16_t r1, int16_t r2, uint8_t mode);
void radialTriangle(int16_t cx, int16_t cy, int16_t deg, int16_t r1, int16_t r2, uint_fast8_t thickness, uint8_t mode);
void radialRect(int16_t cx, int16_t cy, int16_t deg, int16_t r1, int16_t r2, uint_fast8_t thickness, uint8_t mode);

void draw_switch(uint8_t x, uint8_t y, bool active);

#endif /* GRAPH_H */
