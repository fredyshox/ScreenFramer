//
//  main.cpp
//  ScreenFramer
//
//  Created by Kacper Rączy on 24/07/2020.
//

#include <iostream>
#include <string>
#include <regex>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <opencv2/opencv.hpp>
#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
#include "Overlayer.hpp"
#include "Parsing.hpp"
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

void printTemplateHelp(nlohmann::json& configJson) {
    std::cerr << "Available keys:" << std::endl;
    for (auto it = configJson.begin(); it != configJson.end(); ++it) {
        std::cerr << " - " << it.key() << std::endl;
    }
}

/**
 * Usage: avframer VIDEOPATH OUTPUTPATH
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
    cxxopts::Options options("screenframer", "Overlay videos from Apple devices");
    options.add_options()
        ("t,template", "Device model template", cxxopts::value<std::string>()->default_value("auto"))
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
        backgroundColor = {rgbHexStr};
    } catch (const std::exception& e) {
        std::cerr << "Error parsing options: " << e.what() << std::endl << std::endl;
        std::cerr << options.help() << std::endl;
        return 1;
    }

    // check if input file exists
    if (!fs::exists(videoPath)) {
        std::cerr << "Input video file does not exist at: " << videoPath << std::endl;
        return 2;
    }

    // initialize video cap using input video
    cv::VideoCapture cap(videoPath);
    int totalFrames = (int) cap.get(cv::CAP_PROP_FRAME_COUNT);
    int inputWidth = (int) cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int inputHeight = (int) cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    DEBUG_PRINTLN("*** Total frames: " << totalFrames << ", fps: " << fps);
    DEBUG_PRINTLN("*** Input frame dimensions: [" << inputWidth << ", " << inputHeight << "]");

    // load template json from resources
    nlohmann::json configJson;
    parseContentsJson(configJson);

    avo::OverlayConfig config;
    if (templateKey == "auto") {
        // automatic template selection
        std::vector<ContentsEntry> entries;
        for (auto it = configJson.begin(); it != configJson.end(); ++it) {
            entries.push_back(it.value().get<ContentsEntry>());
        }
        int index = autoTemplate(entries, inputWidth, inputHeight);
        entries[index].toOverlayConfig(config);
        // print detected template
        auto pathNoExt = fs::path(config.imagePath).replace_extension("");
        std::cout << "*** Detected template: " << pathNoExt.filename() << std::endl;
    } else {
        try {
            auto result = parseTemplateKey(templateKey);
            auto deviceKey = std::get<0>(result);
            auto colorKey = std::get<1>(result);
            if (!configJson.contains(deviceKey)) {
                throw std::runtime_error("Invalid device key \"" + deviceKey + "\"");
            }

            auto entry = configJson[deviceKey].get<ContentsEntry>();
            entry.toOverlayConfig(config, colorKey);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            printTemplateHelp(configJson);
            // release resources
            cap.release();
            return 3;
        }
    }

    assert(config.isValid());
    DEBUG_PRINTLN("*** Config: path - " << config.imagePath << ", ox - " << config.screenLeft << ", oy - " << config.screenTop);
    avo::Overlayer ovl(config);

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
    avo::Task<cv::Mat> task = ovl.overlayTask<cv::Mat>(output);

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
