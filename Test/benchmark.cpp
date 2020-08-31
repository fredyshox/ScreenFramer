//
// Created by Kacper Rączy on 20/08/2020.
//

#include <iostream>
#include <vector>
#include <filesystem>
#include <chrono>
#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include "Overlayer.hpp"
#include "OutputConfig.hpp"

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
        return;
    }

    cv::ocl::Context context;
    if (!context.create(cv::ocl::Device::TYPE_ALL)) {
        std::cout << "Failed creating the context..." << std::endl;
        return;
    }

    std::cout << context.ndevices() << " OpenCL devices are detected." << std::endl; //This bit provides an overview of the OpenCL devices you have in your computer
    for (unsigned long i = 0; i < context.ndevices(); i++) {
        cv::ocl::Device device = context.device(i);
        std::cout << "name:              " << device.name() << std::endl;
        std::cout << "available:         " << device.available() << std::endl;
        std::cout << "imageSupport:      " << device.imageSupport() << std::endl;
        std::cout << "OpenCL_C_Version:  " << device.OpenCL_C_Version() << std::endl;
        std::cout << std::endl;
    }
    cv::ocl::Device(context.device(0)); //Here is where you change which GPU to use (e.g. 0 or 1)
}

template<class MatType>
void benchmark(avo::Overlayer& overlayer, avo::OutputConfig& output, const int iters = 300) {
    auto frame = genSampleMat<MatType>(886, 1920);
    // START task init
    auto taskStart = chrono::high_resolution_clock::now();
    auto task = overlayer.overlayTask<MatType>(output);
    task.initialize();
    auto taskEnd = chrono::high_resolution_clock::now();
    auto taskDuration = chrono::duration_cast<chrono::microseconds>(taskEnd - taskStart).count();
    std::cout << "   ==> Task creation: " << taskDuration << " us" << std::endl;
    // END task init

    // START average frame processing
    std::vector<long long> results;
    for (int i = 0; i < iters; i++) {
        auto start = chrono::high_resolution_clock::now();
        task.feedFrame(frame);
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
        results.push_back(duration);
    }
    task.finalize();
    auto avgResult = average(results);
    std::cout << "   ==> Avg frame proc.: " << avgResult << std::endl;
    // END average frame processing
}

int main(int argc, char** argv) {
    fs::path dir(RESOURCES_PATH);
    fs::path tempDir = fs::temp_directory_path();

    auto config = avo::OverlayConfig(
        (dir / "Apple iPhone 11 White.png").string(),
        75, 71,
        903, 1863,
        979, 1934);
    // Original tests were made on apples iPhone 11 template
    // with resolution 2286 x 4000, and was padded with 0.16x0.08
    auto output = avo::OutputConfig(
        tempDir / "sfbenchoutputS0SK28SHF73OZP74.mp4",
        60.0, 2286, 4000,
        0.16, 0.08);
    avo::Overlayer overlayer(config);

    cv::ocl::setUseOpenCL(true);
    checkOpenCL();
    std::cout<< "GPU" << std::endl;
    benchmark<cv::UMat>(overlayer, output, 1000);

    cv::ocl::setUseOpenCL(false);
    std::cout<< "CPU" << std::endl;
    benchmark<cv::Mat>(overlayer, output, 1000);

    return 0;
}