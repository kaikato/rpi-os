#ifndef SPHERE_H
#define SPHERE_H

#include "ray.h"
#include "sqrt.h"
#include "materials.h"
#include "hit_record.h"


typedef struct sphere {
    vec3 center;
    float radius;
    material *material;
} sphere;

int sphere_hit(sphere sp, ray r, float t_min, float t_max, hit_record *rec) {
    vec3 oc = vec3_subtract(r.origin, sp.center);
    float a = vec3_length_squared(r.direction);

    float half_b = vec3_dot(oc, r.direction);
    float c = vec3_length_squared(oc) - sp.radius*sp.radius;

    float discriminant = half_b*half_b - a*c;
    if (discriminant < 0) return 0;
    float sqrtd = sqrtf_babylonian(discriminant);

    // Find the nearest root that lies in the acceptable range.
    float root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return 0;
    }

    rec->t = root;
    rec->p = ray_at(r, rec->t);
    rec->material = sp.material;
    vec3 outward_normal = vec3_divide(vec3_subtract(rec->p, sp.center), sp.radius); // (rec.p - sp.center) / sp.radius;
    hit_record_set_face_normal(rec, r, outward_normal);
    return 1;
}

int spheres_hit(sphere* spheres, int sphere_count, ray r, float t_min, float t_max, hit_record *rec) {
    hit_record temp_rec;
    int hit_anything = 0;
    float closest_so_far = t_max;

    for (int i = 0; i < sphere_count; i++) {
        sphere sp = spheres[i];
        if (sphere_hit(sp, r, t_min, closest_so_far, &temp_rec)) {
            hit_anything = 1;
            closest_so_far = temp_rec.t;
            *rec = temp_rec;
        }
    }

    return hit_anything;
}

#endif