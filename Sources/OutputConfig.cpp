//
// Created by Kacper Rączy on 20/08/2020.
//

#include "OutputConfig.hpp"
#include <iomanip>
#include <sstream>

namespace avo {

RGBColor::RGBColor(
    uint8_t red,
    uint8_t green,
    uint8_t blue
): red(red), green(green), blue(blue) {}

RGBColor::RGBColor(uint32_t value) {
    blue = (uint8_t) (value & 0xff);
    green = (uint8_t) ((value >> 8u) & 0xff);
    red = (uint8_t) ((value >> 16u) & 0xff);
}

std::string RGBColor::hexString() {
    uint32_t value = ((uint32_t) red << 16u) | ((uint32_t) blue << 8u) | ((uint32_t) green);
    std::stringstream ss;
    ss << '#' << std::uppercase << std::setfill('0') << std::setw(6) << std::right << std::hex << value;
    return ss.str();
}

OutputConfig::OutputConfig(
    std::string path,
    double fps,
    int width,
    int height,
    RGBColor backgroundColor
): path(path), fps(fps), width(width), height(height), backgroundColor(backgroundColor) {}

bool OutputConfig::isValid() const {
    return width > 0 && height > 0 && fps > 0.0;
}

}