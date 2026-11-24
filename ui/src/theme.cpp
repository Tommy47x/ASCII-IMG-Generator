#include "theme.h"

#include <cstring>
#include <string>

namespace ui
{

static Fonts g_fonts;

Fonts &FontsGlobal()
{
    return g_fonts;
}

void LoadFonts(ImGuiIO &io, Fonts &fonts)
{
    io.Fonts->Clear();

    const std::string dir = std::string(ASCII_PROJECT_ROOT) + "/ui/assets/";
    auto add = [&](const char *file, float size) -> ImFont * {
        ImFontConfig cfg;
        cfg.OversampleH = 2;
        cfg.OversampleV = 2;
        cfg.PixelSnapH = false;
        return io.Fonts->AddFontFromFileTTF((dir + file).c_str(), size, &cfg);
    };

    fonts.body = add("Inter-Regular.ttf", 16.0f);
    fonts.medium = add("Inter-Medium.ttf", 17.0f);
    fonts.small = add("Inter-Regular.ttf", 13.0f);
    if (!fonts.body)
        fonts.body = io.Fonts->AddFontDefault();
    if (!fonts.medium)
        fonts.medium = fonts.body;
    if (!fonts.small)
        fonts.small = fonts.body;
    io.FontDefault = fonts.body;
}

void ApplyStyle()
{
    ImGui::StyleColorsDark();
    ImGuiStyle &s = ImGui::GetStyle();

    s.WindowPadding = ImVec2(20, 18);
    s.FramePadding = ImVec2(12, 8);
    s.ItemSpacing = ImVec2(8, 8);
    s.ItemInnerSpacing = ImVec2(8, 6);
    s.ScrollbarSize = 10;
    s.GrabMinSize = 9;
    s.WindowBorderSize = 0;
    s.ChildBorderSize = 1;
    s.PopupBorderSize = 1;
    s.FrameBorderSize = 1;
    s.WindowRounding = 12;
    s.ChildRounding = 12;
    s.FrameRounding = 8;
    s.PopupRounding = 10;
    s.ScrollbarRounding = 8;
    s.GrabRounding = 8;
    s.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    s.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    s.SelectableTextAlign = ImVec2(0.0f, 0.5f);

    s.Colors[ImGuiCol_Text] = palette::text;
    s.Colors[ImGuiCol_TextDisabled] = palette::text_muted;
    s.Colors[ImGuiCol_WindowBg] = palette::bg;
    s.Colors[ImGuiCol_ChildBg] = palette::surface;
    s.Colors[ImGuiCol_PopupBg] = ImVec4(0.02f, 0.02f, 0.025f, 1.0f);
    s.Colors[ImGuiCol_Border] = palette::border;
    s.Colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    s.Colors[ImGuiCol_FrameBg] = palette::frame;
    s.Colors[ImGuiCol_FrameBgHovered] = palette::frame_hover;
    s.Colors[ImGuiCol_FrameBgActive] = palette::frame;
    s.Colors[ImGuiCol_TitleBg] = palette::bg;
    s.Colors[ImGuiCol_TitleBgActive] = palette::bg;
    s.Colors[ImGuiCol_TitleBgCollapsed] = palette::bg;
    s.Colors[ImGuiCol_MenuBarBg] = palette::bg;
    s.Colors[ImGuiCol_ScrollbarBg] = palette::bg;
    s.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.16f, 0.16f, 0.18f, 1.0f);
    s.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.24f, 0.24f, 0.26f, 1.0f);
    s.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.32f, 0.32f, 0.34f, 1.0f);
    s.Colors[ImGuiCol_CheckMark] = palette::accent;
    s.Colors[ImGuiCol_SliderGrab] = palette::accent;
    s.Colors[ImGuiCol_SliderGrabActive] = palette::accent;
    s.Colors[ImGuiCol_Button] = palette::frame;
    s.Colors[ImGuiCol_ButtonHovered] = palette::frame_hover;
    s.Colors[ImGuiCol_ButtonActive] = palette::frame;
    s.Colors[ImGuiCol_Header] = ImVec4(0.055f, 0.055f, 0.06f, 1.0f);
    s.Colors[ImGuiCol_HeaderHovered] = palette::surface_hover;
    s.Colors[ImGuiCol_HeaderActive] = ImVec4(0.055f, 0.055f, 0.06f, 1.0f);
    s.Colors[ImGuiCol_Separator] = palette::border_soft;
    s.Colors[ImGuiCol_SeparatorHovered] = palette::border;
    s.Colors[ImGuiCol_SeparatorActive] = palette::border;
    s.Colors[ImGuiCol_ResizeGrip] = ImVec4(0, 0, 0, 0);
    s.Colors[ImGuiCol_PopupBg] = ImVec4(0.02f, 0.02f, 0.025f, 1.0f);
    s.Colors[ImGuiCol_Tab] = palette::frame;
    s.Colors[ImGuiCol_TabHovered] = palette::frame_hover;
    s.Colors[ImGuiCol_TabActive] = ImVec4(0.055f, 0.055f, 0.06f, 1.0f);
    s.Colors[ImGuiCol_TabUnfocused] = palette::frame;
    s.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.055f, 0.055f, 0.06f, 1.0f);
    s.Colors[ImGuiCol_DragDropTarget] = palette::accent;
    s.Colors[ImGuiCol_NavHighlight] = palette::border;
}

bool Button(const char *label, bool primary, ImVec2 size, bool enabled)
{
    if (size.x <= 0.0f)
        size.x = ImGui::GetContentRegionAvail().x;
    if (size.y <= 0.0f)
        size.y = 38.0f;

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 pmax = ImVec2(pos.x + size.x, pos.y + size.y);
    if (!enabled)
        ImGui::BeginDisabled();
    ImGui::InvisibleButton(label, size);
    if (!enabled)
        ImGui::EndDisabled();
    const bool hovered = enabled && ImGui::IsItemHovered();
    const bool held = hovered && ImGui::IsMouseDown(0);
    const bool clicked = enabled && ImGui::IsItemClicked();

    ImU32 fill;
    ImU32 outline = 0;
    ImU32 text_col;
    if (primary)
    {
        fill = hovered ? ImGui::GetColorU32(ImVec4(0.90f, 0.90f, 0.92f, 1.0f))
                       : ImGui::GetColorU32(palette::accent);
        if (held)
            fill = ImGui::GetColorU32(ImVec4(0.80f, 0.80f, 0.82f, 1.0f));
        text_col = ImGui::GetColorU32(palette::on_accent);
        if (!enabled)
        {
            fill = ImGui::GetColorU32(ImVec4(0.16f, 0.16f, 0.17f, 1.0f));
            text_col = ImGui::GetColorU32(palette::text_muted);
        }
    }
    else
    {
        fill = ImGui::GetColorU32(hovered ? palette::surface_hover : ImVec4(0, 0, 0, 0));
        outline = ImGui::GetColorU32(palette::border);
        text_col = ImGui::GetColorU32(hovered ? palette::text : palette::text_dim);
        if (!enabled)
        {
            outline = ImGui::GetColorU32(palette::border_soft);
            text_col = ImGui::GetColorU32(palette::text_muted);
        }
    }

    ImDrawList *dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(pos, pmax, fill, 8.0f);
    if (outline)
        dl->AddRect(pos, pmax, outline, 8.0f, 0, 1.0f);

    // Strip ImGui's "##id" suffix before drawing, but keep it in the widget id.
    std::string visible = label;
    if (const char *hash = strchr(label, '#'))
        visible.resize((size_t)(hash - label));
    const ImVec2 ts = ImGui::CalcTextSize(visible.c_str());
    dl->AddText(ImVec2(pos.x + (size.x - ts.x) * 0.5f, pos.y + (size.y - ts.y) * 0.5f),
                text_col, visible.c_str());
    if (hovered)
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    return clicked;
}

void Segmented(const char *id, const char *const *items, int count, int &value,
               float width)
{
    if (width <= 0.0f)
        width = ImGui::GetContentRegionAvail().x;
    const float h = 36.0f;
    const float gap = 4.0f;
    const float item_w = (width - gap * (count - 1)) / count;
    if (item_w <= 0.0f)
        return;

    const ImVec2 start = ImGui::GetCursorScreenPos();
    ImDrawList *dl = ImGui::GetWindowDrawList();
    for (int i = 0; i < count; ++i)
    {
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const ImVec2 pmax(pos.x + item_w, pos.y + h);
        ImGui::PushID(i);
        ImGui::InvisibleButton(id, ImVec2(item_w, h));
        const bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked())
            value = i;
        ImGui::PopID();

        const bool active = i == value;
        if (active)
        {
            dl->AddRectFilled(pos, pmax, ImGui::GetColorU32(palette::accent), 7.0f);
        }
        else
        {
            dl->AddRectFilled(pos, pmax,
                              ImGui::GetColorU32(hovered ? palette::surface_hover
                                                         : ImVec4(0, 0, 0, 0)), 7.0f);
            dl->AddRect(pos, pmax, ImGui::GetColorU32(palette::border), 7.0f, 0, 1.0f);
        }

        const char *label = items[i];
        const ImVec2 ts = ImGui::CalcTextSize(label);
        dl->AddText(ImVec2(pos.x + (item_w - ts.x) * 0.5f, pos.y + (h - ts.y) * 0.5f),
                    ImGui::GetColorU32(active ? palette::on_accent
                                              : (hovered ? palette::text : palette::text_dim)),
                    label);
        if (hovered)
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        ImGui::SetCursorScreenPos(ImVec2(pmax.x + gap, pos.y));
    }
    // Put the cursor back at the left edge on the next row. Without this the
    // next widget starts past the right edge and overlaps the last segment.
    ImGui::SetCursorScreenPos(ImVec2(start.x, start.y + h));
}

void SectionLabel(const char *text)
{
    ImGui::TextDisabled("%s", text);
}

void Hairline()
{
    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddLine(p0, ImVec2(p0.x + ImGui::GetContentRegionAvail().x, p0.y),
                                        ImGui::GetColorU32(palette::border_soft), 1.0f);
    ImGui::Dummy(ImVec2(0, 1));
}

} // namespace ui
