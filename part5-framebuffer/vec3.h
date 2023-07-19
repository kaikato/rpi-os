#ifndef VEC3_H
#define VEC3_H


#if ON_PI
#include "rpi.h"
#include "printf.h"

#else
#include <stdio.h>

#endif

#include "util.h"
#include "sqrt.h"

typedef struct vec3 {
    float x, y, z;
} vec3;

vec3 vec3_add(const vec3 a, const vec3 b);
vec3 vec3_subtract(const vec3 a, const vec3 b);
vec3 vec3_cross(const vec3 a, const vec3 b);
float vec3_dot(const vec3 a, const vec3 b);

vec3 vec3_multiply(const vec3 a, const float t);
vec3 vec3_multiply_vectors(const vec3 a, const vec3 b);
vec3 vec3_divide(const vec3 a, const float t);
vec3 vec3_normalize(const vec3 a);
vec3 vec3_negate(const vec3 a);
float vec3_length(const vec3 a);
float vec3_length_squared(const vec3 a);

uint32_t vec3_to_pixel_value(const vec3 a, const int samples_per_pixel);
void vec3_print(const char *opening, const vec3 a);

vec3 _random();
vec3 random_bounded(const float min, const float max);
vec3 random_in_unit_sphere();
vec3 random_unit_vector();
int vec3_near_zero(const vec3 a);
vec3 vec3_reflect(const vec3 v, const vec3 n);
vec3 vec3_refract(const vec3 uv, const vec3 n, const float etai_over_etat);
float vec3_fabs(const float x);
float vec3_fmin(const float x, const float y);
int less_than_abs(const float x, const float bar);
vec3 random_in_unit_disk();

#endif
