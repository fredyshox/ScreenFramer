#!/usr/bin/env python3
# 
# Retrieve templates from fastlane/frameit
#

import sys
import os 
from os import path
from shutil import copyfile
from tempfile import gettempdir
import re
import json
import cv2
import numpy as np
from common import sanitize_color, sanitize_device_name, sanitize_device_key, apply_default_color

# URL to frameit-frames repository
FRAMEIT_URL = "https://github.com/fastlane/frameit-frames/archive/gh-pages.zip"

def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} resource_dir contents_file")
        exit(1)

    resource_dir = sys.argv[1]
    contents_path = sys.argv[2]
    zip_path = path.join(resource_dir, "gh-pages.zip")
    repo_dir = path.join(resource_dir, "frameit-frames-gh-pages")

    print("Downloading frameit frames...")
    status_code = os.system(f"wget -q --show-progress -O \"{zip_path}\" \"{FRAMEIT_URL}\" && unzip -d \"{resource_dir}\" \"{zip_path}\"")
    print(f"Status code: {status_code}")

    # path to latest frames
    frameit_dir = path.join(repo_dir, "latest")
    with open(contents_path, "r") as cf:
        contents = json.load(cf)

    for frame_path in os.listdir(frameit_dir):
        frame_path = path.join(frameit_dir, frame_path)
        filename = path.basename(frame_path)
        if not path.isfile(frame_path) or not filename_valid(filename):
            continue
        
        device_name = sanitize_device_name(filename)
        device_key = sanitize_device_key(device_name)
        device_color = sanitize_color(filename)
        print(f"Found template: {frame_path}")
        print(f"Template {device_name} - {device_color}")
        
        image = cv2.imread(frame_path, cv2.IMREAD_UNCHANGED) # read preserving alpha
        frame_height, frame_width = image.shape[:2]
        ox, oy, width, height = measure_screen_bounds(image)
        print(f"==> +{ox}+{oy}, {width}x{height}")

        if device_key in contents:
            device_info = contents[device_key]
        else:
            device_info = { 
                "images": {},
                "left": ox,
                "top": oy,
                "right": ox + width,
                "bottom": oy + height,
                "res_height": frame_height,
                "res_width": frame_width
            }
        device_info["images"][device_color] = filename
        
        contents[device_key] = device_info
        copyfile(frame_path, path.join(resource_dir, filename))

    # default colors - first model color which is available in DEFAULT_COLOR array
    for key in contents.keys():
        apply_default_color(contents, key)

    with open(contents_path, "w") as cf:
        json.dump(contents, cf, sort_keys=True, indent=4)

    print("Cleaning up...")
    os.system(f"rm {zip_path} && rm -r {repo_dir}")

def measure_screen_bounds(image):
    alpha = image[:, :, 3]
    alpha = cv2.threshold(alpha, 252, 255, cv2.THRESH_BINARY_INV)[1] # 99% threshold
    # connected component analysis
    n, labels, stats, centroids = cv2.connectedComponentsWithStats(alpha, connectivity=8)
    # compare centroids to image center
    img_center = np.array([alpha.shape[0] // 2, alpha.shape[1] // 2])
    # component which contains image center should be screen
    screen_label = labels[img_center[0], img_center[1]]
    x, y, width, height = stats[screen_label][:4]
    return int(x), int(y), int(width), int(height)

def filename_valid(filename):
    pattern = "^Apple iP.*\.png$"
    return re.search(pattern, filename) is not None

if __name__ == "__main__":
    main()
