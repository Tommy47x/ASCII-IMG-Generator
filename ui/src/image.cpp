#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "image.h"
#include "imgui_impl_opengl3_loader.h"
#include "theme.h"

#include <algorithm>
#include <cstdint>

void Texture::destroy()
{
    if (id)
        glDeleteTextures(1, &id);
    id = 0;
    width = height = 0;
}

bool Texture::load(const std::string &path)
{
    destroy();
    int channels = 0;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data)
        return false;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void DrawTextureFit(const Texture &tex, ImVec2 area_min, ImVec2 area_max, float padding)
{
    if (!tex.valid())
        return;
    const ImVec2 avail(area_max.x - area_min.x - padding * 2.0f,
                       area_max.y - area_min.y - padding * 2.0f);
    if (avail.x <= 0.0f || avail.y <= 0.0f)
        return;
    const float scale = std::min(avail.x / (float)tex.width, avail.y / (float)tex.height);
    const ImVec2 size((float)tex.width * scale, (float)tex.height * scale);
    const ImVec2 pos(area_min.x + (area_max.x - area_min.x - size.x) * 0.5f,
                     area_min.y + (area_max.y - area_min.y - size.y) * 0.5f);
    ImGui::GetWindowDrawList()->AddImage((ImTextureID)(intptr_t)tex.id, pos,
                                         ImVec2(pos.x + size.x, pos.y + size.y));
}

void DrawCenteredHint(const char *title, const char *subtitle, ImVec2 area_min, ImVec2 area_max)
{
    ImDrawList *dl = ImGui::GetWindowDrawList();
    const ImVec2 centre((area_min.x + area_max.x) * 0.5f, (area_min.y + area_max.y) * 0.5f);
    const ImVec2 t = ImGui::CalcTextSize(title);
    dl->AddText(ImVec2(centre.x - t.x * 0.5f, centre.y - t.y - 6.0f),
                ImGui::GetColorU32(ui::palette::text_dim), title);
    if (subtitle)
    {
        const ImVec2 s = ImGui::CalcTextSize(subtitle);
        dl->AddText(ImVec2(centre.x - s.x * 0.5f, centre.y + 8.0f),
                    ImGui::GetColorU32(ui::palette::text_muted), subtitle);
    }
}
