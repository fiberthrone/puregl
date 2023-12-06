#include <cglm/cglm.h>

#define MAX_OBJECT_COUNT 512

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
    int object_count;
    object_t objects[MAX_OBJECT_COUNT];
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