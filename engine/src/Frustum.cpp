#include "Frustum.h"

// Based on https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling

Frustum CreateFrustumFromCamera(
    const Vec3f &cameraPosition,
    const Vec3f &cameraLookingAt,
    const Vec3f &cameraUp,
    float fovDegrees,
    float aspectRatio,
    float near,
    float far
)
{
    Frustum frustum;
    const float halfVSide = far * tanf(degrees_to_radians(fovDegrees) * .5f);
    const float halfHSide = halfVSide * aspectRatio;
    const Vec3f front = (cameraLookingAt - cameraPosition).Normalize();
    const Vec3f frontMultFar = front * far;

    const Vec3f right = front.Cross(cameraUp).Normalize();
    const Vec3f up = right.Cross(front).Normalize();

    frustum.nearFace = {cameraPosition + (front * near), front};
    frustum.farFace = {cameraPosition + frontMultFar, -front};
    frustum.leftFace = {cameraPosition, (frontMultFar - (right * halfHSide)).Cross(up)};
    frustum.rightFace = {cameraPosition, up.Cross(frontMultFar + (right * halfHSide))};
    frustum.topFace = {cameraPosition, right.Cross(frontMultFar - (up * halfVSide))};
    frustum.bottomFace = {cameraPosition, (frontMultFar + (up * halfVSide)).Cross(right)};

    return frustum;
}

void AABB::Extend(Vec3f f)
{
    min.x = isnan(min.x) ? f.x : std::min(min.x, f.x);
    min.y = isnan(min.y) ? f.y : std::min(min.y, f.y);
    min.z = isnan(min.z) ? f.z : std::min(min.z, f.z);
    max.x = isnan(max.x) ? f.x : std::max(max.x, f.x);
    max.y = isnan(max.y) ? f.y : std::max(max.y, f.y);
    max.z = isnan(max.z) ? f.z : std::max(max.z, f.z);
}

AABB AABB::Transform(Mat4f matrix)
{
    // Based on https://gist.github.com/cmf028/81e8d3907035640ee0e3fdd69ada543f
    Vec3f center = (max + min) * 0.5;
    Vec3f extents = max - center;

    // transform center
    Vec3f t_center = (matrix * center.ToVec4f(1.0f)).ToVec3f();

    // transform extents (take maximum)
    Mat4f abs_mat = matrix.Abs();
    Vec3f t_extents = (abs_mat * extents.ToVec4f(0.0f)).ToVec3f();

    // transform to min/max box representation
    Vec3f tmin = t_center - t_extents;
    Vec3f tmax = t_center + t_extents;

    AABB rbox;

    rbox.min = tmin;
    rbox.max = tmax;

    return rbox;
}

float Plane::getSignedDistanceToPlane(const Vec3f &point) const { return normal.Dot(point) - distance; }

bool AABB::isOnOrForwardPlane(const Plane &plane) const
{
    Vec3f center = (max + min) * 0.5;
    Vec3f extents = max - center;

    const float r = extents.x * std::abs(plane.normal.x) + extents.y * std::abs(plane.normal.y) +
        extents.z * std::abs(plane.normal.z);

    return -r <= plane.getSignedDistanceToPlane(center);
}

bool Frustum::HasInside(AABB &aabb) const
{
    return aabb.isOnOrForwardPlane(leftFace) && aabb.isOnOrForwardPlane(rightFace) &&
        aabb.isOnOrForwardPlane(topFace) && aabb.isOnOrForwardPlane(bottomFace) && aabb.isOnOrForwardPlane(nearFace) &&
        aabb.isOnOrForwardPlane(farFace);
}
