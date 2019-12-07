#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "../image.h"

float get_pixel(image im, int x, int y, int c)
{
    // First we'll sanitize our input. We don't want any negative values,
    // and we don't want anything greater than our bounds.
    // The minus one is to account for 0-based indexing.
    x = MAX(0, MIN(x, (im.w - 1)));
    y = MAX(0, MIN(y, (im.h - 1)));
    c = MAX(0, MIN(c, (im.c - 1)));

    return im.data[x + (y * im.w) + (c * im.w * im.h)];
}

void set_pixel(image im, int x, int y, int c, float v)
{
    int desired_index = x + (y * im.w) + (c * im.w * im.h);

    // Check bounds
    if (desired_index > (im.w * im.h * im.c) || desired_index < 0) {
        return;
    } else {
        im.data[desired_index] = v;
    }
}

image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    int col, row, channel;

    for (channel = 0; channel < im.c; channel++) {
        for (row = 0; row < im.h; row++) {
            for (col = 0; col < im.w; col++) {
                copy.data[col + (row * im.w) + (channel * im.w * im.h)] =
                          im.data[col + (row * im.w) + (channel * im.w * im.h)];
            }
        }
    }

    return copy;
}

image rgb_to_grayscale(image im)
{
    int col, row;
    if (im.c == 1) return im;
    image gray = make_image(im.w, im.h, 1);
    
    for (row = 0; row < im.h; row++) {
        for (col = 0; col < im.w; col++) {
            // Y' = 0.299 R' + 0.587 G' + .114 B'
            gray.data[col + (row * im.w)] =
                      (0.299 * im.data[col + (row * im.w) + (0 * im.w * im.h)])
                    + (0.587 * im.data[col + (row * im.w) + (1 * im.w * im.h)])
                    + (0.114 * im.data[col + (row * im.w) + (2 * im.w * im.h)]);
        }
    }

    return gray;
}

void shift_image(image im, int c, float v)
{
    int col, row; 
    c = MAX(0, MIN(c, im.c));

    for (row = 0; row < im.h; row++) {
        for (col = 0; col < im.w; col++) {
            im.data[col + (row * im.w) + (c * im.w * im.h)] += v;
        }
    }
}

void clamp_image(image im)
{
    int x, y, channel;
    float max_val = 1.0, min_val = 0.0, new_val;
    
    for (channel = 0; channel < im.c; channel++) {
        for (y = 0; y < im.h; y++) {
            for (x = 0; x < im.w; x++) {
                new_val = MAX(min_val, MIN(max_val, get_pixel(im,
                                                              x,
                                                              y,
                                                              channel)));
                set_pixel(im, x, y, channel, new_val);
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
    int col, row;
    // Red, Green, Blue, Value, minimum, diff, Saturation, Hue
    double R, G, B, V, m, C, S, H;

    assert(im.c >= 3);

    for (row = 0; row < im.h; row++) {
        for (col = 0; col < im.w; col++) {
            R = im.data[col + (row * im.w) + (0 * im.w * im.h)];
            G = im.data[col + (row * im.w) + (1 * im.w * im.h)];
            B = im.data[col + (row * im.w) + (2 * im.w * im.h)];

            V = three_way_max(R, G, B);
            m = three_way_min(R, G, B);
            C = V - m;

            if (V == 0.0) {
                S = 0.0;
            } else {
                S = C/V;
            }

            if (C == 0.0) {
                H = 0.0;
            } else if (V == R) {
                H = (G - B)/C;
            } else if (V == G) {
                H = ((B - R)/C) + 2.0;
            } else if (V == B) {
                H = ((R - G)/C) + 4.0;
            }

            if (H < 0.0) {
                H = (H/6.0) + 1.0;
            } else {
                H = H/6.0;
            }

            im.data[col + (row * im.w) + (0 * im.w * im.h)] = H;
            im.data[col + (row * im.w) + (1 * im.w * im.h)] = S;
            im.data[col + (row * im.w) + (2 * im.w * im.h)] = V;
        }
    }
}

void hsv_to_rgb(image im)
{
    int col, row;
    // Red, Green, Blue, Value, minimum, diff, Saturation, Hue
    float V, m, C, S, H, angle, val;

    assert(im.c >= 3);

    for (row = 0; row < im.h; row++) {
        for (col = 0; col < im.w; col++) {
            H = im.data[col + (row * im.w) + (0 * im.w * im.h)];
            S = im.data[col + (row * im.w) + (1 * im.w * im.h)];
            V = im.data[col + (row * im.w) + (2 * im.w * im.h)];
            
            if (V == 0) {
                // The max of all three was 0. Assuming nonnegative values, 
                // we can do this:
                im.data[col + (row * im.w) + (0 * im.w * im.h)] = 0;
                im.data[col + (row * im.w) + (1 * im.w * im.h)] = 0;
                im.data[col + (row * im.w) + (2 * im.w * im.h)] = 0;
                continue;
            }

            val = (H * 6) - 1.0;
            angle = H * 6.0;

            // S      = (V - m)/V
            // VS     = V - m
            // VS + m = V
            m         = V - (V * S);
            C     = V - m;

            if (val > 3) {
                val -= 4;
            } else if (val > 1) {
                val -=2;
            }

            val = C * (1.0 - fabs(val));

            if (angle >= 0 && angle < 1) {
                im.data[col + (row * im.w) + (0 * im.w * im.h)] = m + C;
                im.data[col + (row * im.w) + (1 * im.w * im.h)] = m + val;
                im.data[col + (row * im.w) + (2 * im.w * im.h)] = m;
            } else if (angle >= 1 && angle < 2) {
                im.data[col + (row * im.w) + (0 * im.w * im.h)] = m + val;
                im.data[col + (row * im.w) + (1 * im.w * im.h)] = m + C;
                im.data[col + (row * im.w) + (2 * im.w * im.h)] = m;
            } else if (angle >= 2 && angle < 3) {
                im.data[col + (row * im.w) + (0 * im.w * im.h)] = m;
                im.data[col + (row * im.w) + (1 * im.w * im.h)] = m + C;
                im.data[col + (row * im.w) + (2 * im.w * im.h)] = val + m;
            } else if (angle >= 3 && angle < 4) {
                im.data[col + (row * im.w) + (0 * im.w * im.h)] = m;
                im.data[col + (row * im.w) + (1 * im.w * im.h)] = m + val;
                im.data[col + (row * im.w) + (2 * im.w * im.h)] = m + C;
            } else if (angle >= 4 && angle < 5) {
                im.data[col + (row * im.w) + (0 * im.w * im.h)] = val + m;
                im.data[col + (row * im.w) + (1 * im.w * im.h)] = m;
                im.data[col + (row * im.w) + (2 * im.w * im.h)] = m + C;
            } else if (angle >= 5 && angle < 6) {
                im.data[col + (row * im.w) + (0 * im.w * im.h)] = C + m;
                im.data[col + (row * im.w) + (1 * im.w * im.h)] = m;
                im.data[col + (row * im.w) + (2 * im.w * im.h)] = m + val;
            } else {
                im.data[col + (row * im.w) + (0 * im.w * im.h)] = m;
                im.data[col + (row * im.w) + (1 * im.w * im.h)] = m;
                im.data[col + (row * im.w) + (2 * im.w * im.h)] = m;
            }
        }
    }
}
