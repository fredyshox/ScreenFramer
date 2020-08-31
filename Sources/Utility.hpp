//
// Created by Kacper Rączy on 27/08/2020.
//

#ifndef SCREENFRAMER_UTILITY_HPP
#define SCREENFRAMER_UTILITY_HPP

#include <iostream>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>
#include "Overlayer.hpp"

using nlohmann::json;

#ifndef RESOURCES_PATH
#error "RESOURCES_PATH must be defined before compilation"
#endif
#define TEMPLATE_IMAGES_PATH RESOURCES_PATH
#define CONTENTS_JSON_PATH (fs::path(RESOURCES_PATH) / fs::path("contents.json")).string()

// Data structure for single entry of contents.json file
struct ContentsEntry {
    int screenLeft;
    int screenTop;
    int screenRight;
    int screenBottom;
    int resolutionWidth;
    int resolutionHeight;
    std::string defaultImage;
    std::unordered_map<std::string, std::string> images;

    void toOverlayConfig(avo::OverlayConfig& config, std::optional<std::string> color = std::nullopt);
    int screenWidth() const;
    int screenHeight() const;
};

// ContentsEntry deserialization
void from_json(const json& j, ContentsEntry& entry);

// Template key parsing
using TemplateParseResult = std::tuple<std::string, std::optional<std::string>>;
TemplateParseResult parseTemplateKey(const std::string& str);

// Padding parsing
bool parsePadding(const std::string& str, std::tuple<double, double>& padding, const std::tuple<int, int>& dims);

// Contents.json file parsing
void parseContentsJson(json& json);

// Automatic template

/**
 * Finds appropriate config by comparing aspect ratios with input video
 * @param cfgs vector of overlay configs
 * @param inputWidth width of input video
 * @param inputHeight height of input video
 * @return
 */
int autoTemplate(const std::vector<ContentsEntry>& entries, int inputWidth, int inputHeight);

#endif //SCREENFRAMER_UTILITY_HPP
