#pragma once
#include <cstdint>

#if defined(__GNUC__)
#define ESP_API __attribute__((visibility("default")))
#else
#define ESP_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Feed screen-space player data from your own authorized integration.
// Coordinates and dimensions are in physical pixels.
struct EspEntity {
    float center_x;
    float top_y;
    float box_width;
    float box_height;
    float health;       // 0..100
    float distance_m;
    int32_t team_id;
    int32_t is_visible;
    char name[64];
};

// Call once per frame before esp_render().
ESP_API void esp_set_entities(const EspEntity* entities, int32_t count);

// Call on the render thread while an OpenGL ES 3 context is current.
ESP_API void esp_render(float screen_width, float screen_height, float delta_seconds);

// Forward touch input in screen pixel coordinates. Call on touch down/move/up.
ESP_API void esp_touch(float x, float y, int32_t is_down);

// Optional: reset UI/context after GL context recreation.
ESP_API void esp_shutdown();

#ifdef __cplusplus
}
#endif
