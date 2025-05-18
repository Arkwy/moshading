#include <imgui.h>;
#include <sys/types.h>

#include <cstdint>

struct ActiveBorders {
    bool TOP;
    bool BOTTOM;
    bool LEFT;
    bool RIGHT;
};


inline float outer_border = 20;

struct Window {
    void build() {
        ImGui::SetNextWindowPos(outer_pos());
        ImGui::SetNextWindowSize(outer_size());

        ImGui::Begin("outer", nullptr);
        ImGui::End();


        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);

        ImGui::Begin("inner", nullptr);
        ImGui::End();
    }

  private:
    const ImGuiWindowFlags outer_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse |
                                         ImGuiWindowFlags_NoScrollbar;

    const ImGuiWindowFlags inner_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;

    ActiveBorders active_borders;
    ImVec2 size;
    ImVec2 pos;

    ImVec2 outer_pos() { return ImVec2(pos.x - active_borders.LEFT, pos.y - active_borders.RIGHT); }
    ImVec2 outer_size() {
        return ImVec2(
            size.x + (static_cast<float>(active_borders.BOTTOM) + static_cast<float>(active_borders.TOP)) * outer_border,
            size.y + (static_cast<float>(active_borders.BOTTOM) + static_cast<float>(active_borders.TOP)) * outer_border
        );
    }
};
