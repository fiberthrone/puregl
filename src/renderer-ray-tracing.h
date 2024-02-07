#pragma once

#include "renderer.h"
#include "scene.h"
#include "imaging.h"
#include "gl-utils.h"
#include "math.h"

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define Z_NEAR 0.1f
#define Z_FAR 100.0f

void get_object_normal(object_t *object, vec3 p, vec3 normal_dst)
{
    vec3 p_normalized;

    switch (object->type)
    {
    case OBJECT_TYPE_SPHERE:
        glm_vec3_copy(p, normal_dst);
        glm_vec3_normalize(normal_dst);
        break;
    case OBJECT_TYPE_CUBE:
        glm_vec3_copy(p, p_normalized);
        glm_vec3_div(p_normalized, object->size, p_normalized);
        if (fabs(p_normalized[0]) > fabs(p_normalized[1]) && fabs(p_normalized[0]) > fabs(p_normalized[2]))
        {
            glm_vec3_copy((vec3){glm_signf(p_normalized[0]), 0.0f, 0.0f}, normal_dst);
        }
        else if (fabs(p_normalized[1]) > fabs(p_normalized[2]))
        {
            glm_vec3_copy((vec3){0.0f, glm_signf(p_normalized[1]), 0.0f}, normal_dst);
        }
        else
        {
            glm_vec3_copy((vec3){0.0f, 0.0f, glm_signf(p_normalized[2])}, normal_dst);
        }
        break;
    case OBJECT_TYPE_PLANE:
        glm_vec3_copy(object->normal, normal_dst);
        break;
    default:
        break;
    }
}

void intersects_sphere(vec3 ray_origin, vec3 ray_direction, vec3 sphere_position, float sphere_radius, float *t0_dst, float *t1_dst)
{
    vec3 ray_origin_model_space;
    glm_vec3_sub(ray_origin, sphere_position, ray_origin_model_space);

    float a = glm_vec3_norm2(ray_direction);
    float b = 2.0f * glm_vec3_dot(ray_direction, ray_origin_model_space);
    float c = glm_vec3_norm2(ray_origin_model_space) - sphere_radius * sphere_radius;

    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f)
    {
        *t0_dst = INFINITY;
        *t1_dst = INFINITY;
        return;
    }

    float sqrt_discriminant = sqrtf(discriminant);
    float q = b < 0.0f ? -0.5f * (b - sqrt_discriminant) : -0.5f * (b + sqrt_discriminant);
    *t0_dst = q / a;
    *t1_dst = c / q;
}

void intersects_plane(vec3 ray_origin, vec3 ray_direction, vec3 plane_normal, float *t0_dst, float *t1_dst)
{
    float denominator = glm_vec3_dot(plane_normal, ray_direction);
    if (fabsf(denominator) < 0.0001f)
    {
        *t0_dst = INFINITY;
        *t1_dst = INFINITY;
        return;
    }

    float t = -glm_vec3_dot(ray_origin, plane_normal) / denominator;
    *t0_dst = t;
    *t1_dst = t;
}

void intersects_cube(vec3 ray_origin, vec3 ray_direction, vec3 cube_position, vec3 cube_size, float *t0_dst, float *t1_dst)
{
    float t_near = -INFINITY;
    float t_far = INFINITY;

    for (int i = 0; i < 3; i++)
    {
        if (ray_direction[i] == 0.0f)
        {
            if (ray_origin[i] < cube_position[i] - cube_size[i] / 2.0f || ray_origin[i] > cube_position[i] + cube_size[i] / 2.0f)
            {
                *t0_dst = INFINITY;
                *t1_dst = INFINITY;
                return;
            }
        }
        else
        {
            float t1 = (cube_position[i] - cube_size[i] / 2.0f - ray_origin[i]) / ray_direction[i];
            float t2 = (cube_position[i] + cube_size[i] / 2.0f - ray_origin[i]) / ray_direction[i];

            if (t1 > t2)
            {
                float tmp = t1;
                t1 = t2;
                t2 = tmp;
            }

            if (t1 > t_near)
            {
                t_near = t1;
            }

            if (t2 < t_far)
            {
                t_far = t2;
            }

            if (t_near > t_far)
            {
                *t0_dst = INFINITY;
                *t1_dst = INFINITY;
                return;
            }

            if (t_far < 0.0f)
            {
                *t0_dst = INFINITY;
                *t1_dst = INFINITY;
                return;
            }
        }
    }

    *t0_dst = t_near;
    *t1_dst = t_far;
}

void intersects(vec3 ray_origin, vec3 ray_direction, object_t *object, float *t0_dst, float *t1_dst)
{
    vec3 ray_origin_model_space;

    switch (object->type)
    {
    case OBJECT_TYPE_SPHERE:
        intersects_sphere(ray_origin, ray_direction, object->position, object->radius, t0_dst, t1_dst);
        break;
    case OBJECT_TYPE_CUBE:
        glm_vec3_sub(ray_origin, object->position, ray_origin_model_space);
        intersects_cube(ray_origin_model_space, ray_direction, (vec3){0.0f, 0.0f, 0.0f}, object->size, t0_dst, t1_dst);
        break;
    case OBJECT_TYPE_PLANE:
        glm_vec3_sub(ray_origin, object->position, ray_origin_model_space);
        intersects_plane(ray_origin_model_space, ray_direction, object->normal, t0_dst, t1_dst);
        break;
    default:
        break;
    }
}

typedef struct
{
    object_t *object;
    vec3 position;
} hit_t;

hit_t make_hit(object_t *object, vec3 position)
{
    hit_t hit = (hit_t){.object = object};
    glm_vec3_copy(position, hit.position);
    return hit;
}

void blinn_phong_shade(vec3 hit_position, vec3 normal, vec4 light_position, vec3 camera_position, vec3 light_color, material_t *material, vec3 color_dst)
{
    vec3 light_direction;
    if (light_position[3] == 0.0f)
    {
        glm_vec3_copy(light_position, light_direction);
    }
    else
    {
        glm_vec3_sub(light_position, hit_position, light_direction);
        glm_vec3_normalize(light_direction);
    }

    float diffuse_intensity = glm_max(glm_vec3_dot(normal, light_direction), 0.0f);
    vec3 diffuse;
    glm_vec3_scale(light_color, diffuse_intensity, diffuse);
    glm_vec3_mul(diffuse, material->base_color, diffuse);

    vec3 view_direction;
    glm_vec3_sub(camera_position, hit_position, view_direction);
    glm_vec3_normalize(view_direction);

    vec3 halfway_direction;
    glm_vec3_add(light_direction, view_direction, halfway_direction);
    glm_vec3_normalize(halfway_direction);

    float specular_intensity = powf(glm_max(glm_vec3_dot(normal, halfway_direction), 0.0f), material->shininess);
    vec3 specular;
    glm_vec3_scale(light_color, material->specular * specular_intensity, specular);

    glm_vec3_add(diffuse, specular, color_dst);
}

void cast_ray(vec3 origin, vec3 direction, scene_t *scene, float max_distance, hit_t *hit_dst)
{
    float t = max_distance;
    object_t *hit_object = NULL;

    for (int i = 0; i < scene->object_count; i++)
    {
        object_t *object = &scene->objects[i];

        float t0, t1;
        intersects(origin, direction, object, &t0, &t1);

        if (t0 > t1)
        {
            float tmp = t0;
            t0 = t1;
            t1 = tmp;
        }

        if (t0 < 0.0f)
        {
            t0 = t1;
            if (t0 < 0.0f)
            {
                continue;
            }
        }

        if (t0 < t)
        {
            t = t0;
            hit_object = object;
        }
    }

    if (hit_object != NULL)
    {
        vec3 ray_position;
        glm_vec3_scale(direction, t, ray_position);
        glm_vec3_add(origin, ray_position, ray_position);

        hit_dst->object = hit_object;
        glm_vec3_copy(ray_position, hit_dst->position);
    }
    else
    {
        hit_dst->object = NULL;
    }
}

void render_to_image(scene_t *scene, unsigned char *image)
{
    int width = 640, height = 480;
    static struct
    {
        int total_sample_count;
        int hit_count;
    } shadow_cache[640][480][MAX_LIGHT_COUNT] = {0};
    static struct
    {
        int total_sample_count;
        vec3 value;
    } bounce_cache[640][480] = {0};

    static unsigned int last_id = 0;
    if (last_id != scene->id)
    {
        // Invalidate cache
        memset(shadow_cache, 0, sizeof(shadow_cache));
        memset(bounce_cache, 0, sizeof(bounce_cache));
        last_id = scene->id;
    }

    mat4 projection;
    glm_perspective(45.0f, (float)width / (float)height, Z_NEAR, Z_FAR, projection);
    mat4 view;
    glm_look(scene->camera.position, scene->camera.direction, scene->camera.up, view);
    mat4 projection_view;
    glm_mat4_mul(projection, view, projection_view);

    mat4 projection_inv = {0.0f};
    glm_mat4_inv(projection, projection_inv);
    mat4 view_inv = {0.0f};
    glm_mat4_inv(view, view_inv);

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            vec4 pixel_center_clip_space;
            viewport_transform_inverse((vec2){0.5f + x, 0.5f + y}, (vec2){width, height}, pixel_center_clip_space);
            pixel_center_clip_space[2] = -1.0f;
            pixel_center_clip_space[3] = 1.0f;
            vec4 pixel_center_view_space;
            glm_mat4_mulv(projection_inv, pixel_center_clip_space, pixel_center_view_space);
            vec4 pixel_center_world_space;
            glm_mat4_mulv(view_inv, pixel_center_view_space, pixel_center_world_space);
            perspective_division(pixel_center_world_space, pixel_center_world_space);

            vec4 ray_direction_view_space;
            glm_vec4_copy(pixel_center_view_space, ray_direction_view_space);
            // Setting w to 0 prevents translation, so it behaves like a direction vector
            ray_direction_view_space[3] = 0.0f;

            vec4 ray_direction_world_space;
            glm_mat4_mulv(view_inv, ray_direction_view_space, ray_direction_world_space);
            glm_vec4_normalize(ray_direction_world_space);

            vec3 color = {0.0f, 0.0f, 0.0f};

            hit_t hit;
            cast_ray(pixel_center_world_space, ray_direction_world_space, scene, INFINITY, &hit);

            if (hit.object != NULL)
            {
                vec3 *object_position = &hit.object->position;
                vec3 *hit_position = &hit.position;

                vec3 hit_position_model_space;
                glm_vec3_sub(*hit_position, *object_position, hit_position_model_space);

                vec3 *camera_position_world_space = &scene->camera.position;
                vec3 camera_position_model_space;
                glm_vec3_sub(*camera_position_world_space, *object_position, camera_position_model_space);

                vec3 normal;
                get_object_normal(hit.object, hit_position_model_space, normal);

                for (int i = 0; i < scene->light_count; i++)
                {
                    light_t *light = &scene->lights[i];

                    int total_sample_count = shadow_cache[x][y][i].total_sample_count;
                    int light_hit_count = shadow_cache[x][y][i].hit_count;

                    int SAMPLES_TAKEN_PER_FRAME = 1;
                    for (int j = 0; j < SAMPLES_TAKEN_PER_FRAME; j++)
                    {
                        vec3 direction_to_light;
                        float light_distance;
                        if (light->position[3] == 0.0f)
                        {
                            glm_vec3_copy(light->position, direction_to_light);
                            vec3 offset = {
                                (float)rand() / (float)RAND_MAX * 2.0f - 1.0f,
                                (float)rand() / (float)RAND_MAX * 2.0f - 1.0f,
                                (float)rand() / (float)RAND_MAX * 2.0f - 1.0f};
                            glm_vec3_scale(offset, light->angular_radius, offset);
                            glm_vec3_add(direction_to_light, offset, direction_to_light);
                            glm_vec3_normalize(direction_to_light);
                            light_distance = INFINITY;
                        }
                        else
                        {
                            vec3 light_sample_position;
                            glm_vec3_copy(light->position, light_sample_position);
                            vec3 offset = {
                                (float)rand() / (float)RAND_MAX * 2.0f - 1.0f,
                                (float)rand() / (float)RAND_MAX * 2.0f - 1.0f,
                                (float)rand() / (float)RAND_MAX * 2.0f - 1.0f};
                            glm_vec3_scale(offset, light->radius, offset);
                            glm_vec3_add(light_sample_position, offset, light_sample_position);

                            glm_vec3_sub(light_sample_position, hit.position, direction_to_light);
                            light_distance = glm_vec3_norm(direction_to_light);
                            glm_vec3_normalize(direction_to_light);
                        }

                        // FIXME: is this good?
                        vec3 light_ray_origin;
                        glm_vec3_scale(direction_to_light, 0.0001f, light_ray_origin);
                        glm_vec3_add(hit.position, light_ray_origin, light_ray_origin);
                        hit_t light_hit;
                        cast_ray(light_ray_origin, direction_to_light, scene, light_distance, &light_hit);

                        total_sample_count = ++shadow_cache[x][y][i].total_sample_count;
                        if (light_hit.object == NULL)
                        {
                            light_hit_count = ++shadow_cache[x][y][i].hit_count;
                        }
                    }

                    if (light_hit_count == 0)
                    {
                        continue;
                    }

                    vec3 *object_position = &hit.object->position;
                    vec3 *hit_position = &hit.position;

                    vec3 hit_position_model_space;
                    glm_vec3_sub(*hit_position, *object_position, hit_position_model_space);

                    vec3 camera_position_model_space;
                    glm_vec3_sub(*camera_position_world_space, *object_position, camera_position_model_space);

                    vec3 normal;
                    get_object_normal(hit.object, hit_position_model_space, normal);

                    vec4 light_position_model_space;
                    glm_vec4_copy(light->position, light_position_model_space);
                    if (light_position_model_space[3] == 1.0f)
                    {
                        glm_vec4_sub(light_position_model_space, (vec4){*object_position[0], *object_position[1], *object_position[2], 0.0f}, light_position_model_space);
                    }

                    vec3 light_contribution_color;
                    blinn_phong_shade(
                        hit_position_model_space,
                        normal,
                        light_position_model_space,
                        camera_position_model_space,
                        light->color,
                        &hit.object->material,
                        light_contribution_color);

                    glm_vec3_scale(light_contribution_color, (float)light_hit_count / total_sample_count, light_contribution_color);
                    glm_vec3_add(color, light_contribution_color, color);
                }

                int BOUNCE_SAMPLE_COUNT = 1;
                vec3 bounce_contribution = {0};
                for (int i = 0; i < BOUNCE_SAMPLE_COUNT; i++)
                {
                    vec3 random_direction = {
                        (float)rand() / (float)RAND_MAX * 2.0f - 1.0f,
                        (float)rand() / (float)RAND_MAX * 2.0f - 1.0f,
                        (float)rand() / (float)RAND_MAX * 2.0f - 1.0f};
                    glm_vec3_normalize(random_direction);
                    vec3 bounce_ray_origin;
                    glm_vec3_scale(random_direction, 0.0001f, bounce_ray_origin);
                    glm_vec3_add(hit.position, bounce_ray_origin, bounce_ray_origin);
                    hit_t bounce_hit;
                    cast_ray(bounce_ray_origin, random_direction, scene, INFINITY, &bounce_hit);
                    if (bounce_hit.object != NULL && bounce_hit.object != hit.object)
                    {
                        vec3 bounce_value_sample = {};
                        for (int i = 0; i < scene->light_count; i++)
                        {
                            light_t *light = &scene->lights[i];

                            vec3 direction_to_light;
                            glm_vec3_sub(light->position, bounce_hit.position, direction_to_light);
                            float light_distance = glm_vec3_norm(direction_to_light);
                            glm_vec3_normalize(direction_to_light);

                            // FIXME: is this good?
                            vec3 light_ray_origin;
                            glm_vec3_scale(direction_to_light, 0.0001f, light_ray_origin);
                            glm_vec3_add(bounce_hit.position, light_ray_origin, light_ray_origin);

                            hit_t light_hit;
                            cast_ray(light_ray_origin, direction_to_light, scene, light_distance, &light_hit);

                            if (light_hit.object == NULL)
                            {
                                vec3 *bounce_object_position = &bounce_hit.object->position;
                                vec3 *bounce_hit_position = &bounce_hit.position;

                                vec3 bounce_hit_position_bounce_model_space;
                                glm_vec3_sub(*bounce_hit_position, *bounce_object_position, bounce_hit_position_bounce_model_space);

                                vec3 camera_position_model_space;
                                glm_vec3_sub(*camera_position_world_space, *bounce_object_position, camera_position_model_space);

                                vec3 bounce_normal;
                                get_object_normal(bounce_hit.object, bounce_hit_position_bounce_model_space, bounce_normal);

                                vec4 bounce_light_position_bounce_model_space;
                                glm_vec4_copy(light->position, bounce_light_position_bounce_model_space);
                                if (bounce_light_position_bounce_model_space[3] == 1.0f)
                                {
                                    glm_vec4_sub(bounce_light_position_bounce_model_space, (vec4){*bounce_object_position[0], *bounce_object_position[1], *bounce_object_position[2], 0.0f}, bounce_light_position_bounce_model_space);
                                }

                                vec3 hit_position_bounce_model_space;
                                glm_vec3_sub(*hit_position, *bounce_object_position, hit_position_bounce_model_space);

                                vec3 bounce_value_sample_light_contribution = {0};
                                blinn_phong_shade(
                                    bounce_hit_position_bounce_model_space,
                                    bounce_normal,
                                    bounce_light_position_bounce_model_space,
                                    hit_position_bounce_model_space,
                                    light->color,
                                    &bounce_hit.object->material,
                                    bounce_value_sample_light_contribution);

                                glm_vec3_add(bounce_value_sample, bounce_value_sample_light_contribution, bounce_value_sample);
                            }
                        }

                        vec4 bounce_hit_position_model_space = {0.0f, 0.0f, 0.0f, 1.0f};
                        glm_vec3_sub(bounce_hit.position, *object_position, bounce_hit_position_model_space);

                        vec3 bounce_contribution_sample = {0};
                        blinn_phong_shade(
                            hit_position_model_space,
                            normal,
                            bounce_hit_position_model_space,
                            camera_position_model_space,
                            bounce_value_sample,
                            &hit.object->material,
                            bounce_contribution_sample);

                        glm_vec3_add(bounce_contribution, bounce_contribution_sample, bounce_contribution);
                    }
                }

                glm_vec3_add(bounce_cache[x][y].value, bounce_contribution, bounce_cache[x][y].value);
                bounce_cache[x][y].total_sample_count += BOUNCE_SAMPLE_COUNT;
                glm_vec3_scale(bounce_cache[x][y].value, 1.0f / (float)bounce_cache[x][y].total_sample_count, bounce_contribution);
                glm_vec3_add(color, bounce_contribution, color);
            }
            set_pixel(image, x, y, color);
        }
    }
}

typedef struct
{
    void (*create)(renderer_t *renderer);
    void (*render)(renderer_t *renderer, scene_t *scene);
    void (*destroy)(renderer_t *renderer);
    GLuint texture;
    GLuint shader_program;
    GLuint quad_vao;
    GLuint quad_vbo;
} renderer_ray_tracing_t;

void renderer_ray_tracing_create(renderer_t *renderer)
{
    renderer_ray_tracing_t *renderer_ray_tracing = (renderer_ray_tracing_t *)renderer;

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    glGenTextures(1, &renderer_ray_tracing->texture);
    glBindTexture(GL_TEXTURE_2D, renderer_ray_tracing->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    const char *vertex_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec3 a_pos;\n"
        "layout (location = 1) in vec2 a_tex_coord;\n"
        "out vec2 tex_coord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(a_pos, 1.0);\n"
        "    tex_coord = a_tex_coord;\n"
        "}";

    const char *fragment_shader_source =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 tex_coord;\n"
        "uniform sampler2D u_texture;\n"
        "void main()\n"
        "{\n"
        "    FragColor = texture(u_texture, tex_coord);\n"
        "}";

    renderer_ray_tracing->shader_program = create_shader_program(vertex_shader_source, fragment_shader_source);
    glUseProgram(renderer_ray_tracing->shader_program);
    glUniform1i(glGetUniformLocation(renderer_ray_tracing->shader_program, "u_texture"), 0);

    float quad_vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

    glGenVertexArrays(1, &renderer_ray_tracing->quad_vao);
    glGenBuffers(1, &renderer_ray_tracing->quad_vbo);
    glBindVertexArray(renderer_ray_tracing->quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer_ray_tracing->quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void renderer_ray_tracing_render(renderer_t *renderer, scene_t *scene)
{
    unsigned char image[640 * 480 * 3];
    render_to_image(scene, image);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ((renderer_ray_tracing_t *)renderer)->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void renderer_ray_tracing_destroy(renderer_t *renderer)
{
    glDeleteVertexArrays(1, &((renderer_ray_tracing_t *)renderer)->quad_vao);
    glDeleteBuffers(1, &((renderer_ray_tracing_t *)renderer)->quad_vbo);
    glDeleteProgram(((renderer_ray_tracing_t *)renderer)->shader_program);
}
