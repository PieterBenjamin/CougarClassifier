#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"


// column x (im.w)
// row y (im.h)
// channel c (im.c)
float get_pixel(image im, int x, int y, int c)
{
    // TODO Fill this in
    if (x < 0) {
        x = 0;
    }

    if (y < 0) {
        y = 0;
    }

    if (c < 0) {
        c = 0;
    }

    if (x >= im.w) {
        x = im.w - 1;
    }

    if (y >= im.h) {
        y = im.h - 1;
    }

    if (c >= im.c) {
        c = im.c - 1;
    }

    int index = x + y * im.w + c * im.w * im.h;

    return (im.data)[index];
}

void set_pixel(image im, int x, int y, int c, float v)
{
    // TODO Fill this in
    if (x < 0 || x >= im.w || y < 0 || y >= im.h || c < 0 || c >= im.c) {
        return;
    }

    int index = x + y * im.w + c * im.w * im.h;

    (im.data)[index] = v;
}

image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    // TODO Fill this in

    memcpy(copy.data, im.data, (sizeof (*im.data) * im.w * im.h * im.c));
    return copy;
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    image gray = make_image(im.w, im.h, 1);
    // TODO Fill this in

    for (int row = 0; row < im.h; row++) {
        for (int column = 0; column < im.w; column++) {
            float red = get_pixel(im, column, row, 0);
            float green = get_pixel(im, column, row, 1);
            float blue = get_pixel(im, column, row, 2);

            float new_pixel_color = 0.299 * red + 0.587 * green + 0.114 * blue;
            set_pixel(gray, column, row, 0, new_pixel_color);
        }
    }

    return gray;
}

void shift_image(image im, int c, float v)
{
    // TODO Fill this in
    for (int row = 0; row < im.h; row++) {
        for (int column = 0; column < im.w; column++) {
            set_pixel(im, column, row, c, get_pixel(im, column, row, c) + v);
        }
    }
}

void clamp_image(image im)
{
    // TODO Fill this in
    for (int chanel = 0; chanel < im.c; chanel++) {
        for (int row = 0; row < im.h; row++) {
            for (int column = 0; column < im.w; column++) {
                float pixel = get_pixel(im, column, row, chanel);

                if (pixel < 0.0) {
                    pixel = 0.0;
                }

                if (pixel > 1) {
                    pixel = 1.0;
                }

                set_pixel(im, column, row, chanel, pixel);
            }
        }
    }
}


// These might be handy
float three_way_max(float a, float b, float c)
{
    return (a > b) ? ( (a > c) ? a : c) : ( (b > c) ? b : c) ;
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ( (a < c) ? a : c) : ( (b < c) ? b : c) ;
}

void rgb_to_hsv(image im)
{
    // TODO Fill this in

    for (int row = 0; row < im.h; row++) {
        for (int column = 0; column < im.w; column++) {
            float red = get_pixel(im, column, row, 0);
            float green = get_pixel(im, column, row, 1);
            float blue = get_pixel(im, column, row, 2);

            float value = three_way_max(red, green, blue);
            set_pixel(im, column, row, 2, value);

            float c = value - three_way_min(red, green, blue);

            float saturation = 0.0;
            if (value != 0) {
                saturation = c / value;
            }
            set_pixel(im, column, row, 1, saturation);

            float hue = 0.0;
            if (c != 0) {
                if (value == red) {
                    hue = (green - blue) / c;
                } else if (value == green) {
                    hue = (blue - red) / c + 2;
                } else if (value == blue) {
                    hue = (red - green) / c + 4;
                }
            }

            if (hue < 0) {
                hue = (hue / 6) + 1;
            } else {
                hue = hue / 6;
            }

            set_pixel(im, column, row, 0, hue);
        }
    }


}

void hsv_to_rgb(image im)
{
    // TODO Fill this in
    for (int row = 0; row < im.h; row++) {
        for (int column = 0; column < im.w; column++) {
            float hue = get_pixel(im, column, row, 0);
            float saturation = get_pixel(im, column, row, 1);
            float v = get_pixel(im, column, row, 2);

            float c = saturation * v;
            float min = v - c;

            float colorAngle = hue * 6.0;
            float value = hue * 6;
            value = fmod(value, 2.0);
            value -= 1.0;
            value = fabs(value);
            value = 1.0 - value;
            value *= c;

            float red = 0.0;
            float green = 0.0;
            float blue = 0.0;
            if (colorAngle >= 0.0 && colorAngle < 1.0) {
                red = c + min;
                green = value + min;
                blue = min;
            } else if (colorAngle >= 1.0 && colorAngle < 2.0) {
                red = value + min;
                green = c + min;
                blue = min;
            } else if (colorAngle >= 2.0 && colorAngle < 3.0) {
                red = min;
                green = c + min;
                blue = value + min;
            } else if (colorAngle >= 3.0 && colorAngle < 4.0) {
                red = min;
                green = value + min;
                blue = c + min;
            } else if (colorAngle >= 4.0 && colorAngle < 5.0) {
                red = value + min;
                green = min;
                blue = c + min;
            } else if (colorAngle >= 5.0 && colorAngle < 6.0) {
                red = c + min;
                green = min;
                blue = value + min;
            } else {
                red = min;
                green = min;
                blue = min;
            }

            set_pixel(im, column, row, 0, red);
            set_pixel(im, column, row, 1, green);
            set_pixel(im, column, row, 2, blue);

        }
    }
}