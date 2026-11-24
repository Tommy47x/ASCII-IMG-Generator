#pragma once

#include "imgui.h"

#include <string>

// A small OpenGL texture wrapper for previewing input/output images.
struct Texture
{
    unsigned int id = 0;
    int width = 0;
    int height = 0;

    bool valid() const { return id != 0; }
    void destroy();
    bool load(const std::string &path);
};

// Draw a texture centred inside `area`, preserving aspect ratio.
void DrawTextureFit(const Texture &tex, ImVec2 area_min, ImVec2 area_max, float padding = 12.0f);

// Draw a centred dim hint (used for empty preview slots).
void DrawCenteredHint(const char *title, const char *subtitle, ImVec2 area_min, ImVec2 area_max);
