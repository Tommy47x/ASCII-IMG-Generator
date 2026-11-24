#include "file_dialog.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "theme.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

static bool IsImageFile(const fs::path &p)
{
    std::string ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".bmp" ||
           ext == ".webp" || ext == ".tga";
}

void FileDialog::Open(const std::string &start_path)
{
    std::error_code ec;
    fs::path p(start_path);
    if (!p.empty() && fs::exists(p, ec))
    {
        if (fs::is_directory(p, ec))
            directory_ = fs::absolute(p, ec).string();
        else
        {
            directory_ = fs::absolute(p.parent_path(), ec).string();
            path_field_ = fs::absolute(p, ec).string();
        }
    }
    if (directory_.empty())
        directory_ = fs::current_path(ec).string();
    if (!fs::exists(fs::path(path_field_), ec))
        path_field_.clear();
    error_.clear();
    selected_ = -1;
    Refresh();
    requested_ = true;
}

void FileDialog::Refresh()
{
    entries_.clear();
    std::error_code ec;
    for (fs::directory_iterator it(directory_, ec), end; !ec && it != end; it.increment(ec))
    {
        const fs::path &p = it->path();
        const bool dir = it->is_directory(ec);
        if (!dir && !IsImageFile(p))
            continue;
        entries_.push_back({p.filename().string(), p.string(), dir});
    }
    std::sort(entries_.begin(), entries_.end(), [](const Entry &a, const Entry &b) {
        if (a.directory != b.directory)
            return a.directory;
        std::string la = a.name, lb = b.name;
        std::transform(la.begin(), la.end(), la.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        std::transform(lb.begin(), lb.end(), lb.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        return la < lb;
    });
}

void FileDialog::Draw(std::string &chosen)
{
    if (requested_)
    {
        ImGui::OpenPopup("Open image");
        requested_ = false;
        open_ = true;
    }
    if (!open_)
        return;

    const ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize(ImVec2(640, 460), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(display.x * 0.5f, display.y * 0.5f), ImGuiCond_Always,
                            ImVec2(0.5f, 0.5f));
    if (!ImGui::BeginPopupModal("Open image", &open_, ImGuiWindowFlags_NoResize))
    {
        return;
    }

    ImGui::PushFont(ui::FontsGlobal().medium);
    ImGui::TextUnformatted("Open image");
    ImGui::PopFont();
    ImGui::Separator();

    ImGui::TextDisabled("Directory");
    ImGui::SetNextItemWidth(-84.0f);
    if (ImGui::InputText("##dir", &directory_, ImGuiInputTextFlags_EnterReturnsTrue))
        Refresh();
    ImGui::SameLine();
    if (ui::Button("Up##up", false, ImVec2(76, 30)))
    {
        std::error_code ec;
        fs::path parent = fs::path(directory_).parent_path();
        if (!parent.empty() && fs::exists(parent, ec))
        {
            directory_ = parent.string();
            Refresh();
        }
    }
    ImGui::Dummy(ImVec2(0, 4));

    ImGui::BeginChild("files", ImVec2(-1, -148), ImGuiChildFlags_Borders);
    for (size_t i = 0; i < entries_.size(); ++i)
    {
        const Entry &e = entries_[i];
        ImGui::PushID((int)i);
        const std::string label = std::string(e.directory ? "[+]  " : "      ") + e.name;
        if (ImGui::Selectable(label.c_str(), selected_ == (int)i))
        {
            selected_ = (int)i;
            path_field_ = e.path;
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            if (e.directory)
            {
                directory_ = e.path;
                path_field_.clear();
                Refresh();
            }
            else
            {
                chosen = e.path;
                open_ = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::PopID();
    }
    ImGui::EndChild();

    ImGui::Dummy(ImVec2(0, 6));
    ImGui::TextDisabled("File");
    ImGui::SetNextItemWidth(-1);
    const bool enter_pressed = ImGui::InputText("##path", &path_field_,
                                                ImGuiInputTextFlags_EnterReturnsTrue);
    if (!error_.empty())
        ImGui::TextColored(ui::palette::danger, "%s", error_.c_str());

    auto confirm = [&]() {
        std::error_code ec;
        fs::path p(path_field_);
        if (fs::exists(p, ec) && fs::is_regular_file(p, ec) && IsImageFile(p))
        {
            chosen = fs::absolute(p, ec).string();
            open_ = false;
            ImGui::CloseCurrentPopup();
        }
        else
        {
            error_ = "Choose an existing image file (jpg, png, bmp, webp, tga).";
        }
    };

    ImGui::Dummy(ImVec2(0, 8));
    const float half = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;
    if (ui::Button("Cancel##cancel", false, ImVec2(half, 36)))
    {
        open_ = false;
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine(0, 8);
    if (ui::Button("Open##open", true, ImVec2(half, 36)) || enter_pressed)
        confirm();
    ImGui::EndPopup();
}
