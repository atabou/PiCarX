#include "filters.hpp"


FIRFilter::FIRFilter(float alpha, float x0) {

    this->alpha = alpha;
    this->xp = x0;

}


float FIRFilter::pass(float x) {

    xp = alpha * x + (1.0f - alpha) * xp;

    return xp;

}


void FIRFilter::reset(float x0) {

    this->xp = x0;

}
