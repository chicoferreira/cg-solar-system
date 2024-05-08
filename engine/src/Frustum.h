#ifndef CG_SOLAR_SYSTEM_FRUSTUM_H
#define CG_SOLAR_SYSTEM_FRUSTUM_H


#include "Mat.h"
#include "Vec.h"

struct Plane
{
    Vec3f normal = Vec3f{0.f, 1.f, 0.f};
    float distance = 0.f;

    Plane() = default;
    Plane(const Vec3f &normal, const float distance) : normal(normal), distance(distance) {}
    Plane(const Vec3f &p1, const Vec3f &norm) : normal(norm.Normalize()), distance(normal.Dot(p1)) {}
    float getSignedDistanceToPlane(const Vec3f &point) const;
};

struct AABB
{
    Vec3f min{NAN};
    Vec3f max{NAN};
    void Extend(Vec3f f);
    AABB Transform(Mat4f matrix);
    bool isOnOrForwardPlane(const Plane &plane) const;
};

struct Frustum
{
    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane farFace;
    Plane nearFace;
    bool HasInside(AABB &aabb) const;
};

Frustum CreateFrustumFromCamera(
    const Vec3f &cameraPosition,
    const Vec3f &cameraLookingAt,
    const Vec3f &cameraUp,
    float fov,
    float aspectRatio,
    float near,
    float far
);

#endif // CG_SOLAR_SYSTEM_FRUSTUM_H
