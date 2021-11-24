#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
//#include "geometry.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
using namespace cv;
#include <OpenGL/gl.h>
#include <OpenGl/glu.h>
#include <GLUT/glut.h>

bool reflection_flag = false;
bool refraction_flag = false;
bool shadow_flag = false;
bool specular_flag = false;
bool checker_board = false;

// spot light
Vec3f BACKGROUND_COLOR = Vec3f(0, 0, 0);
//Vec3f BACKGROUND_COLOR = Vec3f(0.8, 0.2, 0.5);

bool spot_light_flag = false;
float SPOT_LIGHT_RADIOUS = 10.;

const int   width    = 300;
const int   height   = 300;

int ch;
std::string saved_image_path = "/Users/mehdigorjian/Downloads/csce647/pr_08/polygon_reflection.ppm";
unsigned char *img, *img_array = new unsigned char[width * height * 3], *rendering = new unsigned char[width * height * 3];
//TETRAHEDRON
//eye setting
Vec3f Pe(0, 125, 3); // tetrahedron
//camera setting
Vec3f Vup(1, 1, 1); // tetrahedron
Vec3f V_view(0, -1, 0.5); // tetrahedron
float d = 100, sx = 200; // tetrahedron
Vec3f l(1, 0, -1); // tetrahedron
float sy = sx * height / width;

//camera normals
Vec3f V0 = V_view.cross(Vup);
Vec3f n0 = normalize(V0);
Vec3f n2 = normalize(V_view);
Vec3f n1 = n0.cross(n2);

//camera center
Vec3f P_Cam = Pe + d * n2;
//camera bottom-left coordinate
Vec3f P00 = P_Cam - (n0 * (sx / 2)) - (n1 * (sy / 2));

// camera and view parameters
Vec3f camera_origin = Vec3f(0, 0, 0);
float param = .5;
float fov_param = 3.;

float lenght_vector(Vec3f v)
{
    return sqrt(pow(v[0], 2) + pow(v[1], 2) + pow(v[2], 2));
}

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
 
struct Triangle {
    Vec3f tp0, tp1, tp2;
    Vec3f tv0 = tp0 - tp2;
    Vec3f tv1 = tp1 - tp0;
    Vec3f a = tv0.cross(tv1) * 0.5;
    Vec3f tn = a * 1 / lenght_vector(a);
    Material material;
  
    Triangle(Vec3f &v0, Vec3f &v1, Vec3f &v2, Material mat): tp0(v0), tp1(v1), tp2(v2), material(mat) {}
    
        bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t_hit) const
    {
        float num = tn.dot(tp1 - orig);
        float den = tn.dot(dir);

        if (num < 0 && den < 0)
        {
            t_hit = num / den;

            Vec3f P_hit = orig + dir * t_hit;
            
            Vec3f ta0 = (P_hit - tp2).cross(tp1 - P_hit);
            Vec3f ta1 = (P_hit - tp0).cross(tp2 - P_hit);
            Vec3f ta2 = (P_hit - tp1).cross(tp0 - P_hit);
            
            ta0 = normalize(ta0);
            ta1 = normalize(ta1);
            ta2 = normalize(ta2);
            std::cout << ta0 <<  "   " << ta1 <<  "   " << ta2 << std::endl;

            float s = tn.dot(ta1) / lenght_vector(a);
            float t = tn.dot(ta2) / lenght_vector(a);
            float u = tn.dot(ta0) / lenght_vector(a);
            
            std::cout << s <<  "   " << t <<  "   " << u << std::endl;

            if((s > 0 && s < 1) && (t > 0 && t < 1) && (u > 0 && u < 1))
            {
                return true;
            }
        }
        return false;
    }
};

float max(float a, float b) {return a > b ? a : b;}
float min(float a, float b) {return a > b ? b : a;}

void fresnel(const Vec3f &I, const Vec3f &N, const float &ior, float &kr)
{
    float cosi = max(-1, min(1, (I.dot(N))));
    float etai = 1, etat = ior;
    if (cosi > 0) { std::swap(etai, etat); }
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
    if (sint >= 1) {
        kr = 1;
    }
    else {
        float cost = sqrtf(std::max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        kr = (Rs * Rs + Rp * Rp) / 2;
    }
}

Vec3f reflect(const Vec3f &I, const Vec3f &N) {
    return I - N*2.f*(I*N);
}

Vec3f refract(const Vec3f &I, const Vec3f &N, const float eta_t, const float eta_i=1.f) {
    float cosi = - std::max(-1.f, std::min(1.f, I.dot(N)));
    if (cosi<0) return refract(I, -N, eta_i, eta_t);
    float eta = eta_i / eta_t;
    float k = 1 - eta*eta*(1 - cosi*cosi);
    return k<0 ? Vec3f(1,0,0) : I*eta + N*(eta*cosi - sqrtf(k));
}

bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Triangle> &spheres, Vec3f &hit, Vec3f &N, Material &material) {
    float spheres_dist = std::numeric_limits<float>::max();
    for (size_t i=0; i < spheres.size(); i++) {
        float dist_i;
     
        if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
            spheres_dist = dist_i;
            hit = orig + dir*dist_i;
            N = spheres[i].tn;
            material = spheres[i].material;
        }
    }
    return spheres_dist < 1000;
}

Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Triangle> &spheres, const std::vector<Light> &lights, size_t depth = 0)
{
    Vec3f point, N;
    Material material;
    
    if (depth > 4 || !scene_intersect(orig, dir, spheres, point, N, material))
    {
        return BACKGROUND_COLOR; // background color
    }
    Vec3f reflect_color;
    Vec3f refract_color;
    float kr;
    
    if (reflection_flag)
    {
        Vec3f reflect_dir = normalize(reflect(dir, N));
        Vec3f reflect_orig = reflect_dir.dot(N) < 0 ? point - N * 1e-3 : point + N * 1e-3;
        reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);
    }
    else
    {
        reflect_color = Vec3f(0, 0, 0);
    }
    
    if (refraction_flag)
    {
        Vec3f refract_dir = normalize(refract(dir, N, material.refractive_index));
        Vec3f refract_orig = refract_dir.dot(N) < 0 ? point - N * 1e-3 : point + N * 1e-3;
        refract_color = cast_ray(refract_orig, refract_dir, spheres, lights, depth + 1);
    }
    else
    {
        refract_color = Vec3f(0, 0, 0);
    }
    
    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (size_t i = 0; i < lights.size(); i++)
    {
        Vec3f light_dir = normalize(lights[i].position - point);
        float light_distance = norm(lights[i].position - point);
        
        if (shadow_flag)
        {
            Vec3f shadow_orig = light_dir.dot(N) < 0 ? point - N * 1e-3 : point + N * 1e-3;
            Vec3f shadow_pt, shadow_N;
            Material tmpmaterial;
            if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && norm(shadow_pt - shadow_orig) < light_distance)
                continue;
        }
        diffuse_light_intensity += lights[i].intensity * std::max(0.f, light_dir.dot(N));
        
        if (specular_flag)
        {
            specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, N).dot(dir)), material.specular_exponent) * lights[i].intensity;
        }
        if (spot_light_flag)
        {

            Vec3f light_to_center = normalize(lights[i].position - spheres[i].tp0);
            float t = light_to_center.dot(light_dir);
            
            if (t - cos(SPOT_LIGHT_RADIOUS * M_PI / 180) < 0)
            {
                material.diffuse_color = Vec3f(0, 0, 0);
            }
        }
    }
    if(reflection_flag && refraction_flag)
    {
        fresnel(dir , N, material.refractive_index, kr);
        if (kr < 1) {
            Vec3f refract_dir = normalize(refract(dir, N, material.refractive_index));
            
            Vec3f refract_orig = refract_dir.dot(N) < 0 ? point - N * 1e-3 : point + N * 1e-3;
            refract_color = cast_ray(refract_orig, refract_dir, spheres, lights, depth + 1);
        }
        Vec3f reflect_dir = normalize(reflect(dir, N));
        Vec3f reflect_orig = reflect_dir.dot(N) < 0 ? point - N * 1e-3 : point + N * 1e-3;
        reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);
        return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + Vec3f(1., 1., 1.) * specular_light_intensity * material.albedo[1] + reflect_color * material.albedo[2] * (1 - kr)  + refract_color * material.albedo[3] * kr;
        
    }
        return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + Vec3f(1., 1., 1.) * specular_light_intensity * material.albedo[1] + reflect_color * material.albedo[2] + refract_color * material.albedo[3];
}

void render(const std::vector<Triangle> &spheres, const std::vector<Light> &lights) {
//    std::vector<Vec3f> framebuffer(width*height);
    
    for (int y = 0; y<height; y++) {
        for (int x = 0; x<width; x++) {
            int i = (y * width + x) * 3;

            Vec3f Pp, npe, P_hit;
            Pp = P00 + (n0 * (sx * x / width)) + (n1 * (sy * y / height));
            npe = Pp - Pe;
            npe = normalize(npe);
            
            Vec3f final_col;
//            final_col = cast_ray(Pe, npe, BACKGROUND_COLOR);
            final_col = cast_ray(Pe, npe, spheres, lights);
            rendering[i] = final_col[0];
            rendering[i+1] = final_col[1];
            rendering[i+2] = final_col[2];
        }}
//    stbi_flip_vertically_on_write(true);
//        stbi_write_jpg("/Users/mehdigorjian/Downloads/csce647/pr_08/output000.jpg", width, height, ch, rendering, 200);
    std::cout << "Rendering complete!" << std::endl;
}

static void windowResize(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, (w / 2), 0, (h / 2), 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
static void windowDisplay()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glRasterPos2i(0, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, rendering);
    glFlush();
}
static void init()
{
    glClearColor(1, 1, 1, 1);
}
static void glumain(int argc, char ** argv)
{
    glutInit(&argc, argv);
    glutInitWindowPosition(100, 100); // Where the window will display on-screen.
    glutInitWindowSize(width, height);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutCreateWindow("Image");
    init();
    glutReshapeFunc(windowResize);
    glutDisplayFunc(windowDisplay);
    glutMainLoop();
}


int main(int argc, char ** argv) {
    Material      ivory(1.0, Vec4f(0.6,  0.3, 0.1, 0.0), Vec3f(0.4, 0.4, 0.3),   50.);
    Material      glass(1.5, Vec4f(0.3,  0.5, 0.1, 0.4), Vec3f(0.6, 0.7, 0.8),  850.);
    Material red_rubber(1.0, Vec4f(0.9,  0.1, 0.0, 0.0), Vec3f(0.3, 0.1, 0.1),   10.);
    Material     mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.0, 1.0, 1.0), 1425.);

    std::vector<Triangle> triangles;
         
    Vec3f tp0(57, -40, 3), tp1(-28, -40, -47), tp2(0, 40, 3), tp3(-28, -40, 52);
    
    triangles.push_back(Triangle(tp0, tp1, tp3, mirror));
    triangles.push_back(Triangle(tp0, tp2, tp3, mirror));
    triangles.push_back(Triangle(tp0, tp1, tp2, mirror));
    triangles.push_back(Triangle(tp2, tp3, tp1, mirror));
    
         

    std::vector<Light>  lights;
    lights.push_back(Light(Vec3f(-20, 20,  20), 1.5));
    lights.push_back(Light(Vec3f( 30, 50, -25), 1.8));
    lights.push_back(Light(Vec3f( 30, 20,  30), 1.7));
    
    render(triangles, lights);
    std::cout << "Processing complete!" << std::endl;
    glumain(argc, argv);
    return 0;
}


