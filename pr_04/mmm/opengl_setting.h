#pragma once
#include <GL/glut.h>
#include "render_glw.h"
#include "param_setting.h"

static void windowResize(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, (w / 2), 0, (h / 2), 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
static void windowDisplay(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glRasterPos2i(0, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, result);
    glFlush();
}
static void init(void)
{
    glClearColor(1, 1, 1, 1); // Set background color.
}

int opengl_run(void)
{

    result = new unsigned char[200 * 200 * 3];

    render_glw();

    glutInit();
    glutInitWindowPosition(0, 0); // Where the window will display on-screen.
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutCreateWindow("Homework Zero");
    init();
    glutReshapeFunc(windowResize);
    glutDisplayFunc(windowDisplay);
    glutMouseFunc(processMouse);
    glutMainLoop();
}