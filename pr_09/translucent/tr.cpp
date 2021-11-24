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


int width = 300, height = 300, ch = 3, ind = 0;
unsigned char *img, *img_array = new unsigned char[width * height * 3], *rendering = new unsigned char[width * height * 3];
//const float PI = 3.141592;
//eye setting
Vec3f Pe(0, 100, 60);
//camera setting
Vec3f V_view(1, -2, 0);
Vec3f Vup(1, 1, 1);
float d = 100, sx = 200;
//Vec3f l(20, 0, -1);

Vec3f l(60, -100, 50);
Vec3f L = l.normalize();
Vec3f R(0.0, 0.2, 0.0);
//Vec3f R(0.0, 0.2, 0.0);
Vec3f r = R.normalize();
float sy = sx * height / width;

//camera normals
Vec3f V0 = cross(V_view, Vup);
Vec3f n0 = V0.normalize();
Vec3f n2 = V_view.normalize();
Vec3f n1 = cross(n0, n2);
Vec3f default_col(100, 0, 0);

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
    Vec3f hit_col(10, 100, 0), n;
    int index;
    float t_hit;
    
    if (depth > 4)
        return default_col;
    
    if(trace(Pe, npe, index, t_hit))
    {
        Vec3f P_hit = Pe + (npe * t_hit);
        spheres[index]->normal(P_hit);
        Vec3f n = spheres[index]->n;
        if(spheres[index]->mat.refraction)
        {
            float i = .1;
//            for (float i = -1; i < 1; i+=0.1)
//            {
//                std::cout << "refract-" << i << std::endl;

                Vec3f n_refr = refract(npe, n, spheres[index]->mat.ior);
                n_refr = n_refr * (1 / lenght_vector(n_refr)) + r * i;
                hit_col = hit_col + castRay(P_hit, n_refr, default_col, depth+1);
//            }
            hit_col = hit_col * 0.047;
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
            Vec3f Pp, npe, P_hit, P_h, pixel(0, 0, 0);
            
            Pp = P00 + (n0 * (sx * x / width)) + (n1 * (sy * y / height));
            npe = Pp - Pe;
            npe = npe.normalize();
            
            pixel = pixel + castRay(Pe, npe, default_col);
            
            rendering[i]   = pixel[0];
            rendering[i+1] = pixel[1];
            rendering[i+2] = pixel[2];
            std::cout << "X: " << x << std::endl;
        }
        std::cout << "Y: " << y << std::endl;
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
    Material mat1(Vec3f(0.4, 0.4, 0.3), false, false, 1);
    Material mat2(Vec3f(1.0, 1.0, 0.0), false, true, 1.66);
    
    Vec3f c1(30, -80, 50);
    Vec3f c2(30, -30, 50);
    
    Sphere *sp=new Sphere(c1, 50, mat2);
    spheres[ind]= sp;
    ind+=1;
    //    sp=new Sphere(c2, 50, mat1);
    //    spheres[ind]= sp;
    //    ind+=1;
    processing();
    glumain(argc, argv);
    return 0;
}


