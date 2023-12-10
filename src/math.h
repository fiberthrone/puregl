#pragma once

#include <cglm/cglm.h>

float sdf_sphere(vec3 p, float radius)
{
    return glm_vec3_norm(p) - radius;
}

float sdf_plane(vec3 p, vec3 n, float h)
{
    return glm_vec3_dot(p, n) + h;
}

void viewport_transform(vec2 position, vec2 viewport_size, vec2 position_dst)
{
    glm_vec2_scale(position, 0.5f, position_dst);
    glm_vec2_add(position_dst, (vec2){0.5f, 0.5f}, position_dst);
    glm_vec2_mul(position_dst, viewport_size, position_dst);
}

void viewport_transform_inverse(vec2 position, vec2 viewport_size, vec2 position_dst)
{
    glm_vec2_div(position, viewport_size, position_dst);
    glm_vec2_sub(position_dst, (vec2){0.5f, 0.5f}, position_dst);
    glm_vec2_scale(position_dst, 2.0f, position_dst);
}

void perspective_division(vec4 v, vec4 v_dst)
{
    glm_vec4_scale(v, 1.0f / v[3], v_dst);
}
