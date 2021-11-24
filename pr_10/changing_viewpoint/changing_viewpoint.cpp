#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "geometry.h"
#include <random>
bool reflection_flag = true;
bool refraction_flag = true;
bool glossy_flag = false;
bool shadow_flag = true;
bool specular_flag = true;
bool checker_board = false;

// spot light
//Vec3f BACKGROUND_COLOR = Vec3f(0.2, 0.4, 0.7);
Vec3f BACKGROUND_COLOR = Vec3f(0.8, 0.2, 0.5);

bool spot_light_flag = false;
float SPOT_LIGHT_RADIOUS = 10.;

const int   width    = 300;
const int   height   = 300;
//const float fov      = M_PI/3.;

int ch;
std::string saved_image_path = "/Users/mehdigorjian/Downloads/csce647/pr_08/fresnel/fresnel.ppm";
unsigned char *img, *img_array = new unsigned char[width * height * 3], *rendering = new unsigned char[width * height * 3];

// camera and view parameters
Vec3f camera_origin = Vec3f(0, 0, 0);
float param = .5;
float fov_param = 3.;

struct Light {
    Light(const Vec3f &p, const float i) : position(p), intensity(i) {}
    Vec3f position;
    float intensity;
};

struct Material {
    Material(const float r, const Vec4f &a, const Vec3f &color, const float spec) : refractive_index(r), albedo(a), diffuse_color(color), specular_exponent(spec) {}
    Material() : refractive_index(1), albedo(1,0,0,0), diffuse_color(), specular_exponent() {}
    float refractive_index;
    Vec4f albedo;
    Vec3f diffuse_color;
    float specular_exponent;
};

struct Sphere {
    Vec3f center;
    float radius;
    Material material;
    
    Sphere(const Vec3f &c, const float r, const Material &m) : center(c), radius(r), material(m) {}
    
    bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0) const {
        Vec3f L = center - orig;
        float tca = L*dir;
        float d2 = L*L - tca*tca;
        if (d2 > radius*radius) return false;
        float thc = sqrtf(radius*radius - d2);
        t0       = tca - thc;
        float t1 = tca + thc;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        return true;
    }
};

Vec3f reflect(const Vec3f &I, const Vec3f &N) {
    return I - N*2.f*(I*N);
}

Vec3f refract(const Vec3f &I, const Vec3f &N, const float eta_t, const float eta_i=1.f) { // Snell's law
    float cosi = - std::max(-1.f, std::min(1.f, I*N));
    if (cosi<0) return refract(I, -N, eta_i, eta_t); // if the ray comes from the inside the object, swap the air and the media
    float eta = eta_i / eta_t;
    float k = 1 - eta*eta*(1 - cosi*cosi);
    return k<0 ? Vec3f(1,0,0) : I*eta + N*(eta*cosi - sqrtf(k)); // k<0 = total reflection, no ray to refract. I refract it anyways, this has no physical meaning
}

bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, Vec3f &hit, Vec3f &N, Material &material) {
    float spheres_dist = std::numeric_limits<float>::max();
    for (size_t i=0; i < spheres.size(); i++) {
        float dist_i;
        if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
            spheres_dist = dist_i;
            hit = orig + dir*dist_i;
            N = (hit - spheres[i].center).normalize();
            material = spheres[i].material;
        }
    }
    if (checker_board)
    {
        float checkerboard_dist = std::numeric_limits<float>::max();
        if (fabs(dir.y)>1e-3)  {
            float d = -(orig.y+4)/dir.y; // the checkerboard plane has equation y = -4
            Vec3f pt = orig + dir*d;
            if (d>0 && fabs(pt.x)<10 && pt.z<-10 && pt.z>-30 && d<spheres_dist) {
                checkerboard_dist = d;
                hit = pt;
                N = Vec3f(0,1,0);
                material.diffuse_color = (int(.5*hit.x+1000) + int(.5*hit.z)) & 1 ? Vec3f(.3, .3, .3) : Vec3f(.3, .2, .1);
            }
        }
        return std::min(spheres_dist, checkerboard_dist)<1000;
    }
    return spheres_dist < 1000;
}

Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, const std::vector<Light> &lights, size_t depth = 0)
{
    Vec3f point, N;
    Material material;
    
    if (depth > 4 || !scene_intersect(orig, dir, spheres, point, N, material))
    {
        return BACKGROUND_COLOR; // background color
    }
    Vec3f reflect_color;
    Vec3f refract_color;
    if (reflection_flag)
    {
        Vec3f reflect_dir = reflect(dir, N).normalize();
        Vec3f reflect_orig = reflect_dir * N < 0 ? point - N * 1e-3 : point + N * 1e-3;
        reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);
    }
    else
    {
        reflect_color = Vec3f(0, 0, 0);
    }
    
    if (refraction_flag)
    {
        Vec3f refract_dir = refract(dir, N, material.refractive_index).normalize();
        
        Vec3f refract_orig = refract_dir * N < 0 ? point - N * 1e-3 : point + N * 1e-3;
        refract_color = cast_ray(refract_orig, refract_dir, spheres, lights, depth + 1);
        
    }
    else
    {
        refract_color = Vec3f(0, 0, 0);
    }
    
    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (size_t i = 0; i < lights.size(); i++)
    {
        Vec3f light_dir = (lights[i].position - point).normalize();
        float light_distance = (lights[i].position - point).norm();
        
        if (shadow_flag)
        {
            Vec3f shadow_orig = light_dir * N < 0 ? point - N * 1e-3 : point + N * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance)
                continue;
        }
        diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir * N);
        
        if (specular_flag)
        {
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, N) * dir), material.specular_exponent) * lights[i].intensity;
        }
        if (glossy_flag)
        {
            srand( (unsigned)time( NULL ) );
            float rd = (float) rand()/RAND_MAX;
            
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, N) * dir), material.specular_exponent) * lights[i].intensity * rd;
        }
        
        if (spot_light_flag)
        {
            Vec3f light_to_center = (lights[i].position - spheres[i].center).normalize();
            float t = light_to_center * light_dir;
            
            if (t - cos(SPOT_LIGHT_RADIOUS * M_PI / 180) < 0)
            {
                // std::cout << "yes" << std::endl;
                material.diffuse_color = Vec3f(0, 0, 0);
            }
        }
    }
    return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + Vec3f(1., 1., 1.) * specular_light_intensity * material.albedo[1] + reflect_color * material.albedo[2] + refract_color * material.albedo[3];
}

void render(float count, const std::vector<Sphere> &spheres, const std::vector<Light> &lights)
{
    const float fov = M_PI / fov_param;
    std::vector<Vec3f> framebuffer(width * height);
    
    //#pragma omp parallel for
    for (size_t j = 0; j < height; j++)
    {
        for (size_t i = 0; i < width; i++)
        {
            float dir_x = (i + 0.5) - width / 2. + count;
            float dir_y = -(j + 0.5) + height / 2. + count;
            float dir_z = -height / (2. * tan(fov / 2.)) + count;
            Vec3f moving_cam(count, count, count);
            
            framebuffer[i + j * width] = cast_ray(camera_origin + moving_cam, Vec3f(dir_x, dir_y, dir_z).normalize(), spheres, lights);
        }
    }
    std::string fileName = "./out" + std::to_string(count) + ".ppm";
    std::ofstream ofs;
    //    ofs.open("./out.ppm", std::ios::binary);
    ofs.open(fileName, std::ios::binary);
    ofs << "P6\n"
    << width << " " << height << "\n255\n";
    for (size_t i = 0; i < height * width; ++i)
    {
        Vec3f &c = framebuffer[i];
        float max = std::max(c[0], std::max(c[1], c[2]));
        if (max > 1)
            c = c * (1. / max);
        for (size_t j = 0; j < 3; j++)
        {
            ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
        }
    }
    ofs.close();
    std::cout << "Render completed!" << std::endl;
}


int main() {
    
    std::vector<Light>  lights;
    lights.push_back(Light(Vec3f(-20, 20,  20), 1.5));
    lights.push_back(Light(Vec3f( 30, 50, -25), 1.8));
    lights.push_back(Light(Vec3f( 30, 20,  30), 1.7));
    
    Material      ivory(1.0, Vec4f(0.6,  0.3, 0.1, 0.0), Vec3f(0.4, 0.4, 0.3),   50.);
    Material      glass(1.5, Vec4f(0.3,  0.5, 0.1, 0.4), Vec3f(0.6, 0.7, 0.8),  850.);
    Material red_rubber(1.0, Vec4f(0.9,  0.1, 0.0, 0.0), Vec3f(0.3, 0.1, 0.1),   10.);
    Material     mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.0, 1.0, 1.0), 1425.);
    
    //    PART 3 (MOVING CAMERA)
    for (float i = 0; i < 10; i+=.5)
    {
        std::vector<Sphere> spheres;
        spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 3, glass));
        spheres.push_back(Sphere(Vec3f( 2.5, -.5, -18), 3, red_rubber));
        spheres.push_back(Sphere(Vec3f( 4, 6, -18), 3.5, mirror));
        
        render(i, spheres, lights);
    }
    
    std::cout << "Processing completed!" << std::endl;
    return 0;
}




