#pragma once

#include "renderer.h"
#include "scene.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    void (*create)(renderer_t *renderer);
    void (*render)(renderer_t *renderer, scene_t *scene);
    void (*destroy)(renderer_t *renderer);
} renderer_rasterization_t;

void renderer_rasterization_create(renderer_t *renderer)
{
}

void renderer_rasterization_render(renderer_t *renderer, scene_t *scene)
{
    fprintf(stderr, "Rasterization renderer hasn't been implemented yet, sorry :(\n");
    exit(EXIT_FAILURE);
}

void renderer_rasterization_destroy(renderer_t *renderer)
{
}
