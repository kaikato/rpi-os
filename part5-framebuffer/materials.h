#ifndef MATERIALS_H
#define MATERIALS_H

#include "vec3.h"

typedef struct lambertian {
    vec3 albedo;
} lambertian;

typedef struct metal {
    vec3 albedo;
    float fuzz;
} metal;

typedef struct dielectric {
    float ir; // index of refraction
} dielectric;

typedef enum material_id{
// add other IDs to enum to union
  LAMBERTIAN_ID = 0,
  METAL_ID,
  DIELECTRIC_ID,
} material_id;

typedef union material_data {
    // add other materials to union
    lambertian _lambertian;
    metal _metal;
    dielectric _dielectric;
} material_data;

typedef struct material{
    material_id id;
    union material_data data;
} material;

#endif