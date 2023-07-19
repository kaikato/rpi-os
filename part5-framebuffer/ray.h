#ifndef RAY_H
#define RAY_H

#include "vec3.h"

typedef struct ray {
  vec3 origin;
  vec3 direction;
} ray;


vec3 ray_at(ray r, double t);
#endif