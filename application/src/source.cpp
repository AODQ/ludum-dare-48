/* ludum dare | aodq.net */

#include <scene.hpp>
#include <objects.hpp>

#include <renderer.hpp>

#include <raylib.h>

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

void Entry() {
  InitWindow(600, 600, "whatever");
  SetTargetFPS(60);

  TraceLog(LOG_INFO, "Initializing scene");

  Scene scene;

  ld::RenderInitialize();
  auto mineChasm = ld::MineChasm::Initialize();

  TraceLog(LOG_INFO, "entering loop");
  while (!WindowShouldClose()) {

    Scene_Update(scene);

    BeginDrawing();
      ClearBackground(RAYWHITE);

      ld::RenderScene(mineChasm);

      /* Scene_Render(scene); */
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
