#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_opengl3_loader.h"
#include "imgui_impl_sdl2.h"
#include "imgui_stdlib.h"

#include <SDL.h>

#include "convert.h"
#include "file_dialog.h"
#include "image.h"
#include "theme.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Language
{
    const char *key;
    std::vector<const char *> modes;
};

static const Language kLanguages[] = {
    {"english", {"standard"}},
    {"general", {"simple", "complex"}},
    {"german", {"standard"}},
    {"french", {"standard"}},
    {"spanish", {"standard"}},
    {"italian", {"standard"}},
    {"portuguese", {"standard"}},
    {"polish", {"standard"}},
    {"russian", {"standard"}},
    {"chinese", {"standard"}},
    {"korean", {"standard"}},
    {"japanese", {"hiragana", "katakana"}},
};
static const int kLanguageCount = (int)(sizeof(kLanguages) / sizeof(kLanguages[0]));

struct AppState
{
    std::string input_path;
    std::string output_path;      // pending output path from the settings field
    std::string generated_path;   // the file the last successful run actually wrote
    std::string status = "Ready";
    std::string error;
    Texture input_tex;
    Texture output_tex;
    int preview = 1; // 0 = input, 1 = output
    int color = 0;   // 0 = grayscale, 1 = color
    int background = 0; // 0 = black, 1 = white
    int language = 0;
    int mode = 0;
    int columns = 200;
    int whiteness = 0;
    bool generating = false;
    double started = 0.0;
    ConvertJob job;
    FileDialog dialog;
};

static const char *kPreviewItems[] = {"Input", "Output"};
static const char *kColorItems[] = {"Grayscale", "Color"};
static const char *kBackgroundItems[] = {"Black", "White"};

static std::string DefaultOutput(const std::string &input)
{
    if (input.empty())
        return "data/output.jpg";
    fs::path p(input);
    return (p.parent_path() / (p.stem().string() + "_ascii.jpg")).string();
}

static std::string Ellipsize(const std::string &text, float max_width)
{
    if (ImGui::CalcTextSize(text.c_str()).x <= max_width)
        return text;
    std::string s = text;
    while (!s.empty() && ImGui::CalcTextSize((s + "...").c_str()).x > max_width)
        s.pop_back();
    return s + "...";
}

static void LoadInput(AppState &state)
{
    if (state.input_path.empty())
        return;
    if (!state.input_tex.load(state.input_path))
    {
        state.error = "Could not load image: " + state.input_path;
        state.status = "Input error";
        return;
    }
    state.error.clear();
    state.status = "Image loaded";
    state.output_path = DefaultOutput(state.input_path);
    // A new source invalidates the previous result: don't let the output
    // buttons keep pointing at a stale file from another image.
    state.generated_path.clear();
}

static void StartGenerate(AppState &state)
{
    if (state.generating)
        return;
    if (state.input_path.empty())
    {
        state.error = "Choose an input image first.";
        return;
    }
    if (!fs::exists(state.input_path))
    {
        state.error = "Input file not found.";
        return;
    }
    if (state.output_path.empty())
        state.output_path = DefaultOutput(state.input_path);

    ConvertConfig cfg;
    cfg.input = state.input_path;
    cfg.output = state.output_path;
    cfg.language = kLanguages[state.language].key;
    cfg.mode = kLanguages[state.language].modes[(size_t)state.mode];
    cfg.background = state.background == 0 ? "black" : "white";
    cfg.columns = state.columns;
    cfg.color = state.color != 0;
    cfg.whiteness = state.whiteness;

    state.error.clear();
    state.generated_path.clear();
    state.generating = true;
    state.started = ImGui::GetTime();
    state.status = "Converting image...";
    state.job.start(cfg);
}

static void PollJob(AppState &state)
{
    if (!state.generating || !state.job.ready())
        return;
    ConvertResult result = state.job.take();
    state.generating = false;
    if (result.ok)
    {
        const double elapsed = ImGui::GetTime() - state.started;
        std::error_code ec;
        state.generated_path = fs::absolute(result.output, ec).string();
        if (state.output_tex.load(result.output))
        {
            state.preview = 1;
            state.status = "Done in " + std::to_string((int)(elapsed * 10) / 10.0) + "s  ·  " +
                           result.output;
        }
        else
        {
            state.status = "Saved, but preview could not be loaded";
        }
    }
    else
    {
        state.error = result.error;
        state.status = "Conversion failed";
    }
}

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

static void OpenPath(const std::string &path)
{
    if (path.empty())
        return;
    std::error_code ec;
    const std::string target = fs::absolute(path, ec).string();
#ifdef _WIN32
    ShellExecuteA(nullptr, "open", target.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#elif defined(__APPLE__)
    (void)system(("open " + ShellQuote(target) + " >/dev/null 2>&1 &").c_str());
#else
    (void)system(("xdg-open " + ShellQuote(target) + " >/dev/null 2>&1 &").c_str());
#endif
}

static void DrawHeader(const ImVec2 &display)
{
    ImGui::SetCursorPos(ImVec2(20, 14));
    ImGui::PushFont(ui::FontsGlobal().medium);
    ImGui::TextUnformatted("ASCII STUDIO");
    ImGui::PopFont();
    ImGui::SetCursorPos(ImVec2(20, 38));
    ImGui::TextDisabled("Image to ASCII image  ·  ImGui + Python");

    const char *version = "v1.0";
    const ImVec2 ts = ImGui::CalcTextSize(version);
    ImGui::SetCursorPos(ImVec2(display.x - 20 - ts.x, 26));
    ImGui::TextDisabled("%s", version);
}

static void DrawSourcePanel(AppState &state, float width, float height)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    if (!ImGui::BeginChild("source", ImVec2(width, height), ImGuiChildFlags_Borders))
    {
        ImGui::EndChild();
        ImGui::PopStyleVar();
        return;
    }

    ui::SectionLabel("SOURCE");
    ImGui::Dummy(ImVec2(0, 4));

    const float thumb_w = ImGui::GetContentRegionAvail().x;
    const float thumb_h = 150.0f;
    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##source_thumb", ImVec2(thumb_w, thumb_h));
    const bool hovered = ImGui::IsItemHovered();
    if (ImGui::IsItemClicked())
        state.dialog.Open(state.input_path);
    ImDrawList *dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(p0, ImVec2(p0.x + thumb_w, p0.y + thumb_h),
                      ImGui::GetColorU32(hovered ? ui::palette::surface_hover
                                                 : ImVec4(0.006f, 0.006f, 0.008f, 1.0f)), 10.0f);
    dl->AddRect(p0, ImVec2(p0.x + thumb_w, p0.y + thumb_h),
                ImGui::GetColorU32(ui::palette::border), 10.0f, 0, 1.0f);
    if (state.input_tex.valid())
        DrawTextureFit(state.input_tex, p0, ImVec2(p0.x + thumb_w, p0.y + thumb_h), 8.0f);
    else
        DrawCenteredHint("Drop an image here", "or use Browse", p0,
                         ImVec2(p0.x + thumb_w, p0.y + thumb_h));

    ImGui::Dummy(ImVec2(0, 8));
    const std::string name = fs::path(state.input_path).filename().string();
    ImGui::TextUnformatted(Ellipsize(name.empty() ? "No image selected" : name, thumb_w).c_str());
    if (state.input_tex.valid())
        ImGui::TextDisabled("%d x %d", state.input_tex.width, state.input_tex.height);
    ImGui::Dummy(ImVec2(0, 8));

    if (ui::Button("Browse image##browse", false, ImVec2(-1, 36)))
        state.dialog.Open(state.input_path);

    ImGui::Dummy(ImVec2(0, 16));
    ui::Hairline();
    ImGui::Dummy(ImVec2(0, 12));
    ui::SectionLabel("PRESETS");
    ImGui::Dummy(ImVec2(0, 4));
    const char *preset_labels[] = {"80", "120", "200"};
    const int preset_cols[] = {80, 120, 200};
    const float pw = (ImGui::GetContentRegionAvail().x - 8.0f) / 3.0f;
    for (int i = 0; i < 3; ++i)
    {
        if (i)
            ImGui::SameLine(0, 4);
        std::string label = std::string(preset_labels[i]) + "##preset" + std::to_string(i);
        if (ui::Button(label.c_str(), state.columns == preset_cols[i], ImVec2(pw, 32)))
            state.columns = preset_cols[i];
    }

    ImGui::Dummy(ImVec2(0, 16));
    ui::Hairline();
    ImGui::Dummy(ImVec2(0, 12));
    ui::SectionLabel("PREVIEW");
    ImGui::Dummy(ImVec2(0, 4));
    ui::Segmented("##preview_source", kPreviewItems, 2, state.preview);

    // Declare the full child extent so ImGui doesn't complain about
    // SetCursorPos() being used for the custom-drawn regions above.
    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::Dummy(ImVec2(width - 4, height - 4));
    ImGui::EndChild();
    ImGui::PopStyleVar();
}

static void DrawPreviewPanel(const AppState &state, float width, float height)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    if (!ImGui::BeginChild("preview", ImVec2(width, height), ImGuiChildFlags_Borders))
    {
        ImGui::EndChild();
        ImGui::PopStyleVar();
        return;
    }

    ImGui::SetCursorPos(ImVec2(20, 14));
    ImGui::PushFont(ui::FontsGlobal().medium);
    ImGui::TextUnformatted("PREVIEW");
    ImGui::PopFont();

    const Texture &tex = state.preview == 0 ? state.input_tex : state.output_tex;
    if (tex.valid())
    {
        const std::string info = std::to_string(tex.width) + " x " + std::to_string(tex.height) +
                                 (state.preview == 0 ? "  ·  source" : "  ·  generated");
        const ImVec2 ts = ImGui::CalcTextSize(info.c_str());
        ImGui::SetCursorPos(ImVec2(width - 20 - ts.x, 18));
        ImGui::TextDisabled("%s", info.c_str());
    }

    ImGui::SetCursorPos(ImVec2(20, 50));
    const ImVec2 area_min = ImGui::GetCursorScreenPos();
    const ImVec2 child_pos = ImGui::GetWindowPos();
    const ImVec2 area_max = ImVec2(child_pos.x + width - 14, child_pos.y + height - 14);

    ImDrawList *dl = ImGui::GetWindowDrawList();
    dl->AddRectFilled(ImVec2(area_min.x - 6, area_min.y - 6),
                      ImVec2(area_max.x + 6, area_max.y + 6),
                      ImGui::GetColorU32(ImVec4(0.006f, 0.006f, 0.008f, 1.0f)), 10.0f);

    if (tex.valid())
        DrawTextureFit(tex, area_min, area_max, 10.0f);
    else if (state.preview == 1)
        DrawCenteredHint("No output yet", "Pick settings, then press Generate", area_min, area_max);
    else
        DrawCenteredHint("No input image", "Drop a file or press Browse", area_min, area_max);

    if (state.generating)
    {
        const float phase = (float)ImGui::GetTime();
        const float pulse = 0.55f + 0.45f * (float)sin(phase * 4.0);
        dl->AddCircleFilled(ImVec2(area_min.x + 14, area_min.y + 14), 4.0f,
                            ImGui::GetColorU32(ImVec4(1, 1, 1, pulse)));
    }

    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::Dummy(ImVec2(width - 4, height - 4));
    ImGui::EndChild();
    ImGui::PopStyleVar();
}

static void DrawSettingsPanel(AppState &state, float width, float height)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    if (!ImGui::BeginChild("settings", ImVec2(width, height), ImGuiChildFlags_Borders))
    {
        ImGui::EndChild();
        ImGui::PopStyleVar();
        return;
    }

    ui::SectionLabel("OUTPUT");
    ui::Segmented("##color", kColorItems, 2, state.color);
    ImGui::Dummy(ImVec2(0, 6));
    ui::Segmented("##background", kBackgroundItems, 2, state.background);

    ImGui::Dummy(ImVec2(0, 16));
    ui::Hairline();
    ImGui::Dummy(ImVec2(0, 12));
    ui::SectionLabel("CHARACTERS");

    const char *lang_names[kLanguageCount];
    for (int i = 0; i < kLanguageCount; ++i)
        lang_names[i] = kLanguages[i].key;
    ImGui::SetNextItemWidth(-1);
    if (ImGui::Combo("##language", &state.language, lang_names, kLanguageCount))
    {
        if (state.mode >= (int)kLanguages[state.language].modes.size())
            state.mode = 0;
    }

    ImGui::Dummy(ImVec2(0, 6));
    const auto &modes = kLanguages[state.language].modes;
    std::vector<const char *> mode_names(modes.begin(), modes.end());
    ImGui::SetNextItemWidth(-1);
    ImGui::Combo("##mode", &state.mode, mode_names.data(), (int)mode_names.size());

    ImGui::Dummy(ImVec2(0, 16));
    ui::Hairline();
    ImGui::Dummy(ImVec2(0, 12));
    ui::SectionLabel("DETAIL");

    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##columns", &state.columns, 20, 400, "%d cols", ImGuiSliderFlags_AlwaysClamp);
    ImGui::TextDisabled("Character width of the generated image");

    ImGui::Dummy(ImVec2(0, 10));
    const bool color_mode = state.color == 1;
    if (!color_mode)
        ImGui::BeginDisabled();
    ImGui::SetNextItemWidth(-1);
    ImGui::SliderInt("##whiteness", &state.whiteness, 0, 100, "Whiteness %d%%",
                     ImGuiSliderFlags_AlwaysClamp);
    const bool whiteness_changed = ImGui::IsItemDeactivatedAfterEdit();
    if (!color_mode)
        ImGui::EndDisabled();
    ImGui::TextDisabled(color_mode ? "Lifts dark colours; 0 keeps the script output"
                                   : "Available when output is Color");
    // Regenerate once when the drag ends, not on every frame of the drag.
    if (whiteness_changed && color_mode && !state.generating && !state.input_path.empty())
        StartGenerate(state);

    ImGui::Dummy(ImVec2(0, 16));
    ui::Hairline();
    ImGui::Dummy(ImVec2(0, 12));
    ui::SectionLabel("OUTPUT FILE");
    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##output", &state.output_path);
    ImGui::Dummy(ImVec2(0, 4));
    if (ui::Button("Use default name##default_output", false, ImVec2(-1, 30)))
        state.output_path = DefaultOutput(state.input_path);

    ImGui::Dummy(ImVec2(0, 16));
    if (ui::Button(state.generating ? "Generating...##generate" : "Generate##generate", true,
                   ImVec2(-1, 42), !state.generating))
        StartGenerate(state);
    if (state.generating)
        ImGui::TextDisabled("Running img2img%s.py ...", state.color ? "_color" : "");

    if (!state.error.empty())
    {
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x);
        ImGui::TextColored(ui::palette::danger, "%s", state.error.c_str());
        ImGui::PopTextWrapPos();
    }

    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::Dummy(ImVec2(width - 4, height - 4));
    ImGui::EndChild();
    ImGui::PopStyleVar();
}

static void DrawFooter(const AppState &state, const ImVec2 &display)
{
    const float y = display.y - 46.0f;
    ImDrawList *dl = ImGui::GetWindowDrawList();
    dl->AddLine(ImVec2(20, y - 8), ImVec2(display.x - 20, y - 8),
                ImGui::GetColorU32(ui::palette::border_soft), 1.0f);

    const bool failed = !state.error.empty();
    const ImVec4 dot = failed ? ui::palette::danger
                              : (state.generating ? ui::palette::accent : ui::palette::text_dim);
    dl->AddCircleFilled(ImVec2(26, y + 11), 4.0f, ImGui::GetColorU32(dot));

    const float file_w = 100.0f;
    const float folder_w = 116.0f;
    const float buttons_w = file_w + folder_w + 8.0f;
    ImGui::SetCursorPos(ImVec2(40, y + 2));
    ImGui::TextUnformatted(Ellipsize(state.status, display.x - buttons_w - 90.0f).c_str());

    // These act on the actual last generated file, not the pending filename in
    // the settings field, so editing the output path can't break them.
    const bool has_output = !state.generated_path.empty() && fs::exists(state.generated_path);
    ImGui::SetCursorPos(ImVec2(display.x - 20 - buttons_w, y));
    if (ui::Button("Open file##openfile", false, ImVec2(file_w, 32), has_output))
        OpenPath(state.generated_path);
    ImGui::SameLine(0, 8);
    if (ui::Button("Open folder##folder", false, ImVec2(folder_w, 32), has_output))
        OpenPath(fs::path(state.generated_path).parent_path().string());
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    std::error_code ec;
    fs::current_path(ASCII_PROJECT_ROOT, ec);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

    SDL_Window *window = SDL_CreateWindow(
        "ASCII Studio", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 800,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window)
    {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetWindowMinimumSize(window, 1020, 640);
    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (!gl)
    {
        fprintf(stderr, "OpenGL 3.2 context unavailable: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    ui::LoadFonts(io, ui::FontsGlobal());
    ui::ApplyStyle();

    ImGui_ImplSDL2_InitForOpenGL(window, gl);
    if (!ImGui_ImplOpenGL3_Init("#version 130"))
    {
        fprintf(stderr, "ImGui OpenGL backend init failed.\n");
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_GL_DeleteContext(gl);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    AppState state;
    const fs::path initial = fs::path(ASCII_PROJECT_ROOT) / "demo/input.jpg";
    if (fs::exists(initial))
    {
        state.input_path = initial.string();
        LoadInput(state);
    }

    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT ||
                (event.type == SDL_WINDOWEVENT &&
                 event.window.event == SDL_WINDOWEVENT_CLOSE &&
                 event.window.windowID == SDL_GetWindowID(window)))
                done = true;
            if (event.type == SDL_DROPFILE)
            {
                state.input_path = event.drop.file;
                SDL_free(event.drop.file);
                LoadInput(state);
            }
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Enter))
            StartGenerate(state);

        PollJob(state);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        const ImVec2 display = io.DisplaySize;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(display);
        ImGui::Begin("main", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);

        DrawHeader(display);

        const float pad = 20.0f;
        const float gap = 16.0f;
        const float top = 66.0f;
        const float bottom = display.y - 68.0f;
        float left_w = 240.0f;
        float right_w = 320.0f;
        if (display.x < 1120.0f)
        {
            left_w = 210.0f;
            right_w = 290.0f;
        }
        const float center_w = display.x - pad * 2 - left_w - right_w - gap * 2;
        const float content_h = bottom - top;

        ImGui::SetCursorPos(ImVec2(pad, top));
        DrawSourcePanel(state, left_w, content_h);
        ImGui::SetCursorPos(ImVec2(pad + left_w + gap, top));
        DrawPreviewPanel(state, center_w, content_h);
        ImGui::SetCursorPos(ImVec2(display.x - pad - right_w, top));
        DrawSettingsPanel(state, right_w, content_h);

        DrawFooter(state, display);
        ImGui::End();

        state.dialog.Draw(state.input_path);
        // The dialog writes straight into input_path; refresh derived state.
        static std::string last_dialog_path;
        if (!state.input_path.empty() && state.input_path != last_dialog_path)
        {
            last_dialog_path = state.input_path;
            LoadInput(state);
        }

        ImGui::Render();
        glViewport(0, 0, (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x),
                   (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y));
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    state.input_tex.destroy();
    state.output_tex.destroy();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
