#include "scatter.h"

int scatter(const material m, const ray r_in, const hit_record rec, vec3 *attenuation_color, ray *scattered) {
  switch (m.id)
  {
    case LAMBERTIAN_ID:
        return lambertian_scatter(m.data._lambertian, r_in, rec, attenuation_color, scattered);
    case METAL_ID:
        return metal_scatter(m.data._metal, r_in, rec, attenuation_color, scattered);
    case DIELECTRIC_ID:
        return dielectric_scatter(&r_in, &rec, attenuation_color, scattered, &m.data._dielectric);
    default:
        return 0;
  }
}

int lambertian_scatter(const lambertian la, const ray r_in, const hit_record rec, vec3 *attenuation_color, ray *scattered) {
  vec3 scatter_dir = vec3_add(rec.normal, random_unit_vector());
  ray s = {rec.p, scatter_dir};

  // Catch degenerate scatter direction
  if (vec3_near_zero(scatter_dir))
    scatter_dir = rec.normal;
    
  *scattered = s;
  *attenuation_color = la.albedo;
  return 1;
}

int metal_scatter_fuzz(const metal me, const ray r_in, const hit_record rec, vec3 *attenuation_color, ray *scattered) {
  int f = me.fuzz < 1 ? me.fuzz : 1;
  vec3 reflected = vec3_reflect( vec3_normalize( r_in.direction ), rec.normal );

  *scattered = (ray){rec.p, vec3_add( reflected, vec3_multiply( random_in_unit_sphere(), f ) )};
  *attenuation_color = me.albedo;

  if (vec3_dot( scattered->direction, rec.normal ) > 0)
    return 1;
  return 0;
}

int metal_scatter(const metal me, const ray r_in, const hit_record rec, vec3 *attenuation_color, ray *scattered) {
  vec3 reflected = vec3_reflect(vec3_normalize(r_in.direction), rec.normal);

  ray s = {rec.p, reflected};
  *scattered = s;
  *attenuation_color = me.albedo;
  return (vec3_dot(scattered->direction, rec.normal) > 0);
}

float reflectance(float cosine, float ref_idx) {
    // Use Schlick's approximation for reflectance.
    float r0 = (1.0 - ref_idx) / (1.0 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * (1.0 - cosine)*(1.0 - cosine)*(1.0 - cosine)*(1.0 - cosine)*(1.0 - cosine);
}

int dielectric_scatter(const ray* r_in, const hit_record* rec, vec3* attenuation, ray* scattered, const dielectric* mat)
{
    *attenuation = (vec3){1.0, 1.0, 1.0};
    float refraction_ratio = rec->front_face ? (1.0 / mat->ir) : mat->ir;

    vec3 unit_direction = vec3_normalize(r_in->direction);
    float cos_theta = vec3_fmin(vec3_dot(vec3_negate(unit_direction), rec->normal), 1.0);
    float sin_theta = sqrtf_babylonian(1.0 - cos_theta * cos_theta);

    int cannot_refract = 0;
    if (refraction_ratio * sin_theta > 1.0){
      cannot_refract = 1;
    }
    vec3 direction;
    if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_float()){
      direction = vec3_reflect(unit_direction, rec->normal);
    } else {
      direction = vec3_refract(unit_direction, rec->normal, refraction_ratio);
    }

    ray r = {rec->p, direction};
    *scattered = r;
    return 1;
}


