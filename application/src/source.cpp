/* ludum dare | aodq.net */

#include <button.hpp>
#include <gamestate.hpp>
#include <overlay.hpp>
#include <renderer.hpp>

#include <raylib.h>

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

uint32_t scrWidth = 960;
uint32_t scrHeight = 600;

void Entry() {
  InitWindow(960, 600, "whatever");
  SetTargetFPS(60);

  TraceLog(LOG_INFO, "Initializing scene");

  // -- scene setup
  ld::RenderInitialize();

  ld::GameState gameState;
  gameState.mineChasm = ld::MineChasm::Initialize();
  gameState.minerGroup = ld::MinerGroup::Initialize();
  ld::Overlay overlay(scrWidth, scrHeight);

  // -- start loop
  TraceLog(LOG_INFO, "entering loop");
  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_TAB)) { gameState.isPaused ^= 1; }

    // -- update
    if (!gameState.isPaused) {
      ld::MinerGroup::Update(gameState);
      overlay.Update(gameState);
    }

    // -- misc updates
    gameState.camera.Update();

    // -- render
    BeginDrawing();
      ClearBackground(RAYWHITE);

      ld::RenderScene(gameState);

      overlay.Draw(gameState);
    EndDrawing();
  }

    TraceLog(LOG_INFO, "Closing window\n");
  CloseWindow();
}

int main()
{
  try {
    Entry();
  } catch (std::exception & e) {
    TraceLog(LOG_ERROR, "EXCEPTION CAUGHT!!!");
    TraceLog(LOG_ERROR, "EXCEPTION: %s", e.what());
  }

  return 0;
}
