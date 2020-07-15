#pragma once
extern "C" {
	extern int triangulate_polygon(int, int *, double(*)[2], int(*)[3]);
	extern int is_point_inside_polygon(double *);
}

