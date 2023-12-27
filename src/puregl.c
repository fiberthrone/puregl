#include "renderer-ray-tracing.h"
#include "renderer-rasterization.h"
#include "scene.h"

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void scene_init(scene_t *scene)
{
    *scene = (scene_t){0};

    scene_add_light(scene, (vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 0.5f, 0.5f}, 1.0f);
    scene_add_light(scene, (vec3){5.0f, 5.0f, 0.0f}, (vec3){0.5f, 0.5f, 1.0f}, 1.0f);

    scene_add_plane(scene, (vec3){0.0f, -1.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});

    scene_add_cube(scene, (vec3){0.5f, -0.7f, 2.0f}, (vec3){0.6f, 0.6f, 0.6f});
    scene_add_cube(scene, (vec3){1.5f, -0.2f, 1.0f}, (vec3){0.8f, 1.6f, 0.8f});

    scene_add_sphere(scene, (vec3){-1.0f, -1.0, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-1.0f, -0.6, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-1.0f, -0.2, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-1.0f, 0.2, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-1.0f, 0.6, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-1.0f, 1.0, 0.5f}, 0.2f);

    scene_add_sphere(scene, (vec3){-0.6f, -1.0, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.6f, -0.6, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.6f, -0.2, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.6f, 0.2, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.6f, 0.6, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.6f, 1.0, 0.5f}, 0.2f);

    scene_add_sphere(scene, (vec3){-0.2f, -1.0, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.2f, -0.6, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.2f, -0.2, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.2f, 0.2, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.2f, 0.6, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.2f, 1.0, 0.5f}, 0.2f);

    scene_add_sphere(scene, (vec3){-0.2f, -1.0, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.2f, -0.6, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.2f, -0.2, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.2f, 0.2, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.2f, 0.6, 0.5f}, 0.2f);
    scene_add_sphere(scene, (vec3){-0.2f, 1.0, 0.5f}, 0.2f);
}

void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

renderer_ray_tracing_t renderer_ray_tracing = {
    .create = renderer_ray_tracing_create,
    .render = renderer_ray_tracing_render,
    .destroy = renderer_ray_tracing_destroy,
};
renderer_rasterization_t renderer_rasterization = {
    .create = renderer_rasterization_create,
    .render = renderer_rasterization_render,
    .destroy = renderer_rasterization_destroy,
};
renderer_t *renderer_current = (renderer_t *)&renderer_ray_tracing;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
    {
        return;
    }

    switch (key)
    {
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case GLFW_KEY_TAB:
        renderer_current->destroy(renderer_current);
        if (renderer_current == (renderer_t *)&renderer_ray_tracing)
        {
            renderer_current = (renderer_t *)&renderer_rasterization;
        }
        else
        {
            renderer_current = (renderer_t *)&renderer_ray_tracing;
        }
        renderer_current->create(renderer_current);
        break;
    }
}

int main()
{
    GLFWwindow *window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Demo", NULL, NULL);

    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    int window_width;
    int window_height;
    glfwGetFramebufferSize(window, &window_width, &window_height);
    glViewport(0, 0, window_width, window_height);

    scene_t scene;
    scene_init(&scene);

    renderer_current->create(renderer_current);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        renderer_current->render(renderer_current, &scene);

        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    renderer_current->destroy(renderer_current);

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
