#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#include "limits.h"
#define TWOPI 6.2831853

float get_convolved_value(image im, image filter, int x, int y, int c);


void l1_normalize(image im)
{
    // TODO
    float sum = 0.0;
    for (int c = 0; c < im.c; c++) {
        for (int i = 0; i < im.w; i++) {
            for (int j = 0; j < im.h; j++) {
                sum += get_pixel(im, i, j, c);
            }
        }
    }

    for (int c = 0; c < im.c; c++) {
        for (int i = 0; i < im.w; i++) {
            for (int j = 0; j < im.h; j++) {
                float pixel = get_pixel(im, i, j, c);
                set_pixel(im, i, j, c, pixel / sum);
            }
        }
    }
}

image make_box_filter(int w)
{
    // TODO
    image im = make_image(w, w, 1);
    float uniformVal = 1.0 / (w * w);
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < w; j++) {
            set_pixel(im, i, j, 0, uniformVal);
        }
    }
    return im;
}

image convolve_image(image im, image filter, int preserve)
{
    image filtered_image = make_image(im.w, im.h, im.c);
    for (int c = 0; c < im.c; c++) {
        for (int h = 0; h < im.h; h++) {
            for (int w = 0; w < im.w; w++) {
                set_pixel(filtered_image, w, h, c, get_convolved_value(im, filter, w, h, c));
            }
        }
    }

    if (!preserve) {
        image merged_filtered_image = make_image(filtered_image.w, filtered_image.h, 1);
        for (int c = 0; c < filtered_image.c; c++) {
            for (int h = 0; h < filtered_image.h; h++) {
                for (int w = 0; w < filtered_image.w; w++) {
                    float val = get_pixel(merged_filtered_image, w, h, 0) + get_pixel(filtered_image, w, h, c);
                    set_pixel(merged_filtered_image, w, h, 0, val);
                }
            }
        }

        free_image(filtered_image);
        return merged_filtered_image;
    }

    return filtered_image;
}

float get_convolved_value(image im, image filter, int x, int y, int c) {
    int shift_x = filter.w / 2;
    int shift_y = filter.h / 2;
    int channel = (im.c == filter.c) ? c : 0;

    float sum = 0;
    for (int h = 0; h < filter.h; h++) {
        for (int w = 0; w < filter.w; w++) {
            sum += get_pixel(im, x - shift_x + w, y - shift_y + h, c) * get_pixel(filter, w, h, channel);
        }
    }
    return sum;
}

image make_highpass_filter()
{
    // TODO
  image ret = make_image(3,3,1);
  ret.data[0] = 0;
  ret.data[1] = -1;
  ret.data[2] = 0;
  ret.data[3] = -1;
  ret.data[4] = 4;
  ret.data[5] = -1;
  ret.data[6] = 0;
  ret.data[7] = -1;
  ret.data[8] = 0;
  return ret;
}

image make_sharpen_filter()
{
  image ret = make_image(3,3,1);
  ret.data[0] = 0;
  ret.data[1] = -1;
  ret.data[2] = 0;
  ret.data[3] = -1;
  ret.data[4] = 5;
  ret.data[5] = -1;
  ret.data[6] = 0;
  ret.data[7] = -1;
  ret.data[8] = 0;
  return ret;
}

image make_emboss_filter()
{
  image ret = make_image(3,3,1);
  ret.data[0] = -2;
  ret.data[1] = -1;
  ret.data[2] = 0;
  ret.data[3] = -1;
  ret.data[4] = 1;
  ret.data[5] = 1;
  ret.data[6] = 0;
  ret.data[7] = 1;
  ret.data[8] = 2;
  return ret;
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// Answer: We would have to use preserve for the emboss filter, box filter and sharpen filter. All of these filters have color.

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: We would have to post-process the highpass kernel, emboss kernel and the sharpen kernel. All of these filters can 
// potentially cause overflow of the pixel value so they need to be clamped. 

image make_gaussian_filter(float sigma)
{
    int size = ceil(sigma * 6);
    if (size % 2 == 0) {
        size += 1;
    }

    image res = make_image(size, size, 1);

    float s = 2.0 * sigma * sigma;
    float r = 0.0;
    for (int i = (int)(size / 2) * -1; i < (size / 2); i++) {
        for (int j =  (int)(size / 2) * -1; j < (size / 2); j++) {
            r = sqrt(i * i + j * j);
            set_pixel(res, i + (size / 2), j + (size / 2), 0, (exp(-(r * r) / s)) / (M_PI * s));
        }
    }
    l1_normalize(res);
    return res;
}

image add_image(image a, image b)
{
    // TODO
    if (a.w != b.w || a.h != b.h || a.c != b.c) {
        return make_image(0, 0, 0);
    }

    image res = make_image(a.w, a.h, a.c);
    for (int c = 0; c < a.c; c++) {
        for (int i = 0; i < a.w; i++) {
            for (int j = 0; j < a.h; j++) {
                set_pixel(res, i, j, c, get_pixel(a, i, j, c) + get_pixel(b, i, j, c));
            }
        }
    }
    return res;
}

image sub_image(image a, image b)
{
    if (a.w != b.w || a.h != b.h || a.c != b.c) {
        return make_image(0, 0, 0);
    }

    image res = make_image(a.w, a.h, a.c);
    for (int c = 0; c < a.c; c++) {
        for (int i = 0; i < a.w; i++) {
            for (int j = 0; j < a.h; j++) {
                set_pixel(res, i, j, c, get_pixel(a, i, j, c) - get_pixel(b, i, j, c));
            }
        }
    }
    return res;
}

image make_gx_filter()
{
  image ret = make_image(3,3,1);
  ret.data[0] = -1;
  ret.data[1] = 0;
  ret.data[2] = 1;
  ret.data[3] = -2;
  ret.data[4] = 0;
  ret.data[5] = 2;
  ret.data[6] = -1;
  ret.data[7] = 0;
  ret.data[8] = 1;
  return ret;
}

image make_gy_filter()
{
  image ret = make_image(3,3,1);
  ret.data[0] = -1;
  ret.data[1] = -2;
  ret.data[2] = -1;
  ret.data[3] = 0;
  ret.data[4] = 0;
  ret.data[5] = 0;
  ret.data[6] = 1;
  ret.data[7] = 2;
  ret.data[8] = 1;
  return ret;
}

void feature_normalize(image im)
{
    float min = INT_MAX;
    float max = INT_MIN;
    for (int c = 0; c < im.c; c++) {
        for (int i = 0; i < im.w; i++) {
            for (int j = 0; j < im.h; j++) {
                float pixel = get_pixel(im, i, j, c);
                if (pixel > max) {
                    max = pixel;
                }
                if (pixel < min) {
                    min = pixel;
                }
            }
        }
    }
    float range = max - min;

    for (int c = 0; c < im.c; c++) {
        for (int i = 0; i < im.w; i++) {
            for (int j = 0; j < im.h; j++) {
                float pixel = get_pixel(im, i, j, c);
                pixel -= min;
                if (range == 0) {
                    set_pixel(im, i, j, c, 0);
                } else {
                    pixel /= range;
                    set_pixel(im, i, j, c, pixel);
                }
            }
        }
    }
}

image *sobel_image(image im)
{
    image gx = convolve_image(im, make_gx_filter(), 0);

    image gy = convolve_image(im, make_gy_filter(), 0);

    image mag = make_image(im.w, im.h, 1);
    image theta = make_image(im.w, im.h, 1);
    for (int row = 0; row < im.h; row++) {
        for (int col = 0; col < im.w; col++) {
            float pixel_gx = get_pixel(gx, col, row, 0);
            float pixel_gy = get_pixel(gy, col, row, 0);

            float pixel = pixel_gx * pixel_gx + pixel_gy * pixel_gy;
            pixel = sqrt(pixel);
            set_pixel(mag, col, row, 0, pixel);

            pixel = atan2(pixel_gy, pixel_gx);
            set_pixel(theta, col, row, 0, pixel);
        }
    }

    image * res = calloc(2, sizeof(image));
    res[0] = mag;
    res[1] = theta;
    free_image(gx);
    free_image(gy);
    return res;
}

image colorize_sobel(image im)
{
    image* sobel = sobel_image(im);

    image res = make_image(im.w, im.h, 3);
    
    for (int c = 0; c < 3; c++) {
        for (int i = 0; i < im.h; i++) {
            for (int j = 0; j < im.w; j++) {
                if (c == 0) {
                    set_pixel(res, j, i, c, get_pixel(sobel[0], j, i, 0));
                } else {
                    set_pixel(res, j, i, c, get_pixel(sobel[1], j, i, 0));
                }
            }
        }
    }

    hsv_to_rgb(res);
    return res;
}
