#ifndef UTIL_H
#define UTIL_H

#if ON_PI
#include "io.h"
#include "rpi.h"

#else
#include <stdint.h>

#endif

#include <math.h>
#include <limits.h>
#include <stdlib.h>

// Constants

#define infinity INFINITY
#define pi M_PI
#define RNG_CTRL        ((volatile uint32_t*)(PERIPHERAL_BASE + 0x00104000))
#define RNG_STATUS      ((volatile uint32_t*)(PERIPHERAL_BASE + 0x00104004))
#define RNG_DATA        ((volatile uint32_t*)(PERIPHERAL_BASE + 0x00104008))
#define RNG_INT_MASK    ((volatile uint32_t*)(PERIPHERAL_BASE + 0x00104010))

// Usings - Unsure if we need these at all. 
/* 
typedef struct shared_ptr shared_ptr;
#define make_shared(...) malloc(sizeof(__VA_ARGS__))
#define sqrt(x) sqrtf(x)
*/

// Utility Functions

float degrees_to_radians(float degrees);

float clamp(float x, float min, float max);

// from https://forums.raspberrypi.com/viewtopic.php?t=196015
uint32_t hardware_rand();

float random_float();

float random_float_bounded(float min, float max);

float my_tan(float x);

float my_cos(float x);

float my_fmax(float a, float b);

float my_fmin(float a, float b);

#endif
