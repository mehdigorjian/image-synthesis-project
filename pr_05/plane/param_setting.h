#pragma once
#include "geometry.h"

bool reflection_flag = false;
bool refraction_flag = false;
bool shadow_flag = false;
bool specular_flag = false;


Vec3f BACKGROUND_COLOR = Vec3f(0.2, 0.7, 0.8);
float T = 1.;
bool spot_light_flag = false;
float SPOT_LIGHT_RADIOUS = 25.;
const float PI = 3.14159265;