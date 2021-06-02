#include "util.h"



double normalize(double t, double min, double max) {
	return (t-min)/(max-min);
}

double lerp(double t, double min, double max) {
	return (max-min)*t+min;
}

double map(double t, double s_min, double s_max, double d_min, double d_max) {
	return lerp(normalize(t, s_min, s_max), d_min, d_max);
}


