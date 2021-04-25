/* ludum dare | aodq.net */

#include <button.hpp>
#include <gamestate.hpp>
#include <mob.hpp>
#include <notifs.hpp>
#include <overlay.hpp>
#include <pathfinder.hpp>
#include <renderer.hpp>
#include <sounds.hpp>

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
  ld::SoundInitialize();

  ld::GameState gameState;
  gameState.mobGroup   = ld::MobGroup::Initialize();
  gameState.mineChasm  = ld::MineChasm::Initialize(gameState.mobGroup);
  gameState.minerGroup = ld::MinerGroup::Initialize();
  ld::Overlay overlay(scrWidth, scrHeight);

  ld::pathFindInitialize(&gameState);

  // -- start loop
  TraceLog(LOG_INFO, "entering loop");
  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_TAB)) {
        gameState.isPaused ^= 1;
    }

    // -- update
    if (!gameState.isPaused) {

// DEBUG CODE!!!
#if 1
bool isZPressed = ::IsKeyDown(KEY_Z);
for (int32_t ddd = 0; ddd < (isZPressed ? 10 : 1); ++ ddd) {
#endif
      ld::MinerGroup::Update(gameState);
      ld::MobGroup::Update(gameState);
      ld::MineChasm::Update(gameState);
      ld::NotifGroup::Update(gameState);
      overlay.Update(gameState);

      int32_t foodDecTimer = gameState.minerGroup.miners.size() == 0 ? 5 : 1;
      gameState.foodEatTimer = (gameState.foodEatTimer - foodDecTimer);
      if (gameState.foodEatTimer <= 0) {
        gameState.foodEatTimer = gameState.MaxFoodEatTimer();
        gameState.food -= 1;
      }

      if (gameState.food <= 0) {
        gameState.food = 0;
        if (gameState.minerGroup.miners.size() > 0)
          gameState.minerGroup.miners.begin()->kill();
      }
#if 1
}
#endif
    }

    // -- misc updates
    gameState.camera.Update();
    ld::SoundUpdate();

    // -- render
    BeginDrawing();
      ClearBackground(BLACK);

      ld::RenderScene(gameState);

      overlay.Draw(gameState);
    EndDrawing();
  }

    TraceLog(LOG_INFO, "Closing window\n");

  ::CloseWindow();
  ld::SoundShutdown();
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
