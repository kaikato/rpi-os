#include "ray.h"

vec3 ray_at(ray r, double t) {
  // origin + t *direction
  return vec3_add(r.origin, vec3_multiply(r.direction, t)); 
}