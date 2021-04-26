#include <mob.hpp>

#include <gamestate.hpp>
#include <pathfinder.hpp>

#include <algorithm>
#include <set>

namespace {
  constexpr uint32_t cloudSpreadFrame = 15 * 60;
  uint32_t cloudSpreadTimer = cloudSpreadFrame;


template <typename T>
void moveTowards(T & thing, int32_t x, int32_t y)
{
  thing.positionX -= ld::sgn(thing.positionX - x);
  thing.positionY -= ld::sgn(thing.positionY - y);
}

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

    // look for tile near

    if (slime.chasingMinerId >= 0 && slime.pathSize <= slime.pathIdx) {
      slime.pathIdx = 0;
      slime.sleepTimer = ::GetRandomValue(0, 20);
      continue;
    }

    if (
        slime.attackTimer < 0 && slime.chasingMinerId < 0
     && slime.pathSize == 0
     && state.minerGroup.miners.size() > 0
    ) {
      uint32_t minerIdx =
        ::GetRandomValue(0, state.minerGroup.miners.size()-1);
      auto const & miner = state.minerGroup.miners[minerIdx];

      slime.pathSize = 0;
      slime.pathIdx = 0;

      // dont even bother if distance is too large
      if (
          std::abs(slime.positionX - miner.xPosition)/32.0f
        + std::abs(slime.positionY - miner.yPosition)/32.0f
        <= 7.0f
      ) {
        ld::pathFind(
          state,
          slime.path, slime.pathSize,
          slime.positionX/32, slime.positionY/32,
          miner.xPosition/32, miner.yPosition/32,
          false // cannot mine
        );
      }

      if (slime.pathSize == 0) {
        slime.attackTimer = 30;
      } else {
        slime.chasingMinerId = miner.minerId;
      }
    }

    if (slime.chasingMinerId >= 0 && slime.pathSize == 0)
    {
      // find miner, in O(N) but this wont be called often enough to matter
      int32_t minerIdx = -1;
      for (; minerIdx < (int32_t)state.minerGroup.miners.size(); ++ minerIdx) {
        if (state.minerGroup.miners[minerIdx].minerId == slime.chasingMinerId)
          { break; }
      }

      // force to -1 to avoid size comparisons
      if (minerIdx == (int32_t)state.minerGroup.miners.size())
        minerIdx = -1;

      // can either lose focus if miner has died (minerId cant be found),
      // small random chance, or if miner gets too far away
      bool const breakChance = ::GetRandomValue(0, 100) < 5;

      ld::Miner const * const miner =
        (minerIdx == -1)
          ? nullptr : &state.minerGroup.miners[slime.chasingMinerId]
      ;

      if (
          !miner
       || breakChance
       || (
              std::abs(slime.positionX - miner->xPosition)/32.0f
            + std::abs(slime.positionY - miner->yPosition)/32.0f
            > 12
          )
      ) {
        slime.chasingMinerId = -1;
      } else if(miner) /* not really needed lol */ {
        ld::pathFind(
          state,
          slime.path, slime.pathSize,
          slime.positionX/32, slime.positionY/32,
          miner->xPosition/32, miner->yPosition/32,
          false // cannot mine
        );
      }
    }

    if (slime.attackTimer > 0)
      slime.attackTimer -= 1;

    if (slime.pathSize > 0) {
      if (slime.pathIdx >= slime.pathSize) {
        slime.pathIdx = 0;
        slime.pathSize = 0;
        continue;
      }

      auto & path = slime.path[slime.pathIdx];
      if (slime.targetTileX == -1 && slime.targetTileY == -1) {
        slime.targetTileX = path.x*32.0f - ::GetRandomValue(2, 8);
        slime.targetTileX = path.y*32.0f - ::GetRandomValue(2, 8);
      }
    }

    struct Offsets {
      int32_t x, y;
    };

    constexpr std::array<Offsets, 4> offsets = {{
      { -1, 0 }, { +1, 0 }, {  0, +1 }, {  0, -1 },
    }};

    auto offsetCheck = ::GetRandomValue(0, 3);
    if (slime.targetTileX == -1 && slime.targetTileY == -1)
    {
      auto conformanceX =
        state.mineChasm.limitX(slime.positionX/32 + offsets[offsetCheck].x);
      auto conformanceY =
        state.mineChasm.limitY(slime.positionY/32 + offsets[offsetCheck].y);
      if (state.mineChasm.rock(conformanceX, conformanceY).isMined()) {
        slime.targetTileX =
          slime.positionX + offsets[offsetCheck].x*32 - ::GetRandomValue(2, 8);
        slime.targetTileY =
          slime.positionY + offsets[offsetCheck].y*32 - ::GetRandomValue(2, 8);
      }
    }

    if (slime.targetTileX < 0 && slime.targetTileY < 0) { continue; }

    for (int i = 0; i < (slime.chasingMinerId == -1 ? 1 : 4); ++ i) {
      moveTowards(slime, slime.targetTileX, slime.targetTileY);
    }

    slime.animationIdx = (slime.animationIdx + 5) % (60*5);

    if (
        slime.positionX == slime.targetTileX
     && slime.positionY == slime.targetTileY
    ) {
      if (::GetRandomValue(0, 100) < (slime.chasingMinerId>=0 ? 5 : 20)) {
        slime.sleepTimer = ::GetRandomValue(3, 6)*60;
      }

      if (slime.pathSize > 0)
        slime.pathIdx += 1;

      slime.targetTileX = -1;
      slime.targetTileY = -1;
    }

    // TODO add to FOW if this is detected
  }

  // -- clouds -------------------------------------------

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
