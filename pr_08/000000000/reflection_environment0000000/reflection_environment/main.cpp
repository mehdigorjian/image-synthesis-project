#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <OpenGL/gl.h>
#include <OpenGl/glu.h>
#include <GLUT/glut.h>

using namespace cv;

int width = 300, height = 300, ch;
unsigned char *img, *img_array = new unsigned char[width * height * 3], *rendering = new unsigned char[width * height * 3];

//eye setting
Vec3f Pe(-2, 8, -5);
//camera setting
Vec3f V_view(0, -1, 1);
Vec3f Vup(1, 0, 0);
float d = 110, sx = 200;
Vec3f l(20, 0, -1);

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
    Material(const Vec3f &color, bool refl, bool refr, float ir):diffuse_color(color), reflection(refl), refraction(refr), ior(ir){}
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
    
    void normal(Vec3f &P)
    {
        n = (P - Pc) * (1 / r);
    }
    
    float eq(Vec3f &P)
    {
        return (P - Pc).dot(P - Pc) - pow(r, 2);
    }
    
    bool intersect(Vec3f &P, Vec3f &dir, float &t_hit)
    {
        float b, c, disc;
        b = dir.dot(P - Pc);
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

int trace(std::vector<Sphere> spheres, Vec3f &P, Vec3f &dir, int &index, float &t_hit)
{
    float t_min = std::numeric_limits<float>::max();
    float ind_min = std::numeric_limits<float>::max();
    float t;
    
    for (int k=0; k < spheres.size(); k++)
    {
        
        if ( spheres[k].intersect(P, dir, t) && t<t_min) {
            ind_min = k;
            t_min = t;
        }
    }
    t_hit = t_min;
    index = ind_min;
    return t_hit != std::numeric_limits<float>::max() && index != std::numeric_limits<float>::max();
}

void fresnel(const Vec3f &I, const Vec3f &N, const float &ior, float &kr)
{
    float temp = I.dot(N);
    float _min = std::min(1, temp);
    float cosi = std::max(-1, _min);
    float etai = 1, etat = ior;
    if (cosi > 0) { std::swap(etai, etat); }
    
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
    // Total internal reflection
    if (sint >= 1) {
        kr = 1;
    }
    else {
        float cost = sqrtf(std::max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        kr = (Rs * Rs + Rp*Rp) / 2;
    }
}




/*
void ray_cast1(const Triangle &triangle)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int i = (y * width + x) * 3;
            
            Vec3f Pp, npe, P_hit;
            Pp = P00 + (n0 * (sx * x / width)) + (n1 * (sy * y / height));
            npe = Pp - Pe;
            npe = normalize(npe);
            
            float num = triangle.tn.dot(triangle.tp1 - Pe);
            float den = triangle.tn.dot(npe);
            
            if (num < 0 && den < 0)
            {
                float t_hit = num / den;
                P_hit = Pe + npe * t_hit;
                
                Vec3f ta0 = (P_hit - triangle.tp2).cross(triangle.tp1 - P_hit);
                Vec3f ta1 = (P_hit - triangle.tp0).cross(triangle.tp2 - P_hit);
                Vec3f ta2 = (P_hit - triangle.tp1).cross(triangle.tp0 - P_hit);
                
                //                ta0 = normalize(ta0);
                //                ta1 = normalize(ta1);
                //                ta2 = normalize(ta2);
                
                float s = triangle.tn.dot(ta1 * 0.5) / lenght_vector(triangle.a);
                float t = triangle.tn.dot(ta2 * 0.5) / lenght_vector(triangle.a);
                float u = triangle.tn.dot(ta0 * 0.5) / lenght_vector(triangle.a);
                
                if(s > 0 && s < 1 && t > 0 && t < 1 && u > 0 && u < 1)
                {
                    float u = s, v = t;
                    if (u < 0)
                    {
                        u = u - floor(u);
                        u = 1 - u;
                    }
                    else
                    {
                        u = u - floor(u);
                    }
                    if (v < 0)
                    {
                        v = v - floor(v);
                        v = 1 - v;
                    }
                    else
                    {
                        v = v - floor(u);
                    }
                    int x_pos = floor(u * width + 0.5);
                    int y_pos = floor(v * height + 0.5);
                    
                    int pix = (int)((y_pos * width + x_pos) * 3);
                    
                    rendering[i] = img_array[pix];
                    rendering[i + 1] = img_array[pix + 1];
                    rendering[i + 2] = img_array[pix + 2];
                }
            }
        }
    }
}

void triangle_processing()
{
    
    stbi_set_flip_vertically_on_load(true);
    img = stbi_load("/Users/mehdigorjian/Downloads/csce647/pr_07/map2.jpeg", &width, &height, &ch, STBI_rgb);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            
            int i = (y * width + x) * 3;
            img_array[i] = img[i];
            i++;
            img_array[i] = img[i];
            i++;
            img_array[i] = img[i];
            i++;
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
    glutInitWindowPosition(100, 100); // Where the window will display on-screen.
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
    triangle_processing();
    glumain(argc, argv);
    return 0;
}
*/

