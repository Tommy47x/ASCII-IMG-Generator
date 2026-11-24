#pragma once

#include <atomic>
#include <future>
#include <string>

struct ConvertConfig
{
    std::string input;
    std::string output;
    std::string language = "english";
    std::string mode = "standard";
    std::string background = "black"; // or "white"
    int columns = 200;
    bool color = false;
    int whiteness = 0; // 0..100, applied only to color output
};

struct ConvertResult
{
    bool ok = false;
    std::string output;
    std::string error;
};

// Synchronous helper: runs the matching Python script in the project root.
ConvertResult RunConversion(const ConvertConfig &cfg);

// Non-blocking wrapper used by the UI thread.
class ConvertJob
{
public:
    void start(const ConvertConfig &cfg);
    bool running();
    bool ready();
    ConvertResult take();

private:
    std::future<ConvertResult> future_;
};
