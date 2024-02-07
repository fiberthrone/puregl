#pragma once

#include "camera.h"
#include <cglm/cglm.h>

#define MAX_OBJECT_COUNT 512
#define MAX_LIGHT_COUNT 512

typedef enum
{
    OBJECT_TYPE_SPHERE = 1,
    OBJECT_TYPE_CUBE = 2,
    OBJECT_TYPE_PLANE = 0
} object_type_t;

typedef struct
{
    vec3 base_color;
    float specular;
    float shininess;
} material_t;

typedef struct
{
    object_type_t type;
    material_t material;
    vec3 position;
    vec3 size;
    vec3 normal;
    float radius;
} object_t;

typedef struct
{
    vec4 position; // w = 0.0 for directional light, w = 1.0 for point light
    vec3 color;
    float intensity;
    union
    {
        float radius;         // for point light
        float angular_radius; // for directional light
    };
} light_t;

typedef struct
{
    int object_count;
    object_t objects[MAX_OBJECT_COUNT];
    int light_count;
    light_t lights[MAX_LIGHT_COUNT];
    camera_t camera;
    unsigned int id;
} scene_t;

void scene_add_object(scene_t *scene, object_t object)
{
    scene->objects[scene->object_count] = object;
    scene->object_count++;
}

void make_sphere(object_t *sphere, vec3 center, float radius, material_t material)
{
    *sphere = (object_t){0};
    sphere->type = OBJECT_TYPE_SPHERE;
    glm_vec3_copy(center, sphere->position);
    sphere->radius = radius;
    sphere->material = material;
}

void make_cube(object_t *cube, vec3 center, vec3 size, material_t material)
{
    *cube = (object_t){0};
    cube->type = OBJECT_TYPE_CUBE;
    glm_vec3_copy(center, cube->position);
    glm_vec3_copy(size, cube->size);
    cube->material = material;
}

void make_plane(object_t *plane, vec3 position, vec3 normal, material_t material)
{
    *plane = (object_t){0};
    plane->type = OBJECT_TYPE_PLANE;
    glm_vec3_copy(position, plane->position);
    glm_vec3_copy(normal, plane->normal);
    plane->material = material;
}

void scene_add_sphere(scene_t *scene, vec3 center, float radius, material_t material)
{
    object_t sphere;
    make_sphere(&sphere, center, radius, material);
    scene_add_object(scene, sphere);
}

void scene_add_cube(scene_t *scene, vec3 center, vec3 size, material_t material)
{
    object_t cube;
    make_cube(&cube, center, size, material);
    scene_add_object(scene, cube);
}

void scene_add_plane(scene_t *scene, vec3 position, vec3 normal, material_t material)
{
    object_t plane;
    make_plane(&plane, position, normal, material);
    scene_add_object(scene, plane);
}

void scene_add_point_light(scene_t *scene, vec3 position, vec3 color, float intensity, float radius)
{
    light_t light = {0};
    glm_vec4_copy((vec4){position[0], position[1], position[2], 1.0f}, light.position);
    glm_vec3_copy(color, light.color);
    light.intensity = intensity;
    light.radius = radius;
    scene->lights[scene->light_count] = light;
    scene->light_count++;
}

void scene_add_directional_light(scene_t *scene, vec3 direction, vec3 color, float intensity, float angular_radius)
{
    light_t light = {0};
    glm_vec4_copy((vec4){direction[0], direction[1], direction[2], 0.0f}, light.position);
    glm_vec3_copy(color, light.color);
    light.intensity = intensity;
    light.angular_radius = angular_radius;
    scene->lights[scene->light_count] = light;
    scene->light_count++;
}

void scene_set_camera(scene_t *scene, camera_t *camera)
{
    scene->camera = *camera;
    ++scene->id;
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
