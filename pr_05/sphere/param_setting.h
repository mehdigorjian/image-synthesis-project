#pragma once
#include "geometry.h"

bool reflection_flag = false;
bool refraction_flag = false;
bool shadow_flag = true;
bool specular_flag = true;

// spot light
Vec3f BACKGROUND_COLOR = Vec3f(0.2, 0.7, 0.8);
bool spot_light_flag = true;
float SPOT_LIGHT_RADIOUS = 10.;

const int WIDTH = 1024;
const int HEIGHT = 768;

// camera and view parameters
Vec3f camera_origin = Vec3f(0, 0, 0); // render.h line 26
float param = .5;
float fov_param = 3.;

