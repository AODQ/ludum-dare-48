/* ludum dare | aodq.net */

#include <scene.hpp>
#include <objects.hpp>
#include <miner.hpp>

#include <raylib.h>

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

int width = 640;
int height = 480;
const char* title = "GameTitle";

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
    uint32_t height = 30;
    // Food supply bar
    ::DrawRectangle     (150, 50, game.food,    height, RED);
    ::DrawRectangleLines(150, 50, game.maxFood, height, DARKGRAY);
    ::DrawText("Food", 220, 50, 30, MAROON);

    // Miner info
}

void Entry() {
  InitWindow(width, height, title);
  SetTargetFPS(60);

  TraceLog(LOG_INFO, "Initializing scene");

  Scene scene;

  TraceLog(LOG_INFO, "adding circle object implementation");
  Scene_ObjectImplementationAdd<Object_Circle>(
    scene
  , ObjectImplementation {
      .objectStorage = {},
      .updateAll =
        [](
          uint8_t * const dataRaw
        , std::vector<ObjectId> const & objects
        ) -> void {
          Object_Circle * const data =
            reinterpret_cast<Object_Circle * const>(dataRaw);

          for (auto const & objectId : objects) {
            Object_Circle & circle = data[objectId.id];
            circle.xPosition =
              250 + objectId.id*100
            + static_cast<int32_t>(sin(GetTime() * objectId.id*5) * 50.0)
            ;
            circle.yPosition =
              250 + static_cast<int32_t>(cos(GetTime()) * 50.0 + objectId.id*25)
            ;
          }
        }
    , .renderAll =
        [](
          uint8_t const * const dataRaw
        , std::vector<ObjectId> const & objects
        ) -> void {
          Object_Circle const * const data =
            reinterpret_cast<Object_Circle const * const>(dataRaw);

          for (auto const & objectId : objects) {
            Object_Circle const & circle = data[objectId.id];
            DrawCircleV(
              Vector2{
                static_cast<float>(circle.xPosition),
                static_cast<float>(circle.yPosition)
              }
            , 50
            , circle.color
            );
          }
        }
    }
  );

  Scene_ObjectImplementationAdd<Object_Rectangle>(
    scene
  , ObjectImplementation {
      .objectStorage = {},
      .updateAll =
        [](
          uint8_t * const dataRaw
        , std::vector<ObjectId> const & objects
        ) -> void {
          Object_Rectangle * const data =
            reinterpret_cast<Object_Rectangle * const>(dataRaw);

          for (auto const & objectId : objects) {
            Object_Rectangle & self = data[objectId.id];
            self.xPosition =
              50 + (objectId.id % 20)*25
            + static_cast<int32_t>(sin(GetTime()) * 50.0)
            ;
            self.yPosition =
              50 + static_cast<int32_t>(objectId.id / 20)*25
            + static_cast<int32_t>(cos(GetTime()) * 50.0)
            ;
          }
        }
    , .renderAll =
        [](
          uint8_t const * const dataRaw
        , std::vector<ObjectId> const & objects
        ) -> void {
          Object_Rectangle const * const data =
            reinterpret_cast<Object_Rectangle const * const>(dataRaw);

          for (auto const & objectId : objects) {
            Object_Rectangle const & self = data[objectId.id];
            DrawRectangleV(
              Vector2{
                static_cast<float>(self.xPosition),
                static_cast<float>(self.yPosition)
              }
            , Vector2{50, 50}
            , Color {
                static_cast<uint8_t>((objectId.id % 10)/10.0f * 255.0f),
                static_cast<uint8_t>((objectId.id % 40)/40.0f * 255.0f),
                static_cast<uint8_t>((objectId.id % 80)/80.0f * 255.0f),
                255,
              }
            );
          }
        }
    }
  );

  TraceLog(LOG_INFO, "adding circle object");

  Scene_ObjectAdd(
    scene,
    Object_Circle { .xPosition = 50, .yPosition = 50, .color = YELLOW }
  );

  Scene_ObjectAdd(
    scene,
    Object_Circle { .xPosition = 50, .yPosition = 50, .color = VIOLET }
  );

  Scene_ObjectAdd(
    scene,
    Object_Circle { .xPosition = 50, .yPosition = 50, .color = LIME }
  );

  for (int i = 0; i < 200; ++ i) {
    Scene_ObjectAdd(
      scene,
      Object_Rectangle { .xPosition = 0, .yPosition = 0, .color = GRAY }
    );
  }

  TraceLog(LOG_INFO, "entering loop");
  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_TAB)) { game.isPaused ^= 1; }
    if (!game.isPaused) {
        Scene_Update(scene);
    }

    BeginDrawing();
      ClearBackground(RAYWHITE);

      Scene_Render(scene);

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
