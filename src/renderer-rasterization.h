#pragma once

#define Q(x) #x
#define QUOTE(x) Q(x)
#define LIGHTS_COUNT 2
#define LIGHTS_COUNT_STR QUOTE(LIGHTS_COUNT)

#include "renderer.h"
#include "scene.h"
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>

#define SPHERE_SECTOR_COUNT 128
#define SPHERE_STACK_COUNT 128
#define SPHERE_VERTEX_COUNT (2 + (SPHERE_SECTOR_COUNT + 1) * (SPHERE_STACK_COUNT - 1))
#define SPHERE_INDEX_COUNT (1 + SPHERE_SECTOR_COUNT * (1 + 2 * (SPHERE_STACK_COUNT - 1)))

typedef struct
{
    void (*create)(renderer_t *renderer);
    void (*render)(renderer_t *renderer, scene_t *scene);
    void (*destroy)(renderer_t *renderer);
    GLuint shader_program;
    GLuint plane_vao;
    GLuint plane_vbo;
    GLuint sphere_vao;
    GLuint sphere_vbo;
    GLuint sphere_ebo;
} renderer_rasterization_t;

void put_sphere_vertex(float **vertices, float x, float y, float z)
{
    *(*vertices)++ = x;
    *(*vertices)++ = y;
    *(*vertices)++ = z;
    *(*vertices)++ = x;
    *(*vertices)++ = y;
    *(*vertices)++ = z;
}

void put_index(unsigned int **indices, unsigned int index)
{
    *(*indices)++ = index;
}

void generate_sphere_vertices(int sector_count, int stack_count, float *vertices, unsigned int *indices)
{
    typedef enum
    {
        NORTH_TO_SOUTH,
        SOUTH_TO_NORTH
    } direction_t;

    float *vertices_start = vertices;
    unsigned int *indices_start = indices;

    float sector_step = 2 * M_PI / sector_count;
    float stack_step = M_PI / stack_count;

    int north_pole_index = 0;
    int south_pole_index = 1;
    int indices_per_sector = stack_count * 2 - 2;

    put_sphere_vertex(&vertices, 0.0f, 1.0f, 0.0f);
    put_sphere_vertex(&vertices, 0.0f, -1.0f, 0.0f);
    put_index(&indices, north_pole_index);

    for (int i = 0; i < sector_count + 1; i++)
    {
        direction_t direction = i % 2 == 1 ? NORTH_TO_SOUTH : SOUTH_TO_NORTH;
        float sector_angle = i * sector_step;

        for (int j = 1; j < stack_count; j++)
        {
            float stack_angle = M_PI / 2 - j * stack_step;

            float x = cosf(stack_angle) * cosf(sector_angle);
            float y = sinf(stack_angle);
            float z = cosf(stack_angle) * sinf(sector_angle);

            put_sphere_vertex(&vertices, x, y, z);

            if (i == 0)
            {
                continue;
            }

            int sector_index = i;
            int stack_index = direction == NORTH_TO_SOUTH ? j : stack_count - j;

            put_index(&indices, 2 + (sector_index - 1) * (indices_per_sector) + stack_index);
            put_index(&indices, 2 + sector_index * (indices_per_sector) + stack_index);
        }

        if (i > 0)
        {
            put_index(&indices, direction == NORTH_TO_SOUTH ? south_pole_index : north_pole_index);
        }
    }
}

void render_sphere(renderer_rasterization_t *renderer, object_t *object)
{
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_translate(model, object->position);
    glm_scale(model, (vec3){object->radius, object->radius, object->radius});
    glUniformMatrix4fv(glGetUniformLocation(renderer->shader_program, "model"), 1, GL_FALSE, &model[0][0]);
    glBindVertexArray(renderer->sphere_vao);

    glDrawElements(GL_TRIANGLE_STRIP, SPHERE_INDEX_COUNT, GL_UNSIGNED_INT, 0);
}

void render_plane(renderer_rasterization_t *renderer, object_t *object)
{
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_lookat((vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 0.0f, 1.0f}, object->normal, model);
    glm_translate(model, object->position);
    glm_scale(model, (vec3){100.0f, 100.0f, 100.0f});
    glUniformMatrix4fv(glGetUniformLocation(renderer->shader_program, "model"), 1, GL_FALSE, &model[0][0]);
    glBindVertexArray(renderer->plane_vao);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void render_object(renderer_rasterization_t *renderer, object_t *object)
{
    switch (object->type)
    {
    case OBJECT_TYPE_PLANE:
    {
        render_plane(renderer, object);
        break;
    }
    case OBJECT_TYPE_SPHERE:
    {
        render_sphere(renderer, object);
        break;
    }
    default:
    {
        fprintf(stderr, "Error: unknown object type %d\n", object->type);
        exit(EXIT_FAILURE);
    }
    }
}

void renderer_rasterization_create(renderer_t *renderer)
{
    renderer_rasterization_t *renderer_rasterization = (renderer_rasterization_t *)renderer;

    const char *vertex_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec3 a_position;\n"
        "layout (location = 1) in vec3 a_normal;\n"

        "uniform mat4 projection;\n"
        "uniform mat4 view;\n"
        "uniform mat4 model;\n"
        "out vec3 position;\n"
        "out vec3 normal;\n"

        "void main()\n"
        "{\n"
        "    gl_Position = projection * view * model * vec4(a_position, 1.0);\n"
        "    position = vec3(view * model * vec4(a_position, 1.0));\n"
        "    normal = normalize(vec3(view * model * vec4(a_normal, 0.0)));\n"
        "}\n";

    const char *fragment_shader_source =
        "#version 330 core\n"
        "#define LIGHTS_COUNT " LIGHTS_COUNT_STR "\n"
        "out vec4 FragColor;\n"
        "in vec3 normal;\n"
        "in vec3 position;\n"
        "uniform vec3 light_positions[LIGHTS_COUNT];\n"
        "uniform vec3 light_colors[LIGHTS_COUNT];\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "    for (int i = 0; i < LIGHTS_COUNT; i++)\n"
        "    {\n"
        "        vec3 light_direction = normalize(light_positions[i] - position);\n"
        "        float diffuse_intensity = max(dot(normal, light_direction), 0.0);\n"
        "        vec3 diffuse = diffuse_intensity * light_colors[i];\n"
        "        vec3 view_direction = normalize(-position);\n"
        "        vec3 halfway_direction = normalize(light_direction + view_direction);\n"
        "        float specular_intensity = pow(max(dot(normal, halfway_direction), 0.0), 128);\n"
        "        vec3 specular = specular_intensity * light_colors[i];\n"
        "        FragColor += vec4(diffuse + 0.3 * specular, 1.0);\n"
        "    }\n"
        "}\n";

    renderer_rasterization->shader_program = create_shader_program(vertex_shader_source, fragment_shader_source);
    glUseProgram(renderer_rasterization->shader_program);

    float plane_vertices[] = {
        -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f};

    glGenVertexArrays(1, &renderer_rasterization->plane_vao);
    glGenBuffers(1, &renderer_rasterization->plane_vbo);
    glBindVertexArray(renderer_rasterization->plane_vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer_rasterization->plane_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    ;
    static float sphere_vertices[SPHERE_VERTEX_COUNT * 6];
    static unsigned int sphere_indices[SPHERE_INDEX_COUNT];
    generate_sphere_vertices(SPHERE_SECTOR_COUNT, SPHERE_STACK_COUNT, sphere_vertices, sphere_indices);

    glGenVertexArrays(1, &renderer_rasterization->sphere_vao);
    glGenBuffers(1, &renderer_rasterization->sphere_vbo);
    glGenBuffers(1, &renderer_rasterization->sphere_ebo);
    glBindVertexArray(renderer_rasterization->sphere_vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer_rasterization->sphere_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer_rasterization->sphere_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_indices), sphere_indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void renderer_rasterization_render(renderer_t *renderer, scene_t *scene)
{
    renderer_rasterization_t *renderer_rasterization = (renderer_rasterization_t *)renderer;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int width = 640, height = 480;
    vec3 camera_position_world_space = {0.0f, 0.0f, -2.0f};
    mat4 projection;
    glm_perspective(45.0f, (float)width / (float)height, Z_NEAR, Z_FAR, projection);
    mat4 view;
    glm_lookat(camera_position_world_space, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f}, view);

    vec3 light_positions_view_space[scene->light_count];
    for (int i = 0; i < scene->light_count; i++)
    {
        glm_mat4_mulv3(view, (vec4){scene->lights[i].position[0], scene->lights[i].position[1], scene->lights[i].position[2], 1.0f}, 1.0f, light_positions_view_space[i]);
    }
    vec3 light_colors[scene->light_count];
    for (int i = 0; i < scene->light_count; i++)
    {
        light_colors[i][0] = scene->lights[i].color[0];
        light_colors[i][1] = scene->lights[i].color[1];
        light_colors[i][2] = scene->lights[i].color[2];
    }

    glUseProgram(renderer_rasterization->shader_program);
    glUniformMatrix4fv(glGetUniformLocation(renderer_rasterization->shader_program, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(renderer_rasterization->shader_program, "view"), 1, GL_FALSE, &view[0][0]);
    glUniform3fv(glGetUniformLocation(renderer_rasterization->shader_program, "light_positions"), 2, &light_positions_view_space[0][0]);
    glUniform3fv(glGetUniformLocation(renderer_rasterization->shader_program, "light_colors"), 2, &light_colors[0][0]);

    for (int i = 0; i < scene->object_count; i++)
    {
        render_object(renderer_rasterization, &scene->objects[i]);
    }
}

void renderer_rasterization_destroy(renderer_t *renderer)
{
    glDeleteVertexArrays(1, &((renderer_rasterization_t *)renderer)->plane_vao);
    glDeleteBuffers(1, &((renderer_rasterization_t *)renderer)->plane_vbo);
    glDeleteBuffers(1, &((renderer_rasterization_t *)renderer)->sphere_vbo);
    glDeleteBuffers(1, &((renderer_rasterization_t *)renderer)->sphere_ebo);
    glDeleteBuffers(1, &((renderer_rasterization_t *)renderer)->sphere_vao);
    glDeleteProgram(((renderer_rasterization_t *)renderer)->shader_program);
}
