#include "esp.h"

#include <algorithm>
#include <mutex>
#include <vector>
#include <cstring>

#include "imgui.h"
#include "imgui_impl_opengl3.h"

namespace {
std::mutex g_mutex;
std::vector<EspEntity> g_entities;
bool g_initialized = false;
bool g_menu_open = true;

void init_imgui() {
    if (g_initialized) return;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.FontGlobalScale = 1.0f;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 7.0f;
    style.FrameRounding = 4.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.055f, 0.065f, 0.085f, 0.94f);
    ImGui_ImplOpenGL3_Init("#version 300 es");
    g_initialized = true;
}

void draw_entity(const EspEntity& e, ImDrawList* draw) {
    if (e.box_width <= 0 || e.box_height <= 0) return;
    const ImVec2 a(e.center_x - e.box_width * 0.5f, e.top_y);
    const ImVec2 b(e.center_x + e.box_width * 0.5f, e.top_y + e.box_height);
    const ImU32 color = e.is_visible
        ? IM_COL32(70, 230, 120, 240)
        : IM_COL32(245, 190, 65, 230);

    draw->AddRect(a, b, color, 0.0f, 0, 1.6f);

    const float hp = std::clamp(e.health / 100.0f, 0.0f, 1.0f);
    const ImVec2 hp_bg(a.x - 6.0f, a.y);
    draw->AddRectFilled(hp_bg, ImVec2(hp_bg.x + 3.0f, b.y), IM_COL32(20, 20, 20, 200));
    draw->AddRectFilled(
        ImVec2(hp_bg.x, b.y - e.box_height * hp),
        ImVec2(hp_bg.x + 3.0f, b.y),
        IM_COL32(70, 220, 100, 240));

    char label[128]{};
    const char* name = e.name[0] ? e.name : "Player";
    std::snprintf(label, sizeof(label), "%s  %.0fm", name, e.distance_m);
    draw->AddText(ImVec2(a.x, a.y - 18.0f), IM_COL32(255, 255, 255, 235), label);
}
}

extern "C" ESP_API void esp_set_entities(const EspEntity* entities, int32_t count) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_entities.clear();
    if (!entities || count <= 0) return;
    const int32_t n = std::min<int32_t>(count, 256);
    g_entities.assign(entities, entities + n);
}

extern "C" ESP_API void esp_touch(float x, float y, int32_t is_down) {
    if (!g_initialized) return;
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(x, y);
    io.MouseDown[0] = (is_down != 0);
}

extern "C" ESP_API void esp_render(float screen_width, float screen_height, float delta_seconds) {
    if (screen_width <= 0 || screen_height <= 0) return;
    init_imgui();

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(screen_width, screen_height);
    io.DeltaTime = delta_seconds > 0.0f ? delta_seconds : (1.0f / 60.0f);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImDrawList* bg = ImGui::GetBackgroundDrawList();
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        for (const auto& entity : g_entities) draw_entity(entity, bg);
    }

    if (g_menu_open) {
        ImGui::SetNextWindowSize(ImVec2(270, 180), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("ESP overlay", &g_menu_open)) {
            ImGui::TextUnformatted("Overlay controls");
            ImGui::Separator();
            ImGui::Text("Tracked entities: %d", static_cast<int>(g_entities.size()));
            ImGui::Checkbox("Show menu", &g_menu_open);
            ImGui::TextWrapped("Feed screen-space entity data through esp_set_entities().");
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

extern "C" ESP_API void esp_shutdown() {
    if (!g_initialized) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    g_initialized = false;
}
