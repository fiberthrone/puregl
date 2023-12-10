#pragma once

#include "scene.h"

typedef struct renderer_t
{
    void (*create)(struct renderer_t *context);
    void (*render)(struct renderer_t *context, scene_t *scene);
    void (*destroy)(struct renderer_t *context);
} renderer_t;
