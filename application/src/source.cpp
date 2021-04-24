/* ludum dare | aodq.net */

#include <button.hpp>
#include <gamestate.hpp>

#include <renderer.hpp>

#include <raylib.h>

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

uint32_t scrWidth = 960;
uint32_t scrHeight = 600;


typedef enum {
    eStorageScore = 0,
    eStorageHiScore = 1,
} StorageData;

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

ld::ButtonGroup InitButtons()
{
    uint32_t x = scrWidth - 100;
    uint32_t btnWidth = 70;
    uint32_t btnHeight = 50;

    ld::ButtonGroup buttons;

    buttons.emplace("BuyMiner", ld::Button(x, 100, btnWidth, btnHeight, 5));

    buttons.emplace("BluePrints", ld::Button(x, 150, btnWidth, btnHeight, 5));

    return buttons;
}

void BluePrintsMenu()
{
}

void Overlay(ld::GameState & game, ld::ButtonGroup buttons)
{
    if (buttons.at("BuyMiner").IsClicked())
    {
        game.gold -= 5;
    }

    if (buttons.at("BluePrints").IsClicked())
    {
        BluePrintsMenu();
    }

    // Gold supply bar
    {
        uint32_t xPos = 50;
        uint32_t yPos = 20;
        uint32_t width = 200;
        uint32_t height = 30;
        uint32_t fontSize = 20;

        const char* text = TextFormat("Gold: %i", game.gold);
        int textWidth = ::MeasureText(text, fontSize);
        ::DrawRectangle     (xPos, yPos, width, height, GOLD);
        ::DrawRectangleLines(xPos, yPos, width, height, DARKGRAY);
        ::DrawText(text, xPos + 0.5f*(width-textWidth), yPos + 0.5*(height-fontSize), fontSize, BLACK);
    }

    // Food supply bar
    {
        uint32_t xPos = 50;
        uint32_t yPos = 50;
        uint32_t width = 200;
        uint32_t height = 30;
        uint32_t fontSize = 20;

        const char* text = TextFormat("Food %i/%i", game.food, game.maxFood);
        int textWidth = ::MeasureText(text, fontSize);
        ::DrawRectangle     (xPos, yPos, width*((float)game.food/game.maxFood), height, RED);
        ::DrawRectangleLines(xPos, yPos, width, height, DARKGRAY);
        ::DrawText(text, xPos + 0.5f*(width-textWidth), yPos + 0.5*(height-fontSize), fontSize, BLACK);
    }

    // Miner info
}

void Entry() {
  InitWindow(960, 600, "whatever");
  SetTargetFPS(60);

  TraceLog(LOG_INFO, "Initializing scene");

  // -- scene setup
  ld::RenderInitialize();

  ld::GameState gameState;
  gameState.mineChasm = ld::MineChasm::Initialize();
  gameState.minerGroup = ld::MinerGroup::Initialize();

  auto buttonGroup = InitButtons();

  // -- start loop
  TraceLog(LOG_INFO, "entering loop");
  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_TAB)) { gameState.isPaused ^= 1; }

    // -- update
    if (!gameState.isPaused) {
      gameState.minerGroup.Update();
    }

    // -- misc updates
    gameState.camera.Update();

    // -- render
    BeginDrawing();
      ClearBackground(RAYWHITE);

      ld::RenderScene(gameState);

      ld::RenderOverlay(buttonGroup);

      Overlay(gameState, buttonGroup);

      if (gameState.isPaused) {
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
