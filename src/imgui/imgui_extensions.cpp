/*
 * As the name implies, this source file is an extension of ImGui. It is not in
 * direct association with ImGui.
 */

#include "imgui_extensions.h"

#include <vector>
#include <stdexcept>
#include <cmath>
#include <string>

#include <Windows.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl2.h"

#include "../config.h"

ImVec2 operator+(ImVec2 p1, ImVec2 p2)      { return ImVec2(p1.x + p2.x, p1.y + p2.y);                              }
ImVec2 operator-(ImVec2 p1, ImVec2 p2)      { return ImVec2(p1.x - p2.x, p1.y - p2.y);                              }
ImVec2 operator/(ImVec2 p1, ImVec2 p2)      { return ImVec2(p1.x / p2.x, p1.y / p2.y);                              }
ImVec2 operator*(ImVec2 p1, int value)      { return ImVec2(p1.x * value, p1.y * value);                            }
ImVec2 operator*(ImVec2 p1, float value)    { return ImVec2(p1.x * value, p1.y * value);                            }
ImVec4 operator+(float val, ImVec4 p2)      { return ImVec4(val + p2.x, val + p2.y, val + p2.z, val + p2.w);        }
ImVec4 operator*(float val, ImVec4 p2)      { return ImVec4(val * p2.x, val * p2.y, val * p2.z, val * p2.w);        }
ImVec4 operator*(ImVec4 p2, float val)      { return ImVec4(val * p2.x, val * p2.y, val * p2.z, val * p2.w);        }
ImVec4 operator-(ImVec4 p1, ImVec4 p2)      { return ImVec4(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z, p1.w - p2.w);    }
ImVec4 operator*(ImVec4 p1, ImVec4 p2)      { return ImVec4(p1.x * p2.x, p1.y * p2.y, p1.z * p2.z, p1.w * p2.w);    }
ImVec4 operator/(ImVec4 p1, ImVec4 p2)      { return ImVec4(p1.x / p2.x, p1.y / p2.y, p1.z / p2.z, p1.w / p2.w);    }

IMGUI_API void ImGui::Init(const HWND &hwnd) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    if (!ImGui_ImplWin32_Init(hwnd)) {
        throw std::runtime_error("Init: ImGui_ImplWin32_Init failed.");
    }

    ImGui_ImplOpenGL2_Init();
}

IMGUI_API void ImGui::Shutdown() {
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

IMGUI_API bool ImGui::BeginCanvas() {
    const auto dims = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize({static_cast<float>(dims.x), static_cast<float>(dims.y)});
    ImGui::SetNextWindowPos({0, 0});
    return ImGui::Begin("ImGuiCanvas0X12373", nullptr,
                        ImGuiWindowFlags_NoTitleBar |
                        ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoScrollbar |
                        ImGuiWindowFlags_NoInputs |
                        ImGuiWindowFlags_NoSavedSettings |
                        ImGuiWindowFlags_NoFocusOnAppearing |
                        ImGuiWindowFlags_NoBringToFrontOnFocus |
                        ImGuiWindowFlags_NoBackground);
}

IMGUI_API void ImGui::EndCanvas() {
    ImGui::End();
}

IMGUI_API bool ImGui::BeginTransparent(const char *title, float alpha) {
    ImGui::SetNextWindowBgAlpha(alpha);
    const bool res = ImGui::Begin(title, nullptr,
                                  ImGuiWindowFlags_AlwaysAutoResize |
                                  ImGuiWindowFlags_NoDecoration |
                                  ImGuiWindowFlags_NoNav |
                                  ImGuiWindowFlags_NoMove |
                                  ImGuiWindowFlags_NoInputs |
                                  ImGuiWindowFlags_NoSavedSettings |
                                  ImGuiWindowFlags_NoFocusOnAppearing);
    return res;
}

IMGUI_API void ImGui::EndTransparent() {
    ImGui::End();
}

void ImGui::Columns(int columns_count, const char* id, ImGuiColumnsFlags flags) {
    ImGuiWindow* window = GetCurrentWindow();
    IM_ASSERT(columns_count >= 1);

    ImGuiColumns* columns = window->DC.CurrentColumns;
    if (columns != NULL && columns->Count == columns_count && columns->Flags == flags)
        return;

    if (columns != NULL)
        EndColumns();

    if (columns_count != 1)
        BeginColumns(id, columns_count, flags);
}

bool ImGui::CustomCheckbox(const char *label, bool *v) {
    ImGuiWindow *window = GetCurrentWindow();
    if (window->SkipItems) {
        return false;
    }

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, nullptr, true);

    const float square_sz = GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb{pos,
                          pos + ImVec2(square_sz + !!(label_size.x > 0.0f) *
                                       (style.ItemInnerSpacing.x + label_size.x),
                                       label_size.y + style.FramePadding.y * 2.0f)};
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id)) {
        return false;
    }

    bool hovered = false;
    bool held = false;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed) {
        *v = !(*v);
        MarkItemEdited(id);
    }

    const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));
    RenderNavHighlight(total_bb, id);
    RenderFrame(check_bb.Min, check_bb.Max,
                GetColorU32((held && hovered) ?
                                ImGuiCol_FrameBgActive :
                                hovered ?
                                    ImGuiCol_FrameBgHovered :
                                    ImGuiCol_FrameBg), true, style.FrameRounding);

    if (*v) {
        const float pad = ImMax(1.0f, std::floorf(square_sz / 3.5f));
        ImVec2 beg = check_bb.Min + ImVec2(pad, pad);
        ImVec2 end = check_bb.Max - check_bb.Min - ImVec2(2 * pad, 2 * pad);
        window->DrawList->AddRectFilled(beg, beg + ImVec2(end.x, end.y), GetColorU32(ImGuiCol_CheckMark));
    }

    if (label_size.x > 0.0f) {
        RenderText(ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y), label);
    }

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));

    return pressed;
}

bool ImGui::CustomCheckboxFlags(const char *label, unsigned *flags, unsigned flag) {
    bool v = ((*flags & flag) == flag);
    const bool pressed = CustomCheckbox(label, &v);
    if (pressed) {
        if (v) {
            *flags |= flag;
        } else {
            *flags &= ~flag;
        }
    }

    return pressed;
}

/* Modified version of https://github.com/ocornut/imgui/issues/1496#issuecomment-655048353. */
std::vector<ImRect> panel_clip_rects;
void ImGui::BeginPanel(const char *label,
                       const char *additional_text,
                       bool background,
                       const float width,
                       const ImVec2 item_spacing) {
    ImGui::BeginGroup();
        const auto padding = ImGui::GetStyle().ItemInnerSpacing;

        /* Group inner items to get the joint height. */
        ImGui::BeginGroup();
            /* Temporarily disable any padding to prevent interference. */
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.0f, 0.0f});
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.0f, 0.0f});

            /* Indent the panel title. */
            ImGui::Dummy({padding.x, 0.0f});
            ImGui::SameLine();

            if (background) {
                /* Draw label background and the label itself. */
                const auto cursor_pos = ImGui::GetCurrentWindow()->DC.CursorPos;
                const auto text_size = CalcTextSize(label);
                ImGui::GetWindowDrawList()->AddRectFilled(cursor_pos, cursor_pos + text_size,
                                                          ImColor(ImGui::GetStyleColorVec4(
                                                              ImGuiCol_Border)));
                ImGui::TextUnformatted(label);

                /* Add top side padding. */
                const auto frame_height = ImGui::GetFrameHeight();
                ImGui::Dummy({0.0f, padding.y + frame_height * 0.75f});
            } else {
                /* Render text and add clipping rectangle. */
                ImGui::TextUnformatted(label);
                panel_clip_rects.push_back({ImGui::GetItemRectMin(), ImGui::GetItemRectMax()});

                /* Add top side padding. */
                ImGui::Dummy({0.0f, padding.y});
            }

            /* Add left side padding. */
            ImGui::Dummy({padding.x, 0.0f});
            ImGui::SameLine(0.0f, 0.0f);

            ImGui::PopStyleVar(2);

            /* Group the actual items. */
            ImGui::BeginGroup();
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                                    {0.0f > item_spacing.x ?
                                        ImGui::GetStyle().ItemSpacing.x : item_spacing.x,
                                    item_spacing.y});
                ImGui::PushItemWidth(width - padding.x - CalcTextSize(additional_text).x);
}

void ImGui::EndPanel() {
                ImGui::PopItemWidth();
                ImGui::PopStyleVar();
            ImGui::EndGroup();
        ImGui::EndGroup();

        const auto line_height = ImGui::GetTextLineHeight();
        const auto spacing = ImGui::GetStyle().ItemInnerSpacing;
        const auto min_pos = ImGui::GetItemRectMin();
        const auto max_pos = ImGui::GetItemRectMax();
        const auto color = ImColor(ImGui::GetStyleColorVec4(ImGuiCol_Border));

        /*
         * If the clip vector has any members, we want to clip out the
         * intersecting lines by _spacing.x / 2_ amount.
         */
        if (panel_clip_rects.size()) {
            auto &clip = panel_clip_rects.back();
            clip.Min.x -= spacing.x * 0.5f;
            clip.Max.x += spacing.x * 0.5f;

            for (int i = 0; 4 != i; ++i) {
                switch (i) {
                    /* Left. */
                    case 0: {
                        ImGui::PushClipRect({-FLT_MAX, -FLT_MAX}, {clip.Min.x, FLT_MAX}, true);
                        break;
                    }

                    /* Right. */
                    case 1: {
                        ImGui::PushClipRect({clip.Max.x, -FLT_MAX}, {FLT_MAX, FLT_MAX}, true);
                        break;
                    }

                    /* Top. */
                    case 2: {
                        ImGui::PushClipRect({clip.Min.x, -FLT_MAX}, {clip.Max.x, clip.Min.y},
                                            true);
                        break;
                    }

                    /* Bottom. */
                    case 3: {
                        ImGui::PushClipRect({clip.Min.x, clip.Max.y}, {clip.Max.x, FLT_MAX}, true);
                        break;
                    }
                }

                ImGui::GetWindowDrawList()->AddRect({min_pos.x, min_pos.y + line_height * 0.5f},
                                                    {max_pos.x + spacing.x, max_pos.y + spacing.x},
                                                    color);
                ImGui::PopClipRect();
            }

            panel_clip_rects.pop_back();
        } else {
            /* Otherwise, we want to draw the background only. */
            ImGui::GetWindowDrawList()->AddRect({min_pos.x, min_pos.y + line_height * 0.5f},
                                                {max_pos.x + spacing.x, max_pos.y + spacing.x},
                                                color);
        }
    ImGui::EndGroup();
}

void ImGui::HelpMarker(const char *description, const ImVec2 &padding) {
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ShowTooltip(description, padding);
    }
}

void ImGui::ShowTooltip(const char *description, const ImVec2 &padding) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
    ImGui::BeginTooltipEx(ImGuiWindowFlags_None, ImGuiTooltipFlags_OverridePreviousTooltip);
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 36.5f);
    ImGui::TextUnformatted(description);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
    ImGui::PopStyleVar();
}

void ImGui::VerticalSpacing(float width) {
    auto window = ImGui::GetCurrentWindow();
    if (window->SkipItems) {
        return;
    }

    ImGui::SameLine();
    ItemSize({width, 0.0f});
    ImGui::SameLine();
}

void ImGui::BeginDisabled(float alpha_ratio) {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * alpha_ratio);
}

void ImGui::EndDisabled() {
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();
}

void ImGui::DrawOutlinedRectangle(const ImVec2 &p_min,
                                  const ImVec2 &p_max,
                                  ImU32 col,
                                  float rounding,
                                  ImDrawCornerFlags rounding_corners,
                                  float thickness,
                                  float outline_thickness,
                                  ImU32 outline_col) {
    const auto draw_list = ImGui::GetCurrentWindow()->DrawList;
    draw_list->AddRect(p_min, p_max, outline_col, 0.0f, ImDrawCornerFlags_All,
                       outline_thickness * 2 + thickness);
    draw_list->AddRect(p_min, p_max, col);
}

void ImGui::DrawOutlinedRectangleFilled(const ImVec2 &p_min,
                                        const ImVec2 &p_max,
                                        ImU32 col,
                                        float rounding,
                                        ImDrawCornerFlags rounding_corners,
                                        float outline_thickness,
                                        ImU32 outline_col) {
    const auto draw_list = ImGui::GetCurrentWindow()->DrawList;
    draw_list->AddRect({p_min.x - outline_thickness, p_min.y - outline_thickness},
                       {p_max.x + outline_thickness, p_max.y + outline_thickness},
                       outline_col, 0.0f, ImDrawCornerFlags_All, outline_thickness);
    draw_list->AddRectFilled(p_min, p_max, col);
}