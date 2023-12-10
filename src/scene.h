#pragma once

#include <cglm/cglm.h>

#define MAX_OBJECT_COUNT 512
#define MAX_LIGHT_COUNT 512

typedef enum
{
    OBJECT_TYPE_SPHERE = 1,
    OBJECT_TYPE_PLANE = 0
} object_type_t;

typedef struct
{
    object_type_t type;
    vec3 position;
    vec3 normal;
    float radius;
} object_t;

typedef struct
{
    vec3 position;
    vec3 color;
    float intensity;
} light_t;

typedef struct
{
    int object_count;
    object_t objects[MAX_OBJECT_COUNT];
    int light_count;
    light_t lights[MAX_LIGHT_COUNT];
} scene_t;

void scene_add_object(scene_t *scene, object_t object)
{
    scene->objects[scene->object_count] = object;
    scene->object_count++;
}

void make_sphere(object_t *sphere, vec3 center, float radius)
{
    sphere->type = OBJECT_TYPE_SPHERE;
    glm_vec3_copy(center, sphere->position);
    sphere->radius = radius;
}

void make_plane(object_t *plane, vec3 position, vec3 normal)
{
    plane->type = OBJECT_TYPE_PLANE;
    glm_vec3_copy(position, plane->position);
    glm_vec3_copy(normal, plane->normal);
}

void scene_add_sphere(scene_t *scene, vec3 center, float radius)
{
    object_t sphere;
    make_sphere(&sphere, center, radius);
    scene_add_object(scene, sphere);
}

void scene_add_plane(scene_t *scene, vec3 position, vec3 normal)
{
    object_t plane;
    make_plane(&plane, position, normal);
    scene_add_object(scene, plane);
}

void scene_add_light(scene_t *scene, vec3 position, vec3 color, float intensity)
{
    light_t light;
    glm_vec3_copy(position, light.position);
    glm_vec3_copy(color, light.color);
    light.intensity = intensity;
    scene->lights[scene->light_count] = light;
    scene->light_count++;
}

// TODO: Apply perspective division
void scene_transform(scene_t *scene, mat4 transform, scene_t *scene_dst)
{
    if (scene != scene_dst)
    {
        *scene_dst = *scene;
    }

    for (int i = 0; i < scene_dst->object_count; i++)
    {
        object_t *object = &scene_dst->objects[i];
        glm_mat4_mulv3(transform, object->position, 1.0f, object->position);
        glm_mat4_mulv3(transform, object->normal, 0.0f, object->normal);
    }

    for (int i = 0; i < scene_dst->light_count; i++)
    {
        light_t *light = &scene_dst->lights[i];
        glm_mat4_mulv3(transform, light->position, 1.0f, light->position);
    }
}
