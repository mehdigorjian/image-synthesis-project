#include <cmath>
#include <iostream>
#include <vector>
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
//CUBE
//eye setting
Vec3f Pe(8, 28, -23); // cube
//camera setting
Vec3f V_view(0, -1, 1); // cube
Vec3f Vup(1, 0, 0); // cube
float d = 120, sx = 150; // cube
Vec3f l(20, 0, -1); // cube
//TETRAHEDRON
//eye setting
//Vec3f Pe(0, 50, 3); // tetrahedron
//camera setting
//Vec3f Vup(1, 1, 1); // tetrahedron
//Vec3f V_view(0, -1, 0); // tetrahedron
//float d = 150, sx = 200; // tetrahedron
//Vec3f l(1, 0, -1); // tetrahedron

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

struct Triangle
{
    Vec3f tp0, tp1, tp2;
    Vec3f tv0 = tp0 - tp2;
    Vec3f tv1 = tp1 - tp0;
    Vec3f a = tv0.cross(tv1) * 0.5;
    Vec3f tn = normalize(a);
    Triangle(Vec3f &v0, Vec3f &v1, Vec3f &v2): tp0(v0), tp1(v1), tp2(v2) {}
};

void triangle_processing(Triangle &triangle)
{
    /*
     stbi_set_flip_vertically_on_load(true);
     img = stbi_load("/Users/mehdigorjian/Downloads/map.jpeg", &width, &height, &ch, STBI_rgb);
     
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
     */
    
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
                
                ta0 = normalize(ta0);
                ta1 = normalize(ta1);
                ta2 = normalize(ta2);
                
                float s = triangle.tn.dot(ta1) / lenght_vector(triangle.a);
                float t = triangle.tn.dot(ta2) / lenght_vector(triangle.a);
                float u = triangle.tn.dot(ta0) / lenght_vector(triangle.a);
                
                if(s > 0 && s < 1 && t > 0 && t < 1 && u > 0 && u < 1)
                {
                    Vec3f L = normalize(l);
                    double t = 0.5 * L.dot(triangle.tn) + 0.5;
                    //                    std::cout << t << std::endl;
                    t = clamp(0, 1, t);
                    
                    rendering[i] = 255 * (1 - t) + 255 * t;
                    rendering[i + 1] = 255 * (1 - t) + 0 * t;
                    rendering[i + 2] = 255 * (1 - t) + 255 * t;
                }
            }
        }
    }
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
    //drawing Tetrahedron
    /*
     std::vector<Triangle> triangles;
     
     Vec3f tp0(57, -40, 3), tp1(-28, -40, -47), tp2(0, 40, 3), tp3(-28, -40, 52);
     
     triangles.push_back(Triangle(tp0, tp1, tp3));
     triangles.push_back(Triangle(tp0, tp2, tp3));
     triangles.push_back(Triangle(tp0, tp1, tp2));
     triangles.push_back(Triangle(tp2, tp3, tp1));
     
     for (Triangle tri : triangles) triangle_processing(tri);
     */
    //drawing Cube
    std::vector<Triangle> triangles;
    Vec3f v0(8, -8, -8), v1(-8, -8, -8), v2(8, -8, 8), v3(-8, -8, 8), v4(8, 8, 8), v5(-8, 8, 8), v6(8, 8, -8), v7(-8, 8, -8);
    
    triangles.push_back(Triangle(v0, v1, v2));
    triangles.push_back(Triangle(v2, v0, v3));
    triangles.push_back(Triangle(v2, v3, v4));
    triangles.push_back(Triangle(v4, v2, v5));
    triangles.push_back(Triangle(v6, v4, v2));
    triangles.push_back(Triangle(v6, v2, v0));
    triangles.push_back(Triangle(v1, v3, v5));
    triangles.push_back(Triangle(v1, v5, v7));
    triangles.push_back(Triangle(v0, v1, v6));
    triangles.push_back(Triangle(v6, v1, v7));
    triangles.push_back(Triangle(v6, v7, v5));
    triangles.push_back(Triangle(v6, v5, v4));
    
    
    for (Triangle tri : triangles) triangle_processing(tri);
    
    glumain(argc, argv);
    return 0;
}
