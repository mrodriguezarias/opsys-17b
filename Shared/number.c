#include "number.h"
#include <math.h>

t_number number_ceiling(t_number x) {
	return ceil(x);
}

bool number_equals(t_number x, t_number y) {
	return fabs(x - y) < 0.000001;
}
