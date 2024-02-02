#include "filters.hpp"


FIRFilter::FIRFilter(float alpha=1.0f, float x0=0.0f) {

    this->alpha = alpha;
    this->xp = x0;

}


float FIRFilter::pass(float x) {

    xp = alpha * x + (1.0f - alpha) * xp;

    return xp;

}


void FIRFilter::reset(float x0=0.0f) {

    this->xp = x0;

}