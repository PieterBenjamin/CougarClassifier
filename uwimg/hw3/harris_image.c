#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#include "matrix.h"
#include <time.h>

void printMatrix(image im) {
    for (int i = 0; i < im.c; i++) {
        for (int j = 0; j < im.h; j++) {
            for (int k = 0; k < im.w; k++) {
                printf("%f ", get_pixel(im, k, j, i));
            }
            printf("\n");
        }
    }
}

// Frees an array of descriptors.
// descriptor *d: the array.
// int n: number of elements in array.
void free_descriptors(descriptor *d, int n)
{
    int i;
    for(i = 0; i < n; ++i){
        free(d[i].data);
    }
    free(d);
}

// Create a feature descriptor for an index in an image.
// image im: source image.
// int i: index in image for the pixel we want to describe.
// returns: descriptor for that index.
descriptor describe_index(image im, int i)
{
    int w = 5;
    descriptor d;
    d.p.x = i%im.w;
    d.p.y = i/im.w;
    d.data = calloc(w*w*im.c, sizeof(float));
    d.n = w*w*im.c;
    int c, dx, dy;
    int count = 0;
    // If you want you can experiment with other descriptors
    // This subtracts the central value from neighbors
    // to compensate some for exposure/lighting changes.
    for(c = 0; c < im.c; ++c){
        float cval = im.data[c*im.w*im.h + i];
        for(dx = -w/2; dx < (w+1)/2; ++dx){
            for(dy = -w/2; dy < (w+1)/2; ++dy){
                float val = get_pixel(im, i%im.w+dx, i/im.w+dy, c);
                d.data[count++] = cval - val;
            }
        }
    }
    return d;
}

// Marks the spot of a point in an image.
// image im: image to mark.
// ponit p: spot to mark in the image.
void mark_spot(image im, point p)
{
    int x = p.x;
    int y = p.y;
    int i;
    for(i = -9; i < 10; ++i){
        set_pixel(im, x+i, y, 0, 1);
        set_pixel(im, x, y+i, 0, 1);
        set_pixel(im, x+i, y, 1, 0);
        set_pixel(im, x, y+i, 1, 0);
        set_pixel(im, x+i, y, 2, 1);
        set_pixel(im, x, y+i, 2, 1);
    }
}

// Marks corners denoted by an array of descriptors.
// image im: image to mark.
// descriptor *d: corners in the image.
// int n: number of descriptors to mark.
void mark_corners(image im, descriptor *d, int n)
{
    int i;
    for(i = 0; i < n; ++i){
        mark_spot(im, d[i].p);
    }
}

// Creates a 1d Gaussian filter.
// float sigma: standard deviation of Gaussian.
// returns: single row image of the filter.
image make_1d_gaussian(float sigma)
{
    int width = ceil(sigma * 6);
    if (width % 2 == 0) {
        width += 1;
    }

    image res = make_image(width, 1, 1);
    for (int i = -width / 2; i <= width / 2; i++) {
        double exponent = exp(-((pow(i, 2)) / (2 * pow(sigma, 2))));
        float scale = sqrt(TWOPI * pow(sigma, 2));

        set_pixel(res, i + width / 2, 0, 0, exponent / scale);
    }
    return res;
}

// Smooths an image using separable Gaussian filter.
// image im: image to smooth.
// float sigma: std dev. for Gaussian.
// returns: smoothed image.
image smooth_image(image im, float sigma)
{
    image gaus = make_1d_gaussian(sigma);
    image filter = convolve_image(im, gaus, 1);
    gaus.h = gaus.w;
    gaus.w = 1;
    image res = convolve_image(filter, gaus, 1);

    free_image(gaus);
    free_image(filter);

    return res;
}

// Calculate the structure matrix of an image.
// image im: the input image.
// float sigma: std dev. to use for weighted sum.
// returns: structure matrix. 1st channel is Ix^2, 2nd channel is Iy^2,
//          third channel is IxIy.
image structure_matrix(image im, float sigma)
{
    image S = make_image(im.w, im.h, 3);
    // TODO: calculate structure matrix for im.
    image gx = make_gx_filter();
    image gy = make_gy_filter();

    image ix = convolve_image(im, gx, 0);
    image iy = convolve_image(im, gy, 0);

    for (int row = 0; row < im.h; row++) {
        for (int col = 0; col < im.w; col++) {
            float ixx = get_pixel(ix, col, row, 0);
            float iyy = get_pixel(iy, col, row, 0);

            float ixiy = ixx * iyy;

            set_pixel(S, col, row, 0, pow(ixx, 2));
            set_pixel(S, col, row, 1, pow(iyy, 2));
            set_pixel(S, col, row, 2, ixiy);
        }
    }

    free_image(gx);
    free_image(gy);
    free_image(ix);
    free_image(iy);

    return smooth_image(S, sigma);
}

// Estimate the cornerness of each pixel given a structure matrix S.
// image S: structure matrix for an image.
// returns: a response map of cornerness calculations.
image cornerness_response(image S)
{
    image R = make_image(S.w, S.h, 1);
    // TODO: fill in R, "cornerness" for each pixel using the structure matrix.
    // We'll use formulation det(S) - alpha * trace(S)^2, alpha = .06.
    for (int i = 0; i < S.h; i++) {
        for (int j = 0; j < S.w; j++) {
            float a = get_pixel(S, j, i, 0);
            float d = get_pixel(S, j, i, 1);
            float bc = get_pixel(S, j, i, 2);

            float det = (a * d) - (bc * bc);
            float trace = a + d;

            float cor = det - 0.06 * trace * trace;
            set_pixel(R, j, i, 0, (trace == 0.0) ? 0.0 : cor);        
        }
    }
    return R;
}

void nms_helper(image im, image nms, int row, int col, int w) {
    float max = get_pixel(im, col, row, 0);
    for (int i = col - w; i <= col + w; i++) {
        for (int j = row - w; j <= row + w; j++) {
            if (get_pixel(im, i, j, 0) >= max) {
                max = get_pixel(im, i, j, 0);
            }
        }
    }

    for (int i = col - w; i <= col + w; i++) {
        for (int j = row - w; j <= row + w; j++) {
            if (get_pixel(im, i, j, 0) < max) {
                set_pixel(nms, i, j, 0, -999999.0);
            }
        }
    }
}

// Perform non-max supression on an image of feature responses.
// image im: 1-channel image of feature responses.
// int w: distance to look for larger responses.
// returns: image with only local-maxima responses within w pixels.
image nms_image(image im, int w)
{
    image r = copy_image(im);
    // TODO: perform NMS on the response map.
    // for every pixel in the image:
    //     for neighbors within w:
    //         if neighbor response greater than pixel response:
    //             set response to be very low (I use -999999 [why not 0??])

    for (int row = 0; row < im.h; row++) {
        for (int col = 0; col < im.w; col++) {
            float pixel = get_pixel(r, col, row, 0);
            if (pixel != -999999.0) {
                nms_helper(im, r, row, col, w);
            }
        }
    }
    return r;
}

// Perform harris corner detection and extract features from the corners.
// image im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
// int *n: pointer to number of corners detected, should fill in.
// returns: array of descriptors of the corners in the image.
descriptor *harris_corner_detector(image im, float sigma, float thresh, int nms, int *n)
{
    // Calculate structure matrix
    image S = structure_matrix(im, sigma);

    // Estimate cornerness
    image R = cornerness_response(S);

    // Run NMS on the responses
    image Rnms = nms_image(R, nms);

    //TODO: count number of responses over threshold
    int count = 0;
    for (int i = 0; i < Rnms.h; i++) {
        for (int j = 0; j < Rnms.w; j++) {
            float pixel = get_pixel(Rnms, j, i, 0);
            if (pixel > thresh) {
                count++;
            }
        }
    }

    // printMatrix(Rnms);
    printf("%d\n", count);
    *n = count; // <- set *n equal to number of corners in image.
    descriptor *d = calloc(count, sizeof(descriptor));

    int index = 0;
    for (int i = 0; i < Rnms.h; i++) {
        for (int j = 0; j < Rnms.w; j++) {
            float pixel = get_pixel(Rnms, j, i, 0);
            if (pixel > thresh) {
                d[index] = describe_index(im, (i * Rnms.w) + j);
                index++;
            }
        }
    }

    free_image(S);
    free_image(R);
    free_image(Rnms);
    return d;
}

// Find and draw corners on an image.
// image im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
void detect_and_draw_corners(image im, float sigma, float thresh, int nms)
{
    int n = 0;
    descriptor *d = harris_corner_detector(im, sigma, thresh, nms, &n);
    mark_corners(im, d, n);
}
