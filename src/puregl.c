#include "scene.h"
#include "imaging.h"

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define Z_FAR 1.0f

float sdf_sphere(vec3 p, float radius)
{
    return glm_vec3_norm(p) - radius;
}

float sdf_plane(vec3 p, vec3 n, float h)
{
    return glm_vec3_dot(p, n) + h;
}

float sdf(vec3 p, object_t *object)
{
    switch (object->type)
    {
    case OBJECT_TYPE_SPHERE:
        return sdf_sphere(p, object->radius);
    case OBJECT_TYPE_PLANE:
        return sdf_plane(p, object->normal, 0.0f);
    default:
        return INFINITY;
    }
}

void get_object_normal(object_t *object, vec3 p, vec3 normal_dst)
{
    switch (object->type)
    {
    case OBJECT_TYPE_SPHERE:
        glm_vec3_copy(p, normal_dst);
        glm_vec3_normalize(normal_dst);
        break;
    case OBJECT_TYPE_PLANE:
        glm_vec3_copy(object->normal, normal_dst);
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

void blinn_phong_shade(vec3 hit_position, vec3 normal, vec3 light_position, vec3 camera_position, vec3 light_color, vec3 color_dst)
{
    vec3 light_direction;
    glm_vec3_sub(light_position, hit_position, light_direction);
    glm_vec3_normalize(light_direction);

    float diffuse_intensity = glm_max(glm_vec3_dot(normal, light_direction), 0.0f);
    vec3 diffuse;
    glm_vec3_scale(light_color, 1.0f * diffuse_intensity, diffuse);

    vec3 view_direction;
    glm_vec3_sub(camera_position, hit_position, view_direction);
    glm_vec3_normalize(view_direction);

    vec3 halfway_direction;
    glm_vec3_add(light_direction, view_direction, halfway_direction);
    glm_vec3_normalize(halfway_direction);

    float specular_intensity = powf(glm_max(glm_vec3_dot(normal, halfway_direction), 0.0f), 128.0f);
    vec3 specular;
    glm_vec3_scale(light_color, 0.3f * specular_intensity, specular);

    glm_vec3_add(diffuse, specular, color_dst);
}

hit_t cast_ray(vec3 origin, vec3 direction, scene_t *scene)
{
    float t = 0.0f;
    for (int i = 0; i < 100; i++)
    {
        vec3 ray_position;
        glm_vec3_scale(direction, t, ray_position);
        glm_vec3_add(origin, ray_position, ray_position);

        if (ray_position[2] > Z_FAR)
        {
            break;
        }

        float min_distance = INFINITY;
        for (int i = 0; i < scene->object_count; i++)
        {
            object_t *object = &scene->objects[i];

            vec3 ray_position_model_space;
            glm_vec3_sub(ray_position, object->position, ray_position_model_space);

            float d = sdf(ray_position_model_space, object);
            if (d < 0.001f)
            {
                return make_hit(object, ray_position);
            }

            if (d < min_distance)
            {
                min_distance = d;
            }
        }
        t += min_distance;
    }

    return (hit_t){.object = NULL};
}

void render_scene(scene_t *scene, unsigned char *image)
{
    int width = 640, height = 480;
    float pixel_size = 2.0f / glm_max(width, height);
    vec3 light_color = {1.0f, 1.0f, 1.0f};
    vec3 light_position = {1.0f, 1.0f, -1.0f};
    vec3 camera_position = {0.0f, 0.0f, -1.0f};

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            vec3 pixel_center = {
                pixel_size * (0.5f + x - width / 2),
                pixel_size * (0.5f + y - height / 2),
                0.0f};

            vec3 ray_direction;
            glm_vec3_sub(pixel_center, camera_position, ray_direction);
            glm_vec3_normalize(ray_direction);

            hit_t hit = cast_ray(pixel_center, ray_direction, scene);
            if (hit.object != NULL)
            {
                vec3 *object_position = &hit.object->position;
                vec3 *hit_position = &hit.position;

                vec3 hit_position_model_space;
                glm_vec3_sub(*hit_position, *object_position, hit_position_model_space);

                vec3 camera_position_model_space;
                glm_vec3_sub(camera_position, *object_position, camera_position_model_space);

                vec3 normal;
                get_object_normal(hit.object, hit_position_model_space, normal);

                vec3 light_position_model_space;
                glm_vec3_sub(light_position, *object_position, light_position_model_space);

                vec3 color;
                blinn_phong_shade(
                    hit_position_model_space, normal,
                    light_position_model_space,
                    camera_position_model_space,
                    light_color,
                    color);

                set_pixel(image, x, y, color);
            }
            else
            {
                set_pixel(image, x, y, (vec3){1.0f, 1.0f, 1.0f});
            }
        }
    }
}

void scene_init(scene_t *scene)
{
    scene_add_plane(scene, (vec3){0.0f, -1.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});

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
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    unsigned char test_image[640 * 480 * 3];
    scene_t scene;
    scene_init(&scene);

    clock_t start = clock();
    render_scene(&scene, test_image);
    clock_t end = clock();
    printf("Rendering time: %f\n", (double)(end - start) / CLOCKS_PER_SEC);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, test_image);

    const char *vertex_shader_source =
        "#version 330 core\n"
        "layout (location = 0) in vec3 a_pos;\n"
        "layout (location = 1) in vec2 a_tex_coord;\n"
        "out vec2 tex_coord;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(a_pos, 1.0);\n"
        "   tex_coord = a_tex_coord;\n"
        "}";

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);

    GLint success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
        fprintf(stderr, "Vertex shader compilation error: %s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    const char *fragment_shader_source =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 tex_coord;\n"
        "uniform sampler2D u_texture;\n"
        "void main()\n"
        "{\n"
        "   FragColor = texture(u_texture, tex_coord);\n"
        "}";

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
        fprintf(stderr, "Fragment shader compilation error: %s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        fprintf(stderr, "Shader program linking error: %s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "u_texture"), 0);

    float quad_vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

    GLuint quad_vao, quad_vbo;
    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glViewport(0, 0, 640, 480);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &quad_vao);
    glDeleteBuffers(1, &quad_vbo);
    glDeleteProgram(shader_program);

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
