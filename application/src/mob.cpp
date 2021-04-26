#include <mob.hpp>

#include <gamestate.hpp>
#include <pathfinder.hpp>
#include <sounds.hpp>

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
  for (
      int32_t slimeIdx = 0;
      slimeIdx < static_cast<int32_t>(state.mobGroup.slimes.size());
      ++ slimeIdx
  ) {

    auto & slime = state.mobGroup.slimes[slimeIdx];

    // dying :(
    if (slime.health <= 0) {
      slime.sleepTimer -= 1;
      slime.alpha = (slime.sleepTimer) / 120.0f;

      if (slime.sleepTimer <= 0) {
        state.mobGroup.slimes.erase(state.mobGroup.slimes.begin() + slimeIdx);
        -- slimeIdx;
      }
    }

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

    if (
        slime.chasingMinerId >= 0
     && slime.pathIdx >= slime.pathSize
     && slime.pathSize != 0
    ) {
      slime.pathIdx = 0;
      slime.pathSize = 0;
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
        slime.pathSize = 0;
        ld::pathFind(
          state,
          slime.path, slime.pathSize,
          slime.positionX/32, slime.positionY/32,
          miner.xPosition/32, miner.yPosition/32,
          nullptr // cannot mine
        );
      }

      if (slime.pathSize == 0) {
        slime.attackTimer = 3;
      } else {
        slime.chasingMinerId = miner.minerId;
      }
    }

    ld::Miner * attackingMiner = nullptr;
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

      attackingMiner =
        (minerIdx == -1)
          ? nullptr : &state.minerGroup.miners[slime.chasingMinerId]
      ;

      if (
          !attackingMiner
       || (
            // broke off
              attackingMiner
           && slime.inCombat
           && attackingMiner->aiState != ld::Miner::AiState::Fighting
          )
       || (!slime.inCombat && breakChance)
       || attackingMiner->aiState == ld::Miner::AiState::Surfaced
       || (
              std::abs(slime.positionX - attackingMiner->xPosition)/32.0f
            + std::abs(slime.positionY - attackingMiner->yPosition)/32.0f
            > 12
          )
      ) {
        slime.chasingMinerId = -1;
        if (attackingMiner && slime.inCombat) {
          attackingMiner->aiState = ld::Miner::AiState::Traversing;
          slime.inCombat = false;
        }
        attackingMiner = nullptr;
      } else if(attackingMiner && !slime.inCombat) /* not really needed lol */ {
        slime.pathSize = 0;
        ld::pathFind(
          state,
          slime.path, slime.pathSize,
          slime.positionX/32, slime.positionY/32,
          attackingMiner->xPosition/32, attackingMiner->yPosition/32,
          nullptr // cannot mine
        );
      }
    }

    if (slime.inCombat && !attackingMiner) {
      slime.inCombat = false;
    }

    if (slime.inCombat) {
      slime.animationIdx = (slime.animationIdx + 5) % (60*5);

      auto & miner = *attackingMiner;

      if ((slime.animationIdx + 5) % (60 * 5) < slime.animationIdx) {
        miner.reduceEnergy(
          miner.inventory[Idx(ld::ItemType::Armor)].owns
            ? 20 : 50
        );
        miner.damageEquipment(ld::ItemType::Armor);
      }

      if (
          miner.animationFinishesThisFrame()
       && !miner.aiStateInternal.fighting.hasSwung
      ) {
        miner.aiStateInternal.fighting.hasSwung = true;
        slime.health -= 1;

        ld::SoundPlay(
          ld::SoundType::Slime,
          (miner.yPosition - slime.positionY) * 0.25f // more audible
        );

        bool const breakChance =
            ::GetRandomValue(0, 100)
          < (miner.energy >= miner.wantsToSurface() ? 25 : 5)
        ;

        if (breakChance) {
          slime.chasingMinerId = -1;
          slime.inCombat = false;
          attackingMiner->aiState = ld::Miner::AiState::Traversing;
        }
      }

      // force miner to face this direction
      miner.prevXPosition =
          slime.positionX > miner.xPosition
        ? miner.xPosition-1 : miner.xPosition+1
      ;
      miner.prevYPosition =
          slime.positionY > miner.yPosition
        ? miner.yPosition-1 : miner.yPosition+1
      ;

      if (slime.health <= 0) {
        miner.aiState = ld::Miner::AiState::Traversing;
        slime.sleepTimer = 120;
        slime.animationIdx = 0;
      }

      continue;

    } else if (
        attackingMiner
     && ::CheckCollisionPointCircle(
          ::Vector2 {
            (float)attackingMiner->xPosition + 16,
            (float)attackingMiner->yPosition + 16,
          },
          ::Vector2 {
            (float)slime.positionX + 8,
            (float)slime.positionY + 8,
          },
          32.0f
        )
    ) {
      slime.inCombat = true;
      attackingMiner->aiState = ld::Miner::AiState::Fighting;
    }

    if (slime.attackTimer >= 0)
      slime.attackTimer -= 1;

    if (slime.pathSize > 0) {
      if (slime.pathIdx >= slime.pathSize) {
        slime.pathIdx = 0;
        slime.pathSize = 0;
        continue;
      }

      auto & path = slime.path[slime.pathIdx];
      if (slime.targetTileX == -1 && slime.targetTileY == -1) {
        slime.targetTileX = path.x*32.0f + ::GetRandomValue(0, 16);
        slime.targetTileY = path.y*32.0f + ::GetRandomValue(0, 16);
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
          ((slime.positionX/32) + offsets[offsetCheck].x)*32
        + ::GetRandomValue(0, 16)
        ;
        slime.targetTileY =
          ((slime.positionY/32) + offsets[offsetCheck].y)*32
        + ::GetRandomValue(0, 16)
        ;
      }
    }

    if (slime.targetTileX < 0 && slime.targetTileY < 0) { continue; }

    for (int i = 0; i < (slime.chasingMinerId == -1 ? 1 : 2); ++ i) {
      moveTowards(slime, slime.targetTileX, slime.targetTileY);
    }

    slime.animationIdx = (slime.animationIdx + 5) % (60*5);

    if (
        slime.positionX == slime.targetTileX
     && slime.positionY == slime.targetTileY
    ) {
      if (::GetRandomValue(0, 100) < (slime.chasingMinerId>=0 ? 5 : 20)) {
        slime.sleepTimer = ::GetRandomValue(1, 3)*60;
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
