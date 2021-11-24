#include <cmath>
#include <iostream>
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

//sphere setting
Vec3f Pc(-80, 80, -20);
//Vec3f Pc(-100, 100, -10);
float radius = 55;

//camera setting
Vec3f Vup(180, 20, -80);
//Vec3f Vup(150, 15, -100);
//float d = 140, sx = 150;
float d = 200, sx = 100;
//Vec3f V_view(10, 10, 100);
Vec3f V_view(20, 20, 110);

//eye setting
//Vec3f Pe(-100, 100, -200);
Vec3f Pe(-110, 90, -180);

//plane
Vec3f P_pl(-100, 100, -8);
Vec3f n_v2(25, -60, 50);

void processing()
{
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
    //camera normals
    Vec3f V0 = V_view.cross(Vup);
    Vec3f n0 = normalize(V0);
    Vec3f n2 = normalize(V_view);
    Vec3f n1 = n0.cross(n2);
    
    float sy = sx * height / width;
    
    //camera center
    Vec3f P_Cam;
    P_Cam = Pe + d * n2;
    
    //camera bottom-left coordinate
    Vec3f P00;
    P00 = P_Cam - (n0 * (sx / 2)) - (n1 * (sy / 2));
    
    Vec3f n_pl = normalize(P_pl);
    n_v2 = normalize(n_v2);
    
    Vec3f n_p2 = n_pl.cross(n_v2);
    Vec3f n_p3 = n_pl.cross(n_p2);
    float p_sx = 30, p_sy = p_sx * height / width;
        
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int i = (y * width + x) * 3;
            
            Vec3f Pp,npe,P_hit;
            Pp = P00 + (n0 * (sx * x / width)) + (n1 * (sy * y / height));
            npe = Pp - Pe;
            npe = normalize(npe);
            float num = n_pl.dot(P_pl-Pe);
            float den = n_pl.dot(npe);
            
            if (num < 0 && den < 0)
            {
                float t_hit = num / den;
                P_hit = Pe + npe * t_hit;
                float u = n_p2.dot(P_hit - P_pl) / p_sx;
                float v = n_p3.dot(P_hit - P_pl) / p_sy;
                
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
                    v = v - floor(v);
                    float x = u * width;
                    float y = v * height;
                    int _x = floor(x + 0.5);
                    int _y = floor(y + 0.5);
                    int pix = (int) ((_y * width + _x) * 3);
                    rendering[i] = img_array[pix];
                    rendering[i+1] = img_array[pix + 1];
                    rendering[i+2] = img_array[pix + 2];
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
    processing();
    glumain(argc, argv);
    return 0;
}
