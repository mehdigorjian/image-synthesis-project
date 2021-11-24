#pragma once
#include "geometry.h"
#include "sphere.h"
#include "plane.h"
#include "param_setting.h"

bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Plane> &planes, Vec3f &hit, Vec3f &N, Material &material)
{
    float planes_dist = std::numeric_limits<float>::max();
    for (size_t i = 0; i < planes.size(); i++)
    {
        float dist_i;
        if (planes[i].ray_intersect(orig, dir, dist_i) && dist_i < planes_dist)
        {
            planes_dist = dist_i;
            hit = orig + dir * dist_i;
            N = planes[i].normal_vec(hit);
            // N = (hit - spheres[i].center).normalize();
            material = planes[i].material;
        }
    }

    float checkerboard_dist = std::numeric_limits<float>::max();
    if (fabs(dir.y) > 1e-3)
    {
        float d = -(orig.y + orig.x + 4) / dir.y; // the checkerboard plane has equation y = -4
        Vec3f pt = orig + dir * d;
        if (d > 0 && fabs(pt.x) < 10 && pt.z < -10 && pt.z > -30 && d < planes_dist)
        {
            checkerboard_dist = d;
            hit = pt;
            N = Vec3f(0, 1, 0);
            material.diffuse_color = (int(.5 * hit.x + 1000) + int(.5 * hit.z)) & 1 ? Vec3f(.3, .3, .3) : Vec3f(.3, .2, .1);
        }
    }
    return std::min(planes_dist, checkerboard_dist) < 1000;
}