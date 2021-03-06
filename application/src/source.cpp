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
#include <emscripten/emscripten.h>

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

uint32_t scrWidth = 960;
uint32_t scrHeight = 600;

ld::GameState gameState;
ld::Overlay overlay(scrWidth, scrHeight);

void UpdateFrame() {

  if (IsKeyPressed(KEY_TAB)) {
      gameState.isPaused ^= 1;
  }

  // -- update
  if (!gameState.isPaused) {

    overlay.Update(gameState);

    for (int32_t ddd = 0; ddd < gameState.timeScale; ++ ddd) {
      ld::MinerGroup::Update(gameState);
      ld::MobGroup::Update(gameState);
      ld::MineChasm::Update(gameState);
      ld::NotifGroup::Update(gameState);

      int32_t foodDecTimer = gameState.minerGroup.miners.size() == 0 ? 120 : 1;
      gameState.foodEatTimer =
        std::clamp(
          (int32_t)(gameState.foodEatTimer - foodDecTimer),
          (int32_t)(0),
          (int32_t)(gameState.MaxFoodEatTimer())
        );
      if (gameState.foodEatTimer <= 0 && gameState.food > 0) {
        gameState.foodEatTimer = gameState.MaxFoodEatTimer();
        gameState.food -= gameState.minerGroup.miners.size() == 0 ? 10 : 1;
      }

      if (gameState.food <= 0) {
        gameState.food = 0;
        if (gameState.minerGroup.miners.size() > 0) {
          for (auto & miner : gameState.minerGroup.miners) {
            if (miner.animationFinishesThisFrame()) {
              miner.reduceEnergy(15);
            }
          }
        }
      }
    }
  }

  // -- misc updates
  gameState.camera.Update(gameState);
  ld::SoundUpdate(gameState);

  // -- render
  BeginDrawing();
    ClearBackground(BLACK);

    ld::RenderScene(gameState);

    overlay.Draw(gameState);
  EndDrawing();
}

void Entry() {
  ::InitWindow(960, 600, "whatever");

  // this only works with 3.7
  ::InitAudioDevice();

  SetTargetFPS(60);

  TraceLog(LOG_INFO, "Initializing scene");

  // -- scene setup
  ld::RenderInitialize();
  ld::SoundInitialize();

  gameState.mobGroup   = ld::MobGroup::Initialize();
  gameState.mineChasm  = ld::MineChasm::Initialize(gameState.mobGroup);
  gameState.minerGroup = ld::MinerGroup::Initialize();

  ld::pathFindInitialize(&gameState);

  // -- start loop
  TraceLog(LOG_INFO, "entering loop");
  emscripten_set_main_loop(UpdateFrame, 60, true);

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
