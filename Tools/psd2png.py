#!/usr/bin/env python3

import sys
import json
from os import path
from argparse import ArgumentParser
import numpy as np
from psd_tools import PSDImage, compose
from psd_tools.constants import ChannelID, Compression
from psd_tools.psd.layer_and_mask import ChannelData, ChannelInfo, MaskData

def set_layer_mask_data(layer, channel_data, channel_info, array):
    """
    Writes input ndarray into channel data.
    Input array is float32 [0.0 - 1.0]
    """
    width, height = layer.mask.width, layer.mask.height
    depth, version = layer._psd.depth, layer._psd.version
    byte_aray = (array * 255).astype(np.uint8)
    buf = data_from_array(byte_aray.flatten(), depth)
    channel_data.set_data(buf, width, height, depth, version)
    channel_info.length = channel_data._length

def data_from_array(array, depth):
    """
    Array -> byte buffer encoding. Big-endian.
    """
    if depth == 8:
        # buffer is big-endian, but its not applicable since we're dealing only with bytes 
        return array.tobytes()
    else:
        raise ValueError(f"Unsupported depth: {depth}")

def find_background_layer(psd):
    layer = psd[0]
    return layer if layer.kind == 'solidcolorfill' else None

def find_device_layer(psd):
    pixel_layers = [layer for layer in psd.descendants() if layer.kind == 'pixel']
    if len(pixel_layers) != 0:
        # take the top one
        return pixel_layers.pop()
    return None

def find_content_layer(psd):
    """
    Content layer is smart object
    """
    for layer in psd.descendants():
        if layer.kind == 'smartobject':
            return layer

    return None

def db_load(p):
    with open(p, 'r') as f:
        return json.load(f)

def db_save(obj, p):
    with open(p, 'w') as f:
        json.dump(obj, f, sort_keys=True, indent=4)

# command line arguments
parser = ArgumentParser(description='AVO PSD to PNG converter')
parser.add_argument('input_file', type=str, nargs=1, help='Input PSD file')
parser.add_argument('output_file', type=str, nargs=1, help='Output PNG/PSD file')
parser.add_argument('--psd', action='store_true', help='Output as PSD')
parser.add_argument('--db', type=str, default=None, help='Path to JSON database')
args = parser.parse_args()

input_path = args.input_file[0]
output_path = args.output_file[0]
db_path = args.db
save_as_psd = args.psd

# open psd file
psd = PSDImage.open(input_path)

# each psd has 3 layers:
# - white background
# - device image
# - layer for content
background_layer = find_background_layer(psd)
pixel_layer = find_device_layer(psd)
content_layer = find_content_layer(psd)

# pixel layer and content layer are required
# background layer is optional
if pixel_layer is None or content_layer is None:
    print("Error: Invalid PSD. PSD lacks device pixel layer or content smart object layer.")
    exit(1)

if not content_layer.has_mask():
    # create artifical mask from rectangular area of content layer
    left, top, right, bottom = content_layer.bbox
    width, height = content_layer.width, content_layer.height
    depth, version = psd.depth, psd.version
    # artificial channeldata, channelinfo, maskdata mimicking ones from iPhone11
    data = b'\xff' * (width * height)
    channel_data = ChannelData(compression=Compression.RLE)
    channel_data.set_data(data, width, height, depth, version)
    channel_info = ChannelInfo(id=ChannelID.USER_LAYER_MASK, length=channel_data._length)
    mask_data = MaskData(
        top=top,
        left=left,
        bottom=bottom,
        right=right,
        background_color=0
    )
else:
    # find mask channel info/data
    channel_index, channel_info = next((
        (i, info) 
        for i, info in enumerate(content_layer._record.channel_info)
        if info.id == ChannelID.USER_LAYER_MASK
    ))
    channel_data = content_layer._channels[channel_index]
    mask_data = content_layer._record.mask_data
    # remove mask from content layer
    del content_layer._record.channel_info[channel_index]
    del content_layer._channels[channel_index]
    content_layer._record.mask_data = None

# add mask from content layer to pixel layer
pixel_layer._record.channel_info.append(channel_info)
pixel_layer._channels.append(channel_data)
pixel_layer._record.mask_data = mask_data

# invert mask 
pixel_layer._record.mask_data.background_color = 255
mask_image = pixel_layer.numpy('mask')
mask_image = np.abs(mask_image - 1.0)
set_layer_mask_data(pixel_layer, channel_data, channel_info, mask_image)

# hide layers other than masked pixel layer
if background_layer is not None:
    background_layer.visible = False
content_layer.visible = False

# save result
if save_as_psd:
    psd.save(output_path)
else:
    # convert to image
    #image = psd.composite(ignore_preview=True, force=True, layer_filter=lambda l: l.visible)
    #image = psd.topil()
    image = psd.compose(force=True, layer_filter=lambda l: l.visible)
    image.save(output_path)

# save metadata to json database
res_width, res_height = psd.width, psd.height
offset_x, offset_y = mask_data.left, mask_data.top
if db_path is not None:
    template_name = path.basename(output_path)
    key = path.splitext(template_name)[0]
    key = key.lower().replace(" ", "")
    db = db_load(db_path)
    db[key] = {
        "png_template_name": template_name,
        "offset_x": offset_x,
        "offset_y": offset_y,
        "res_width": res_width,
        "res_height": res_height
    }
    db_save(db, db_path)
    
