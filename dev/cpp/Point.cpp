#include "Point.h"

#include <cmath>

Point::Point(float _x, float _y) {
	x = _x;
	y = _y;
}

Point::Point(vec2 v) {
	x = v[0];
	y = v[1];
}



extern float dist2(float x1, float y1, float x2, float y2) {
    return (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
}
extern float dist(float x1, float y1, float x2, float y2) {
    return sqrt(dist2(x1, y1, x2, y2));
}




extern Point pAdd(Point p1, Point p2) {
	return {p1.x + p2.x, p1.y + p2.y};
}

extern Point pSub(Point p1, Point p2) {
    return {p1.x - p2.x, p1.y - p2.y};
}
extern float pLength2(Point p) {
    return p.x * p.x + p.y * p.y;
}
extern float pLength(Point p) {
    return sqrt(pLength2(p));
}
extern float pDistance2(Point p1, Point p2) {
    return dist2(p1.x, p1.y, p2.x, p2.y);
}
extern float pDistance(Point p1, Point p2) {
    return sqrt(pDistance2(p1, p2));
}
extern Point pNormalize(Point p) {
    float length = pLength(p);

    if (0.0 == length) {
        return {1.0, 0};
    } else {
        return {p.x / length, p.y / length};
    }
}
extern Point pMult(Point p, float factor) {
    return {p.x * factor, p.y * factor};
}
extern float pDot(Point p1, Point p2) {
    return p1.x * p2.x + p1.y * p2.y;
}
extern float pCross(Point p1, Point p2) {
    return p1.x * p2.y - p1.y * p2.x;
}
extern Point pLerp(Point p1, Point p2, float alpha) {
  return pAdd(pMult(p1, 1-alpha), pMult(p2, alpha));
}
