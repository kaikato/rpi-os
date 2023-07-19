#include "vec3.h"
#include "util.h"

inline vec3 vec3_add(const vec3 a, const vec3 b){
    vec3 ret = {a.x + b.x, a.y + b.y, a.z + b.z};
    return ret;
}

inline vec3 vec3_subtract(const vec3 a, const vec3 b) {
    vec3 ret = {a.x - b.x, a.y - b.y, a.z - b.z};
    return ret;
}

inline vec3 vec3_cross(const vec3 a, const vec3 b) {
    vec3 ret = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
    return ret;
}

inline float vec3_dot(const vec3 a, const vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline vec3 vec3_multiply(const vec3 a, const float t) {
    vec3 ret = {t * a.x, t * a.y, t * a.z};
    return ret;
}

inline vec3 vec3_multiply_vectors(const vec3 a, const vec3 b) {
    vec3 ret = {
        .x = a.x * b.x,
        .y = a.y * b.y,
        .z = a.z * b.z
    };
    return ret;
}

inline vec3 vec3_divide(const vec3 a, const float t) {
    return vec3_multiply(a, 1 / t);
}

inline vec3 vec3_negate(const vec3 a) {
    return vec3_multiply(a, -1);
}

inline vec3 vec3_normalize(const vec3 a) {
    return vec3_divide(a, vec3_length(a));
}

inline float vec3_length(const vec3 a) {
    return sqrtf_babylonian(vec3_length_squared(a));
}

inline float vec3_length_squared(const vec3 a) {
    return a.x*a.x + a.y*a.y + a.z*a.z;
}

inline uint32_t vec3_to_pixel_value(const vec3 a, const int samples_per_pixel) {
    float r = a.x;
    float g = a.y;
    float b = a.z;

    float scale = 1.0 / samples_per_pixel;
    r = sqrtf_babylonian(scale * r);
    g = sqrtf_babylonian(scale * g);
    b = sqrtf_babylonian(scale * b);

    int rInt = (int)(256 * clamp(r, 0.0, 0.999));
    int gInt = (int)(256 * clamp(g, 0.0, 0.999));
    int bInt = (int)(256 * clamp(b, 0.0, 0.999));
    return (rInt << 16) | (gInt << 8) | bInt;
}

inline void vec3_print(const char *opening, const vec3 a) {
    printf("%s: (%f, %f, %f)\n", opening, a.x, a.y, a.z);
}

inline vec3 _random() {
    vec3 r = {random_float(), random_float(), random_float()};
    return r;
}

inline vec3 random_bounded(const float min, const float max) {
    vec3 r = {random_float_bounded(min, max), random_float_bounded(min, max), random_float_bounded(min, max)};
    return r;
}

inline vec3 random_in_unit_sphere() {
    while (1) {
        vec3 p = random_bounded(-1, 1);
        if (vec3_length_squared(p) >= 1) continue;
        return p;
    }
}

inline vec3 random_unit_vector() {
    return vec3_normalize(random_in_unit_sphere());
}

inline int less_than_abs(const float x, const float bar) {
    return (x < bar && x > 0) || (x > bar && x < 0);
}

inline float vec3_fabs(const float x) {
    if (x < 0) {
        return -x;
    } else {
        return x;
    }   
}

inline int vec3_near_zero(const vec3 a) {
    const float s = 1e-8;
    return less_than_abs(a.x, s) && less_than_abs(a.y, s) && less_than_abs(a.z, s);
}

inline vec3 vec3_reflect(const vec3 v, const vec3 n) {
    float multi = 2 * vec3_dot(v, n);
    return vec3_subtract(v, vec3_multiply(n, multi));
}

inline float vec3_fmin(const float x, const float y) {
    if (x < y) {
        return x;
    } else return y;
}

inline vec3 vec3_refract(const vec3 uv, const vec3 n, const float etai_over_etat) {
    float cos_theta = vec3_fmin(vec3_dot(vec3_negate(uv), n), 1.0);
    vec3 r_out_perp = vec3_multiply(vec3_add(uv, vec3_multiply(n, cos_theta)), etai_over_etat);
    vec3 r_out_parallel = vec3_multiply(n, -sqrtf_babylonian(vec3_fabs(1.0 - vec3_length_squared(r_out_perp))));
    return vec3_add(r_out_perp, r_out_parallel);
}

inline vec3 random_in_unit_disk(){
    while (1){
        vec3 p = {random_float_bounded(-1, 1), random_float_bounded(-1, 1), 0};
        if (vec3_length_squared(p) >= 1) continue;
        return p;
    }
}

