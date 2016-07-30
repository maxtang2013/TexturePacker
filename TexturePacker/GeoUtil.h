#ifndef __GEOUTIL_H__
#define __GEOUTIL_H__

// Test if point (xp, yp) is on the left side of segment (x0, y0) --> (x1, y1)
inline bool Left(int x0, int y0, int x1, int y1, int xp, int yp)
{
	double area2 = (x1 - x0) * (double)(yp - y0) - (xp - x0) * (double)(y1 - y0);
	return area2 < 0;
}

// 
inline bool LeftOn(int x0, int y0, int x1, int y1, int xp, int yp)
{
	double area2 = (x1 - x0) * (double)(yp - y0) - (xp - x0) * (double)(y1 - y0);
	return area2 <= 0;
}
#endif