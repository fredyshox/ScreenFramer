//
// Created by Kacper Rączy on 23/07/2020.
//

#include "OverlayTask.hpp"
#include "Overlayer.hpp"
#include "Debug.hpp"
#include <opencv2/imgproc.hpp>

#ifdef MACOS_APP
#define API_PREFERENCE cv::CAP_AVFOUNDATION
#else
#define API_PREFERENCE cv::CAP_ANY
#endif

namespace avo {

template<class MatType>
Task<MatType>::Task(
    const cv::Mat &device,
    const cv::Mat &mask,
    const OverlayConfig &overlayConfig,
    const OutputConfig &outputConfig
): _outputConfig(outputConfig) {
    if (device.empty() || mask.empty() || mask.channels() != 1) {
        throw std::invalid_argument("DeviceFrame/Mask are invalid (are empty or have invalid channel count");
    }

    if (!overlayConfig.isValid()) {
        throw std::invalid_argument("OverlayConfig is not valid");
    }

    if (!outputConfig.isValid()) {
        throw std::invalid_argument("OutputConfig is not valid!");
    }

    // translate offsets/dimensions according to config
    double frameWidth = (double) outputConfig.width / (1.0 + 2 * outputConfig.paddingHorizontal);
    double frameHeight = (double) outputConfig.height / (1.0 + 2 * outputConfig.paddingVertical);
    double fx = frameWidth / (double) device.cols;
    double fy = frameHeight / (double) device.rows;
    int translatedOriginX = (int) round(overlayConfig.screenLeft * fx);
    int translatedOriginY = (int) round(overlayConfig.screenTop * fy);
    int translatedEndingX = (int) round(overlayConfig.screenRight * fx);
    int translatedEndingY = (int) round(overlayConfig.screenBottom * fy);
    int screenWidth = translatedEndingX - translatedOriginX;
    int screenHeight = translatedEndingY - translatedOriginY;
    _frameOriginX = (int) (outputConfig.paddingHorizontal * frameWidth);
    _frameOriginY = (int) (outputConfig.paddingVertical * frameHeight);
    _screenOriginX = translatedOriginX + _frameOriginX;
    _screenOriginY = translatedOriginY + _frameOriginY;
    _screenWidth = screenWidth;
    _screenHeight = screenHeight;
    _frameWidth = (int) frameWidth;
    _frameHeight = (int) frameHeight;
    // opencv default is BGR
    _backgroundColor = {
        (double) outputConfig.backgroundColor.blue,
        (double) outputConfig.backgroundColor.green,
        (double) outputConfig.backgroundColor.red
    };

    DEBUG_PRINTLN("*** Embedded frame dimensions: [" << _frameWidth << ", " << _frameHeight << "]");
    DEBUG_PRINTLN("*** Embedded screen dimensions: [" << _screenWidth << ", " << _screenHeight << "]");
    DEBUG_PRINTLN("*** Translated frame ox - " << _frameOriginX << ", oy - " << _frameOriginY);
    DEBUG_PRINTLN("*** Translated screen ox - " << _screenOriginX << ", oy - " << _screenOriginY);

    // resize device frame and mask to desired size
    cv::Mat tempMask;
    cv::Mat tempDevice;
    cv::resize(device, tempDevice, {_frameWidth, _frameHeight});
    cv::resize(mask, tempMask, {_frameWidth, _frameHeight});

    // mask -> 3 float channels [0.0, 1.0]
    tempMask.convertTo(tempMask, CV_32F);
    tempMask /= 255.0;
    cv::cvtColor(tempMask, tempMask, cv::COLOR_GRAY2BGR);

    // device -> float
    tempDevice.convertTo(tempDevice, CV_32F);

    // device * mask -> _device
    cv::multiply(tempDevice, tempMask, _device);

    // invert mask
    cv::subtract(1.0, tempMask, tempMask);
    tempMask.copyTo(_mask);
}

template<class MatType>
void Task<MatType>::initialize() {
    if (isActive()) {
        throw std::runtime_error("Task is already active");
    }

    // allocate memory and prepare output frame
    int outputWidth = _outputConfig.width, outputHeight = _outputConfig.height;
    _screenFrame.create(outputHeight, outputWidth, CV_32FC3);
    _screenFrame.setTo(_backgroundColor);
    // screen bounds + 1-pix border
    cv::Rect roi(_screenOriginX - 1, _screenOriginY - 1, _screenWidth + 2, _screenHeight + 2);
    _screenFrame(roi).setTo(cv::Scalar(0.0, 0.0, 0.0));
    // mats for storing output
    _outputFloatFrame.create(outputHeight, outputWidth, CV_32FC3);
    _outputFloatFrame.setTo(_backgroundColor);
    _outputFrame.create(outputHeight, outputWidth, CV_8UC3);

    // allocate memory for floating point frame and uint8 frame (resized frame)
    _u8Frame.create(_screenHeight, _screenWidth, CV_32FC3);

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

template<class MatType>
void Task<MatType>::feedFrame(MatType &rawFrame) {
    if (!isActive()) {
        throw std::runtime_error("Task is not active");
    }

    // frame resize +  float convertion
    cv::resize(rawFrame, _u8Frame, {_screenWidth, _screenHeight});

    // embed float frame inside output frame, in screen bounds
    cv::Rect screenRect(_screenOriginX, _screenOriginY, _screenWidth, _screenHeight);
    _u8Frame.convertTo(_screenFrame(screenRect), CV_32F);

    // alpha blending
    cv::Rect frameRect(_frameOriginX, _frameOriginY, _frameWidth, _frameHeight);
    cv::multiply(_screenFrame(frameRect), _mask, _outputFloatFrame(frameRect));
    cv::add(_outputFloatFrame(frameRect), _device, _outputFloatFrame(frameRect));

    // back to uint8
    _outputFloatFrame.convertTo(_outputFrame, CV_8U);

    // write generated frame to writer
    _outputWriter.write(_outputFrame);
}

template<class MatType>
bool Task<MatType>::isActive() const {
    return _outputWriter.isOpened();
}

template<class MatType>
void Task<MatType>::finalize() {
    _outputWriter.release();
}

// explicit instantiation
template class Task<cv::Mat>;
template class Task<cv::UMat>;

}
