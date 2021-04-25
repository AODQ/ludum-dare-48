#include <mob.hpp>

#include <gamestate.hpp>
#include <pathfinder.hpp>

#include <algorithm>

namespace {

} // -- namespace

void ld::MobGroup::Update(ld::GameState & state)
{
  for (auto & slime : state.mobGroup.slimes) {
    slime.animationIdx = (slime.animationIdx + 5) % (60*5);

    if (slime.pathIdx == 0) {
      slime.targetTileX = slime.positionX/32 + ::GetRandomValue(-4, +4);
      slime.targetTileY = slime.positionY/32 + ::GetRandomValue(-4, +4);

      slime.targetTileX = std::clamp(slime.targetTileX, 0, 29);
      slime.targetTileY = std::clamp(slime.targetTileY, 0, 500);

      slime.pathSize = 0;
      slime.pathIdx = 0;
      ld::pathFind(
        state,
        slime.path, slime.pathSize,
        slime.positionX/32, slime.positionY/32,
        slime.targetTileX, slime.targetTileY,
        false // cannot mine
      );
    }

    if (slime.pathSize <= slime.pathIdx) {
      slime.pathIdx = 0;
      continue;
    }

    auto & path = slime.path[slime.pathIdx];

    slime.positionX -= ld::sgn(slime.positionX - path.x*32.0f);
    slime.positionY -= ld::sgn(slime.positionY - path.y*32.0f);

    ::Rectangle rect = {
      .x = path.x*32.0f, .y = path.y*32.0f,
      .width = 32.0f, .height = 32.0f,
    };

    if (
      ::CheckCollisionCircleRec(
        ::Vector2 {
          static_cast<float>(slime.positionX),
          static_cast<float>(slime.positionY),
        },
        24.0f,
        rect
      )
    ) {
      slime.pathIdx += 1;

      if (slime.targetTileX == path.x && slime.targetTileY == path.y) {
        slime.pathIdx = 0;
      }
    }

    // TODO add to FOW if this is detected
  }

  for (auto & cloud : state.mobGroup.poisonClouds) {
    cloud.animationIdx = (cloud.animationIdx + 5) % (60*2);

    // TODO add to FOW if this is detected
  }
}

ld::MobGroup ld::MobGroup::Initialize()
{
  MobGroup self;

  return self;
}
