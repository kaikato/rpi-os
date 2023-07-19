#include "util.h"

float degrees_to_radians(float degrees) {
    return degrees * pi / 180.0;
}

float clamp(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

#if ON_PI
//from https://forums.raspberrypi.com/viewtopic.php?t=196015
uint32_t hardware_rand() {
 if( !((get32(RNG_CTRL)) & 1) ) {       // initialize on first call
    put32(RNG_STATUS, 0x40000);         // not sure why is this important, but linux does it this way
    put32(RNG_INT_MASK, get32(RNG_INT_MASK) | 1);            // mask interrupt
    put32(RNG_CTRL, get32(RNG_CTRL) | 1);                // enable the generator
    while( !((get32(RNG_STATUS))>>24) ); // wait until it's entropy good enough
  }
  return get32(RNG_DATA);
}

#else
uint32_t hardware_rand() {
    return (uint32_t) rand();
}

#endif

inline float random_float() {
    // returns a random real in [0, 1)
    return (float)hardware_rand()/((float)RAND_MAX + 1.0);
}

inline float random_float_bounded(float min, float max) {
    // return a random real in [min, max)
    return min + (max - min)*random_float();
}

float my_tan(float x) {
    float sum = x;
    float term = x;
    float x_squared = x*x;
    int i = 1;
    
    while (term != 0) {
        term *= -x_squared / ((2*i)*(2*i+1));
        sum += term;
        i++;
    }
    
    return sum;
}

float my_cos(float x) {
    float sum = 1;
    float term = 1;
    float x_squared = x*x;
    int i = 1;
    
    while (term != 0) {
        term *= -x_squared / ((2*i-1)*(2*i));
        sum += term;
        i++;
    }
    
    return sum;
}

float my_fmax(float a, float b) {
    return a > b ? a : b;
}

float my_fmin(float a, float b) {
    return a < b ? a : b;
}

