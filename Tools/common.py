#!/usr/bin/env python3

import re
from os import path

# all available device colors (also non-apple)
DEVICE_COLORS = [
  "white band", # apple watch 
  "black band", # apple watch
  "Space Gray",
  "Rose Gold",
  "Jet Black",
  "Matte Black",
  "Pacific Blue",
  "Sky Blue",
  "Clearly White",
  "Just Black",
  "Not Pink",
  "Silver Titanium",
  "Arctic Silver",
  "Coral Blue",
  "Maple Gold",
  "Midnight Black",
  "Midnight Green",
  "Orchid Gray",
  "Burgundy Red",
  "Lilac Purple",
  "Sunrise Gold",
  "Titanium Gray",
  "Flamingo Pink",
  "Prism Black",
  "Prism Blue",
  "Prism Green",
  "Prism White",
  "Ceramic White",
  "Oh So Orange",
  "Aura Black",
  "Aura Glow",
  "Aura Pink",
  "Aura Red",
  "Aura White",
  "Aura Blue",
  "Black",
  "White",
  "Gold",
  "Silver",
  "Blue",
  "Red",
  "Yellow",
  "Green",
  "Pink",
  "Green",
  "Gray",
  "Coral",
  "Purple",
  "Graphite",
]

# every apple device right now is available in one of these colors
# darker colors have precedence over lighter ones
DEFAULT_COLORS = [
    "black-band", # apple watch series 5
    "space-gray",
    "black",
    "matte-black", # only iphone 7
    "silver",
    "white", # only iphone 5c
    "gold",
    "blue"
]

# renaming, prefer generation over years
IPAD_AIR_YEAR = "2019"
NEW_IPAD_AIR_YEAR = "2020"
IPAD_MINI_YEAR = "2019"
IPAD_AIR_GEN = "3"
NEW_IPAD_AIR_GEN = "4"
IPAD_MINI_GEN = "5"
IPAD_PRO_12_9_GEN = "3"
RENAME_SCHEME = {
    "iPad Pro (11 inch)": "iPad Pro 11",
    f"iPad Pro (12.9 inch) ({IPAD_PRO_12_9_GEN}rd generation)": "iPad Pro 12.9",
    f"iPad Air ({IPAD_AIR_YEAR})": f"iPad Air {IPAD_AIR_GEN}",
    f"iPad Air ({IPAD_AIR_YEAR}) {NEW_IPAD_AIR_YEAR}": f"iPad Air {NEW_IPAD_AIR_GEN}",
    f"iPad Mini ({IPAD_MINI_YEAR})": f"iPad Mini {IPAD_MINI_GEN}",
    f"iPodTouch Portrait": "iPod Touch"
}

def sanitize_color(filename):
    for color in DEVICE_COLORS:
        if color in filename:
            result = color.replace(" ", "-")
            result = result.lower()
            return result

    return "default"

def sanitize_device_name(filename):
    basename = path.basename(path.splitext(filename)[0])
    basename = basename.replace("Apple", "")
    basename = basename.replace("-", " ")
    # remove color
    for color in DEVICE_COLORS:
        basename = basename.replace(color, "")
    basename = basename.strip()
    # renaming using dict
    if basename in RENAME_SCHEME:
        basename = RENAME_SCHEME[basename]
    return basename

def sanitize_device_key(device_name):
    key = device_name.replace(" ", "")
    key = re.sub(r"[\.\(\)]", "", key)
    return key.lower()

def apply_default_color(template_dict, key):
    value = template_dict[key]
    if "default_image" in value:
        return

    print(f"Setting default color for {key}...")
    available_colors = value["images"].keys()
    try:
        color = next(color for color in DEFAULT_COLORS if color in available_colors)
        value["default_image"] = color
        print(f"==> {color}")
    except:
        print("==> unable to deduct default color")

