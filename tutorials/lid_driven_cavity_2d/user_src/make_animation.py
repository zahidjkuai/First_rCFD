"""
make_animation.py
Converts rCFD JPEG contour shots into an MP4 animation.
Place this script in the same folder as the .jpg files (post\rCFD_xxx).
Usage: python make_animation.py
"""

import cv2
import glob
import os
import sys

# ── settings ────────────────────────────────────────────────────────────────
IMG_PREFIX  = "rCFD_mix_"      # prefix of your jpg files  e.g. rCFD_mix_circle_
OUTPUT_FILE = "animation.mp4"
FPS         = 10               # frames per second (increase = faster video)
# ────────────────────────────────────────────────────────────────────────────

def make_mp4(img_prefix, output_file, fps):

    # find all matching jpgs sorted by name (= by episode number)
    pattern = img_prefix + "*.jpg"
    frames  = sorted(glob.glob(pattern))

    if len(frames) == 0:
        print(f"ERROR: no files matching '{pattern}' found in {os.getcwd()}")
        sys.exit(1)

    print(f"Found {len(frames)} frames  -->  {output_file}  @ {fps} fps")

    # read first frame to get dimensions
    first = cv2.imread(frames[0])
    if first is None:
        print(f"ERROR: cannot read {frames[0]}")
        sys.exit(1)

    h, w = first.shape[:2]

    # mp4v codec -- widely supported on Windows
    fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    writer = cv2.VideoWriter(output_file, fourcc, fps, (w, h))

    for f in frames:
        img = cv2.imread(f)
        if img is None:
            print(f"  WARNING: skipping unreadable frame {f}")
            continue
        writer.write(img)

    writer.release()
    print(f"Done: {output_file}  ({w}x{h}  {len(frames)} frames)")

if __name__ == "__main__":
    # allow overriding prefix from command line:  python make_animation.py rCFD_mix_circle_
    prefix = sys.argv[1] if len(sys.argv) > 1 else IMG_PREFIX
    make_mp4(prefix, OUTPUT_FILE, FPS)
