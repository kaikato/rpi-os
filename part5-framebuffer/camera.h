#ifndef CAMERA_H
#define CAMERA_H

#include "vec3.h"
#include "ray.h"

typedef struct camera {
  vec3 origin;
  vec3 lower_left_corner;
  vec3 horizontal;
  vec3 vertical;
  vec3 u;
  vec3 v;
  vec3 w;
  float lens_radius;
} camera;

void camera_init(camera *cam, vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect_ratio, float aperture, float focus_dist) {
  float theta = degrees_to_radians(vfov);
  float h = my_tan(theta/2);
  float viewport_height = 2.0 * h;
  float viewport_width = aspect_ratio * viewport_height;

    //float aspect_ratio = 16.0 / 9.0;
    //float viewport_height = 2.0;
    //float viewport_width = aspect_ratio * viewport_height;
  float focal_length = 1.0;
  cam->w = vec3_normalize(vec3_subtract(lookfrom, lookat));
  cam->u = vec3_normalize(vec3_cross(vup, cam->w));
  cam->v = vec3_cross(cam->w, cam->u);
  cam->origin = lookfrom;
  cam->horizontal = vec3_multiply(cam->u, viewport_width*focus_dist);
  cam->vertical = vec3_multiply(cam->v, viewport_height*focus_dist);
  //cam->lower_left_corner = vec3_subtract(cam->origin, vec3_subtract(vec3_divide(cam->horizontal, 2), vec3_subtract(vec3_negate(vec3_divide(cam->vertical, 2)), vec3_negate(w))));
  cam->lower_left_corner = vec3_add(cam->origin, vec3_add(vec3_negate(vec3_divide(cam->horizontal, 2)), vec3_add(vec3_negate(vec3_divide(cam->vertical, 2)), vec3_negate(vec3_multiply(cam->w, focus_dist)))));
  cam->lens_radius = aperture/2;

/*
  cam->origin = (vec3){0, 0, 0};
  cam->horizontal = (vec3){viewport_width, 0.0, 0.0};
  cam->vertical = (vec3){0.0, viewport_height, 0.0};
  vec3 foc = {0, 0, focal_length};
  cam->lower_left_corner = vec3_add(cam->origin, vec3_add(vec3_negate(vec3_divide(cam->horizontal, 2)), vec3_add(vec3_negate(vec3_divide(cam->vertical, 2)), vec3_negate(foc))));
*/
}

ray get_ray(camera c, float u, float v) {
  vec3 rd = vec3_multiply(random_in_unit_disk(), c.lens_radius);
  vec3 offset = vec3_add(vec3_multiply(c.u, rd.x), vec3_multiply(c.v, rd.y));
  vec3 newOrigin = vec3_add(c.origin, offset);
  vec3 direction = vec3_add(c.lower_left_corner, vec3_add(vec3_multiply(c.horizontal, u), vec3_add(vec3_multiply(c.vertical, v), vec3_add(vec3_negate(c.origin), vec3_negate(offset)))));
  ray r = {newOrigin, direction};
  return r;
}

#endif