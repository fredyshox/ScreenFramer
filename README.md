<div align="center">
  <img src="./Images/AppIcon.png" width="200" height="200">
  <h1>ScreenFramer</h1>
  <h5>Frame recordings from your apps using device mockups.</h5>
  <br>
</div>


This is command-line tool which helps to put screen recordings from your apps or prototypes in device frames from iPhone, iPad and Apple Watch.

## Why?

I often record my app prototypes running in iOS simulator or on device to share on Github or social media. But these recordings don’t come with device frame overlay, and applying it is just not as easy as it is with still images. It’s also really inconvienient to open Final Cut Pro or iMovie (frankly it’s not even possible to do it there without tricks) just for that. So I’ve created simple command-line tool for that (and then macOS GUI app too).

<div align="center">
  <img src="./Images/Showcase.png" width="900">
</div>

## Features

* Templates for modern models of iPhone, iPad and Apple Watch
* Ability to choose video background color
* Ability to control video dimensions
* Device frame padding support
* Outputs video using H.264 codec
* Command line interface

## macOS App

If you prefer using graphical user interface, there is native macOS app available for symbolic price.

[![](https://linkmaker.itunes.apple.com/assets/shared/badges/en-us/macappstore-lrg.svg)](https://apps.apple.com/pl/app/screenframer/id1527621584?l=pl&mt=12)

<img src="Images/macOS app screenshot.png">

## Install

If you're using macOS, you can install via [Homebrew](https://brew.sh):

```
brew tap fredyshox/tools
brew install screenframer
```

For platforms other than mac, you can build from source.

## Usage

To overlay screen recording at `INPUTPATH`, and save output at `OUTPUTPATH` run:

`screenframer [OPTIONS...] INPUTPATH OUTPUTPATH`

Available options:

* `-t, --template arg` Device template in format `device_color` (default - `auto`). Look at available templates below.
* `-w, --width arg` Output video width (default - template width)
* `-h, --height arg` Output video height (default - template height)
* `-p, --padding arg` Device frame padding (default - `0.16:`). Look at padding syntax below.
* `-c, --color arg` Background color in hex (default - #000000)

### Padding syntax 

Padding can be specified using fraction of template dimensions. Available options:

* `X.Y` uniform padding of `X.Y`
* `X.Y:` horizontal padding of `X.Y`, and automatic vertical padding to be equal
* `:X.Y` vertical padding of `X.Y`, and automatic horizontal padding to be equal

### Available templates

##### iPhones

* iPhone 12: `iphone12`
  * Colors: `black`, `blue`, `green`, `red`, `white`
* iPhone 12 Mini: `iphone12mini`
  * Colors: `black`, `blue`, `green`, `red`, `white`
* iPhone 12 Pro: `iphone12pro`
  * Colors: `gold`, `graphite`, `pacific-blue`, `silver`
* iPhone 12 Pro: `iphone12promax`
  * Colors: `gold`, `graphite`, `pacific-blue`, `silver`
* iPhone 11: `iphone11` 
  * Colors:  `black`,  `green`,  `purple`,  `red`,  `white`,  `yellow`
* iPhone 11 Pro: `iphone11pro`
  * Colors: `gold`,  `midnight-green`,  `silver`,  `space-gray`
* iPhone 11 Pro Max: `iphone11promax`
  * Colors: `gold`,  `midnight-green`,  `silver`,  `space-gray`
* iPhone Xs: `iphonexs`
  * Colors: `gold`, `silver`, `space-gray`
* iPhone Xs Max: `iphonexsmax`
  * Colors: `gold`, `silver`, `space-gray`
* iPhone Xr: `iphonexr`
  * Colors: `blue`, `coral`, `red`, `silver`, `space-gray`, `yellow`
* iPhone X: `iphonex`
  * Colors: `silver`, `space-gray`
* iPhone 8: `iphone8`
  * Colors: `gold`, `silver`, `space-gray`
* iPhone 8 Plus: `iphone8plus`
  * Colors: `gold`, `silver`, `space-gray`
* iPhone 7: `iphone7`:
  * Colors: `gold`, `jet-black`, `matte-black`, `rose-gold`, `silver`
* iPhone 7 Plus: `iphone7plus`
  * Colors: `gold`, `jet-black`, `matte-black`, `rose-gold`, `silver`
* iPhone 6s: `iphone6s`
  * Colors: `gold`, `rose-gold`, `silver`, `space-gray`
* iPhone 6s Plus: `iphone6splus`:
  * Colors: `gold`, `rose-gold`, `silver`, `space-gray`
* iPhone SE 1st gen: `iphonese`
  * Colors: `gold`,  `rose-gold`,  `silver`,  `space-gray`
* iPhone 5s: `iphone5s`
  * Colors: `gold`, `silver`, `space-gray`
* iPhone 5c: `iphone5c`:
  * Colors: `blue`, `green`, `red`, `white`, `yellow`

##### iPad

* iPad 10.2: `ipad102`
  * Colors: `gold`, `silver`, `space-gray`
* iPad Pro 10.5 (2 gen): `ipadpro`
  * Colors: `gold`, `silver`, `space-gray`
* iPad Pro 11 (3/4 gen): `ipadpro11`
  * Colors: `silver`, `space-gray`
* iPad Pro 12.9 (3/4 gen): `ipadpro129`
  * Colors: `silver`, `space-gray`
* iPad Air 4 (2020): `ipadair4`
  * Colors: `green`, `rose-gold`, `silver`, `sky-blue`, `space-gray`
* iPad Air 3 (2019): `ipadair3`
  * Colors: `gold`, `silver`, `space-gray`
* iPad Air 2: `ipadair2`
  * Colors: `gold`, `silver`, `space-gray`
* iPad Mini 5 (2019): `ipadmini5`
  * Colors: `gold`, `silver`, `space-gray`

##### Apple Watch

* Apple Watch Series 5: `watchseries5`
  * Colors: `black-band`, `white-band`

##### iPod Touch

* iPod Touch (5/6/7 gen): `ipodtouch`
  * Colors: `blue`, `silver`

### Examples

Create video with iPhone 11 Pro frame with default color, over white background, with width equal to 960 (while maintaining proper aspect ratio).

```
screenframer --template iphone11pro --width 960 --color '#FFFFFF' INPUTPATH OUTPUTPATH
```

Create video with Apple Watch Series 5 with black band frame, over green background, with height equal to 480.

```
screenframer --template watchseries5_black-band --height 480 --color '#03BD5B' INPUTPATH OUTPUTPATH
```

Create video with green iPhone 11 frame, over black background with padding  `0.1` on each side.

```
screenframer --template iphone11_green --color '#000000' --padding 0.1 INPUTPATH OUTPUTPATH
```

## Build

### Requirements

* C++17
* clang++
* CMake 3.16+

### Dependencies

* [nhlomann-json](https://github.com/nlohmann/json) 3.8+
* [cxxopts](https://github.com/jarro2783/cxxopts) 2.0+
* [OpenCV](https://opencv.org) 4+

If you're using Homebrew, just type `brew install nhlomann-json cxxopts opencv`.

Project is also using [cpptqdm](https://github.com/aminnj/cpptqdm), which is located in [Dependecies directory](/Dependencies/tqdm/tqdm.hpp).

### Building from source

```
mkdir Build/
cd Build/
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
```

### Install from source

Install at path `prefix`

```
mkdir Build/
cd Build/
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${prefix}
make install
```

## FAQ

### Gif support

Not at the moment. But there are cool convertion tools for that, like [Gifski](https://gif.ski/).

## TODO

* [x] Auto template selection based on video aspect ratio
* [x] Templates for older devices
* [ ] GPU support using OpenCL
* [ ] Templates in landscape mode
* [ ] Gradient background
* [ ] Linux support (distribution)
