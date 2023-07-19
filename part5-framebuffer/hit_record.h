#ifndef HIT_RECORD_H
#define HIT_RECORD_H

#include "materials.h"
#include "ray.h"

typedef struct hit_record {
    float t;
    vec3 p;
    vec3 normal;
    int front_face;
    material *material;
} hit_record;

void hit_record_set_face_normal(hit_record *rec, ray r, vec3 outward_normal);

#endif