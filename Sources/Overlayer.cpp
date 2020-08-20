//
// Created by Kacper Rączy on 23/07/2020.
//

#include "Overlayer.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <filesystem>

namespace avo {

OverlayConfig::OverlayConfig(
    std::string imagePath,
    int screenLeft,
    int screenTop,
    int screenRight,
    int screenBottom,
    int templateWidth,
    int templateHeight
): imagePath(imagePath), screenLeft(screenLeft),
   screenTop(screenTop), screenRight(screenRight),
   screenBottom(screenBottom), templateWidth(templateWidth),
   templateHeight(templateHeight) {}

bool OverlayConfig::isValid() const {
    bool screenBoundsValid =
        screenLeft >= 0 && screenTop >= 0 && screenBottom <= templateHeight && screenRight <= templateWidth;
    return std::filesystem::exists(imagePath) && screenBoundsValid;
}

int OverlayConfig::screenWidth() const {
    return screenRight - screenLeft;
}

int OverlayConfig::screenHeight() const {
    return screenBottom - screenTop;
}

Overlayer::Overlayer(const OverlayConfig &config) : _config(config) {
    cv::Mat bgraImage = cv::imread(config.imagePath, cv::IMREAD_UNCHANGED);
    if (bgraImage.empty()) {
        throw std::invalid_argument("File is not image: " + config.imagePath);
    }
    
    if (bgraImage.channels() != 4) {
        throw std::invalid_argument("Image is not BGRA: " + config.imagePath);
    }
    
    cv::extractChannel(bgraImage, _mask, 3);
    cv::cvtColor(bgraImage, _backgroundImage, cv::COLOR_BGRA2BGR);
}

OverlayConfig Overlayer::config() const {
    return _config;
}

template<class MatType>
Task<MatType> Overlayer::overlayTask(const OutputConfig &outputConfig) {
    return Task<MatType>(_backgroundImage, _mask, _config, outputConfig);
}

// explicit instantiation
template Task<cv::Mat> Overlayer::overlayTask<cv::Mat>(const OutputConfig &outputConfig);
template Task<cv::UMat> Overlayer::overlayTask<cv::UMat>(const OutputConfig &outputConfig);

}
