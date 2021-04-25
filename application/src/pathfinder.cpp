#include <pathfinder.hpp>

#include <gamestate.hpp>

#include <algorithm>
#include <array>

void ld::pathFind(
  ld::GameState const & state,
  std::array<::Vector2, 8> & path, size_t & pathSize,
  int32_t const origTileX,   int32_t const origTileY,
  int32_t const targetTileX, int32_t const targetTileY,
  bool const canMine,
  int32_t (*tileValue)(
    ld::GameState const & state, int32_t tileX, int32_t tileY
  )
) {
  // *very* naive path finder
  // the AI are DUMB! and very near-sighted!
  /*

       build a cone in a direction to pick

            *
          * *
      X * * *
          * *
            *
  */
  int32_t previousTileX2 = 0;
  int32_t previousTileY2 = 0;
  int32_t previousTileX = static_cast<int32_t>(origTileX / 32.0f);
  int32_t previousTileY = static_cast<int32_t>(origTileY / 32.0f);

  pathSize = 0;

  for (size_t i = 0; i < 3ul; ++ i) {
    struct PossLocs {
      int32_t x, y;
    };

    constexpr std::array<PossLocs, 4> directions = {{
      { -1, 0 }, { +1, 0 }, {  0, +1 }, {  0, -1 },
    }};

    std::array<int32_t, 4> pathValue = {{ 0, 0, 0, 0, }};

    if (previousTileX > targetTileX) pathValue[0] = 250;
    if (previousTileX < targetTileX) pathValue[1] = 250;
    if (previousTileY < targetTileY) pathValue[2] = 450;
    if (previousTileY > targetTileY) pathValue[3] = 450;

    if (previousTileX - 1 < 0)
      pathValue[0] = -55000;
    if (previousTileX + 1 >= static_cast<int32_t>(state.mineChasm.columns))
      pathValue[1] = -55000;
    if (previousTileY - 1 < 0)
      pathValue[3] = -55000;

    for (auto & pathVal : pathValue)
      pathVal += ::GetRandomValue(-20, 80);

    for (size_t directionIt = 0ul; directionIt < 4ul; ++ directionIt) {
      auto const direction = directions[directionIt];
      constexpr std::array<PossLocs, 9> offsets = {{
        { +1, 0 }, { +2,  0 }, { +3,  0 },
                   { +2, +1 }, { +3, +2 },
                   { +2, -1 }, { +3, -2 },
                               { +3, +2 },
                               { +3, -2 },
      }};

      for (auto const offset : offsets) {
        // transform cone into local space
        int32_t pickTileX =
           direction.x == -1 ? -offset.x
        : (direction.x == +1 ?  offset.x
        : (direction.y == -1 ? -offset.y
        :                       offset.y
        ));

        int32_t pickTileY =
           direction.x == -1 ? -offset.y
        : (direction.x == +1 ?  offset.y
        : (direction.y == -1 ? -offset.x
        :                       offset.x
        ));

        // transform cone into global space & select rock
        pathValue[directionIt] +=
          tileValue(
            state,
            previousTileX+pickTileX,
            previousTileY+pickTileY
          )
          * (1.0f / (offset.x+offset.y));

        if (
            previousTileX+pickTileX == previousTileX2
         && previousTileY+pickTileY == previousTileY2
        ) {
          pathValue[directionIt] -= 1000;
        }

        if (!canMine) {
          if (
            !state.mineChasm.rocks[
              state.mineChasm.rockId(
                previousTileX+pickTileX,
                previousTileY+pickTileY
              )
            ].isMined()
          ) {
            pathValue[directionIt] -= 50'000;
          }
        }
      }
    }

    int32_t selectedPath = -1;
    int32_t selectedPathMaxValue = -500;
    for (int32_t pathIt = 0; pathIt < 4; ++ pathIt) {
      if (pathValue[pathIt] > selectedPathMaxValue) {
        selectedPath = pathIt;
        selectedPathMaxValue = pathValue[pathIt];
      }
    }

    if (selectedPath == -1) { break; }

    int32_t newTileX = previousTileX + directions[selectedPath].x;
    int32_t newTileY = previousTileY + directions[selectedPath].y;

    path[pathSize] =
      ::Vector2{static_cast<float>(newTileX), static_cast<float>(newTileY)};
    ++ pathSize;

    previousTileX2 = previousTileX;
    previousTileY2 = previousTileY;
    previousTileX = newTileX;
    previousTileY = newTileY;
  }
}
