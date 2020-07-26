//
// Created by Kacper Rączy on 23/07/2020.
//

#ifndef APPLEVIDEOOVERLAY_OVERLAYTASK_HPP
#define APPLEVIDEOOVERLAY_OVERLAYTASK_HPP

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

namespace avo {

struct OverlayConfig;
struct OutputConfig;

class Task {
private:
    cv::VideoWriter _outputWriter;
    cv::Mat _bg;
    cv::Mat _mask;
    cv::Mat _floatFrame;
    cv::Mat _outputFrame;
    cv::Scalar _backgroundColor;
    int _translatedOriginX;
    int _translatedOriginY;
    int _frameWidth;
    int _frameHeight;
public:
    Task(
        const cv::Mat &background,
        const cv::Mat &mask,
        const OverlayConfig &overlayConfig,
        const OutputConfig &outputConfig
    );

    void initialize();
    virtual void feedFrame(cv::Mat &rawFrame);
    bool isActive();
    void finalize();
};

}; // namespace avo


#endif //APPLEVIDEOOVERLAY_OVERLAYTASK_HPP
