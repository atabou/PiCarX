#include "utilities.hpp"

#include <math.h>

float saturate(float value, float min, float max) {

    return (value < min) ? min : (value > max) ? max : value;

}

float map(float value, float min1, float max1, float min2, float max2) {

    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);

}


float logdiff(float a, float b, float bias) {

    return log(a + bias) - log(b + bias);

}
