#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "../image.h"
#define TWOPI 6.2831853

// Returns the result of applying @filter to the pixel describe in @im.
float convolve_pixel(image im, image filter, int x, int y, int channel);

// Returns the result of convolving an image while preserving the channels
image convolve_image_preserve(image im, image filter);

// Returns the result of convolving an image without preserving the channels
image convolve_image_no_preserve(image im, image filter);

void create_ronbledore();

void l1_normalize(image im)
{
    for (int chan = 0; chan < im.c; chan++) {
        float denom = 0;

        for (int row = 0; row < im.h; row++) {
            for (int col = 0; col < im.w; col++) {
                denom += get_pixel(im, row, col, chan);
            }
        }

        if (denom == 0) denom = im.w * im.c;

        for (int row = 0; row < im.h; row++) {
            for (int col = 0; col < im.w; col++) {
                set_pixel(im, row, col, chan, 
                                           get_pixel(im, row, col, chan)/denom);
            }
        }
    }
}

image make_box_filter(int w)
{
    image filter = make_image(w, w, 1);
    for (int i = 0; i < w * w; i++) { filter.data[i] = 1; }
    l1_normalize(filter);

    return filter;
}

image convolve_image(image im, image filter, int preserve)
{
    assert((im.c == filter.c) || ((filter.c == 1) && (im.c > 1)));

    return (preserve == 1) ? convolve_image_preserve(im, filter)
                           : convolve_image_no_preserve(im, filter);
}

image convolve_image_preserve(image im, image filter)
{
    image new_im = make_image(im.w, im.h, im.c);
    float filtered;

    for (int channel = 0; channel < im.c; channel++) {
        for (int y = 0; y < im.h; y++) {
            for (int x = 0; x < im.w; x++) {
                filtered = convolve_pixel(im, filter, x, y, channel);

                set_pixel(new_im, x, y, channel, filtered);
            }
        }
    }

    return new_im;
}

image convolve_image_no_preserve(image im, image filter)
{
    image new_im = make_image(im.w, im.h, 1);
    float filtered;

    for (int channel = 0; channel < im.c; channel++) {
        for (int y = 0; y < im.h; y++) {
            for (int x = 0; x < im.w; x++) {
                filtered = 0;
                filtered += convolve_pixel(im, filter, x, y, channel);
                // If we've already done some work, we don't want to 
                // overwrite it.
                set_pixel(new_im, x, y, 0, 
                channel > 0 ? get_pixel(new_im, x, y, 0) + filtered : filtered);
            }
        }
    }

    return new_im;
}

float convolve_pixel(image im, image filter, int x, int y, int channel)
{
    float filtered = 0;

    int offset = floor(((float)filter.w)/2.0);
    for (int filter_y = 0; filter_y < filter.h; filter_y++) {
        for (int filter_x = 0; filter_x < filter.w; filter_x++) {
            // If we've ever been passed a filter with > 1 channel,
            // we know that we are supposed to apply the channel c
            // of filter to channel c of im.
            filtered += get_pixel(filter,
                                  filter_x,
                                  filter_y,
                                  (filter.c > 1) ? channel : 0)
                      * get_pixel(im,
                                  x - offset + filter_x,
                                  y - offset + filter_y,
                                  channel);
        }
    }

    return filtered;
}

image make_highpass_filter()
{
    image filter = make_image(3, 3, 1);

    float arr[9] = { 0, -1,  0,
                    -1,  4, -1,
                     0, -1,  0};
    for (int i = 0; i < 9; i++) { filter.data[i] = arr[i]; }

    return filter;
}

image make_sharpen_filter()
{
    image filter = make_image(3, 3, 1);

    float arr[9] = { 0, -1,  0,
                    -1,  5, -1,
                     0, -1,  0};
    for (int i = 0; i < 9; i++) { filter.data[i] = arr[i]; }
    
    return filter;
}

image make_emboss_filter()
{
    image filter = make_image(3, 3, 1);

    float arr[9] = {-2, -1, 0, 
                    -1,  1, 1, 
                     0,  1, 2};
    for (int i = 0; i < 9; i++) { filter.data[i] = arr[i]; }

    return filter;
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// Answer: I think Highpass should not be prserved because it looks like it should be BW

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: We have tp clamp emboss and sharpen because they have a tendency to either underflow/overflow.

image make_gaussian_filter(float sigma)
{
    float exp_denom, dist, val, r, x2, y2;
    int size = (int)ceilf(sigma * 6);
    while ((size % 2) == 0) size ++;

    image filter = make_image(size, size, 1);
    exp_denom = 2.0 * sigma * sigma;

    // The center of the filter is at ceil((float)size/2)
    dist = floor(((float)size)/2);
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            x2  = x - dist;
            y2  = y - dist;
            r   = (x2 * x2) + (y2 * y2);
            val = M_PI * exp_denom;
            val = exp((-1) * (r) / exp_denom) / val;

            set_pixel(filter, x, y, 0, val);
        }
    }

    l1_normalize(filter);

    return filter;
}

image add_image(image a, image b)
{
    assert((a.w = b.w) && (a.h == b.h) && (a.c == b.c));
    image new_im =  make_image(a.w, a.h, a.c);

    for (int channel = 0; channel < new_im.c; channel++) {
        for (int y = 0; y < new_im.h; y++) {
            for (int x = 0; x < new_im.w; x++) {
                set_pixel(new_im, x, y, channel, get_pixel(a, x, y, channel)
                                               + get_pixel(b, x, y, channel));
            }
        }
    }

    return new_im;
}

image sub_image(image a, image b)
{
    assert((a.w = b.w) && (a.h == b.h) && (a.c == b.c));
    image new_im =  make_image(a.w, a.h, a.c);

    for (int channel = 0; channel < new_im.c; channel++) {
        for (int y = 0; y < new_im.h; y++) {
            for (int x = 0; x < new_im.w; x++) {
                set_pixel(new_im, x, y, channel, get_pixel(a, x, y, channel)
                                               - get_pixel(b, x, y, channel));
            }
        }
    }

    return new_im;
}

image make_gx_filter()
{
    image filter = make_image(3, 3, 1);

    float arr[9] = {-1, 0, 1,
                    -2, 0, 2,
                    -1, 0, 1};

    for (int i = 0; i < 9; i++) { filter.data[i] = arr[i]; }

    return filter;
}

image make_gy_filter()
{
    image filter = make_image(3, 3, 1);

    float arr[9] = {-1, -2, -1,
                     0,  0,  0,
                     1,  2,  1};

    for (int i = 0; i < 9; i++) { filter.data[i] = arr[i]; }

    return filter;
}

void feature_normalize(image im)
{
    float min = im.data[0], max = im.data[0], val;

    for (int c = 0; c < im.c; c++) {
        for (int y = 0; y < im.h; y++) {
            for (int x = 0; x < im.w; x++) {
                min = MIN(min, get_pixel(im, x, y, c));
                max = MAX(max, get_pixel(im, x, y, c));
            }
        }
    }

    // Let's not divide by 0
    if ((max == 0) || (min == max)) {
        for (int i = 0; i < im.w*im.h*im.c; i++) { im.data[i] = 0.0; }
    }

    for (int c = 0; c < im.c; c++) {
        for (int y = 0; y < im.h; y++) {
            for (int x = 0; x < im.w; x++) {
                val = get_pixel(im, x, y, c);
                set_pixel(im, x, y, x, (val - min)/(max - min));
            }
        }
    }
}

image *sobel_image(image im)
{
    image *new_ims = calloc(2, sizeof(image));
    image Gx       = make_gx_filter();
    image Gy       = make_gy_filter();
    // Magnitude
    *new_ims       = convolve_image(im, Gx, 0);
    // Direction
    *(new_ims + 1) = convolve_image(im, Gy, 0);

    float gx, gy;

    for (int y = 0; y < im.h; y++) {
        for (int x = 0; x < im.w; x++) {
            gx = get_pixel(*new_ims, x, y, 0);
            gy = get_pixel(*(new_ims + 1), x, y, 0);

            set_pixel(*new_ims, x, y, 0, sqrt((gx * gx) + (gy * gy)));
            set_pixel(*(new_ims + 1), x, y, 0, atan2(gy, gx));
        }
    }

    return new_ims;
}

image colorize_sobel(image im)
{
    // Makes a cool rainbow depending on angle
    image colorized = make_image(im.w, im.h, im.c);
    image *sobels   = sobel_image(im);
    feature_normalize(*sobels);
    feature_normalize(*(sobels + 1));
    float threshhold = 1.0, mag, ang;

    for (int y = 0; y < im.h; y++) {
        for (int x = 0; x < im.w; x++) {
            mag = fabsf(get_pixel(*sobels, x, y, 0));
            ang = fabsf(get_pixel(*(sobels + 1), x, y, 0)) * (180 / 3.141592) ;
        
            //printf("%f\n", ang);
            if (mag >= threshhold) {
                // Score!
                set_pixel(colorized, x, y, 0, ang * (6.0/360.0)); // Hue
                set_pixel(colorized, x, y, 1, mag);     // Saturation
                set_pixel(colorized, x, y, 2, mag);     // Value
            }
        }
    }


    hsv_to_rgb(colorized);
    convolve_image(colorized, make_gaussian_filter(10), 1);
    return colorized;
}

void create_ronbledore() {
    image tmp1 = load_image("data/ron.png");
    image tmp2 = load_image("data/dumbledore.png");
    assert((tmp1.w == tmp2.w) && (tmp1.h == tmp2.h) && (tmp1.c == tmp2.c));
    image ronbledore = convolve_image(tmp2, make_gaussian_filter(3), 1);

    image tmp3 = sub_image(tmp1, convolve_image(tmp1, make_gaussian_filter(2), 1));
    image final = add_image(ronbledore , tmp3);

    clamp_image(final);
    save_image(final, "ronbledore");

    free_image(ronbledore);
    free_image(tmp1);
    free_image(tmp2);
    free_image(tmp3);
    free_image(final);
}