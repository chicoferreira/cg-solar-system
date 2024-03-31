#include "World.h"

#define _USE_MATH_DEFINES
#include <iostream>
#include <math.h>

constexpr auto sensitivity = 0.1f;
constexpr auto scroll_sensitivity = 0.1f;

namespace engine::world
{
    void Camera::UpdateCameraRotation(float x_offset, float y_offset)
    {
        float radius, alpha, beta;
        x_offset = degrees_to_radians(x_offset);
        y_offset = degrees_to_radians(y_offset);

        if (first_person_mode)
        {
            (looking_at - position).ToSpherical(radius, alpha, beta);
            alpha -= x_offset * sensitivity;
            beta += y_offset * sensitivity;
        }
        else
        {
            (position - looking_at).ToSpherical(radius, alpha, beta);

            alpha -= x_offset * sensitivity;
            beta -= y_offset * sensitivity;
        }

        if (beta > M_PI_2)
        {
            beta = M_PI_2 - 0.001f;
        }
        else if (beta < -M_PI_2)
        {
            beta = -M_PI_2 + 0.001f;
        }

        const auto after = Vec3fSpherical(radius, alpha, beta);
        if (first_person_mode)
        {
            looking_at = after + position;
        }
        else
        {
            position = after + looking_at;
        }
    }

    void Camera::Tick(const Vec3f input_movement, const float scroll_input, const float timestep)
    {
        const Vec3f forward = (looking_at - position).Normalize();
        const Vec3f right = forward.Cross(up).Normalize();

        const Vec3f move_dir =
            (forward * input_movement.z + right * input_movement.x).Normalize() + up * input_movement.y;
        const auto acceleration = move_dir * acceleration_per_second * timestep;
        speed += acceleration;

        scroll_speed += scroll_input * scroll_sensitivity * acceleration_per_second;

        if (speed.Length() > max_speed_per_second)
        {
            speed = speed.Normalize() * max_speed_per_second;
        }

        if ((looking_at - position).Length() > 1.0f || scroll_input < 0)
        {
            position += forward * scroll_speed * timestep;
        }

        position += speed * timestep;
        looking_at += speed * timestep;

        if (move_dir.x == 0 && move_dir.y == 0 && move_dir.z == 0)
        {
            speed -= speed * timestep * friction_per_second;
        }

        if (scroll_input == 0)
        {
            scroll_speed -= scroll_speed * timestep * friction_per_second;
        }
    }

    void GroupTransform::UpdateTransformMatrix()
    {
        m_final_transform = Mat4fIdentity;
        for (auto &transformation : GetTransformations())
        {
            std::visit([&](auto &&arg) { m_final_transform *= arg.GetTransform(); }, transformation);
        }

        m_final_transform = m_final_transform.transpose();
    }
} // namespace engine::world
