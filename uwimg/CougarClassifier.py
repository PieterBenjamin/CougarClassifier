from uwimg import *
import os

"""
    Variable Initialization
"""
VIDEO_FILE = sys.argv[1]
FPS = int(sys.argv[2])
THRESHOLD = 0.4
# Getting all frame names
FRAME_NAMES = []
FRAME_DIR = os.path.dirname(os.path.realpath(__file__)) + "/frames/"
for frame_name in os.listdir(FRAME_DIR):
    FRAME_NAMES.append(FRAME_DIR + frame_name)
FRAME_NAMES.sort()

print
print
print("analyzing video: " + VIDEO_FILE)
print
print


# Number of files, and number to process at once
NUM_FILES = len(FRAME_NAMES)
BATCH_SIZE = 500
# Make sure we don't look at way too much stuff
STEP_SIZE = int(FPS/3) if (FPS >= 15) else 1

"""
    Helper Methods
"""


def gather_frames(start, stop):
    """
    Gets all frames which should have been placed in frames/frame*.png

    :return: an array of frames,
    """

    print("Gathering frames [" + str(start) + ", " + str(stop) + "] from " + FRAME_DIR + " . . .")

    frames = []

    # Get a fresh batch
    for index in range(start, stop, STEP_SIZE):
        # make sure we don't go overboard
        if index >= NUM_FILES:
            break
        frames.append(load_image(FRAME_NAMES[index]))

    print("done")
    return frames


def detect_movement(frames, frame_movement, start):
    """

    :param frames:
    :return: an array of booleans, the same length as @frames with 1 at the indices that "high movement" was detected in
             @frames (where the index is the first of the two frames with suspected movement).
    """

    print("detecting movement in " + str(len(frames)) + " frames . . .")

    for frame in range(len(frames) - 1):
        if (frame + start) >= NUM_FILES:
            break
        flow = optical_flow_images(frames[frame + 1], frames[frame], 15, 8)

        # only look at dx, dy
        sum = 0
        for row in range(flow.h):
            for col in range(flow.c):
                # we only care about magnitude (hence abs)
                dx = get_pixel(flow, col, row, 0)
                dy = get_pixel(flow, col, row, 1)

                val = math.sqrt((dx * dx) + (dy * dy))
                if val >= THRESHOLD:
                    sum += val

        free_image(flow)
        # Progress update
        if frame % 100 == 0:
            print(frame)
        frame_movement.append((sum, frame + start))

    print("done")
    
def nth_most_movement(frame_movement, n):
    frame_movement.sort(key=lambda x: -x[0])
    print("The " + str(n) + " frame(s) with the highest movement are: ")
    for frame in range(n):
        tup = frame_movement[frame]
        print("frames/frame%05d.jpg with a rank of %s" % (tup[1] + 1, tup[0]))

        if tup[0] > 0:
            os.rename((FRAME_DIR + "frame%05d.jpg") % (tup[1] + 1),
                      (os.path.dirname(os.path.realpath(__file__)) + "/../high_movement_frames/frame%05d.jpg") % (
                                  tup[1] + 1))


"""
    Main method
"""


def main():
    frame_movement = []
    for batch in range((NUM_FILES / BATCH_SIZE) + 1):
        start = batch * BATCH_SIZE
        stop = (batch + 1) * BATCH_SIZE
        # Read in the next batch of frames
        frames = gather_frames(start, stop)

        # Look through the given frames and calculate movement
        detect_movement(frames, frame_movement, start)

        # Free everything we're done with (there will be one image at the end
        # that will be freed and read back in immediately)
        for frame in frames:
            free_image(frame)

    nth_most_movement(frame_movement, 20)


if __name__ == "__main__":
    main()

