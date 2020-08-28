#!/usr/bin/env python3

import cv2
import sys
import numpy as np

if len(sys.argv) < 2:
    print(f"Usage: {sys.argv[0]} image")
    exit(1)

img = cv2.imread(sys.argv[1], cv2.IMREAD_UNCHANGED)
alpha = img[:, :, 3]
alpha = cv2.threshold(alpha, 252, 255, cv2.THRESH_BINARY_INV)[1]

n, labels, stats, centroids = cv2.connectedComponentsWithStats(alpha, connectivity=8)

# compare centroids to image center
img_center = np.array([alpha.shape[0] // 2, alpha.shape[1] // 2])
# component which contains image center should be screen
screen_label = labels[img_center[0], img_center[1]]
x, y, width, height = stats[screen_label][:4]
print(f"+{x}+{y},{width}x{height}")

