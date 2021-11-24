#pragma once
#include "geometry.h"
#include "material_light.h"
#include "param_setting.h"

struct Plane
{
    Vec3f point;
    Vec3f normal;
    Material material;

    Plane(const Vec3f &c, const Vec3f N, const Material &m) : point(c), normal(N), material(m) {}

    Vec3f const normal_vec(Vec3f const _hit) const
    {
        return normal;
    }

    bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0) const
    {
        float num = normal * (point - orig);
        float den = normal * dir;
        if (num < 0 && den < 0)
        {
            t0 = num / den;
            return true;
        }
        return false;
    }
};
