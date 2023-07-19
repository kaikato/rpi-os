#include "io.h"
#include "fb.h"
#include "rpi.h"
#include "ray.h"
#include "printf.h"
#include "sphere.h"
#include "camera.h"
#include "scatter.h"
#include "materials.h"


static int print_info = 0;

void makeMaterial(material *m, material_id id, float r, float g, float b) {
    m->id = id;
    vec3 albedo = {r, g, b};
    switch (id)
    {
        case LAMBERTIAN_ID:
            m->data._lambertian.albedo = albedo;
            break;
        
        case METAL_ID:
            m->data._metal.albedo = albedo;
            break;
        
        case DIELECTRIC_ID:
            m->data._dielectric.ir = r;
    }
}

void makeSphere(sphere *s, float x, float y, float z, float radius, material *m) {
    vec3 center = {x, y, z};
    s->center = center;
    s->radius = radius;
    s->material = m;
}

vec3 background_color(vec3 ray_direction) {
    vec3 unit_direction = vec3_normalize(ray_direction);
    float t = 0.5*(unit_direction.y + 1.0);
    vec3 maxColor = {1.0, 1.0, 1.0};
    vec3 minColor = {0.5, 0.7, 1.0};
    // (1-t)*maxColor + t*minColor
    return vec3_add(vec3_multiply(maxColor, 1.0-t), vec3_multiply(minColor, t));
}

vec3 ray_color_from_world(sphere *spheres, int sp_count, ray r, int depth) {
    hit_record rec;
    if (depth <= 0){
        vec3 color = {0, 0, 0};
        return color;
    }

    if (spheres_hit(spheres, sp_count, r, 0.001, infinity, &rec)) {
        ray scattered;
        vec3 attenuation;
        if (scatter(*rec.material, r, rec, &attenuation, &scattered)) {
            vec3 bounce = ray_color_from_world(spheres, sp_count, scattered, depth-1);
            vec3 color = {attenuation.x * bounce.x, attenuation.y * bounce.y, attenuation.z * bounce.z};
            return color;
        }

        vec3 color = {0, 0, 0};
        return color;
    }

    return background_color(r.direction);
}

vec3 ray_color_part7(ray* r, sphere* s, int sp_count) {
    hit_record rec;

    if (spheres_hit(s, sp_count, *r, 0, infinity, &rec)) {
        vec3 color = {1,1,1};
        return vec3_multiply(vec3_add(rec.normal, color), (float)0.5);
    }

    vec3 minColor = {0.5, 0.7, 1.0};
    vec3 maxColor = {1.0, 1.0, 1.0};
    vec3 unit_direction = vec3_normalize(r->direction);

    float t = 0.5 * (unit_direction.y + 1.0);
    return vec3_add(vec3_multiply(maxColor, 1.0-t), vec3_multiply(minColor, t));
}

void drawRays() {
    uart_init();
    fb_init();

    // Image
    const float aspect_ratio = 16.0 / 9.0;
    const int image_width = 400;
    const int image_height = (int)(image_width / aspect_ratio);
    const int max_depth = 50;
    const int samples_per_pixel = 5;

    //printf("the frame buffer size = %i\n", FRAMEBUFFER_SIZE);
    //char imageArray[FRAMEBUFFER_SIZE];

    // World up to 10
    
    int sphere_count = 4;
    sphere world[sphere_count];
    material materials[sphere_count];

    //makeMaterial(&materials[0], LAMBERTIAN_ID, 0.7, 0.3, 0.3); // center
    makeMaterial(&materials[0], LAMBERTIAN_ID, 0.1, 0.2, 0.5); // center
    //makeMaterial(&materials[0], DIELECTRIC_ID, 1.5, 0, 0); // center
    makeMaterial(&materials[1], LAMBERTIAN_ID, 0.8, 0.8, 0.0); // ground 
    //makeMaterial(&materials[2], METAL_ID, 0.8, 0.8, 0.8); // left 
    makeMaterial(&materials[2], DIELECTRIC_ID, 1.5, 0, 0); // left 
    makeMaterial(&materials[3], METAL_ID, 0.8, 0.6, 0.2); // right

    makeSphere(&world[0], 0.0, 0, -1, 0.5, &materials[0]);
    makeSphere(&world[1], 0.0, -100.5, -1, 100, &materials[1]);
    //makeSphere(&world[2], -1, 0, -1, 0.5, &materials[2]);
    makeSphere(&world[2], -1, 0, -1, -0.4, &materials[2]); // the negative value makes this a hollow glass
    makeSphere(&world[3], 1.0, 0, -1.0, 0.5, &materials[3]);
    

    // World for 11
    /*
    int sphere_count = 2;
    sphere world[sphere_count];
    material materials[sphere_count];
    float R = my_cos(pi/4);
    
    makeMaterial(&materials[0], LAMBERTIAN_ID, 0, 0, 1);
    makeMaterial(&materials[1], LAMBERTIAN_ID, 1, 0, 0);

    makeSphere(&world[0], -R, 0, -1, R, &materials[0]);
    makeSphere(&world[1], R, 0, -1, R, &materials[1]);
    */

    // Camera
    camera cam;
    vec3 lookfrom = {-2, 2, 1};
    vec3 lookat = {0, 0, -1};
    vec3 vup = {0, 1, 0};
    float dist_to_focus = vec3_length(vec3_subtract(lookfrom, lookat));
    float aperture = 2.0;
    camera_init(&cam, lookfrom, lookat, vup, 20.0, aspect_ratio, aperture, dist_to_focus);

    float viewport_height = 2.0;
    float viewport_width = aspect_ratio * viewport_height;
    float focal_length = 1.0;
    printf("Height: %f, width: %f, focal: %f\n", viewport_height, viewport_width, focal_length);

    vec3 origin = {0, 0, 0};
    vec3 horizontal = {viewport_width, 0, 0};
    vec3 vertical = {0, viewport_height, 0};

    // lower_left_corner = origin - horizontal/2 - vertical/2 - focal
    vec3 focal = {0, 0, focal_length};
    vec3 lower_left_corner = vec3_subtract(origin, vec3_subtract(vec3_divide(horizontal, 2), vec3_subtract(vec3_negate(vec3_divide(vertical, 2)), focal)));
    vec3_print("Lower left corner at", lower_left_corner);


    // Render

    for (int j = 0; j < image_height; j++) {
        for (int i = 0; i < image_width; i++) {
            vec3 pixel_color = {0,0,0};
            for (int s = 0; s < samples_per_pixel; s++) {
                float u = (float)(i + random_float()) / (image_width  - 1);
                float v = (float)(j + random_float()) / (image_height - 1); 

                ray r = get_ray(cam, u, v);
                pixel_color = vec3_add(pixel_color, ray_color_from_world(world, sphere_count, r, max_depth));
            }

            uint32_t pixel_value = vec3_to_pixel_value(pixel_color, samples_per_pixel);

            if (print_info) {
                //printf("u=%f; v=%f\n", u, v);
                vec3_print("Ray color", lower_left_corner);
                printf("Pixel value: %x\n", pixel_value);
            }

            drawPixel(i, image_height-j, pixel_value);
            //drawPixelArray(imageArray, i, image_height-j, pixel_value);
        }
            //drawImage(imageArray);
    }
    
    while (1) {};
}   

void draw() {
    uart_init();
    fb_init();

//    drawRect(150,150,400,400,0x03,0);
//    drawRect(300,300,350,350,0x2e,1);
//
//    drawCircle(960,540,250,0x0e,0);
//    drawCircle(960,540,50,0x13,1);
//
//    drawPixel(250,250,0x0e);
//
//    drawChar('O',500,500,0x05);
//    drawString(100,100,"Hello world!",0x0f);
//
//    drawLine(100,500,350,700,0x0c);
//
//    int width = 1920;
//    int height = 1080;
//    int pixel_value = 0;
//    while (1) {
//        for( int y = 0; y < height; y++ )
//        {
//            int line_offset = y * width;
//
//            for( int x = 0; x < width; x++ )
//            {
//                drawPixel(x, y, pixel_value);
//            }
//        }
//    }
//    #include "img.h"

//    uint32_t index = 0;
//    for ( int y = 0; y < imgHeight; y++ ) {
//        
//        for ( int x = 0; x < imgWidth; x++ ) {
//            drawPixel(x, y, array[index++]);
//        }
//    }
//
//    while(1);
}


void notmain() {
    drawRays();
}
