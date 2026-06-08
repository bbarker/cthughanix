/*
 * cmd_tunnel.c
 *
 * Generate a tunnel/starfield warp translation table.
 * Combines radial zoom (toward center) with slow rotation,
 * creating a flying-through-a-tunnel effect.
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int BUFF_WIDTH = 0;
int BUFF_HEIGHT = 0;

int main(int argc, char **argv) {
    int xx, yy, cx, cy;
    int map_x, map_y;
    unsigned int map;
    double zoom = 0.03;
    double twist = 0.02;

    if (argc < 3) {
	fprintf(stderr, "Usage: %s WIDTH HEIGHT [zoom] [twist]\n", argv[0]);
	return 1;
    }
    BUFF_WIDTH = atoi(argv[1]);
    BUFF_HEIGHT = atoi(argv[2]);
    if (argc > 3) zoom = atof(argv[3]);
    if (argc > 4) twist = atof(argv[4]);

    cx = BUFF_WIDTH / 2;
    cy = BUFF_HEIGHT / 2;

    for (yy = 0; yy < BUFF_HEIGHT; yy++) {
	for (xx = 0; xx < BUFF_WIDTH; xx++) {
	    double dx = xx - cx;
	    double dy = yy - cy;
	    double radius = sqrt(dx * dx + dy * dy);
	    double angle = atan2(dy, dx);

	    /* Zoom toward center + rotate */
	    double new_r = radius * (1.0 + zoom);
	    double new_a = angle + twist;

	    map_x = cx + (int)(new_r * cos(new_a));
	    map_y = cy + (int)(new_r * sin(new_a));

	    while (map_x < 0) map_x += BUFF_WIDTH;
	    while (map_y < 0) map_y += BUFF_HEIGHT;
	    map_x = map_x % BUFF_WIDTH;
	    map_y = map_y % BUFF_HEIGHT;

	    map = map_y * BUFF_WIDTH + map_x;

	    if (fwrite(&map, sizeof(int), 1, stdout) != 1) {
		fprintf(stderr, "Error writing output\n");
		return 2;
	    }
	}
    }
    return 0;
}
