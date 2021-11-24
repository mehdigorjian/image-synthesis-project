#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <random>
#include "geometry.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <OpenGL/gl.h>
#include <OpenGl/glu.h>
#include <GLUT/glut.h>


int width = 300, height = 300, ch, ind = 0;
unsigned char *img, *img_array = new unsigned char[width * height * 3], *rendering = new unsigned char[width * height * 3];
//const float PI = 3.141592;
//eye setting
Vec3f Pe(0, 145, 0);
//camera setting
Vec3f V_view(0, -1, 0);
Vec3f Vup(1, 1, 1);
float d = 100, sx = 200;

Vec3f l(60, -100, 50);
Vec3f L = l.normalize();
float sy = sx * height / width;

//camera normals
Vec3f V0 = cross(V_view, Vup);
Vec3f n0 = V0.normalize();
Vec3f n2 = V_view.normalize();
Vec3f n1 = cross(n0, n2);
Vec3f default_col(0, 0, 0);

int eye_sx = 20, eye_sy = 20, M = 15, N = 15;
Vec3f Pe_ll= Pe - n0 * eye_sx * 0.5 - n1 * eye_sy * 0.5;


////camera center
Vec3f P_Cam = Pe + n2 * d;
////camera bottom-left coordinate
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

float max(float a, float b) {return a > b ? a : b;}
float min(float a, float b) {return a > b ? b : a;}

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
    
    bool intersect(Vec3f& P, Vec3f &dir, float &t_hit)
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
    return I - cross(N, cross(I, N) * 2);
}

Sphere *spheres[3];

int trace(Vec3f& P, Vec3f& dir, int& index, float& t_hit)
{
    float t, t_min = std::numeric_limits<float>::max(), ind_min = std::numeric_limits<float>::max();
    
    for (int k=0; k < ind; k++)
    {
        if ( spheres[k]->intersect(P, dir, t) && t < t_min) {
            ind_min = k;
            t_min = t;
        }
    }
    t_hit = t_min;
    index = ind_min;
    return t_hit != std::numeric_limits<float>::max() && index != std::numeric_limits<float>::max();
    
}

Vec3f castRay(Vec3f& Pe, Vec3f& npe, Vec3f& default_col, const int& depth=0)
{
    Vec3f dark(0, 0, 0), n;
    Vec3f hit_col = dark;
    
    int index;
    float t_hit;
    Vec3f refl_col = dark, refr_col = dark;
    
    
    if (depth > 4)
        return default_col;
    
    if(trace(Pe, npe, index, t_hit))
    {
        Vec3f P_hit = Pe + (npe * t_hit);
        spheres[index]->normal(P_hit);
        Vec3f n = spheres[index]->n;
        
        bool outside = npe * n < 0;
        Vec3f bias = n * 0.0001;
        
        if(spheres[index]->mat.reflection && spheres[index]->mat.refraction)
        {
            float kr;
            fresnel(npe, n, spheres[index]->mat.ior, kr);
            
            if (kr < 1)
            {
                Vec3f n_refr = refract(npe, n, spheres[index]->mat.ior);
                n_refr = n_refr * (1 / lenght_vector(n_refr));
                
                P_hit = outside ? P_hit + bias : P_hit - bias;
                refr_col = castRay(P_hit, n_refr, hit_col, depth+1);
            }
            
            Vec3f n_refl = reflect(npe, n);
            n_refl = n_refl * (1 / lenght_vector(n_refl));
            
            P_hit = outside ? P_hit - bias : P_hit + bias;
            refr_col = castRay(P_hit, n_refl, hit_col, depth+1);
            
            default_col = (hit_col + refl_col * (1 - kr)+ refr_col * kr) * 0.8;
            default_col = hit_col * (1 - kr) + spheres[index]->mat.diffuse_color * kr;
            
        }
        else if(spheres[index]->mat.reflection)
        {
            Vec3f n_refl = reflect(npe, n);
            n_refl = n_refl * (1 / lenght_vector(n_refl));
            
            P_hit = outside ? P_hit + bias : P_hit - bias;
            refr_col = castRay(P_hit, n_refl, hit_col, depth+1);
        }
        
        else if(spheres[index]->mat.refraction)
        {
            float kr;
            fresnel(npe, n, spheres[index]->mat.ior, kr);
            
            if (kr < 1) {
                Vec3f n_refr = refract(npe, n, spheres[index]->mat.ior);
                n_refr = n_refr * (1 / lenght_vector(n_refr));
                
                P_hit = outside ? P_hit - bias : P_hit + bias;
                refr_col = castRay(P_hit, n_refr, hit_col, depth+1);
            }
        }
        else
            hit_col = spheres[index]->mat.diffuse_color;
    }
    else
        hit_col = default_col;
    return hit_col;
}

void processing()
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int i = (y * width + x) * 3;
            Vec3f pixel(0, 0, 0);
            
            for(int m = 0; m < M; M++)
            {
                for(int n = 0; n < N; N++)
                {
                    Vec3f Pp, npe, P_hit, P_h;
                    Vec3f eye_sample_pos = Pe_ll + n0 * eye_sx * ((m + 0.4) / M) + n1 * eye_sy * ((n + 0.7) / N);
                    Pp = P00 + (n0 * (sx * x / width)) + (n1 * (sy * y / height));
                    npe = Pp - eye_sample_pos;
                    npe = npe.normalize();
                    
                    pixel = pixel + castRay(eye_sample_pos, npe, default_col);
                    std::cout << "Inner loop!" << m << std::endl;
                }
            }
//            std::cout << "x: " << x << std::endl;

            rendering[i]   = pixel[0] / (M * N);
            rendering[i+1] = pixel[1] / (M * N);
            rendering[i+2] = pixel[2] / (M * N);
        }
//        std::cout << "Y: " << y << std::endl;
    }
//    stbi_flip_vertically_on_write(true);
//    stbi_write_jpg("output.jpg", width, height, ch, rendering, 200);
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
    Material mat1(Vec3f(0.0, 0.8, 0.9), false, false, 1.0);
    Material mat2(Vec3f(1.0, 0.0, 0.8), false, false, 1.0);
    Material mat3(Vec3f(0.2, 0.1, 0.9), false, false, 1.0);
    
    Vec3f c1(0, -30, 0);
    Vec3f c2(-50, 20, 0);
    Vec3f c3(150, -80, 0);
    
    Sphere *sp=new Sphere(c1, 50, mat1);
    spheres[ind]= sp;
    ind+=1;
    sp=new Sphere(c2, 50, mat2);
    spheres[ind]= sp;
    ind+=1;
    sp=new Sphere(c3, 50, mat3);
    spheres[ind]= sp;
    ind+=1;
    
    processing();
    glumain(argc, argv);
    return 0;
}
