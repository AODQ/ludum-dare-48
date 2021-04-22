/* pulcher | aodq.net */

#include <raylib.h>

#include <chrono>
#include <string>
#include <thread>
#include <utility>
#include <vector>

int main() {

  InitWindow(640, 480, "whatever");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {

    BeginDrawing();
      ClearBackground(RAYWHITE);

      DrawCircleV(Vector2{100.0f, 100.0f}, 50, MAROON);
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
