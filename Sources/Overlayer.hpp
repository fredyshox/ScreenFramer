//
// Created by Kacper Rączy on 23/07/2020.
//

#ifndef APPLEVIDEOOVERLAY_OVERLAYER_HPP
#define APPLEVIDEOOVERLAY_OVERLAYER_HPP

#include <opencv2/core.hpp>
#include <string>
#include <cstdint>
#include "OverlayTask.hpp"

namespace avo {

struct RGBColor {
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    RGBColor(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0);
    RGBColor(uint32_t value);
    std::string hexString();
};

struct OverlayConfig {
    std::string imagePath;
    int originX;
    int originY;
    int templateWidth;
    int templateHeight;

    OverlayConfig() = default; // default constructable for json
    OverlayConfig(
        std::string imagePath,
        int originX,
        int originY,
        int templateWidth,
        int templateHeight
    );
    ~OverlayConfig() = default;
    bool isValid();
};

struct OutputConfig {
    std::string path;
    double fps;
    int width;
    int height;
    RGBColor backgroundColor;

    OutputConfig(std::string path, double fps, int width, int height, RGBColor backgroundColor = {});
    ~OutputConfig() = default;
};

class Overlayer {
private:
    OverlayConfig _config;
protected:
    cv::Mat _backgroundImage;
    cv::Mat _mask;
public:
    explicit Overlayer(const OverlayConfig &config);
    OverlayConfig config();
    Task overlayTask(const OutputConfig& outputConfig);
};

}; // namespace avo

#endif //APPLEVIDEOOVERLAY_OVERLAYER_HPP
