#pragma once

#ifndef IMGUI_EXTENSIONS_GUARD
#define IMGUI_EXTENSIONS_GUARD

/*
 * As the name implies, this header file is an (personal) extension of ImGui.
 * It is not in direct association with ImGui.
 */

#include <string>
#include <vector>

#include <Windows.h>

#include "imgui.h"
#include "imgui_internal.h"

#define BEGIN_DISABLED(expr) if (expr) { ImGui::BeginDisabled(); }
#define END_DISABLED(expr) if (expr) { ImGui::EndDisabled(); }

/* FIXME: refactor this at some point. */

namespace ImGui {
    /* Initialises ImGui for OpenGL 2.0 on Windows. */
    IMGUI_API void Init(const HWND &hwnd);
    IMGUI_API void Shutdown();

    /* Creates a fullscreen canvas to use when drawing outside of ImGui windows. */
    IMGUI_API bool BeginCanvas();
    IMGUI_API void EndCanvas();

    /* Creates a transparent window without any styling. */
    IMGUI_API bool BeginTransparent(const char *title, float alpha = 0.7f);
    IMGUI_API void EndTransparent();

    IMGUI_API void Columns(int columns_count = 1, const char* id = NULL,
                           ImGuiColumnsFlags flags = ImGuiColumnsFlags_None);

    IMGUI_API bool CustomCheckbox(const char *label, bool *v);
    IMGUI_API bool CustomCheckboxFlags(const char *label, unsigned *flags, unsigned flag);

    /* Bordered panel. */
    void BeginPanel(const char *label,
                    const char *additional_text = "",
                    bool background = false,
                    const float width = -1.0f,
                    const ImVec2 item_spacing = {6.0f, 6.0f});
    void EndPanel();

    /* Displays a question mark with explanatory text when hovered over. */
    void HelpMarker(const char *description, const ImVec2 &padding = {5.0f, 5.0f});

    void ShowTooltip(const char *description, const ImVec2 &padding = {5.0f, 5.0f});

    void VerticalSpacing(float width = 0.0f);

    void BeginDisabled(float alpha_ratio = 0.5f);
    void EndDisabled();

    void DrawOutlinedRectangle(const ImVec2& p_min,
                               const ImVec2& p_max,
                               ImU32 col,
                               float rounding = 0.0f,
                               ImDrawCornerFlags rounding_corners = ImDrawCornerFlags_All,
                               float thickness = 1.0f,
                               float outline_thickness = 1.0f,
                               ImU32 outline_col = IM_COL32(0, 0, 0, 255));

    void DrawOutlinedRectangleFilled(const ImVec2& p_min,
                                     const ImVec2& p_max,
                                     ImU32 col,
                                     float rounding = 0.0f,
                                     ImDrawCornerFlags rounding_corners = ImDrawCornerFlags_All,
                                     float outline_thickness = 1.0f,
                                     ImU32 outline_col = IM_COL32(0, 0, 0, 255));
} // namespace ImGui

#endif // IMGUI_EXTENSIONS_GUARD