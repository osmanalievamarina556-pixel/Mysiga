# Android ARM64 ESP + Dear ImGui scaffold

Builds `libesp_overlay.so` for `arm64-v8a` using Android NDK and Dear ImGui.

## Build
Push to `main` or run **Actions → Android ARM64 ESP ImGui → Run workflow**.
The resulting `.so` is uploaded as the `esp-imgui-arm64-v8a` artifact.

## Integration contract
This library is a rendering component, not an injector or game-memory reader.
Your authorized host integration must:
1. Make an OpenGL ES 3 context current on the render thread.
2. Call `esp_set_entities()` with screen-space entity rectangles and labels.
3. Call `esp_render(width, height, delta_seconds)` once per frame.
4. Forward touch events to `esp_touch()` so ImGui can interact.
5. Call `esp_shutdown()` before destroying/recreating the GL context.

See `src/esp.h` for the C ABI. `EspEntity` coordinates are physical pixels.
The project intentionally does not include process injection, anti-cheat evasion,
or game-specific memory access. Dear ImGui is fetched from its upstream Git tag
during the CI build.
