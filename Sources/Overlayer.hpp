//
// Created by Kacper Rączy on 23/07/2020.
//

#ifndef SCREENFRAMER_OVERLAYER_HPP
#define SCREENFRAMER_OVERLAYER_HPP

#include <opencv2/core.hpp>
#include <string>
#include <cstdint>
#include "OverlayTask.hpp"
#include "OutputConfig.hpp"

namespace avo {

struct OverlayConfig {
    std::string imagePath;
    int screenLeft;
    int screenTop;
    int screenRight;
    int screenBottom;
    int templateWidth;
    int templateHeight;

    OverlayConfig() = default; // default constructable for json
    OverlayConfig(
        std::string imagePath,
        int screenLeft,
        int screenTop,
        int screenRight,
        int screenBottom,
        int templateWidth,
        int templateHeight
    );
    ~OverlayConfig() = default;
    bool isValid() const;
    int screenWidth() const;
    int screenHeight() const;
};

class Overlayer {
private:
    OverlayConfig _config;
protected:
    cv::Mat _backgroundImage;
    cv::Mat _mask;
public:
    explicit Overlayer(const OverlayConfig &config);
    OverlayConfig config() const;

    template<class MatType>
    Task<MatType> overlayTask(const OutputConfig& outputConfig);
};

} // namespace avo

#endif //SCREENFRAMER_OVERLAYER_HPP
