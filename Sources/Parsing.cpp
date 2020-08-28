//
// Created by Kacper Rączy on 27/08/2020.
//

#include "Parsing.hpp"
#include <regex>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include "Utility.hpp"

namespace fs = std::filesystem;

// ContentsEntry

void ContentsEntry::toOverlayConfig(avo::OverlayConfig &config, std::optional<std::string> color) {
    fs::path dir(TEMPLATE_IMAGES_PATH);
    std::string imagePath = dir / images.at(color.value_or(defaultImage));
    config = {imagePath, screenLeft, screenTop, screenRight, screenBottom, resolutionWidth, resolutionHeight};
}

int ContentsEntry::screenWidth() const {
    return screenRight - screenLeft;
}

int ContentsEntry::screenHeight() const {
    return screenBottom - screenTop;
}

void from_json(const json& j, ContentsEntry& entry) {
    j.at("left").get_to(entry.screenLeft);
    j.at("top").get_to(entry.screenTop);
    j.at("right").get_to(entry.screenRight);
    j.at("bottom").get_to(entry.screenBottom);
    j.at("res_width").get_to(entry.resolutionWidth);
    j.at("res_height").get_to(entry.resolutionHeight);
    j.at("default_image").get_to(entry.defaultImage);
    j.at("images").get_to(entry.images);
}

// Template key

TemplateParseResult parseTemplateKey(const std::string& str) {
    std::regex regex("_+");
    std::vector<std::string> components(
        std::sregex_token_iterator(str.begin(), str.end(), regex, -1),
        std::sregex_token_iterator()
    );
    if (components.empty()) {
        throw std::runtime_error("Template string should be in format: \"device[_color]\"");
    }

    std::string device = components.at(0);
    std::optional<std::string> color;
    if (components.size() > 1) {
        color = components.at(1);
    }

    return {device, color};
}

// Contents.json file parsing

void parseContentsJson(nlohmann::json& json) {
    DEBUG_PRINTLN("*** Contents JSON path: " << CONTENTS_JSON_PATH);
    std::ifstream configFile(CONTENTS_JSON_PATH);
    configFile >> json;
    configFile.close();
}

// Automatic template

int autoTemplate(const std::vector<ContentsEntry>& entries, int inputWidth, int inputHeight) {
    // ratio = w / h
    double inputRatio = (double) inputWidth / (double) inputHeight;
    int n = entries.size();
    std::vector<double> diffs(n);
    std::transform(entries.begin(), entries.end(), diffs.begin(), [inputRatio](auto entry) {
        return abs(inputRatio - (double) entry.screenWidth() / (double) entry.screenHeight());
    });

    // find min value
    auto minIt = std::min_element(diffs.begin(), diffs.end());
    int minIndex = std::distance(diffs.begin(), minIt);
    return minIndex;
}