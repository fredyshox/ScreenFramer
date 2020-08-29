//
// Created by Kacper Rączy on 20/08/2020.
//

#ifndef SCREENFRAMER_OUTPUTCONFIG_HPP_
#define SCREENFRAMER_OUTPUTCONFIG_HPP_

#include <string>
#include <cstdint>

namespace avo {

struct RGBColor {
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    RGBColor(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0);
    RGBColor(uint32_t value);
    RGBColor(const std::string& rgbHexStr);
    std::string hexString();
};

struct OutputConfig {
    std::string path;
    double fps;
    int width;
    int height;
    double padding;
    RGBColor backgroundColor;

    OutputConfig(std::string path, double fps, int width, int height, double padding, RGBColor backgroundColor = {});
    ~OutputConfig() = default;
    bool isValid() const;
};

} // namespace avo

#endif //SCREENFRAMER_OUTPUTCONFIG_HPP_
