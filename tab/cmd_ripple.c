/*
 * cmd_ripple.c
 *
 * Generate a concentric ripple/water-wave translation table.
 * Pixels are displaced radially with sinusoidal oscillation,
 * creating a pulsing ripple-from-center effect.
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int BUFF_WIDTH = 0;
int BUFF_HEIGHT = 0;

int main(int argc, char **argv) {
    int xx, yy, cx, cy;
    double radius, angle, displacement, new_r;
    int map_x, map_y;
    unsigned int map;
    double amplitude = 8.0;
    double frequency = 0.15;

    if (argc < 3) {
	fprintf(stderr, "Usage: %s WIDTH HEIGHT [amplitude] [frequency]\n", argv[0]);
	return 1;
    }
    BUFF_WIDTH = atoi(argv[1]);
    BUFF_HEIGHT = atoi(argv[2]);
    if (argc > 3) amplitude = atof(argv[3]);
    if (argc > 4) frequency = atof(argv[4]);

    cx = BUFF_WIDTH / 2;
    cy = BUFF_HEIGHT / 2;

    for (yy = 0; yy < BUFF_HEIGHT; yy++) {
	for (xx = 0; xx < BUFF_WIDTH; xx++) {
	    double dx = xx - cx;
	    double dy = yy - cy;
	    radius = sqrt(dx * dx + dy * dy);

	    if (radius < 1.0) {
		map = yy * BUFF_WIDTH + xx;
	    } else {
		angle = atan2(dy, dx);
		displacement = amplitude * sin(radius * frequency);
		new_r = radius + displacement;

		map_x = cx + (int)(new_r * cos(angle));
		map_y = cy + (int)(new_r * sin(angle));

		while (map_x < 0) map_x += BUFF_WIDTH;
		while (map_y < 0) map_y += BUFF_HEIGHT;
		map_x = map_x % BUFF_WIDTH;
		map_y = map_y % BUFF_HEIGHT;

		map = map_y * BUFF_WIDTH + map_x;
	    }

	    if (fwrite(&map, sizeof(int), 1, stdout) != 1) {
		fprintf(stderr, "Error writing output\n");
		return 2;
	    }
	}
    }
    return 0;
}
