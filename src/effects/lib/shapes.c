#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "utils.h"

static void heart_layer(uint8_t x, uint16_t intensity);

void heart_shape(void)
{
	heart_layer(1, (float)MAX_INTENSITY / 100);
	heart_layer(2, (float)MAX_INTENSITY / 25);
	heart_layer(3, MAX_INTENSITY);
	heart_layer(4, MAX_INTENSITY);
	heart_layer(5, (float)MAX_INTENSITY / 25);
	heart_layer(6, (float)MAX_INTENSITY / 100);
}

static void heart_layer(uint8_t x, uint16_t intensity) {
	set_row(x, 0, 1, 2, intensity);
	set_row(x, 0, 5, 6, intensity);
	set_row(x, 1, 0, 7, intensity);
	set_row(x, 2, 0, 7, intensity);
	set_row(x, 3, 0, 7, intensity);
	set_row(x, 4, 1, 6, intensity);
	set_row(x, 5, 2, 5, intensity);
	set_row(x, 6, 3, 4, intensity);
}

void sphere_shape(float xi, float yi, float zi, float rsq_min, float rsq_max, float fac)
{
	for(float x = xi; x < LEDS_X + xi; x++) {
		for(float y = yi; y < LEDS_Y + yi; y++) {
			for(float z = zi; z < LEDS_Z + zi; z++) {
				float sq = x * x + y * y + z * z;

				if(rsq_min * fac < sq && sq < rsq_max * fac) {
					set_led(x - zi, y - yi, z - zi, MAX_INTENSITY);
				}
			}
		}
	}
}

/*
 * Bresenham's algorithm in 3D.
 *
 * Reference: ftp://ftp.isc.org/pub/usenet/comp.sources.unix/volume26/line3d
 */
void line(uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2,
	uint16_t intensity)
{
	uint8_t x, y, z, ax, ay, az;
	int8_t xd, yd, zd, dx, dy, dz, sx, sy, sz;

	dx = x2 - x1;
	dy = y2 - y1;
	dz = z2 - z1;

	ax = ABS(dx) << 1;
	ay = ABS(dy) << 1;
	az = ABS(dz) << 1;

	sx = ZSGN(dx);
	sy = ZSGN(dy);
	sz = ZSGN(dz);

	x = x1;
	y = y1;
	z = z1;

	if(ax >= MAX(ay, az)) { /* x dominant */
		yd = ay - (ax >> 1);
		zd = az - (ax >> 1);

		for(;;) {
			set_led(x, y, z, intensity);

			if(x == x2) return;

			if(yd >= 0) {
				y += sy;
				yd -= ax;
			}

			if(zd >= 0) {
				z += sz;
				zd -= ax;
			}

			x += sx;
			yd += ay;
			zd += az;
		}
	}
	else if(ay >= MAX(ax, az)) { /* y dominant */
		xd = ax - (ay >> 1);
		zd = az - (ay >> 1);

		for(;;) {
			set_led(x, y, z, intensity);

			if(y == y2) return;

			if(xd >= 0) {
				x += sx;
				xd -= ay;
			}

			if(zd >= 0) {
				z += sz;
				zd -= ay;
			}

			y += sy;
			xd += ax;
			zd += az;
		}
	}
	else if(az >= MAX(ax, ay)) { /* z dominant */
		xd = ax - (az >> 1);
		yd = ay - (az >> 1);

		for(;;) {
			set_led(x, y, z, intensity);

			if(z == z2) return;

			if(xd >= 0) {
				x += sx;
				xd -= az;
			}

			if(yd >= 0) {
				y += sy;
				yd -= az;
			}

			z += sz;
			xd += ax;
			yd += ay;
		}
	}
}

void cube_shape(uint8_t x1, uint8_t y1, uint8_t z1, uint8_t x2, uint8_t y2, uint8_t z2,
	uint16_t intensity)
{
	line(x1, y1, z1, x1, y1, z2, intensity);
	line(x1, y1, z1, x2, y1, z1, intensity);
	line(x1, y1, z1, x1, y2, z1, intensity);

	line(x2, y2, z2, x2, y2, z1, intensity);
	line(x2, y2, z2, x2, y1, z2, intensity);
	line(x2, y2, z2, x1, y2, z2, intensity);

	line(x1, y2, z1, x2, y2, z1, intensity);
	line(x1, y2, z1, x1, y2, z2, intensity);
	line(x2, y1, z1, x2, y1, z2, intensity);
	line(x2, y1, z1, x2, y2, z1, intensity);
	line(x1, y1, z2, x1, y2, z2, intensity);
	line(x1, y1, z2, x2, x1, z2, intensity);
}
