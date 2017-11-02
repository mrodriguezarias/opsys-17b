#include "number.h"
#include <math.h>

t_number number_ceiling(t_number x) {
	return ceil(x);
}

bool number_equals(t_number x, t_number y) {
	return fabs(x - y) < 0.000001;
}

t_number number_min(t_number x, t_number y) {
	return x < y ? x : y;
}

t_number number_max(t_number x, t_number y) {
	return x > y ? x : y;
}

t_number number_round(t_number number, int precision) {
	double factor = pow(10, precision);
	return round(number * factor) / factor;
}
