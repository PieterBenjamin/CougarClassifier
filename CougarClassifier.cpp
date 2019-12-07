#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>

#include "image.h"

#define VIDEO_FILE "SampleVideo_640x360_5mb.mp4"
#define VELOCITY_THRESHOLD 1.0
#define PIXELS_WITH_MOVEMENT_THRESHOLD 7

using cv::Mat,
      std::vector,
      std::string,
      std::shared_ptr;

/**
 * Converts a given (3-channel) Mat to a (3-channel) image.
 */
image bgr_mat_to_image(cv::Mat *frame);

/**
 * Converts a given (1-channel) Mat to a (1-channel) image.
 */
image grayscale_mat_to_image(cv::Mat *frame);

/**
 * Converts a given Mat to an image.
 */
image mat_to_image(cv::Mat *frame);

/**
 * Extracts all the frames from a given .mp4 file and returns them
 * inside a vector.
 */
shared_ptr<vector<image>> gather_frames(const string &fname);

/**
 * @param frames a vector containing all the images to look through. It is assumed that 
 *               these images came one after another inside a video.
 * @return an array of integers, the same length as @frames, which contains
 *         at every index i:
 *             the number of pixels over a given movement threshold for the
 *             frame @frames[i]
 */
shared_ptr<vector<uint32_t>> detect_movement(shared_ptr<vector<image>> frames);

int main(int argc, char **argv)
{
    // 1. Get all our frames from the video.
    const string fname = VIDEO_FILE;
    auto frames = gather_frames(fname);

    // 2. Record how much movement is in every frame.
    auto high_movement_frames = detect_movement(frames);

    // 3. look for all the high movement frames.
    for (auto i = high_movement_frames->begin; i != high_movement_frames->end; i++) {
        if (i > VELOCITY_THRESHOLD) {

        }
    }

    return 0;
}

image bgr_mat_to_image(cv::Mat *frame) {
    // We will only enter this method with a frame of 3 channels.
    auto channels = 3;
    auto rows     = frame->rows;
    auto columns  = frame->cols;
    auto im = make_image(columns, rows, channels);

    for (int32_t row = 0; row < rows; row++) {
        for (int32_t col = 0; col < columns; col++) {
            const uchar* pixels = frame->ptr(row, col);
           
            // Important to remember that colors are stored in BGR format. Another
            // weird artifact of opencv is that the colors seem to have been inverted.
            set_pixel(im, col, row, 0, (float)(pixels[2] ^ 0xFF));
            set_pixel(im, col, row, 1, (float)(pixels[1] ^ 0xFF));
            set_pixel(im, col, row, 2, (float)(pixels[0] ^ 0xFF));
        }
    }

    return im;
}

image grayscale_mat_to_image(cv::Mat *frame) {
    // We will only enter this method with a frame of 3 channels.
    auto channels = 1;
    auto rows     = frame->rows;
    auto columns  = frame->cols;
    auto im = make_image(columns, rows, channels);

    for (int32_t row = 0; row < rows; row++) {
        for (int32_t col = 0; col < columns; col++) {
            const uchar* pixels = frame->ptr(row, col);
           
            // Important to remember that colors are stored in BGR format. Another
            // weird artifact of opencv is that the colors seem to have been inverted.
            // TODO - check B&W images are also inverted
            set_pixel(im, col, row, 0, (float)(pixels[0] ^ 0xFF));
        }
    }

    return im;
}

image mat_to_image(cv::Mat *frame) {
    image im;
    if (frame->channels == 3) {
        return bgr_mat_to_image(frame);
    } else if (frame->channels == 1) {
        return grayscale_mat_to_image(frame);
    }

    // Some kinda funky nonsense.
    im.data = nullptr;
    return im;
}

shared_ptr<vector<image>> gather_frames(const string &fname) {
    std::cout << "Gathering frames . . . ";

    cv::Mat frame;
    shared_ptr<vector<image>> frames(new vector<image>);
    auto cap = cv::VideoCapture(fname);

    // Read all frames.
    while (cap.isOpened()) {
        // Check that read was successful.
        if (!cap.read(frame)) {
            break;
        }

        auto tmp = mat_to_image(&frame);
        if (tmp.data == nullptr) {
            std::cerr << "skipping image with unrecognized dimensions." << std::endl;
        } else {
            // Add the last read frame as an image
            frames->push_back(tmp);
        }
    }

    cap.release();
    cv::destroyAllWindows();  

    std::cout << "done." << std::endl;

    return frames;
}

shared_ptr<vector<uint32_t>> detect_movement(shared_ptr<vector<image>> frames) {
    std::cout << "Detecting movement . . . ";

    uint32_t num_frames = frames->size(),
             num_high_velocity,
             num_pixels,
             pixel,
             i;
    // TODO: it would be ideal to preallocate here, since we know the # of frames.
    shared_ptr<vector<uint32_t>> high_movement_frames(new vector<uint32_t>);

    for (i = 0; i < num_frames; i++) {
        auto flow = optical_flow_images(frames->at(i), frames->at(i+1), 15, 8);

        num_pixels = flow.w * flow.h * flow.c;
        num_high_velocity = 0;
        for (pixel = 0; pixel < num_pixels; pixel++) {
            if (flow.data[i] >= VELOCITY_THRESHOLD) {
                num_high_velocity++;
            }
        }

        high_movement_frames->push_back(num_high_velocity);
    }

    std::cout << "done." << std::endl;

    return high_movement_frames;
}