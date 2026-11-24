#pragma once

#include <string>
#include <vector>

// Minimal modal file picker: directory navigation + typed path, filtered to
// raster images. No native dialog dependency, so it works on any SDL platform.
class FileDialog
{
public:
    void Open(const std::string &start_path);

    // Draws the modal. Sets `chosen` when the user confirms a file.
    void Draw(std::string &chosen);

private:
    struct Entry
    {
        std::string name;
        std::string path;
        bool directory = false;
    };

    bool requested_ = false;
    bool open_ = false;
    std::string directory_;
    std::string path_field_;
    std::string error_;
    std::vector<Entry> entries_;
    int selected_ = -1;

    void Refresh();
};
