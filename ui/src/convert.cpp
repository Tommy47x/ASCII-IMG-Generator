#include "convert.h"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <sstream>

#ifdef _WIN32
#include <cstdlib>
#else
#include <sys/wait.h>
#endif

namespace fs = std::filesystem;

static std::string ShellQuote(const std::string &s)
{
    std::string out = "'";
    for (char c : s)
    {
        if (c == '\'')
            out += "'\\''";
        else
            out += c;
    }
    out += "'";
    return out;
}

struct CommandResult
{
    int code = -1;
    std::string log;
};

static CommandResult ExecCommand(const std::string &command)
{
    CommandResult result;
    std::string full = command + " 2>&1";
    FILE *pipe = popen(full.c_str(), "r");
    if (!pipe)
        return result;

    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe))
        result.log += buffer;
#ifdef _WIN32
    result.code = _pclose(pipe);
#else
    const int status = pclose(pipe);
    result.code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
    return result;
}

ConvertResult RunConversion(const ConvertConfig &cfg)
{
    ConvertResult result;
    result.output = cfg.output;

    if (!fs::exists(cfg.input))
    {
        result.error = "Input file not found: " + cfg.input;
        return result;
    }

    const std::string script = cfg.color ? "img2img_color.py" : "img2img.py";
    std::ostringstream cmd;
    cmd << ASCII_PYTHON << " " << script
        << " --input " << ShellQuote(cfg.input)
        << " --output " << ShellQuote(cfg.output)
        << " --language " << ShellQuote(cfg.language)
        << " --mode " << ShellQuote(cfg.mode)
        << " --background " << ShellQuote(cfg.background)
        << " --num_cols " << cfg.columns;

    CommandResult run = ExecCommand(cmd.str());
    if (run.code != 0)
    {
        result.error = "Conversion failed (exit " + std::to_string(run.code) + ").\n";
        if (!run.log.empty())
            result.error += run.log;
        return result;
    }

    std::error_code ec;
    if (!fs::exists(cfg.output, ec) || fs::file_size(cfg.output, ec) == 0)
    {
        result.error = "Python finished but no output was written.";
        if (!run.log.empty())
            result.error += "\n" + run.log;
        return result;
    }

    if (cfg.color && cfg.whiteness > 0)
    {
        std::ostringstream whiten;
        whiten << ASCII_PYTHON << " "
               << ShellQuote(std::string(ASCII_PROJECT_ROOT) + "/ui/whiten.py")
               << " --input " << ShellQuote(cfg.output)
               << " --output " << ShellQuote(cfg.output)
               << " --whiteness " << cfg.whiteness;
        CommandResult adjusted = ExecCommand(whiten.str());
        if (adjusted.code != 0)
        {
            result.error = "Whiteness adjustment failed.\n";
            if (!adjusted.log.empty())
                result.error += adjusted.log;
            return result;
        }
    }

    result.ok = true;
    return result;
}

void ConvertJob::start(const ConvertConfig &cfg)
{
    if (future_.valid())
        return;
    future_ = std::async(std::launch::async, [cfg] { return RunConversion(cfg); });
}

bool ConvertJob::running()
{
    return future_.valid() && future_.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
}

bool ConvertJob::ready()
{
    return future_.valid() && future_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

ConvertResult ConvertJob::take()
{
    if (!future_.valid())
        return {};
    ConvertResult result = future_.get();
    return result;
}
