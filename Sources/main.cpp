//
//  main.cpp
//  AppleVideoOverlay
//
//  Created by Kacper Rączy on 24/07/2020.
//

#include <iostream>
#include <string>
#include <fstream>
#include <cassert>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
#include "Overlayer.hpp"
#include "Utility.hpp"
#include "tqdm.hpp"

using nlohmann::json;
namespace fs = std::filesystem;

#ifndef RESOURCES_PATH
#error "RESOURCES_PATH must be defined before compilation"
#endif
#ifndef VERSION_NUMBER
#error "VERSION NUMBER must be defined before compilation"
#endif
#define TEMPLATE_IMAGES_PATH RESOURCES_PATH
#define CONTENTS_JSON_PATH (fs::path(RESOURCES_PATH) / fs::path("contents.json")).string()

// OverlayConfig json deserialization
namespace avo {
void from_json(const json& j, avo::OverlayConfig& cfg) {
    fs::path dir(TEMPLATE_IMAGES_PATH);
    std::string templateName = j.at("png_template_name");
    fs::path fullPath = dir / templateName;
    cfg.imagePath = fullPath.string();
    j.at("offset_x").get_to(cfg.originX);
    j.at("offset_y").get_to(cfg.originY);
    j.at("res_width").get_to(cfg.templateWidth);
    j.at("res_height").get_to(cfg.templateHeight);
}
} // namespace avo

avo::RGBColor parseHex(const std::string &rgbHexStr) {
    uint32_t hexValue;
    if (rgbHexStr.length() != 7 || sscanf(rgbHexStr.c_str(), "#%6x", &hexValue) != 1) {
        throw std::invalid_argument("RGB hex string is invalid");
    }

    return { hexValue };
}

/**
 * Usage: avo VIDEOPATH OUTPUTPATH
 *
 * Options:
 * -t,template - Device model template
 * -w,width - Output video width
 * -h,height - Output video height
 */
int main(int argc, char** argv) {
    std::string videoPath;
    std::string outputPath;
    std::string templateKey;
    avo::RGBColor backgroundColor;
    int width, height;

    // parse command line arguments
    cxxopts::Options options("AppleVideoOverlay", "Overlay videos from Apple devices");
    options.add_options()
        ("t,template", "Device model template", cxxopts::value<std::string>()->default_value("iphone11"))
        ("w,width", "Output video width", cxxopts::value<int>()->default_value("0"))
        ("h,height", "Output video height", cxxopts::value<int>()->default_value("0"))
        ("c,color", "Background color", cxxopts::value<std::string>()->default_value("#000000"))
        ("help", "Print help")
        ("version", "Print version")
        ("inputVideo", "Input video", cxxopts::value<std::string>())
        ("outputVideo", "Output video", cxxopts::value<std::string>())
        ;
    options.parse_positional({"inputVideo", "outputVideo"});
    options.positional_help("VIDEOPATH OUTPUTPATH");

    try {
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if (result.count("version")) {
            std::cout << VERSION_NUMBER << std::endl;
            return 0;
        }
        videoPath = result["inputVideo"].as<std::string>();
        outputPath = result["outputVideo"].as<std::string>();
        templateKey = result["template"].as<std::string>();
        width = result["width"].as<int>();
        height = result["height"].as<int>();
        std::string rgbHexStr = result["color"].as<std::string>();
        backgroundColor = parseHex(rgbHexStr);
    } catch (const std::exception& e) {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        std::cerr << options.help() << std::endl;
        return 1;
    }

    // check if input file exists
    if (!fs::exists(videoPath)) {
        std::cerr << "Input video file does not exist at: " << videoPath << std::endl;
        return 2;
    }

    // load template json from resources
    DEBUG_PRINTLN("*** Contents JSON path: " << CONTENTS_JSON_PATH);
    std::ifstream configFile(CONTENTS_JSON_PATH);
    nlohmann::json configJson;
    configFile >> configJson;
    configFile.close();

    if (!configJson.contains(templateKey)) {
        std::cerr << "Error: invalid template " << templateKey << std::endl;
        return 3;
    }
    auto config = configJson[templateKey].get<avo::OverlayConfig>();
    assert(config.isValid());
    DEBUG_PRINTLN("*** Config: path - " << config.imagePath << ", ox - " << config.originX << ", oy - " << config.originY);
    avo::Overlayer ovl(config);

    // initialize video cap using input video
    cv::VideoCapture cap(videoPath);
    int totalFrames = (int) cap.get(cv::CAP_PROP_FRAME_COUNT);
    int inputWidth = (int) cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int inputHeight = (int) cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    DEBUG_PRINTLN("*** Total frames: " << totalFrames << ", fps: " << fps);
    DEBUG_PRINTLN("*** Input frame dimensions: [" << inputWidth << ", " << inputHeight << "]");

    // only one of width,height nonzero values is used to retain proper aspect ratio
    if (width > 0) {
        double f = (double) width / config.templateWidth;
        height = (int) round(f * config.templateHeight);
    } else if (height > 0) {
        double f = (double) height / config.templateHeight;
        width = (int) round(f * config.templateWidth);
    } else {
        width = config.templateWidth;
        height = config.templateHeight;
    }
    DEBUG_PRINTLN("*** Output frame dimensions: [" << width << ", " << height << "]");

    // start overlay task
    avo::OutputConfig output(outputPath, fps, width, height, backgroundColor);
    std::cout << "*** Output configuration: " << width << "x" << height << ", " << fps << "fps" << ", " << backgroundColor.hexString() << std::endl;
    avo::Task task = ovl.overlayTask(output);

    task.initialize();

    cv::Mat frame;
    tqdm pbar;
    int index = 0;
    while (cap.isOpened()) {
        if (!cap.read(frame)) {
            break;
        }

        task.feedFrame(frame);
        pbar.progress(index, totalFrames);
        index += 1;
    }
    pbar.finish();
    cap.release();
    task.finalize();

    return 0;
}
