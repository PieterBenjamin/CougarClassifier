#!/bin/bash
cd "$(dirname "$0")"
DIRECTORY="uwimg/frames/"

# Script designed to simplify process of frame splitting/motion analysis
# Splitting
if [[ -f $1 ]]; then
  echo "Splitting " + pwd + "/$1 into frames"
  ffmpeg -i $1 frame%05d.jpg
else
  echo
  echo "Sorry, $1 is not a file."
  echo "note: spaces are not permitted in a file name"
  echo
fi

# Saving
if [[ -d ${DIRECTORY} ]]; then
    echo "removing all frames from $DIRECTORY"
    rm -rf ${DIRECTORY}
fi

# Output dir
rm -rf "high_movement_frames"
mkdir "high_movement_frames"

mkdir uwimg/frames
mv frame*.jpg uwimg/frames/

# Motion analysis
fps=$(ffprobe -v error -select_streams v -of default=noprint_wrappers=1:nokey=1 -show_entries stream=r_frame_rate $1)
fps=$(bc <<< ${fps})

python uwimg/CougarClassifier.py $1 ${fps}
