#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "geometry.h"
//#include <opencv2/opencv.hpp>
//#include <opencv2/highgui.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <OpenGL/gl.h>
#include <OpenGl/glu.h>
#include <GLUT/glut.h>

//using namespace cv;

int width = 300, height = 300, ch;
unsigned char *img, *img_array = new unsigned char[width * height * 3], *rendering = new unsigned char[width * height * 3];
const float PI = 3.141592;
//eye setting
Vec3f Pe(0, 125, 3);
//camera setting
Vec3f V_view(0, -1, .5);
Vec3f Vup(1, 1, 1);
float d = 100, sx = 200;
//Vec3f l(20, 0, -1);

Vec3f l(60, -100, 50);
Vec3f L = l.normalize();
float sy = sx * height / width;

//camera normals
Vec3f V0 = cross(V_view, Vup);
Vec3f n0 = V0.normalize();
Vec3f n2 = V_view.normalize();
Vec3f n1 = cross(n0, n2);

Vec3f default_col(255, 255, 255);

//camera center
Vec3f P_Cam = Pe + n2 * d;
//camera bottom-left coordinate
Vec3f P00 = P_Cam - (n0 * (sx / 2)) - (n1 * (sy / 2));

float lenght_vector(Vec3f v)
{
    return sqrt(pow(v[0], 2) + pow(v[1], 2) + pow(v[2], 2));
}

float clamp(float min, float max, float x) {
    x = (x - min) / (max - min);
    if (x > 1) x = 1;
    if (x < 0) x = 0;
    return -2 * pow(x, 3) + 3 * pow(x, 2);
}

struct Material {
    Material(){};
    Material(const Vec3f& color, bool refl, bool refr, float ir): diffuse_color(color), reflection(refl), refraction(refr), ior(ir){}
    Vec3f diffuse_color;
    bool reflection;
    bool refraction;
    float ior;
};


class Sphere
{
public:
    Vec3f Pc, n;
    float r;
    Material mat;
    Sphere(Vec3f sp_center, float radius, Material material)
    {
        Pc = sp_center;
        r = radius;
        mat = material;
    }
    
    void normal(Vec3f& P) {n = (P - Pc) * (1 / r);}
    
    float eq(Vec3f& P) {return (P - Pc) * (P - Pc) - pow(r, 2);}
    
    bool intersect(Vec3f &P, Vec3f &dir, float &t_hit)
    {
        float b, c, disc;
        b = dir * (P - Pc);
        c = eq(P);
        disc = sqrt(pow(b, 2) - c);
        if((b > 0 || isnan(disc))){}
        else
        {
            t_hit = -b - disc;
            return true;
        }
        return false;
    }
};


float max(float a, float b) {return a > b ? a : b;}

float min(float a, float b) {return a > b ? b : a;}

void fresnel(const Vec3f& I, const Vec3f& N, const float& ior, float& kr)
{
    float cosi = max(-1, min(1, (I * N)));
    float etai = 1, etat = ior;
    if (cosi > 0) { std::swap(etai, etat); }
    
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
    if (sint >= 1) kr = 1;
    else {
        float cost = sqrtf(std::max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        kr = (Rs * Rs + Rp * Rp) / 2;
    }
}

Vec3f refract(const Vec3f &I, const Vec3f &N, const float &ior)
{
    float cosi = max(-1, min(1, (I * N)));
    float etai = 1, etat = ior;
    Vec3f n = N;
    Vec3f def(0, 0, 0);
    
    if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n = N * (-1); }
    float eta = etai / etat;
    float k = 1 - eta * eta * (1 - cosi * cosi);
    return k < 0 ? def : I * eta + n * (eta * cosi - sqrtf(k)) ;
}

Vec3f reflect(const Vec3f& I, const Vec3f& N)
{
    return I - N*2.f*(I*N);
}

int trace(Sphere sphere, Vec3f& P, Vec3f& dir, float& t_hit)
{
    float t, t_min = std::numeric_limits<float>::max();
    if ( sphere.intersect(P, dir, t) && t < t_min) {
        t_min = t;
    }
    t_hit = t_min;
    return t_hit != std::numeric_limits<float>::max();
}

Vec3f castRay(Sphere sphere, Vec3f& Pe, Vec3f& npe, Vec3f& default_col, const int& depth=0)
{
    Vec3f n;
    float t_hit;
    Vec3f hit_col = sphere.mat.diffuse_color , refl_col = sphere.mat.diffuse_color, refr_col = sphere.mat.diffuse_color;
    
    if (depth > 4)
        return default_col;
    if(trace(sphere, Pe, npe, t_hit))
    {
        Vec3f P_hit = Pe + (npe * t_hit);
        sphere.normal(P_hit);
        Vec3f n = sphere.n;

        bool outside = npe * n < 0;
        Vec3f bias = n * 0.0001;
        
        if(sphere.mat.reflection && sphere.mat.refraction)
        {
            float kr;
            fresnel(npe, n, sphere.mat.ior , kr);
            
            if (kr < 1) {
                Vec3f n_refr = refract(npe, n, sphere.mat.ior);
                n_refr = n_refr * (1 / lenght_vector(n_refr));
                
                P_hit = outside ? P_hit + bias : P_hit - bias;
                refr_col = castRay(sphere, P_hit, n_refr, default_col, depth + 1);
            }
            
            Vec3f n_refl = reflect(npe, n);
            n_refl = n_refl * (1 / lenght_vector(n_refl));
            
            P_hit = outside ? P_hit - bias : P_hit + bias;
            refl_col = castRay(sphere, P_hit, n_refl, default_col, depth + 1);
            
            hit_col = hit_col + refl_col * (1 - kr)  + refr_col * kr;
        }
        else if(sphere.mat.reflection)
        {
            Vec3f n_refl = reflect(npe, n);
            n_refl = n_refl * (1 / lenght_vector(n_refl));
            
            P_hit = outside ? P_hit - bias : P_hit + bias;
            hit_col = hit_col+ castRay(sphere, P_hit, n_refl, default_col, depth + 1);
        }
        else if(sphere.mat.refraction)
        {
            float kr;
            fresnel(npe, n, sphere.mat.ior, kr);
            
            if (kr < 1) {
                Vec3f n_refr = refract(npe, n, sphere.mat.ior);
                n_refr = n_refr * (1 / lenght_vector(n_refr));
                
                P_hit = outside ? P_hit + bias : P_hit - bias;
                refr_col = castRay(sphere, P_hit, n_refr, default_col, depth + 1);
            }
        }
        else
            hit_col = sphere.mat.diffuse_color;
    }
    else
        hit_col = default_col;
    
    return hit_col;
}

void processing(Sphere sphere)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int i = (y * width + x) * 3;
            Vec3f Pp, npe, P_hit, P_h;
            
            Pp = P00 + (n0 * (sx * x / width)) + (n1 * (sy * y / height));
            npe = Pp - Pe;
            npe = npe.normalize();
            
            Vec3f final_col;
            final_col = castRay(sphere, Pe, npe, default_col);
            rendering[i] = final_col[0];
            rendering[i+1] = final_col[1];
            rendering[i+2] = final_col[2];
        }
    }
    stbi_flip_vertically_on_write(true);
    stbi_write_jpg("output.jpg", width, height, ch, rendering, 200);
    
    std::cout << "Processing complete!" << std::endl;
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
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(width, height);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutCreateWindow("Image");
    init();
    glutReshapeFunc(windowResize);
    glutDisplayFunc(windowDisplay);
    glutMainLoop();
}

int main(int argc, char ** argv)
{
    Material mat1(Vec3f(0.4, 0.4, 0.3), true, false, 1.66);
    Material mat2(Vec3f(0.2, 0.4, 0.3), false, false, 5.66);

    Vec3f c1(30, -30, 50);
    Vec3f c2(50, -50, 40);
    
    std::vector<Sphere> spheres;
    
    spheres.push_back(Sphere(c1, 50, mat1));
//    spheres.push_back(Sphere(c2, 70, mat1));

    
    for (Sphere sp : spheres) processing(sp);
    glumain(argc, argv);
    return 0;
}

//codesign -f -s "mehdi.gorjian@yahoo.com" /usr/local/Cellar/opencv/4.5.1_3/lib/libopencv_core.4.5.1.dylib
//codesign -f -s "mehdi.gorjian@yahoo.com" /usr/local/opt/opencv/lib/libopencv_core.4.5.dylib
