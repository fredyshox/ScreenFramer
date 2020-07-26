//
// Created by Kacper Rączy on 23/07/2020.
//

#include "OverlayTask.hpp"
#include "Overlayer.hpp"
#include "Utility.hpp"
#include <opencv2/imgproc.hpp>

namespace avo {

Task::Task(
    const cv::Mat &background,
    const cv::Mat &mask,
    const OverlayConfig &overlayConfig,
    const OutputConfig &outputConfig
) {
    double fx = (double) outputConfig.width / (double) background.cols;
    double fy = (double) outputConfig.height / (double) background.rows;
    int translatedOriginX = (int) round(overlayConfig.originX * fx);
    int translatedOriginY = (int) round(overlayConfig.originY * fy);
    int frameWidth = outputConfig.width - 2 * translatedOriginX;
    int frameHeight = outputConfig.height - 2 * translatedOriginY;
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

    int fourcc = cv::VideoWriter::fourcc('X', '2', '6', '4');
    cv::Size size = {outputConfig.width, outputConfig.height};
    _outputWriter.open(outputConfig.path, fourcc, outputConfig.fps, size);
}

void Task::initialize() {

}

void Task::feedFrame(cv::Mat &rawFrame) {
    if (!isActive()) {
        // throw something
        return;
    }

    // frame resize +  float convertion
    cv::resize(rawFrame, _outputFrame, {_frameWidth, _frameHeight});
    // 2 pixel black border to avoid letting background through on frame border
    cv::copyMakeBorder(
        _outputFrame,
        _outputFrame,
        2, 2, 2, 2,
        cv::BORDER_CONSTANT,
        {0, 0, 0});
    // background
    cv::copyMakeBorder(
            _outputFrame,
            _outputFrame,
            _translatedOriginY - 2,
            _translatedOriginY - 2,
            _translatedOriginX - 2,
            _translatedOriginX - 2,
            cv::BORDER_CONSTANT,
            _backgroundColor
    );
    _outputFrame.convertTo(_floatFrame, CV_32F);

    // alpha blending
    cv::multiply(_mask, _floatFrame, _floatFrame);
    cv::add(_floatFrame, _bg, _floatFrame);

    // back to uint8
    _floatFrame.convertTo(_outputFrame, CV_8U);

    // write generated frame to writer
    _outputWriter.write(_outputFrame);
}

bool Task::isActive() {
    return _outputWriter.isOpened();
}

void Task::finalize() {
    _outputWriter.release();
}

}