//
// Created by Kacper Rączy on 20/08/2020.
//

#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include "Overlayer.hpp"
#include "OverlayTask.hpp"

namespace fs = std::filesystem;
namespace chrono = std::chrono;

#ifndef RESOURCES_PATH
#error "RESOURCES_PATH must be defined before compilation"
#endif

template<typename Image>
Image genSampleMat(int width, int height) {
    Image img(width, height, CV_8UC3);
    img.setTo(cv::Scalar(0xff, 0xff, 0xff));

    return img;
}

template<typename Numeric>
Numeric average(std::vector<Numeric>& values) {
    Numeric acc = 0;
    for (Numeric& n : values) {
        acc += n;
    }

    return acc / values.size();
}

void checkOpenCL() {
    if (!cv::ocl::haveOpenCL()) {
        std::cout << "OpenCL is not available..." << std::endl;
        //return;
    }

    cv::ocl::Context context;
    if (!context.create(cv::ocl::Device::TYPE_ALL)) {
        std::cout << "Failed creating the context..." << std::endl;
        //return;
    }

    std::cout << context.ndevices() << " CPU devices are detected." << std::endl; //This bit provides an overview of the OpenCL devices you have in your computer
    for (int i = 0; i < context.ndevices(); i++) {
        cv::ocl::Device device = context.device(i);
        std::cout << "name:              " << device.name() << std::endl;
        std::cout << "available:         " << device.available() << std::endl;
        std::cout << "imageSupport:      " << device.imageSupport() << std::endl;
        std::cout << "OpenCL_C_Version:  " << device.OpenCL_C_Version() << std::endl;
        std::cout << std::endl;
    }
    cv::ocl::Device(context.device(0)); //Here is where you change which GPU to use (e.g. 0 or 1)
}

void benchmark(avo::Overlayer& overlayer, avo::OutputConfig& output) {
    cv::UMat frame = genSampleMat<cv::UMat>(886, 1920);
    auto taskStart = chrono::high_resolution_clock::now();
    auto task = overlayer.overlayTask(output);
    auto taskEnd = chrono::high_resolution_clock::now();
    auto taskDuration = chrono::duration_cast<chrono::microseconds>(taskEnd - taskStart).count();
    std::cout << "   ==> Task creation: " << taskDuration << " us" << std::endl;
    std::vector<long long> results;
    for (int i = 0; i < 300; i++) {
        auto start = chrono::high_resolution_clock::now();
        task.feedFrame(frame);
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
        results.push_back(duration);
    }
    task.finalize();
    auto avgResult = average(results);
    std::cout << "   ==> Avg frame proc.: " << avgResult << std::endl;
}

int main(int argc, char** argv) {
    fs::path dir(RESOURCES_PATH);
    fs::path tempDir = fs::temp_directory_path();

    auto config = avo::OverlayConfig(
        (dir / "iPhone 11 - vertical.png").string(),
        416,428,
        1870,3572,
        2286,4000);
    auto output = avo::OutputConfig(
        tempDir / "movieASKDPWIE234234VS33SSDF9.mp4",
        60.0,
        2286,
        4000
        );
    avo::Overlayer overlayer(config);

    cv::ocl::setUseOpenCL(true);
    //checkOpenCL();
    //std::cout<< "GPU" << std::endl;
    //benchmark(overlayer, output);

    cv::ocl::setUseOpenCL(false);
    std::cout<< "CPU" << std::endl;
    benchmark(overlayer, output);

    return 0;
}