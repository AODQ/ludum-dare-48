/* ludum dare | aodq.net */

#include <scene.hpp>
#include <miner.hpp>
#include <camera.hpp>

#include <renderer.hpp>

#include <raylib.h>

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

typedef enum {
    eStorageScore = 0,
    eStorageHiScore = 1,
} StorageData;

struct GameState
{
    uint32_t food = 100;
    uint32_t maxFood = 200;
    uint32_t gold = 100;
    uint32_t minerCost = 5;
    size_t selection = -1;
    bool isPaused = false;

    std::vector<Miner> miners;
} game;

void PauseScreen()
{
    int score = ::LoadStorageValue(eStorageScore);
    int hiScore = ::LoadStorageValue(eStorageHiScore);
    //::DrawText(title, 150, 50, 50, BLACK);
    ::DrawText(TextFormat("Score: %i\t Hi-Score: %i", score, hiScore), 150, 130, 30, BLACK);
    ::DrawText("PRESS [TAB] TO RESUME", 100, 170, 30, GRAY);
}

void GameOverScreen()
{

}

// Displays miner info and player overlay
void Overlay()
{
    // TODO create generic button with centering text
    uint32_t xPos = 150;
    uint32_t yPos = 50;
    uint32_t width = game.maxFood;
    uint32_t height = 30;
    uint32_t fontSize = 20;
    const char* text = TextFormat("Food %i/%i", game.food, game.maxFood);

    int textWidth = ::MeasureText(text, fontSize);
    // Food supply bar
    ::DrawRectangle     (xPos, yPos, game.food   , height, RED);
    ::DrawRectangleLines(xPos, yPos, width, height, DARKGRAY);
    ::DrawText(text, xPos + 0.5f*(width - textWidth), yPos, fontSize, BLACK);

    // Miner info
}

void Entry() {
  InitWindow(960, 600, "whatever");
  SetTargetFPS(60);

  TraceLog(LOG_INFO, "Initializing scene");

  ld::Camera camera;

  ld::RenderInitialize();
  auto mineChasm = ld::MineChasm::Initialize();

  TraceLog(LOG_INFO, "entering loop");
  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_TAB)) { game.isPaused ^= 1; }

    // -- update
    if (!game.isPaused) {
    }

    // -- misc updates
    camera.Update();

    // -- render
    BeginDrawing();
      ClearBackground(RAYWHITE);

      ld::RenderScene(mineChasm, camera);

      Overlay();
      if (game.isPaused) {
        PauseScreen();
      }
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
