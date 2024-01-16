#pragma once

#include <cglm/cglm.h>

typedef struct
{
    vec3 position;
    vec3 direction;
    vec3 up;
    vec3 target;
} camera_t;

void camera_copy(camera_t *camera, camera_t *camera_dst)
{
    if (camera_dst != camera)
    {
        *camera_dst = *camera;
    }
}

void camera_move_forwards(camera_t *camera, float distance, camera_t *camera_dst)
{
    vec3 delta;
    glm_vec3_copy(camera->direction, delta);
    glm_vec3_scale(delta, distance, delta);

    camera_copy(camera, camera_dst);
    glm_vec3_add(camera->position, delta, camera_dst->position);
    glm_vec3_add(camera->target, delta, camera_dst->target);
}

void camera_move_backwards(camera_t *camera, float distance, camera_t *camera_dst)
{
    vec3 delta;
    glm_vec3_copy(camera->direction, delta);
    glm_vec3_scale(delta, -distance, delta);

    camera_copy(camera, camera_dst);
    glm_vec3_add(camera->position, delta, camera_dst->position);
    glm_vec3_add(camera->target, delta, camera_dst->target);
}

void camera_move_left(camera_t *camera, float distance, camera_t *camera_dst)
{
    vec3 delta;
    glm_vec3_cross(camera->direction, camera->up, delta);
    glm_vec3_normalize(delta);
    glm_vec3_scale(delta, -distance, delta);

    camera_copy(camera, camera_dst);
    glm_vec3_add(camera->position, delta, camera_dst->position);
    glm_vec3_add(camera->target, delta, camera_dst->target);
}

void camera_move_right(camera_t *camera, float distance, camera_t *camera_dst)
{
    vec3 delta;
    glm_vec3_cross(camera->direction, camera->up, delta);
    glm_vec3_normalize(delta);
    glm_vec3_scale(delta, distance, delta);

    camera_copy(camera, camera_dst);
    glm_vec3_add(camera->position, delta, camera_dst->position);
    glm_vec3_add(camera->target, delta, camera_dst->target);
}

void camera_rotate(camera_t *camera, float delta_phi, float delta_theta, camera_t *camera_dst)
{
    vec3 move, move_negative;
    glm_vec3_copy(camera->position, move);
    glm_vec3_scale(move, -1.0f, move_negative);

    mat4 transform;
    glm_mat4_identity(transform);

    glm_translate(transform, move);                  // move to camera target space
    glm_rotate_y(transform, delta_phi, transform);   // rotate around y axis
    glm_rotate_x(transform, delta_theta, transform); // rotate around x axis
    glm_translate(transform, move_negative);         // move back to world space

    camera_copy(camera, camera_dst);
    glm_mat4_mulv3(transform, camera->position, 1.0f, camera_dst->position);
    glm_mat4_mulv3(transform, camera->direction, 0.0f, camera_dst->direction);
}
