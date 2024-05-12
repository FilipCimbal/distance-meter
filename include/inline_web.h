#pragma once
#include <exception>
#include <string>
#include <unordered_map>

struct EmbededFile{
    size_t len;
    const uint8_t* data;
};

using EmbededFiles = std::unordered_map<std::string, EmbededFile>; 

const EmbededFile& getEmbededFile(std::string path);
const EmbededFiles& getEmbededFiles();
