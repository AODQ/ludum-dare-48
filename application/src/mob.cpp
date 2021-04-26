#include <mob.hpp>

#include <gamestate.hpp>
#include <pathfinder.hpp>

#include <algorithm>
#include <set>

namespace {
  constexpr uint32_t cloudSpreadFrame = 15 * 60;
  uint32_t cloudSpreadTimer = cloudSpreadFrame;
} // -- namespace

void ld::MobGroup::Update(ld::GameState & state)
{
  for (auto & slime : state.mobGroup.slimes) {

    if (slime.sleepTimer > 0) {
      slime.sleepTimer -= 1;

      // allow slime to come to rest
      if (slime.animationIdx > 10) {
        slime.animationIdx = (slime.animationIdx + 5) % (60*5);
      } else {
        slime.animationIdx = 0;
      }
      continue;
    }

    slime.animationIdx = (slime.animationIdx + 5) % (60*5);

    // look for tile near

    if (slime.pathSize <= slime.pathIdx) {
      slime.pathIdx = 0;
      slime.sleepTimer = ::GetRandomValue(0, 2)*60;
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

  const auto spread = --cloudSpreadTimer == 0;
  std::set<std::pair<int32_t, int32_t>> spreading;
  for (auto & cloud : state.mobGroup.poisonClouds) {
    cloud.animationIdx = (cloud.animationIdx + 5) % (60*2);

    // spread only if fully visible
    if (spread && state.mineChasm.fowU8(cloud) > 127) {
      spreading.emplace(
        cloud.positionY / 32,
        cloud.positionX / 32
      );
    }

    // TODO add to FOW if this is detected
  }
  if (spread) cloudSpreadTimer = cloudSpreadFrame;

  const auto rows = state.mineChasm.rocks.size() / state.mineChasm.columns;
  const auto spreadingCopy = spreading;
  for (auto [thisRow, thisCol] : spreadingCopy) {
    for (auto [rowOff, colOff] : {
      std::tuple
      { 1,  0},
      { 0,  1},
      {-1,  0},
      { 0, -1},
    })
    {
      if ((thisRow + rowOff) < 0 || (thisCol + colOff) < 0) continue;
      const auto row = static_cast<uint32_t>(thisRow + rowOff);
      const auto col = static_cast<uint32_t>(thisCol + colOff);
      if (row >= rows || col >= state.mineChasm.columns) continue;
      const auto& rock = state.mineChasm.rock(col, row);
      if (
        rock.tier != ld::RockTier::Mined
        || spreading.count({row, col})
      ) {
        continue;
      }

      state.mobGroup.poisonClouds.push_back({
        .positionX = static_cast<int32_t>(col * 32 + 16 / 2),
        .positionY = static_cast<int32_t>(row * 32 + 16 / 2)
      });
      spreading.emplace(row, col);
    }
  }
}

ld::MobGroup ld::MobGroup::Initialize()
{
  MobGroup self;

  return self;
}
