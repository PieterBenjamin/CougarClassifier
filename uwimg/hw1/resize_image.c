#include <math.h>
#include "../image.h"

typedef float(*interpolate_fun)(image, float, float, int);

image resize(image im, int w, int h, interpolate_fun f) {
    image new_im = make_image(w, h, im.c);
    float row, col, chan;

    for (chan = 0; chan < im.c; chan++) {
        for (row = 0; row < h; row++) {
            for (col= 0; col < w; col++) {
                set_pixel(new_im,
                          col,
                          row,
                          chan, 
                          (*f)(im, 
                               ((col + 0.5) * im.w)/w - 0.5,
                               ((row + 0.5) * im.h)/h - 0.5,
                               chan));
            }
        }
    }
    return new_im;
}

float nn_interpolate(image im, float x, float y, int c)
{
    return get_pixel(im, (int)round(x), (int)round(y), c);
}

image nn_resize(image im, int w, int h)
{
    return resize(im, w, h, &nn_interpolate);
}

float bilinear_interpolate(image im, float x, float y, int c)
{
    int topleft_x, topleft_y;
    float a1, a2, a3, a4, p1, p2, p3, p4;

    // get the coordinates in the left corner above (x, y)
    topleft_x = (int)floorf(x);
    topleft_y = (int)floorf(y);

    // going in counter-clockwise order
    p1 = get_pixel(im, topleft_x, topleft_y, c);
    p2 = get_pixel(im, topleft_x, topleft_y + 1, c);
    p3 = get_pixel(im, topleft_x + 1, topleft_y + 1, c);
    p4 = get_pixel(im, topleft_x + 1, topleft_y, c);

    // area of squares in counter-clockwise order
    a1 = (x - topleft_x) * (y - topleft_y);
    a2 = (x - topleft_x) * (topleft_y + 1 - y);
    a3 = (topleft_x + 1 - x) * (topleft_y + 1 - y);
    a4 = (topleft_x + 1 - x) * (y - topleft_y);

    return (p1 * a3) + (p2 * a4) + (p3 * a1) + (p4 * a2);
}

image bilinear_resize(image im, int w, int h)
{
    return resize(im, w, h, &bilinear_interpolate);
}

