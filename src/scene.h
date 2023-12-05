#include <cglm/cglm.h>

#define MAX_OBJECT_COUNT 512

typedef struct
{
    vec3 center;
    float radius;
} sphere_t;

typedef struct
{
    int sphere_count;
    sphere_t spheres[MAX_OBJECT_COUNT];
} scene_t;

void scene_add_sphere(scene_t *scene, sphere_t sphere)
{
    scene->spheres[scene->sphere_count] = sphere;
    scene->sphere_count++;
}
