#include <math.h>
#include "image.h"

float nn_interpolate(image im, float x, float y, int c)
{
    // TODO Fill in
    int new_x = floor(x);
    int new_y = floor(y);

    if (new_x < 0) {
        new_x = 0;
    }
    if (new_y < 0) {
        new_y = 0;
    }

    return get_pixel(im, new_x, new_y, c);
}

image nn_resize(image im, int w, int h)
{
    // TODO Fill in (also fix that first line)
    image res = make_image(w, h, im.c);
    float ratio_w = ((float)w / (float)im.w);
    float ratio_h = ((float)h / (float)im.h);

    for (int c = 0; c < im.c; c++) {
        for (int i = 0; i < w; i++) {
            for (int j = 0; j < h; j++) {
                float x = (i + 0.5) / ratio_w;
                float y = (j + 0.5) / ratio_h;
                set_pixel(res, i, j, c, nn_interpolate(im, x, y, c));
            }
        }
    }

    return res;
}

float bilinear_interpolate(image im, float x, float y, int c)
{
    // YOUR CODE HERE
    int upX = ceilf(x);
    int lowX = floorf(x);
    int upY = ceilf(y);
    int lowY = floorf(y);

    float V1 = get_pixel(im, lowX, lowY, c);
    float V2 = get_pixel(im, upX, lowY, c);
    float V3 = get_pixel(im, lowX, upY, c);
    float V4 = get_pixel(im, upX, upY, c);

    return V1 * (upX - x) * (upY - y)
            + V4 * (x - lowX) * (y - lowY) 
            + V3 * (upX - x) * (y - lowY)
            + V2 * (x - lowX) * (upY - y);
}

image bilinear_resize(image im, int w, int h)
{
    // TODO
    image res = make_image(w, h, im.c);
    float ratio_w = ((float)w / (float)im.w);
    float ratio_h = ((float)h / (float)im.h);
    float b_w = -0.5 + 0.5 / ratio_w;
    float b_h = -0.5 + 0.5 / ratio_h;
    for (int c = 0; c < im.c; c++) {
        for (int i = 0; i < w; i++) {
            for (int j = 0; j < h; j++) {
                float x = i / ratio_w + b_w;
                float y = j / ratio_h + b_h;

                set_pixel(res, i, j, c, bilinear_interpolate(im, x, y, c));
            }
        }
    }
    
    return res;
}

