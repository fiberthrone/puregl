#include "renderer-ray-tracing.h"
#include "renderer-rasterization.h"
#include "scene.h"
#include "camera.h"

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct
{
    bool dragging;
    vec2 drag_start_position;
    camera_t drag_start_camera;
} ui_state_t;

typedef struct
{
    scene_t *scene;
    ui_state_t *ui_state;
} window_context_t;

void scene_init(scene_t *scene)
{
    *scene = (scene_t){0};

    camera_t camera = {
        .position = {0.0f, 0.0f, -2.0f},
        .direction = {0.0f, 0.0f, 1.0f},
        .up = {0.0f, 1.0f, 0.0f},
        .target = {0.0f, 0.0f, 0.0f}};
    scene_set_camera(scene, &camera);

    scene_add_light(scene, (vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 0.5f, 0.5f}, 1.0f, 1.0f);
    scene_add_light(scene, (vec3){5.0f, 5.0f, 0.0f}, (vec3){0.5f, 0.5f, 1.0f}, 1.0f, 1.0f);

    material_t material_yellow = {.base_color = {1.0f, 1.0f, 0.0f}, .specular = 0.3f, .shininess = 128.0f};
    material_t material_white = {.base_color = {1.0f, 1.0f, 1.0f}, .specular = 0.3f, .shininess = 128.0f};

    scene_add_plane(scene, (vec3){0.0f, -1.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f}, material_yellow);

    scene_add_cube(scene, (vec3){0.5f, -0.7f, 2.0f}, (vec3){0.6f, 0.6f, 0.6f}, material_white);
    scene_add_cube(scene, (vec3){1.5f, -0.2f, 1.0f}, (vec3){0.8f, 1.6f, 0.8f}, material_white);

    scene_add_sphere(scene, (vec3){-1.0f, -1.0, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-1.0f, -0.6, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-1.0f, -0.2, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-1.0f, 0.2, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-1.0f, 0.6, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-1.0f, 1.0, 0.5f}, 0.2f, material_white);

    scene_add_sphere(scene, (vec3){-0.6f, -1.0, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-0.6f, -0.6, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-0.6f, -0.2, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-0.6f, 0.2, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-0.6f, 0.6, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-0.6f, 1.0, 0.5f}, 0.2f, material_white);

    scene_add_sphere(scene, (vec3){-0.2f, -1.0, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-0.2f, -0.6, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-0.2f, -0.2, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-0.2f, 0.2, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-0.2f, 0.6, 0.5f}, 0.2f, material_white);
    scene_add_sphere(scene, (vec3){-0.2f, 1.0, 0.5f}, 0.2f, material_white);
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

    window_context_t *window_context = (window_context_t *)glfwGetWindowUserPointer(window);
    scene_t *scene = window_context->scene;
    camera_t *camera = &scene->camera;
    camera_t new_camera;

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
    case GLFW_KEY_W:
        camera_move_forwards(camera, 0.1f, &new_camera);
        scene_set_camera(scene, &new_camera);
        break;
    case GLFW_KEY_S:
        camera_move_backwards(camera, 0.1f, &new_camera);
        scene_set_camera(scene, &new_camera);
        break;
    case GLFW_KEY_A:
        camera_move_left(camera, 0.1f, &new_camera);
        scene_set_camera(scene, &new_camera);
        break;
    case GLFW_KEY_D:
        camera_move_right(camera, 0.1f, &new_camera);
        scene_set_camera(scene, &new_camera);
        break;
    }
}

void mouse_move_callback(GLFWwindow *window, double xpos, double ypos)
{
    window_context_t *window_context = (window_context_t *)glfwGetWindowUserPointer(window);
    scene_t *scene = window_context->scene;
    ui_state_t *ui_state = window_context->ui_state;
    camera_t new_camera;

    if (!ui_state->dragging)
    {
        return;
    }

    float delta_theta = (xpos - ui_state->drag_start_position[0]) / 100.0f;
    float delta_phi = (ypos - ui_state->drag_start_position[1]) / 100.0f;
    camera_rotate(&ui_state->drag_start_camera, delta_theta, delta_phi, &new_camera);
    scene_set_camera(scene, &new_camera);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    window_context_t *window_context = (window_context_t *)glfwGetWindowUserPointer(window);
    scene_t *scene = window_context->scene;
    ui_state_t *ui_state = window_context->ui_state;

    if (button != GLFW_MOUSE_BUTTON_LEFT)
    {
        return;
    }

    if (action == GLFW_PRESS)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        ui_state->dragging = true;
        ui_state->drag_start_camera = scene->camera;
        vec2 mouse_pos = {xpos, ypos};
        glm_vec2_copy(mouse_pos, ui_state->drag_start_position);
    }
    else if (action == GLFW_RELEASE)
    {
        ui_state->dragging = false;
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
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, mouse_move_callback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    int window_width;
    int window_height;
    glfwGetFramebufferSize(window, &window_width, &window_height);
    glViewport(0, 0, window_width, window_height);

    scene_t scene;
    ui_state_t ui_state = {0};
    window_context_t window_context = {.scene = &scene, .ui_state = &ui_state};
    scene_init(&scene);

    glfwSetWindowUserPointer(window, &window_context);

    renderer_current->create(renderer_current);

    double previousTime = glfwGetTime();
    int frameCount = 0;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        renderer_current->render(renderer_current, &scene);

        glfwSwapBuffers(window);

        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - previousTime >= 1.0)
        {
            fprintf(stderr, "%d fps\n", frameCount);
            frameCount = 0;
            previousTime = currentTime;
        }

        /* Poll for and process events */
        glfwPollEvents();
    }

    renderer_current->destroy(renderer_current);

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
