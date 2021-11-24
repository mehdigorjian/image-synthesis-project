#include <cmath>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include "OBJ_Loader.h"

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
//Vec3f Pe(-2, 5, -5);
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
//----------------------------------------------------------------------------- LOAD OBJ HERE
objl::Loader Loader;
bool isLoaded = Loader.LoadFile("/Users/mehdigorjian/Downloads/csce647/pr_07/object2.obj");
//-----------------------------------------------------------------------------
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

void ray_cast(const Triangle &triangle)
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
    
    if (isLoaded)
    {
        for (int i = 0; i < Loader.LoadedMeshes.size(); i++)
        {
            objl::Mesh mesh = Loader.LoadedMeshes[i];
            for (int j = 0; j < mesh.Vertices.size()-3; j+=3)
            {
                Vec3f tp0(mesh.Vertices[j].Position.X, mesh.Vertices[j].Position.Y, mesh.Vertices[j].Position.Z);
                Vec3f tp1(mesh.Vertices[j + 1].Position.X, mesh.Vertices[j + 1].Position.Y, mesh.Vertices[j + 1].Position.Z);
                Vec3f tp2(mesh.Vertices[j + 2].Position.X, mesh.Vertices[j + 2].Position.Y, mesh.Vertices[j + 2].Position.Z);
                
                Triangle triangle(tp0, tp1, tp2);
                ray_cast(triangle);
            }
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

