#if ON_PI

#include "io.h"
#include "fb.h"
#include "rpi.h"
#include "printf.h"
#include "pi-sd.h"
#include "fat32.h"

#else
#include <stdio.h>
#include <time.h>

#endif

#include "ray.h"
#include "sphere.h"
#include "camera.h"
#include "scatter.h"
#include "materials.h"

#define SPHERE_MAX 550
static int sphere_count;
static sphere world[SPHERE_MAX];
static material materials[SPHERE_MAX];
static char ppm_filename[] = "IMAGE.PPM";

#define P3_HEADER_OFFSET 4

const vec3 adaptiveSamplingThreshold = {0.4, 0.3, 0.6};

void makeMaterial(material *m, material_id id, float r, float g, float b, float fuzz) {
    m->id = id;
    vec3 albedo = {r, g, b};
    switch (id)
    {
        case LAMBERTIAN_ID:
            m->data._lambertian.albedo = albedo;
            break;
        
        case METAL_ID:
            m->data._metal.albedo = albedo;
            m->data._metal.fuzz = fuzz;
            break;
        
        case DIELECTRIC_ID:
            m->data._dielectric.ir = r;
    }
}

int scatterMaterials(const material m, const ray r_in, const hit_record rec, vec3 *attenuation_color, ray *scattered) {
  switch (m.id)
  {
    case LAMBERTIAN_ID:
        return lambertian_scatter(m.data._lambertian, r_in, rec, attenuation_color, scattered);
    case METAL_ID:
        return metal_scatter_fuzz(m.data._metal, r_in, rec, attenuation_color, scattered);
    case DIELECTRIC_ID:
        return dielectric_scatter(&r_in, &rec, attenuation_color, scattered, &m.data._dielectric);
    default:
        return 0;
  }
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
        if (scatterMaterials(*rec.material, r, rec, &attenuation, &scattered)) {
            vec3 bounce = ray_color_from_world(spheres, sp_count, scattered, depth-1);
            vec3 color = {attenuation.x * bounce.x, attenuation.y * bounce.y, attenuation.z * bounce.z};
            return color;
        }

        vec3 color = {0, 0, 0};
        return color;
    }

    return background_color(r.direction);
}

void editSphere(sphere* s, vec3 center, float radius, material* mat) {
    s->center   = center;
    s->radius   = radius;
    s->material = mat;
}

void random_scene() {
    unsigned int index = 0;
    makeMaterial(&materials[index], LAMBERTIAN_ID, 0.5, 0.5, 0.5, 0);

    vec3 point0     = {0, -1000, 0};
    sphere mySphere = {point0, 1000, &materials[index]};

    world[index++]  = mySphere;

    point0.x = 4;
    point0.y = 0.2;
    point0.z = 0;

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = random_float();
            vec3 center = {a + 0.9 * random_float(), 0.2, b + 0.9 * random_float()};

            if ( vec3_length(vec3_subtract(center, point0)) > 0.9 ) {
                //material sphere_material;

                if ( choose_mat < 0.8 ) {
                    // diffuse
                    vec3 albedo = vec3_multiply_vectors(_random(), _random());
                    makeMaterial(&materials[index], LAMBERTIAN_ID, albedo.x, albedo.y, albedo.z, 0);
                    editSphere(&mySphere, center, 0.2, &materials[index]);
                    world[index++] = mySphere;
                } else if ( choose_mat < 0.95 ) {
                    // metal 
                    vec3 albedo = _random();
                    float fuzz  = random_float_bounded(0, 0.5);
                    makeMaterial(&materials[index], METAL_ID, albedo.x, albedo.y, albedo.z, fuzz);
                    editSphere(&mySphere, center, 0.2, &materials[index]);
                    world[index++] = mySphere;
                } else {
                    // glass
                    makeMaterial(&materials[index], DIELECTRIC_ID, 1.5, 0, 0, 0);
                    editSphere(&mySphere, center, 0.2, &materials[index]);
                    world[index++] = mySphere;
                }
            }
        }
    }

    vec3 point1 = {0,  1, 0};
    vec3 point2 = {-4, 1, 0};
    vec3 point3 = {4,  1, 0};

    makeMaterial(&materials[index], DIELECTRIC_ID, 1.5, 0, 0, 0);
    editSphere(&mySphere, point1, 1.0, &materials[index]);
    world[index++] = mySphere;

    makeMaterial(&materials[index], LAMBERTIAN_ID, 0.4, 0.2, 0.1, 0);
    editSphere(&mySphere, point2, 1.0, &materials[index]);
    world[index++] = mySphere;

    makeMaterial(&materials[index], METAL_ID, 0.7, 0.6, 0.5, 0.0);
    editSphere(&mySphere, point3, 1.0, &materials[index]);
    world[index++] = mySphere;

    sphere_count = index;
}

float calcIntensity(float a, float b) {
    float max = my_fmax(a, b);
    float min = my_fmin(a, b);
    return (max - min) / (max + min);
}

int sample_is_close(vec3 sample, vec3 current) {
    float rIntensity = calcIntensity(sample.x, current.x);
    float gIntensity = calcIntensity(sample.y, current.y);
    float bIntensity = calcIntensity(sample.z, current.z);

    if (rIntensity < adaptiveSamplingThreshold.x 
        && gIntensity < adaptiveSamplingThreshold.y 
            && bIntensity < adaptiveSamplingThreshold.z) {
        return 1;
    }

    return 0;
}

void custom_scene() {
    int spheresNeeded = 100;
    makeMaterial(&materials[0], LAMBERTIAN_ID, 0.5, 0.5, 0.5, 0);
    for (int i = 1; i < spheresNeeded; i++) {
        float choose_mat = random_float();
        if ( choose_mat < 0.8 ) {
            // diffuse
            vec3 albedo = vec3_multiply_vectors(_random(), _random());
            makeMaterial(&materials[i], LAMBERTIAN_ID, albedo.x, albedo.y, albedo.z, 0);
        } else if ( choose_mat < 0.95 ) {
            // metal 
            vec3 albedo = _random();
            float fuzz  = random_float_bounded(0, 0.5);
            makeMaterial(&materials[i], METAL_ID, albedo.x, albedo.y, albedo.z, fuzz);
        } else {
            // glass
            makeMaterial(&materials[i], DIELECTRIC_ID, 1.5, 0, 0, 0);
        }
    }

    // GROUND
    int index = 0;
    vec3 point0     = {0, -1000, 0};
    sphere mySphere = {point0, 1000, &materials[0]};
    world[index++]  = mySphere;
    float r = 0.25;

// z; positive is left, negative is right
//y is up/down
    // 1
    const float oneXOff = 0;
    const float oneZOff = 3;
    editSphere(&mySphere, (vec3){oneXOff, 2, oneZOff}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){oneXOff, 1.5, oneZOff}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){oneXOff, 1, oneZOff}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){oneXOff, 0.5, oneZOff}, r, &materials[index]);
    world[index++] = mySphere;

    // 4
    
    //long vertical
    const float fourXOff = 0;
    const float fourZOff = 1;
    editSphere(&mySphere, (vec3){fourXOff, 2, fourZOff}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){fourXOff, 1.5, fourZOff}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){fourXOff, 1, fourZOff}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){fourXOff, 0.5, fourZOff}, r, &materials[index]);
    world[index++] = mySphere;

    // // short vertical
    editSphere(&mySphere, (vec3){fourXOff, 2, fourZOff+1}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){fourXOff, 1.5, fourZOff+1}, r, &materials[index]);
    world[index++] = mySphere;

    // connector
    editSphere(&mySphere, (vec3){fourXOff, 1.5, fourZOff+0.5}, r, &materials[index]);
    world[index++] = mySphere;

    // 0

    // left
    const float zeroXLeft = 0;
    const float zeroZLeft = 0;
    editSphere(&mySphere, (vec3){zeroXLeft, 2, zeroZLeft}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){zeroXLeft, 1.5, zeroZLeft}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){zeroXLeft, 1, zeroZLeft}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){zeroXLeft, 0.5, zeroZLeft}, r, &materials[index]);
    world[index++] = mySphere;

    // right
    const float zeroXRight = 0;
    const float zeroZRight = -1;
    editSphere(&mySphere, (vec3){zeroXLeft, 2, zeroZRight}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){zeroXLeft, 1.5, zeroZRight}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){zeroXLeft, 1, zeroZRight}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){zeroXLeft, 0.5, zeroZRight}, r, &materials[index]);
    world[index++] = mySphere;

    // connectors
    const float zeroZMid = -0.5;
    editSphere(&mySphere, (vec3){0, 2, zeroZMid}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){0, 0.5, zeroZMid}, r, &materials[index]);
    world[index++] = mySphere;


    // E
    const float eZLeft = -2;
    editSphere(&mySphere, (vec3){0, 2, eZLeft}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){0, 1.5, eZLeft}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){0, 1, eZLeft}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){0, 0.5, eZLeft}, r, &materials[index]);
    world[index++] = mySphere;

//top
    editSphere(&mySphere, (vec3){0, 2, eZLeft-0.5}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){0, 2, eZLeft-1}, r, &materials[index]);
    world[index++] = mySphere;

//middle
    editSphere(&mySphere, (vec3){0, 1.25, eZLeft-0.5}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){0, 1.25, eZLeft-1}, r, &materials[index]);
    world[index++] = mySphere;

    //bottom
    editSphere(&mySphere, (vec3){0, 0.5, eZLeft-0.5}, r, &materials[index]);
    world[index++] = mySphere;
    editSphere(&mySphere, (vec3){0, 0.5, eZLeft-1}, r, &materials[index]);
    world[index++] = mySphere;

    sphere_count = index;
}

void drawRays() {
    // Image
    const int max_depth = 50;
    const int samples_per_pixel = 100;
    const int min_samples_per_pixel = 1;
    const float aspect_ratio = 3.0 / 2.0;

    const int image_width  = 600;
    const int image_height = (int)(image_width / aspect_ratio);

    #if ON_PI
    uart_init();
    fb_init();

    #else
    FILE* fp;
    fp = fopen ("imagetest.ppm", "w");
    fprintf(fp, "P3 \n");
    fprintf(fp, "%d %d\n", image_width, image_height);
    fprintf(fp, "255\n");
    
    #endif

    // World
    //random_scene();
    custom_scene();
    
    // Camera
    vec3 lookfrom = {13, 2, 3};
    vec3 lookat   = {0, 0, 0};
    vec3 vup      = {0, 1, 0};

    float aperture      = 0.1;
    float dist_to_focus = 10.0;

    camera cam;
    camera_init(&cam, lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

    // Render
    printf("image_width = %i image height = %i\n", image_width, image_height);

    for (int j = image_height-1; j >= 0; j--) {
        for (int i = 0; i < image_width; i++) {
            vec3 pixel_color = {0,0,0};
            int actual_sample_count = samples_per_pixel;

            for (int s = 0; s < samples_per_pixel; s++) {
                float u = (float)(i + random_float()) / (image_width  - 1);
                float v = (float)(j + random_float()) / (image_height - 1); 

                ray r = get_ray(cam, u, v);
                vec3 new_color = ray_color_from_world(world, sphere_count, r, max_depth);
                //if we are getting colors within the threshold, break out (adaptive sampling)
                if (s >= min_samples_per_pixel && sample_is_close(new_color, pixel_color)) {
                    pixel_color = vec3_add(pixel_color, new_color);
                    actual_sample_count = s + 1;
                    break;
                }

                pixel_color = vec3_add(pixel_color, new_color);
            }

            uint32_t pixel_value = vec3_to_pixel_value(pixel_color, actual_sample_count);
            #if ON_PI
            drawPixel(i, image_height-j, pixel_value);
        }
            #else
            fprintf(fp, "%d %d %d ", (pixel_value >> 16) & 0xFF, (pixel_value >> 8) & 0xFF, pixel_value & 0xFF);  
        }
        fprintf(fp, "\n");
        #endif
    }

    #if ON_PI
        while (1) {};
    #endif
}

#if ON_PI
int readInt(pi_file_t *file, int *idx) {
    int cur = *idx;
    int ret = 0;
    int hasRead = 0;
    while (cur < file->n_data) {
        switch (file->data[cur])
        {
            case ' ':
            case '\n':
                *idx = cur + 1;
                if (hasRead)
                    return ret;
                break;
            
            default:
                ret = (10 * ret) + (int)(file->data[cur] - '0');
                hasRead = 1;
                break;
        }
        cur++;
    }

    *idx = cur;
    return ret;
}


void displayPPM(char *name) {
    uart_init();
    kmalloc_init();
    pi_sd_init();
    fb_init();

    printk("Reading the MBR.\n");
    mbr_t *mbr = mbr_read();

    printk("Loading the first partition.\n");
    mbr_partition_ent_t partition;
    memcpy(&partition, mbr->part_tab1, sizeof(mbr_partition_ent_t));
    assert(mbr_part_is_fat32(partition.part_type));

    printk("Loading the FAT.\n");
    fat32_fs_t fs = fat32_mk(&partition);

    printk("Loading the root directory.\n");
    pi_dirent_t root = fat32_get_root(&fs);

    pi_directory_t files = fat32_readdir(&fs, &root);
    printk("Got %d files.\n", files.ndirents);
    for (int i = 0; i < files.ndirents; i++) {
        if (files.dirents[i].is_dir_p) {
        printk("\tD: %s (cluster %d)\n", files.dirents[i].name, files.dirents[i].cluster_id);
        } else {
        printk("\tF: %s (cluster %d; %d bytes)\n", files.dirents[i].name, files.dirents[i].cluster_id, files.dirents[i].nbytes);
        }
    }

    pi_dirent_t *ppm = fat32_stat(&fs, &root, name);
    demand(ppm, "ppm file not found!\n");

    printk("Reading ppm file.\n");
    pi_file_t *file = fat32_read(&fs, &root, name);
    int idx = 0 + P3_HEADER_OFFSET;
    int imageWidth = readInt(file, &idx);
    int imageHeight = readInt(file, &idx);
    int imageDepth = readInt(file, &idx);
    printf("Read dimensions as width=%d, height=%d, depth=%d from ppm file (idx=%d).\n", imageWidth, imageHeight, idx);

    int pixelCount = 0;
    while (idx < file->n_data) {
        int r = readInt(file, &idx);
        int g = readInt(file, &idx);
        int b = readInt(file, &idx);

        uint32_t pixel_value = (r << 16) | (g << 8) | b;

        int x = pixelCount % imageWidth;
        int y = pixelCount / imageWidth;
        drawPixel(x, y, pixel_value);
        pixelCount++;
        if (pixelCount < 11) {
            printf("Drawing (%d, %d, %d) at location (%d, %d)\n", r, g, b, x, y);
        }
        //printf("Drawing (%d, %d, %d) at location (%d, %d)\n", r, g, b, x, y);
    }

    while(1) {};
}
#endif

#if ON_PI
void notmain() {
    //drawRays();
    displayPPM(ppm_filename);
}

#else
int main() {  
    time_t t;
   
    /* Intializes random number generator */
    srand((unsigned) time(&t));
    drawRays();
    return 0;
}

#endif
