import cv2
import numpy
from uwimg import *

"""
    Variable Initialization
"""
# TODO - make this take a CL argument
video_file = "SampleVideo_640x360_5mb.mp4"
# Will store all frames in video


"""
    Helper Methods
"""


def bgr_mat_to_image(frame):
    """
        Used for converting 3 channel Mat objects to images
    """
    # set_pixel(im, col, row, 0, frame[row][col][2] ^ 0xFF)
    # set_pixel(im, col, row, 1, frame[row][col][1] ^ 0xFF)
    # set_pixel(im, col, row, 2, frame[row][col][0] ^ 0xFF)

    # we always know the number of channels here
    channels = 3
    rows     = frame.shape[0]
    columns  = frame.shape[1]

    im = make_image(columns, rows, channels)

    for chan in range(3):
        for row in range(rows):
            for col in range(columns):

                # Colors are stored in BGR format, and weirdly inverted
                if chan == 0:  # we want the R channel, which is at 2
                    set_pixel(im, col, row, chan, frame[row][col][2] ^ 0xFF)
                elif chan == 1:  # we want the G channel, which is at 1
                    set_pixel(im, col, row, chan, frame[row][col][1] ^ 0xFF)
                else:  # we want the B channel, which is at 0
                    set_pixel(im, col, row, chan, frame[row][col][0] ^ 0xFF)

    return im


def grayscale_mat_to_image(frame):
    """
        Used for converting grayscale Mat objects to images
    """
    rows    = frame.shape[0]
    columns = frame.shape[1]

    im = make_image(columns, rows, 1)

    return im


def mat_to_image(frame):
    """
        Given a Mat object @frames (the object returned by ,
    """
    if frame.shape[2] == 3:
        return bgr_mat_to_image(frame)
    elif frame.shape[2] == 1:
        return grayscale_mat_to_image(frame)
    else:
        raise ValueError('invalid image format')


def gather_frames():
    """
        A substantial portion of this loop came from the url below
        https://stackoverflow.com/questions/18954889/how-to-process-images-of-a-video-frame-by-frame-in-video-streaming-using-opencv

        This method will use opencv to open the global variable video_file, and store every frame inside the returned
        numpy array.
    """
    print("gathering frames . . .")
    frames = []
    count = 0
    cap = cv2.VideoCapture(video_file)

    # go through all frames
    while cap.isOpened():
        ret, frame = cap.read()

        # check that read was successful
        if count > 3 or not ret or cv2.waitKey(10) & 0xFF == ord('q'):
            break

        frames.append(mat_to_image(frame))
        save_image(frames[count - 1], "test%s" % count)
        count = count + 1

    cap.release()
    cv2.destroyAllWindows()  # destroy all the opened windows

    print("done")

    frames = numpy.array(frames)
    return frames


def detect_movement(frames):
    """

    :param frames:
    :return: an array of booleans, the same length as @frames with 1 at the indices that "high movement" was detected in
             @frames (where the index is the first of the two frames with suspected movement).
    """
    print("detecting movement . . .")

    num_frames = len(frames)
    high_movement_frames = numpy.zeros(num_frames, dtype=bool)

    for index in range(num_frames - 1):
        a = frames[index]
        b = frames[index + 1]
        flow = optical_flow_images(b, a, 15, 8)
        draw_flow(a, flow, 8)
        save_image(a, "lines%s" % index)

    print("done")

    return high_movement_frames


"""
    Main method
"""


def main():
    frames = gather_frames()
    high_movement_frames = detect_movement(frames)


if __name__ == "__main__":
    main()

