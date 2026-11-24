#pragma once

#include "imgui.h"

namespace ui
{

// Compact dark-theme palette and metrics for this small image tool:
// black surfaces, hairline borders, dim text and a white accent.
namespace palette
{
inline const ImVec4 bg(0.0f, 0.0f, 0.0f, 1.0f);
inline const ImVec4 surface(0.0f, 0.0f, 0.0f, 1.0f);
inline const ImVec4 surface_hover(0.035f, 0.035f, 0.04f, 1.0f);
inline const ImVec4 frame(0.012f, 0.012f, 0.015f, 1.0f);
inline const ImVec4 frame_hover(0.03f, 0.03f, 0.035f, 1.0f);
inline const ImVec4 border(0.12f, 0.12f, 0.135f, 1.0f);
inline const ImVec4 border_soft(0.075f, 0.075f, 0.085f, 1.0f);
inline const ImVec4 text(0.95f, 0.95f, 0.965f, 1.0f);
inline const ImVec4 text_dim(0.62f, 0.62f, 0.66f, 1.0f);
inline const ImVec4 text_muted(0.40f, 0.40f, 0.44f, 1.0f);
inline const ImVec4 accent(1.0f, 1.0f, 1.0f, 1.0f);
inline const ImVec4 on_accent(0.0f, 0.0f, 0.0f, 1.0f);
inline const ImVec4 danger(0.95f, 0.42f, 0.42f, 1.0f);
} // namespace palette

struct Fonts
{
    ImFont *body = nullptr;
    ImFont *medium = nullptr;
    ImFont *small = nullptr;
};

void LoadFonts(ImGuiIO &io, Fonts &fonts);
Fonts &FontsGlobal();
void ApplyStyle();

// Small custom widgets --------------------------------------------------------
// Rounded filled button. `primary` draws the white/black accent version.
bool Button(const char *label, bool primary = false, ImVec2 size = ImVec2(0, 0),
            bool enabled = true);

// Outlined segmented control; writes the selected index back through `value`.
void Segmented(const char *id, const char *const *items, int count, int &value,
               float width = -1.0f);

// Uppercase, dim section label used above each group of controls.
void SectionLabel(const char *text);

// Thin hairline separator between control groups.
void Hairline();

} // namespace ui
