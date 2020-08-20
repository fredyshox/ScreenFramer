//
// Created by Kacper Rączy on 23/07/2020.
//

#include "OverlayTask.hpp"
#include "Overlayer.hpp"
#include "Utility.hpp"
#include <opencv2/imgproc.hpp>

#ifdef MACOS_APP
#define API_PREFERENCE cv::CAP_AVFOUNDATION
#else
#define API_PREFERENCE cv::CAP_ANY
#endif

namespace avo {

Task::Task(
    const cv::Mat &background,
    const cv::Mat &mask,
    const OverlayConfig &overlayConfig,
    const OutputConfig &outputConfig
): _outputConfig(outputConfig) {
    if (background.empty() || mask.empty() || mask.channels() != 1) {
        throw std::invalid_argument("Background/Mask are invalid (are empty or have invalid channel count");
    }

    if (!overlayConfig.isValid()) {
        throw std::invalid_argument("OverlayConfig is not valid");
    }

    if (!outputConfig.isValid()) {
        throw std::invalid_argument("OutputConfig is not valid!");
    }

    double fx = (double) outputConfig.width / (double) background.cols;
    double fy = (double) outputConfig.height / (double) background.rows;
    int translatedOriginX = (int) round(overlayConfig.screenLeft * fx);
    int translatedOriginY = (int) round(overlayConfig.screenTop * fy);
    int translatedEndingX = (int) round(overlayConfig.screenRight * fx);
    int translatedEndingY = (int) round(overlayConfig.screenBottom * fy);
    int frameWidth = translatedEndingX - translatedOriginX;
    int frameHeight = translatedEndingY - translatedOriginY;
    _frameHeight = frameHeight;
    _frameWidth = frameWidth;
    _translatedOriginX = translatedOriginX;
    _translatedOriginY = translatedOriginY;
    // opencv default is BGR
    _backgroundColor = {
        (double) outputConfig.backgroundColor.blue,
        (double) outputConfig.backgroundColor.green,
        (double) outputConfig.backgroundColor.red
    };

    DEBUG_PRINTLN("*** Embedded frame dimensions: [" << _frameWidth << ", " << _frameHeight << "]");
    DEBUG_PRINTLN("*** Translated ox - " << _translatedOriginX << ", oy - " << _translatedOriginY);

    // resize background and mask to desired size
    cv::resize(background, _bg, {outputConfig.width, outputConfig.height});
    cv::resize(mask, _mask, {outputConfig.width, outputConfig.height});

    // mask -> 3 channels, float, and invert
    _mask.convertTo(_mask, CV_32F);
    _mask /= 255.0;
    cv::Mat temp[] = {_mask, _mask, _mask};
    cv::merge(temp, 3, _mask);

    // background -> float
    _bg.convertTo(_bg, CV_32F);
    cv::multiply(_bg, _mask, _bg);

    // invert mask
    cv::subtract(1.0, _mask, _mask);
}

void Task::initialize() {
    if (isActive()) {
        throw std::runtime_error("Task is already active");
    }

    // allocate memory and prepare output frame
    int outputWidth = _outputConfig.width, outputHeight = _outputConfig.height;
    _screenFrame.create(outputHeight, outputWidth, CV_32FC3);
    _screenFrame.setTo(_backgroundColor);
    // screen bounds + 1-pix border
    cv::Rect roi(_translatedOriginX - 1, _translatedOriginY - 1, _frameWidth + 2, _frameHeight + 2);
    _screenFrame(roi).setTo(cv::Scalar(0.0, 0.0, 0.0));
    // mats for storing output
    _outputFloatFrame.create(outputHeight, outputWidth, CV_32FC3);
    _outputFrame.create(outputHeight, outputWidth, CV_8UC3);

    // allocate memory for floating point frame and uint8 frame (resized frame)
    _u8Frame.create(_frameHeight, _frameWidth, CV_32FC3);

    // setup and open output video
    int fourcc = cv::VideoWriter::fourcc('a', 'v', 'c', '1');
    cv::Size size = {outputWidth, outputHeight};
    bool res = _outputWriter.open(_outputConfig.path, API_PREFERENCE, fourcc, _outputConfig.fps, size);
    DEBUG_PRINT("*** OPEN result: " << res);
    if (res) {
        DEBUG_PRINTLN(", backend: " << _outputWriter.getBackendName());
    } else {
        DEBUG_PRINT("\n");
    }
}

void Task::feedFrame(cv::Mat &rawFrame) {
    if (!isActive()) {
        throw std::runtime_error("Task is not active");
    }

    // frame resize +  float convertion
    cv::resize(rawFrame, _u8Frame, {_frameWidth, _frameHeight});

    // embed float frame inside output frame, in screen bounds
    cv::Rect screenRect(_translatedOriginX, _translatedOriginY, _frameWidth, _frameHeight);
    _u8Frame.convertTo(_screenFrame(screenRect), CV_32F);

    // alpha blending
    cv::multiply(_mask, _screenFrame, _outputFloatFrame);
    cv::add(_outputFloatFrame, _bg, _outputFloatFrame);

    // back to uint8
    _outputFloatFrame.convertTo(_outputFrame, CV_8U);

    // write generated frame to writer
    _outputWriter.write(_outputFrame);
}

bool Task::isActive() const {
    return _outputWriter.isOpened();
}

void Task::finalize() {
    _outputWriter.release();
}

}
