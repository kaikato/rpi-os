#ifndef SCATTER_H
#define SCATTER_H

#include "materials.h"
#include "ray.h"
#include "hit_record.h"

int scatter(const material m, const ray r_in, const hit_record rec, vec3 *attenuation_color, ray *scattered);
int lambertian_scatter(const lambertian la, const ray r_in, const hit_record rec, vec3 *attenuation_color, ray *scattered);
int metal_scatter(const metal me, const ray r_in, const hit_record rec, vec3 *attenuation_color, ray *scattered);
int dielectric_scatter(const ray* r_in, const hit_record* rec, vec3* attenuation, ray* scattered, const dielectric* mat);
int metal_scatter_fuzz(const metal me, const ray r_in, const hit_record rec, vec3 *attenuation_color, ray *scattered);

#endif