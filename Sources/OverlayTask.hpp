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
    // device frame as bgr
    MatType _device;
    // device frame mask (alpha) in 3-channel
    MatType _mask;
    // CV_8UC3 mat for resized video-frames
    MatType _u8Frame;
    // float bottom layer (background + screen)
    MatType _screenFrame;
    // float and u8 mats for storing result
    MatType _outputFloatFrame;
    MatType _outputFrame;
    // bgr background color
    cv::Scalar _backgroundColor;
    // offset of device frame (template)
    int _frameOriginX;
    int _frameOriginY;
    // offset of screen area
    int _screenOriginX;
    int _screenOriginY;
    // dimensions of screen - area where video-frames will be embedded in
    int _screenWidth;
    int _screenHeight;
    // dimensions of device frame (template)
    int _frameWidth;
    int _frameHeight;
public:
    Task(
        const cv::Mat &device,
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
