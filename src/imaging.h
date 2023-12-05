#include <cglm/cglm.h>

void generate_test_image(unsigned char *image, int width, int height)
{
    int x, y;
    for (y = 0; y < height; y++)
    {
        unsigned char *row = &image[y * width * 3];
        for (x = 0; x < width; x++)
        {
            row[x * 3 + 0] = 255 * x / (width - 1);
            row[x * 3 + 1] = 255 * y / (height - 1);
            row[x * 3 + 2] = 255;
        }
    }
}

void set_pixel(unsigned char *image, int x, int y, vec3 color)
{
    image[(y * 640 + x) * 3 + 0] = 255 * glm_clamp(color[0], 0.0f, 1.0f);
    image[(y * 640 + x) * 3 + 1] = 255 * glm_clamp(color[1], 0.0f, 1.0f);
    image[(y * 640 + x) * 3 + 2] = 255 * glm_clamp(color[2], 0.0f, 1.0f);
}
