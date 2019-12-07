import cv2
import numpy
from uwimg import *
import os

"""
    Variable Initialization
"""
# TODO - make this take a CL argument
video_file = sys.argv[1]
# Will store all frames in video


# Getting all frame names
frame_names = []
frame_dir = os.path.dirname(os.path.realpath(__file__)) + "/frames/"
for frame_name in os.listdir(frame_dir):
    frame_names.append(frame_dir + frame_name)
frame_names.sort()

print
print
print("analying video: " + video_file)
print
print


# Number of files, and number to process at once
num_files = len(frame_names)
batch_size = 500

"""
    Helper Methods
"""


def gather_frames(start, stop):
    """
    Gets all frames which should have been placed in frames/frame*.png

    :return: an array of frames,
    """

    print("Gathering frames [" + str(start) + ", " + str(stop) + "] from " + frame_dir + " . . .")

    frames = []

    for i in range(stop - start):
        # make sure we don't go overboard
        if i >= num_files:
            break
        frames.append(load_image(frame_names[start + i]))

    print("done")
    return frames


def detect_movement(frames, frame_movement, start):
    """

    :param frames:
    :return: an array of booleans, the same length as @frames with 1 at the indices that "high movement" was detected in
             @frames (where the index is the first of the two frames with suspected movement).
    """

    print("detecting movement in " + str(batch_size) + " frames . . .")

    for frame in range(len(frames) - 1):
        flow = optical_flow_images(frames[frame], frames[frame + 1], 15, 8)
        frame_sum = 0

        # only look at dx, dy
        for channel in [0, 1]:
            for row in range(flow.h):
                for col in range(flow.c):
                    # we only care about magnitude
                    frame_sum = max(frame_sum, abs(get_pixel(flow, col, row, channel)))
        free_image(flow)

        if frame % 100 == 0:
            print(frame)
        frame_movement.append((frame_sum, frame + start))

    print("done")


def nth_most_movement(frame_movement, n):
    frame_movement.sort(key=lambda x: -x[0])

    print("The " + str(n) + " frame(s) with the highest movement are: ")
    for frame in range(n):
        print("frames/frame" + str(frame_movement[frame]) + ".jpg")


"""
    Main method
"""


def main():
    frame_movement = []
    for batch in range((num_files / batch_size) + 1):
        start = batch * batch_size
        stop = (batch + 1) * batch_size
        # Read in the next batch of frames
        frames = gather_frames(start, stop)

        # Look through the given frames and calculate movement
        detect_movement(frames, frame_movement, start)

        # Free everything we're done with (there will be one image at the end
        # that will be freed and read back in immediately)
        for frame in frames:
            free_image(frame)

    nth_most_movement(frame_movement, 6)


if __name__ == "__main__":
    main()
