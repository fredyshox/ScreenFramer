//
// Created by Kacper Rączy on 23/07/2020.
//

#ifndef SCREENFRAMER_OVERLAYTASK_HPP
#define SCREENFRAMER_OVERLAYTASK_HPP

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include "OutputConfig.hpp"

namespace avo {

struct OverlayConfig;

template<class MatType>
class Task {
private:
    OutputConfig _outputConfig;
    cv::VideoWriter _outputWriter;
    MatType _bg;
    MatType _mask;
    MatType _u8Frame;
    MatType _screenFrame;
    MatType _outputFloatFrame;
    MatType _outputFrame;
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
    virtual void feedFrame(MatType &rawFrame);
    bool isActive() const;
    void finalize();
};

}; // namespace avo


#endif //SCREENFRAMER_OVERLAYTASK_HPP
